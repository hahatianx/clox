#include <stdio.h>

#include "common.h"
#include "basic/memory.h"
#include "vm/runtime.h"
#include "value/object/class.h"
#include "component/valuetable.h"

list_t temporary_objs;

static int print_function(object_function_t* func) {
    if (!func->name) {
        return printf("<script>");
    }
    return printf("<fn %s>", func->name->chars);
}

int print_object(value_t value) {
    switch(OBJ_TYPE(value)) {
        case OBJ_CLASS:
            return printf("<class %s>", AS_CLASS(value)->name->chars);
        case OBJ_INSTANCE: {
            return printf("<instance %s>", AS_INSTANCE(value)->klass->name->chars);
        }
        case OBJ_STRING:
            return printf("%s", AS_CSTRING(value));
        case OBJ_FUNCTION:
            return print_function(AS_FUNCTION(value));
        case OBJ_NATIVE:
            return printf("<native fn>");
        case OBJ_CLOSURE: {
            int len = 0;
            void* enclosed = AS_CLOSURE(value)->function;
            len += printf("<closure ");
            len += printf(" [%p] ", enclosed);
            if (enclosed)
                len += print_function(AS_CLOSURE(value)->function);
            len += printf(">");
            return len;
        }
        case OBJ_UPVALUE:
            return printf("[upvalue]");
    }
    return 0;
}

void blacken_object(object_t* object) {
#ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void*)object);
    print_value(OBJECT_VAL(object));
    printf("\n");
#endif
    switch (object->type) {
        case OBJ_CLASS: {
            object_class_t* klass = (object_class_t*)object;
            mark_object((object_t*)klass->name);
            break;
        }
        case OBJ_INSTANCE: {
            object_instance_t* instance = (object_instance_t*)object;
            mark_table_value(&instance->fields);
            mark_object((object_t*)instance->klass);
            break;
        }
        case OBJ_CLOSURE: {
            object_closure_t *closure = (object_closure_t*)object;
            mark_object((object_t*)closure->function);
            for (int i = 0; i < closure->upvalue_count; ++i) {
                mark_object((object_t*)closure->upvalues[i]);
            }
            break;
        }
        case OBJ_FUNCTION: {
            object_function_t *function = (object_function_t*)object;
            mark_object((object_t*)function->name);
            mark_array(&function->chunk.constants);
        }
        case OBJ_UPVALUE:
            mark_value(((object_upvalue_t*)object)->closed);
            break;
        case OBJ_NATIVE:
        case OBJ_STRING:
            break;
    }
}


object_t* allocate_object(size_t size, object_type_t type) {
    object_t* object = (object_t*)reallocate(NULL, 0, size);
    object->type = type;
    object->is_marked = false;
    list_link_init(&object->link);
    list_insert_head(&temporary_objs, &object->link);

#ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void*)object, size, type);
#endif

    return object;
}

void merge_temporary() {
    list_insert_multi(&vm.obj, &temporary_objs);
}

void free_object(object_t *obj) {

#ifdef DEBUG_LOG_GC
    printf("%p free type %d\n", (void*)obj, obj->type);
#endif

    switch (obj->type) {
        case OBJ_CLASS: {
            FREE(object_class_t, obj);
            break;
        }
        case OBJ_INSTANCE: {
            object_instance_t *instance = (object_instance_t*)obj;
            free_table_value(&instance->fields);
            FREE(object_instance_t, obj);
            break;
        }
        case OBJ_FUNCTION: {
            object_function_t* function = (object_function_t*)obj;
            free_chunk(&function->chunk);
            FREE(object_function_t, obj);
            break;
        }
        case OBJ_STRING: {
            object_string_t *string = (object_string_t*)obj;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(object_string_t, obj);
            break;
        }
        case OBJ_NATIVE: {
            FREE(object_native_func_t, obj);
            break;
        }
        case OBJ_CLOSURE: {
            object_closure_t* closure = (object_closure_t*)obj;
            FREE_ARRAY(object_upvalue_t*, closure->upvalues, closure->upvalue_count);
            FREE(object_closure_t, obj);
            break;
        }
        case OBJ_UPVALUE: {
            FREE(object_upvalue_t, obj);
            break;
        }
        default: return;
    }
}