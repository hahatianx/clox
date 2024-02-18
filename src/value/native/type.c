//
// Created by shenshuhan on 2/4/24.
//

#include <string.h>

#include "value/object/string.h"
#include "value/native/type.h"
#include "value/object/class.h"


value_t type_native(__attribute__((unused)) int argc, value_t* args) {
    value_t input = args[0];
    char buff[20];
    memset(buff, 0, sizeof buff);
    switch (input.type) {
        case VAL_BOOL:
            strcpy(buff, "bool");
            break;
        case VAL_FLOAT:
            strcpy(buff, "float");
            break;
        case VAL_INT:
            strcpy(buff, "int");
            break;
        case VAL_NIL:
            strcpy(buff, "null");
            break;
        case VAL_OBJ: {
            object_t *obj = input.as.obj;
            switch (obj->type) {
                case OBJ_BOUND_METHOD:
                    strcpy(buff, "method");
                    break;
                case OBJ_CLOSURE:
                case OBJ_FUNCTION:
                    strcpy(buff, "function");
                    break;
                case OBJ_NATIVE:
                    strcpy(buff, "native-function");
                    break;
                case OBJ_STRING:
                    strcpy(buff, "string");
                    break;
                case OBJ_INSTANCE:
                    strcpy(buff, "instance");
                    break;
                case OBJ_LIST:
                    strcpy(buff, "list");
                    break;
                default:
                    strcpy(buff, "undefined");
            }
            break;
        }
        default:
            strcpy(buff, "undefined");
    }
    return OBJECT_VAL(copy_string(buff, strlen(buff)));
}