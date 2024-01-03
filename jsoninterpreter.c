#include "jsoninterpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define JSON_PROPERTY_MAX_BUFFER_SIZE 256

#define OPEN_BRACE '{'
#define CLOSE_BRACE '}'
#define OPEN_BRACKET '['
#define CLOSE_BRACKET ']'
#define BACKSLASH '\\'
#define QUOTE '\"'
#define COLON ':'
#define COMMA ','
#define CHARACTER_NULL '\0'
#define DOT '.'

// using the upper 9 bit as flags, the lower 7 bit as indendation counter
#define FLAG_16_OFF                                 0b0000000000000000
#define FLAGS_16_IGNORED                            0b0000000001111111

#define FLAG_16_BACKSLASH_BEFORE                    0b0000000100000000
#define FLAG_16_INSIDE_STRING                       0b0000001000000000
#define FLAG_16_IS_KEY                              0b0000010000000000
#define FLAG_16_SKIP_CURRENT_KEY                    0b0000100000000000
#define FLAG_16_SKIP_CURRENT_VALUE                  0b0001000000000000
#define FLAG_16_INSIDE_CORRECT_KEY                  0b0010000000000000
#define FLAG_16_INSIDE_CORRECT_VALUE_FIRST_TIME     0b0100000000000000
#define FLAG_16_INSIDE_CORRECT_VALUE                0b1000000000000000
#define FLAG_16_IS_CORRECT_VALUE_STRING             0b0000000010000000

#define FLAG_activate(variable, flag)           (variable | flag)
#define FLAG_deactivate(variable, flag)         (variable & ~flag)
#define FLAG_invert(variable, flag)             (variable ^ flag)
#define FLAG_true(variable, flag)               ((variable & flag) != 0)
#define FLAG_false(variable, flag)              ((variable & flag) == 0)

#define SUBNAME_DEFAULT 8

// serialize

struct JSON_Serializable* JSON_newSerializable() {
    struct JSON_Serializable* restrict serializable = malloc(sizeof(struct JSON_Serializable));
    if (serializable == NULL) {
        return NULL;
    }
    serializable->length = 2;
    serializable->payload = malloc(2);
    if (serializable->payload == NULL) {
        free(serializable);
        return NULL;
    }
    strcpy(serializable->payload, "{}\0");
    return serializable;
}

void JSON_serializeProperty(struct JSON_Serializable* restrict serializable, const char* const restrict propertyName, const unsigned int propertyNameLength, const char* const restrict format, const void* const restrict propertyValue) {
    char buffer[JSON_PROPERTY_MAX_BUFFER_SIZE];
    sprintf(buffer, format, *(char*)propertyValue);
    // 4 for "":,
    const int bufferLength = strlen(buffer);

    const unsigned int newLength = serializable->length + bufferLength + propertyNameLength + 4;

    serializable->payload = realloc(serializable->payload, newLength);
    memcpy(serializable->payload + serializable->length, propertyName, propertyNameLength);
    serializable->payload[serializable->length - 1] = QUOTE;
    serializable->payload[serializable->length + propertyNameLength] = QUOTE;

    serializable->payload[serializable->length + propertyNameLength + 1] = COLON;
    memcpy(serializable->payload + serializable->length + propertyNameLength + 2, buffer, bufferLength);

    serializable->payload[newLength - 2] = COMMA;
    serializable->payload[newLength - 1] = CLOSE_BRACE;
    serializable->payload[newLength] = CHARACTER_NULL;

    serializable->length = newLength;
}

char* JSON_serialize(struct JSON_Serializable* restrict serializable) {
    char* restrict json = malloc(serializable->length);
    if (json == NULL) {
        return NULL;
    }
    serializable->payload[serializable->length - 1] = CHARACTER_NULL;
    serializable->payload[serializable->length - 2] = CLOSE_BRACE;
    strcpy(json, serializable->payload);
    free(serializable->payload);
    free(serializable);
    return json;
}

// deserialize

struct JSON_Deserializable* JSON_newDeserializable(const char* restrict json, const unsigned int length) {
    struct JSON_Deserializable* restrict deserializable = malloc(sizeof(struct JSON_Deserializable));
    if (deserializable == NULL) {
        return NULL;
    }
    deserializable->length = length;
    deserializable->payload = malloc(length);
    if (deserializable->payload == NULL) {
        free(deserializable);
        return NULL;
    }
    memcpy(deserializable->payload, json, length);
    return deserializable;
}

