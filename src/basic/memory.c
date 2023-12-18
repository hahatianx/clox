#include <stdlib.h>

#include "constant.h"
#include "common.h"
#include "switch.h"

#include "basic/memory.h"

#include "vm/runtime.h"
#include "vm/compiler.h"
#include "component/vartable.h"
#include "component/graystack.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug/debug.h"
#endif



static void mark_roots() {
    for (value_t* slot = vm.stack; slot < vm.stack_top; slot++) {
#ifdef DEBUG_LOG_GC
        printf("Mark stack value ");
        print_value(*slot);
        printf("\n");
#endif
        mark_value(*slot);
    }

    for (int i = 0; i < vm.frame_count; ++i) {
        mark_object((object_t*)vm.frames[i].closure);
    }

    object_upvalue_t * iter = NULL;
    list_iterate_begin(object_upvalue_t, link, &vm.open_upvalues, iter) {
        mark_object((object_t*)iter);
    } list_iterate_end();

    mark_compiler_roots();
    mark_table_var(&vm.globals);
}
static void trace_references() {
    while (vm.gray_stack.count) {
        object_t* object = pop_gray_stack(&vm.gray_stack);
        blacken_object(object);
    }
}

void* reallocate(void* pointer, size_t old_size, size_t new_size) {
    /*
     * The VM needs a global switch to decide when to do garbage collection.
     * Garbage collection at compile time might lead to nullptr issues on constant strings.
     */
    if (do_garbage_collector && new_size > old_size) {
#ifdef DEBUG_STRESS_GC
        collect_garbage();
#endif
    }

    if (new_size == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, new_size);
    if (result == NULL) exit(1);
    return result;
}

void mark_object(object_t* object) {
    if (object == NULL) return;
    if (object->is_marked) return;
#ifdef DEBUG_LOG_GC
    printf("%p mark ", (void*)object);
    print_value(OBJECT_VAL(object));
    printf("\n");
#endif
    object->is_marked = true;

    push_gray_stack(&vm.gray_stack, object);
}

void mark_value(value_t value) {
    if (IS_OBJECT(value)) {
        mark_object(AS_OBJECT(value));
    }
}

void mark_array(value_array_t* array) {
    for (int i = 0; i < array->count; ++i) {
        mark_value(array->values[i]);
    }
}

static void sweep() {
    object_t* iter = NULL;
    object_t* objects_to_sweep[OBJECT_MAX];
    int count = 0;
    list_iterate_begin(object_t, link, &vm.obj, iter) {
#ifdef DEBUG_PRINT_OBJECT
        printf("object iterated %p, value ", iter);
        print_value(OBJECT_VAL(iter));
        printf("\n");
#endif
        if (count == OBJECT_MAX) {
            goto STOP_LOOP;
        }
        if (!iter->is_marked) {
#ifdef DEBUG_LOG_GC
            printf("Found a unmarked object %p, type %d, value ", iter, iter->type);
            print_value(OBJECT_VAL(iter));
            printf("\n");
#endif
            objects_to_sweep[count++] = iter;
        } else {
            iter->is_marked = false;
        }
    } list_iterate_end();
    STOP_LOOP:;
#ifdef DEBUG_LOG_GC
    printf("%d objects need to be freed.\n", count);
#endif
    for (int i = 0; i < count; i++) {
        object_t* obj = objects_to_sweep[i];
#ifdef DEBUG_PRINT_FREED
        printf("item getting freed: ");
        print_value(OBJECT_VAL(obj));
        printf("\n");
#endif
        list_remove(&obj->link);
        free_object(obj);
    }
}

void collect_garbage() {
#ifdef DEBUG_LOG_GC
    printf("-- gc begin\n");
#endif

    mark_roots();
    trace_references();
    remove_unused_strings();
    sweep();

#ifdef DEBUG_LOG_GC
    printf("-- gc end\n");
#endif
}
