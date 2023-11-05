#ifndef __CLOX_TRIE_H__
#define __CLOX_TRIE_H__

#include "common.h"

#define MAX_TRIE_NODE_NUM 1000
#define MAX_TRIE_CHAR_ID  26

/**
 * This is a trie which takes lower english letters. 
 * Other uses may result in pointer errors
 * 
 * The number of trie nodes of a tree cannot exceed 1000
 * Exceeding the number can cause unexpected issues.
 */


typedef struct __trie_node_t {
    bool is_end;
    struct __trie_node_t* ch[MAX_TRIE_CHAR_ID];
} trie_node_t;

typedef void (*trie_node_alloc_func)(trie_node_t** return_ptr);
typedef void (*trie_node_free_func)(trie_node_t* ptr);

typedef struct {
    int total_nodes;
    trie_node_t* root;
} trie_t;


void trie_init(trie_t* trie);
void trie_free(trie_t* trie);
void trie_insert(trie_t* trie, const char* keyword);
trie_node_t* trie_find(trie_t* trie, const char* keyword);
trie_node_t* __trie_find(trie_t* trie, const char* str, int length);
bool trie_match(trie_t* trie, const char* keyword);

trie_node_t* __trie_insert(trie_t* trie, const char* keyword, trie_node_alloc_func func);
void __trie_free(trie_t* trie, trie_node_free_func func);

#define trie_find_node(type, member, trie, keyword, pointer) \
    do { \
        trie_node_t* node = trie_find(trie, keyword); \
        if (node == NULL)  \
            *pointer = NULL; \
        else  \
            *pointer = ((type*)__object(type, node, member)); \
    } while (0)

#define __trie_find_node(type, member, trie, str, length, pointer) \
    do { \
        trie_node_t* node = __trie_find(trie, str, length); \
        if (node == NULL)  \
            *pointer = NULL; \
        else \
            *pointer = ((type*)__object(type, node, member)); \
    } while (0)



void trie_node_init(trie_node_t* trie_node);
void trie_node_free(trie_node_t* trie_node);

void trie_debug(trie_t* trie);


#endif