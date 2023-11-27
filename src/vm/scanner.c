#include <string.h>

#include "common.h"
#include "vm/scanner.h"
#include "utils/trie.h"
#include "component/keywordtrie.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} scanner_t;

scanner_t scanner;

static keyword_trie_t keyword_trie;

static bool is_at_end() {
    return *scanner.current == '\0';
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

static bool match(char expected) {
    if (is_at_end()) return false;
    if (*scanner.current != expected) return false;
    scanner.current++;
    return true;
}

static token_t make_token(tokentype_t type) {
    token_t token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

static token_t error_token(const char* message) {
    token_t token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

static char advance() {
    scanner.current++;
    return scanner.current[-1];
}

static char peek() {
    return *scanner.current;
}

static char peek_next() {
    if (is_at_end()) return '\0';
    return scanner.current[1];
}

static void skip_whitespace() {
    for (;;) {
        char c = peek();
        switch(c) {
            case '\n':
                scanner.line++;
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '/':
                if (peek_next() == '/') {
                    while (peek() != '\n' && !is_at_end()) advance();
                    break;
                } else {
                    return;
                }
            default:
                return;
        }
    }
}

static token_t parse_number() {
    bool is_integer = true;
    while (is_digit(peek())) advance();
    if (peek() == '.' && is_digit(peek_next())) {
        is_integer = false;
        advance();
        while (is_digit(peek())) advance();
    }

    if (is_integer)
        return make_token(TOKEN_INTEGER);
    else
        return make_token(TOKEN_NUMBER);
}

static token_t parse_string() {
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') scanner.line++;
        advance();
    }

    if (is_at_end()) return error_token("Unterminated string.");

    advance();
    return make_token(TOKEN_STRING);
}

static tokentype_t identifier_type() {
    
    keyword_trie_node_t* keyword_found;
    int length = scanner.current - scanner.start;
    __trie_find_node(keyword_trie_node_t, trie_node, &keyword_trie.trie, scanner.start, length, &keyword_found);

    if (keyword_found != NULL)
        return keyword_found->tokentype;

    return TOKEN_IDENTIFIER;
}

static token_t parse_identifier() {
    while (is_alpha(peek()) || is_digit(peek())) advance();
    return make_token(identifier_type());
}

static void init_keyword_trie() {
    keyword_trie_init(&keyword_trie);
    
    keyword_trie_insert(&keyword_trie, "and",      TOKEN_AND);
    keyword_trie_insert(&keyword_trie, "class",    TOKEN_CLASS);
    keyword_trie_insert(&keyword_trie, "else",     TOKEN_ELSE);
    keyword_trie_insert(&keyword_trie, "if",       TOKEN_IF);
    keyword_trie_insert(&keyword_trie, "or",       TOKEN_OR);
    keyword_trie_insert(&keyword_trie, "super",    TOKEN_SUPER);
    keyword_trie_insert(&keyword_trie, "mut",      TOKEN_MUT);

    keyword_trie_insert(&keyword_trie, "var",      TOKEN_VAR);
    keyword_trie_insert(&keyword_trie, "fun",      TOKEN_FUN);

    keyword_trie_insert(&keyword_trie, "print",    TOKEN_PRINT);
    keyword_trie_insert(&keyword_trie, "println",  TOKEN_PRINTLN);

    keyword_trie_insert(&keyword_trie, "while",    TOKEN_WHILE);
    keyword_trie_insert(&keyword_trie, "for",      TOKEN_FOR);

    keyword_trie_insert(&keyword_trie, "return",   TOKEN_RETURN);
    keyword_trie_insert(&keyword_trie, "break",    TOKEN_BREAK);
    keyword_trie_insert(&keyword_trie, "continue", TOKEN_CONTINUE);

    keyword_trie_insert(&keyword_trie, "nil",      TOKEN_NIL);
    keyword_trie_insert(&keyword_trie, "true",     TOKEN_TRUE);
    keyword_trie_insert(&keyword_trie, "false",    TOKEN_FALSE);
}

static void free_keyword_trie() {
    keyword_trie_free(&keyword_trie);
}

void launch_scanner() {
    init_keyword_trie();
}

void free_scanner() {
    free_keyword_trie();
}

void init_scanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

token_t scan_token() {
    skip_whitespace();
    scanner.start = scanner.current;

    if (is_at_end()) return make_token(TOKEN_EOF);

    char c = advance();

    if (is_alpha(c)) return parse_identifier();
    if (is_digit(c)) return parse_number();

    switch(c) {
        case '(': return make_token(TOKEN_LEFT_PAREN);
        case ')': return make_token(TOKEN_RIGHT_PAREN);
        case '{': return make_token(TOKEN_LEFT_BRACE);
        case '}': return make_token(TOKEN_RIGHT_BRACE);
        case ';': return make_token(TOKEN_SEMICOLON);
        case ',': return make_token(TOKEN_COMMA);
        case '.': return make_token(TOKEN_DOT);
        case '-': return make_token(TOKEN_MINUS);
        case '+': return make_token(TOKEN_PLUS);
        case '*': return make_token(TOKEN_STAR);
        case '%': return make_token(TOKEN_PERCENT);
        case '&': return make_token(TOKEN_AMPERSAND);
        case '|': return make_token(TOKEN_PIPE);
        case '^': return make_token(TOKEN_CAP);
        case '/': 
            return make_token(
                match('#') ? TOKEN_FLOOR_DIVIDE : TOKEN_SLASH);
        case '!':
            return make_token(
                match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return make_token(
                match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            if (match('=')) {
                return make_token(TOKEN_LESS_EQUAL);
            } else if (match('<')) {
                return make_token(TOKEN_LEFT_SHIFT);
            } else {
                return make_token(TOKEN_LESS);
            }
        case '>':
            if (match('=')) {
                return make_token(TOKEN_GREATER_EQUAL);
            } else if (match('>')) {
                return make_token(TOKEN_RIGHT_SHIFT);
            } else {
                return make_token(TOKEN_GREATER);
            }
        case '"':
            return parse_string();
    }
    return error_token("Unexpected character.");
}