# jsoninterpreter
a interpreter for JSON written in C

## Link library like
Recompile if need be, or for Windows 10 just use the compiled file

```
target_link_libraries(projectname PRIVATE jsoninterpreter ${CMAKE_CURRENT_SOURCE_DIR}/lib/jsoninterpreter/jsoninterpreter.a)
```

## Example
Examples with the syntax of the library
### Serializing
```
int main() {
    int i = 9;
    struct JSON_Serializable* serializable = JSON_newSerializable();

    JSON_serializeProperty(serializable, "example", strlen("example"), "%d", &i);

    char* finishedJSON = JSON_serialize(serializable);
    printf("%s", finishedJSON);
    JSON_desintegrateSerializable(serializable);
    free(finishedJSON);

    return 0;
}
```
### Deserializing
```
int main() {
    char* xy = "{\"erstes\":\"Hallo\",\"zweites\":123,\"a\":123,\"b\":[{\"x\":1234,\"y\":789},{\"x\":1234,\"y\":789},{\"x\":1234,\"y\":789}],\"c\":\"hiiii\"}";

    struct JSON_Deserializable* deserializable = JSON_newDeserializable(xy, strlen(xy));
    struct JSON_Deserializable* inner = JSON_deserializeProperty(deserializable, "b", strlen("b"));
    struct JSON_Deserializable* array = JSON_deserialize_getFromArrayWithIndex(deserializable, 0);

    printf("\n%s - %d\n", inner->payload, strlen(inner->payload));
    printf("\n%s - %d\n", array->payload, strlen(array->payload));

    JSON_desintegrateDeserializable(deserializable);
    JSON_desintegrateDeserializable(inner);
    JSON_desintegrateDeserializable(array);
```
