#ifndef JSONINTERPRETER_LIBRARY_H
#define JSONINTERPRETER_LIBRARY_H

struct JSON_Serializable {
    unsigned int length;
    char* payload;
};

struct JSON_Deserializable {
    unsigned int length;
    char* payload;
};

// serialize
struct JSON_Serializable* JSON_newSerializable();
void JSON_serializeProperty(struct JSON_Serializable* restrict serializable, const char* const restrict propertyName, const unsigned int propertyNameLength, const char* const restrict format, const void* const restrict propertyValue);
char* JSON_serialize(struct JSON_Serializable* restrict serializable);

// deserialize
struct JSON_Deserializable* JSON_newDeserializable(const char* restrict json, const unsigned int length);
struct JSON_Deserializable* JSON_deserializeProperty(const struct JSON_Deserializable* restrict deserializable, const char* propertyName, const unsigned int propertyLength);
void JSON_desintegrate(struct JSON_Deserializable* restrict deserializable);

#endif //JSONINTERPRETER_LIBRARY_H
