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
/**
 * \brief Create a new Serializeable struct pointer
 * \return JSON_Serializable*
 */
struct JSON_Serializable* JSON_newSerializable();

/**
 * \brief serialize a new property onto a serializable
 * \param serializable the serializeable on which the property should be added
 * \param propertyName name of the property to be serialized
 * \param propertyNameLength length of the name of the property - can be a call to strlen(propertyName)
 * \param format format for the property, how it would be in printf()
 * \param propertyValue pointer to the property to be serialized itsself
 */
void JSON_serializeProperty(struct JSON_Serializable* restrict serializable, const char* const restrict propertyName, const unsigned int propertyNameLength, const char* const restrict format, const void* const restrict propertyValue);

/**
 * \brief creates a char* of the json
 * \param serializable serializeable to be serialized
 * \return char* of the json string
 */
char* JSON_serialize(struct JSON_Serializable* restrict serializable);

/**
 * \brief frees a serializable
 * \param serializable serializeable to be freed
 */
void JSON_desintegrateSerializable(struct JSON_Serializable* restrict serializable);

// deserialize
/**
 * \brief creates a deserializable from a string of json
 * \param json char* of the json to be deserialized
 * \param length length of the json - can be call to strlen(json)
 * \return deserializable
 */
struct JSON_Deserializable* JSON_newDeserializable(const char* restrict json, const unsigned int length);

/**
 * \brief deserializes a property from a desierializable without deleteing it
 * \param deserializable deserializable to be deserialized from
 * \param propertyName name of the property
 * \param propertyLength length of the name of the property - can be a call to strlen(propertyName)
 * \return new deserializable with only the deserialized property inside
 */
struct JSON_Deserializable* JSON_deserializeProperty(const struct JSON_Deserializable* restrict deserializable, const char* propertyName, const unsigned int propertyLength);

/**
 * \brief deserialize an element from a json array
 * \param deserializable deserializable to be deserialized from (has to be a json array)
 * \param index index in the json array
 * \return new deserializable with only the deserialized element inside
 */
struct JSON_Deserializable* JSON_deserialize_getFromArrayWithIndex(const struct JSON_Deserializable* restrict deserializable, const unsigned int index);

/**
 * \brief frees a deserializable
 * \param deserializable deserializable to be freed
 */
void JSON_desintegrateDeserializable(struct JSON_Deserializable* restrict deserializable);

#endif //JSONINTERPRETER_LIBRARY_H
