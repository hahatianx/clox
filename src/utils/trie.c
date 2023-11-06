#include <string.h>

#include "common.h"

#include "error/error.h"
#include "basic/memory.h"
#include "utils/trie.h"


void trie_init(trie_t* trie) {
    alloc_block(trie_node_t, &trie->root);
    trie->total_nodes = 1;
}

trie_node_t* __trie_insert(trie_t* trie, const char* keyword, trie_node_alloc_func func) {
    trie_node_t* u = trie->root;
    for (int p = 0; keyword[p] != '\0'; p ++) {
        int c = keyword[p] - 'a';
        if (u->ch[c] == NULL) {
            if (func == NULL)
                alloc_block(trie_node_t, &u->ch[c]);
            else
                func(&u->ch[c]);
            if (++trie->total_nodes > MAX_TRIE_NODE_NUM) {
                __CLOX_ERROR("The utils trie can support up to 1000 trie nodes.");
            }
#ifdef DEBUG_MEMORY_BLOCK
            printf("allocated block: %p\n", u->ch[c]);
#endif
            trie_node_init(u->ch[c]);
        }
        u = u->ch[c];
    }
    u->is_end = true;
    return u;
}

void trie_insert(trie_t* trie, const char* keyword) {
    __trie_insert(trie, keyword, NULL);
}

void __trie_free(trie_t* trie, trie_node_free_func func) {
    trie_node_t* stack[MAX_TRIE_NODE_NUM];
    uint8_t      ptr[MAX_TRIE_NODE_NUM];
    int stack_ptr = 0;

    trie_node_t* cur = trie->root;
    uint8_t    s_ptr = 0;
    uint8_t    c_ptr = 0;

    for (;;) {
        if (stack_ptr == 0 && s_ptr == MAX_TRIE_CHAR_ID) {
            // no elements in stack and current pointer reaches the end
            // all nodes are tranversed.
            break;
        }

        bool all_cleaned = true;
        for (c_ptr = s_ptr; c_ptr < MAX_TRIE_CHAR_ID; c_ptr++) if (cur->ch[c_ptr]) {
            all_cleaned = false;

            stack[stack_ptr] = cur;
            ptr[stack_ptr]   = c_ptr + 1;
            ++stack_ptr;

            trie_node_t* aux = cur->ch[c_ptr];
            cur->ch[c_ptr] = NULL;

            cur = aux;
            s_ptr = 0;
            break;
        }
        
        if (!all_cleaned)
            continue;

#ifdef DEBUG_MEMORY_BLOCK
        printf("Freeing %p\n", cur);
#endif
        // call default discontructor if not defined func, or trying to free the trie root
        if (func == NULL || cur == trie->root)
            trie_node_free(cur);
        else
            func(cur);
        s_ptr = MAX_TRIE_CHAR_ID;

        if (stack_ptr) {
            --stack_ptr;
            cur = stack[stack_ptr];
            s_ptr = ptr[stack_ptr];
        }
    }
}

void trie_free(trie_t* trie) {
    __trie_free(trie, NULL);
}

bool trie_match(trie_t* trie, const char* keyword) {
    trie_node_t* node = trie_find(trie, keyword);
    if (node == NULL)
        return false;
    else
        return node->is_end;
}

trie_node_t* trie_find(trie_t* trie, const char* keyword) {
    trie_node_t* u = trie->root;
    for (int p = 0; keyword[p] != '\0'; p++) {
        int c = keyword[p] - 'a';
        u = u->ch[c];
        if (u == NULL)
            return u;
    }
    return u;
}

trie_node_t* __trie_find(trie_t* trie, const char* str, int length) {
    trie_node_t* u = trie->root;
    for (int i = 0; i < length; i++) {
        int c = str[i] - 'a';
        u = u->ch[c];
        if (u == NULL)
            return NULL;
    }
    if (!u->is_end)
        return NULL;
    return u;
}

void trie_node_init(trie_node_t* trie_node) {
    memset(trie_node->ch, 0, sizeof(trie_node->ch));
    trie_node->is_end = false;
}

void trie_node_free(trie_node_t* trie_node) {
    for (int i = 0; i < MAX_TRIE_CHAR_ID; i ++) if (trie_node->ch[i]) {
        __CLOX_ERROR("The trie tries to free a trie node with active child trie node.");
    }
    free(trie_node);
}

static void test_trie_with_expected(trie_t* trie, bool expected, const char* keyword) {
    if (expected != trie_match(trie, keyword)) {
        printf("Searching %-15s in trie, but the result is not as expected. Expected %d, but got %d.\n", keyword, expected, !expected);
    } else {
        printf("Searching %-15s in trie, and the result is expected. Result %d\n", keyword, expected);
    }
}

void trie_debug(trie_t* trie) {
    trie_init(trie);
    
    trie_insert(trie, "and");
    trie_insert(trie, "class");
    trie_insert(trie, "else");
    trie_insert(trie, "false");
    trie_insert(trie, "for");
    trie_insert(trie, "fun");

    test_trie_with_expected(trie, true, "and");
    test_trie_with_expected(trie, true, "class");
    test_trie_with_expected(trie, true, "else");
    test_trie_with_expected(trie, true, "false");
    test_trie_with_expected(trie, true, "for");
    test_trie_with_expected(trie, true, "fun");


    test_trie_with_expected(trie, false, "foa");
    test_trie_with_expected(trie, false, "fua");
    test_trie_with_expected(trie, false, "ans");
    test_trie_with_expected(trie, false, "andfda");
    test_trie_with_expected(trie, false, "ff");
    test_trie_with_expected(trie, false, "qaq");
    test_trie_with_expected(trie, false, "f");
    test_trie_with_expected(trie, false, "a");
    test_trie_with_expected(trie, false, "els");

    trie_free(trie);

}