static inline _Bool equalsAfterFirstChar(const char* restrict a, const char* restrict b, const unsigned int length) {
    for (int i = 1; i < length; i++) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

struct JSON_Deserializable* JSON_deserializeProperty(const struct JSON_Deserializable* restrict deserializable, const char* propertyName, const unsigned int propertyLength) {
    register uint16_t flags = FLAG_16_OFF;

    unsigned int valueStart;
    unsigned int i;
    for (i = 0; i < deserializable->length; i++) {
        // Is this character a backslash?
        if (deserializable->payload[i] == BACKSLASH) {
            flags = FLAG_activate(flags, FLAG_16_BACKSLASH_BEFORE);
            continue;
        }
        // Is the last character a slash? If not, if this character is a " the string status changes
        if (FLAG_true(flags, FLAG_16_BACKSLASH_BEFORE)) {
            flags = FLAG_deactivate(flags, FLAG_16_BACKSLASH_BEFORE);
        } else {
            if (deserializable->payload[i] == QUOTE) {
                flags = FLAG_invert(flags, FLAG_16_INSIDE_STRING);
                continue;
            }
        }
        // indentation?
        if (FLAG_false(flags, FLAG_16_INSIDE_STRING) && (deserializable->payload[i] == OPEN_BRACE || deserializable->payload[i] == OPEN_BRACKET)) {
            flags++;
        } else if (FLAG_false(flags, FLAG_16_INSIDE_STRING) && (deserializable->payload[i] == CLOSE_BRACE || deserializable->payload[i] == CLOSE_BRACKET)) {
            flags--;
        }

        // in key?
        if ((flags & FLAGS_16_IGNORED) == 1) {
            if (FLAG_false(flags, FLAG_16_INSIDE_STRING) && (deserializable->payload[i] == COLON )) {
                flags = FLAG_deactivate(flags, FLAG_16_IS_KEY);
                continue;
            } if (FLAG_false(flags, FLAG_16_INSIDE_STRING) && (deserializable->payload[i] == COMMA || deserializable->payload[i] == OPEN_BRACE)) {
                flags = FLAG_activate(flags, FLAG_16_IS_KEY);
                continue;
            }
        }

        // skipping key-value pair
        if (FLAG_true(flags, FLAG_16_SKIP_CURRENT_KEY)) {
            if (FLAG_true(flags, FLAG_16_IS_KEY)) {
                continue;
            }
            flags = FLAG_deactivate(flags, FLAG_16_SKIP_CURRENT_KEY);
            flags = FLAG_activate(flags, FLAG_16_SKIP_CURRENT_VALUE);
        }
        if (FLAG_true(flags, FLAG_16_SKIP_CURRENT_VALUE)) {
            if (FLAG_false(flags, FLAG_16_IS_KEY)) {
                continue;
            }
            flags = FLAG_deactivate(flags, FLAG_16_SKIP_CURRENT_VALUE);
        }

        if (FLAG_true(flags, FLAG_16_INSIDE_CORRECT_KEY)) {
            if (FLAG_true(flags, FLAG_16_IS_KEY)) {
                continue;
            }
            flags = FLAG_deactivate(flags, FLAG_16_INSIDE_CORRECT_KEY);
            flags = FLAG_activate(flags, FLAG_16_INSIDE_CORRECT_VALUE_FIRST_TIME);
        }
        if (FLAG_true(flags, FLAG_16_INSIDE_CORRECT_VALUE)) {
            if (FLAG_false(flags, FLAG_16_IS_KEY)) {
                continue;
            }
            break;
        }
        if (FLAG_true(flags, FLAG_16_INSIDE_CORRECT_VALUE_FIRST_TIME)) {
            valueStart = i;
            flags = FLAG_deactivate(flags, FLAG_16_INSIDE_CORRECT_VALUE_FIRST_TIME);
            flags = FLAG_activate(flags, FLAG_16_INSIDE_CORRECT_VALUE);
            if (FLAG_true(flags, FLAG_16_INSIDE_STRING)) {
                flags = FLAG_activate(flags, FLAG_16_IS_CORRECT_VALUE_STRING);
            }
            continue;
        }

        if (FLAG_true(flags, FLAG_16_IS_KEY)) {
            if ((deserializable->payload[i] == propertyName[0])
                && (deserializable->payload[i + propertyLength + 1] == COLON)
                && (equalsAfterFirstChar(deserializable->payload + i, propertyName, propertyLength))
                ) {
                flags = FLAG_activate(flags, FLAG_16_INSIDE_CORRECT_KEY);
            } else {
                flags = FLAG_activate(flags, FLAG_16_SKIP_CURRENT_KEY);
            }
        }
    }

    struct JSON_Deserializable* restrict result = malloc(sizeof(struct JSON_Deserializable));
    if (result == NULL) {
        return NULL;
    }

    // more hack than below if result is a json itsself :((((
    if (deserializable->payload[i - 2] != CLOSE_BRACE && deserializable->payload[i - 2] != CLOSE_BRACKET) {
        i--;
    }

    // if string, hack so that it works :(
    result->length = i - valueStart;
    if (FLAG_true(flags, FLAG_16_IS_CORRECT_VALUE_STRING)) {
        result->length--;
    }

    result->payload = malloc(result->length);
    if (result->payload == NULL) {
        return NULL;
    }
    memcpy(result->payload, deserializable->payload + valueStart, result->length - 1);
    result->payload[result->length - 1] = '\0';


    return result;
}

void JSON_desintegrate(struct JSON_Deserializable* restrict deserializable) {
    free(deserializable->payload);
    free(deserializable);
}
