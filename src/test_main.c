#include <stdio.h>
#include <assert.h>

#include "common.h"

#include "component/keywordtrie.h"
#include "utils/trie.h"


int main(int argc, const char** argv) {

    trie_t trie;
    trie_debug(&trie);

    keyword_trie_t test;
    keyword_trie_init(&test);

    keyword_trie_insert(&test, "and", TOKEN_AND);

    keyword_trie_node_t* x;
    trie_find_node(keyword_trie_node_t, trie_node, &test.trie, "and", &x);
    assert(x != NULL);
    assert(TOKEN_AND == x->tokentype);

    trie_find_node(keyword_trie_node_t, trie_node, &test.trie, "ans", &x);
    assert(x == NULL);

    keyword_trie_free(&test);

    return 0;
}