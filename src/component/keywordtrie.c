#include "common.h"

#include "utils/trie.h"
#include "basic/memory.h"
#include "component/keywordtrie.h"

static void alloc_node(trie_node_t** return_ptr) {
    alloc_struct(keyword_trie_node_t, trie_node, return_ptr);
}

static void free_node(trie_node_t* ptr) {
    keyword_trie_node_t* obj = __object(keyword_trie_node_t, ptr, trie_node);
    free(obj);
}

void keyword_trie_init(keyword_trie_t* trie) {
    trie_init(&trie->trie);
}

void keyword_trie_insert(keyword_trie_t* trie, const char* keyword, tokentype_t tokentype) {
    trie_node_t* node = __trie_insert(&trie->trie, keyword, alloc_node);
    keyword_trie_node_t* obj = __object(keyword_trie_node_t, node, trie_node);
    obj->tokentype = tokentype;
}

void keyword_trie_free(keyword_trie_t* trie) {
    __trie_free(&trie->trie, free_node);
}