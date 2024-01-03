#jsoninterpreter
a interpreter for JSON written in C

## Link library like
Recompile if need be, or for Windows 10 just use the compiled file

```
target_link_libraries(projectname PRIVATE jsoninterpreter ${CMAKE_CURRENT_SOURCE_DIR}/lib/jsoninterpreter/jsoninterpreter.a)
```

## Example
Examples with the syntax of the library
```
// Serializing
int main() {
    int i = 9;
    struct JSON_Serializable* serializable = JSON_newSerializable();

    JSON_serializeProperty(serializable, "example", strlen("example"), "%d", &i);

    char* finishedJSON = JSON_serialize(serializable);
    printf("%s", finishedJSON);
    free(finishedJSON);

    return 0;
}
```

```
// Deserializing
int main() {
    char* xy = "{\"erstes\":\"Hallo\",\"zweites\":123,\"a\":123,\"b\":[{\"x\":1234,\"y\":789},{\"x\":1234,\"y\":789},{\"x\":1234,\"y\":789}],\"c\":\"hiiii\"}";

    struct JSON_Deserializable* deserializable = JSON_newDeserializable(xy, strlen(xy));
    struct JSON_Deserializable* inner = JSON_deserializeProperty(deserializable, "b", strlen("b"));

    printf("\n%s - %d\n", inner->payload, strlen(inner->payload));

    JSON_desintegrate(deserializable);
    JSON_desintegrate(inner);
    free(xy);
```
