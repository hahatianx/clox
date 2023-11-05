#ifndef __CLOX_KEYWORD_TRIE_H__
#define __CLOX_KEYWORD_TRIE_H__

#include "common.h"
#include "utils/trie.h"
#include "vm/scanner.h"


typedef struct {
    trie_t trie;
} keyword_trie_t;

typedef struct {
    tokentype_t tokentype;
    trie_node_t trie_node;
} keyword_trie_node_t;

void keyword_trie_init  (keyword_trie_t* trie);
void keyword_trie_insert(keyword_trie_t* trie, const char* keyword, tokentype_t tokentype);
void keyword_trie_free  (keyword_trie_t* trie);

#endif