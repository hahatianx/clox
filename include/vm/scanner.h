#ifndef __CLOX_SCANNER_H__
#define __CLOX_SCANNER_H__

typedef enum {
    // single char
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,
    TOKEN_PERCENT, TOKEN_AMPERSAND, TOKEN_PIPE, TOKEN_CAP,

    // one or two char tokens
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL, TOKEN_FLOOR_DIVIDE,
    TOKEN_LEFT_SHIFT, TOKEN_RIGHT_SHIFT,

    // literals
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER, TOKEN_INTEGER,

    // keywords
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
    TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE, TOKEN_MUT, 
    TOKEN_BREAK, TOKEN_CONTINUE,
    TOKEN_PRINT, TOKEN_PRINTLN,

    TOKEN_ERROR, TOKEN_EOF
} tokentype_t;

typedef struct {
    tokentype_t type;
    const char* start;
    int length;
    int line;
} token_t;

void init_scanner(const char* source);
token_t scan_token();

void launch_scanner();
void free_scanner();

#endif