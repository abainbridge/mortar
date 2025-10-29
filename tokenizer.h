#pragma once


#include "strview.h"


typedef enum {
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_ASSIGN,       // =
    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_MULTIPLY,     // *
    TOKEN_DIVIDE,       // /
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
} TokenType;

typedef struct {
    TokenType type;
    strview_t lexeme;
    int line;
    int column;
} Token;


extern Token current_token;

void tokenizer_init(char const *source_code);
bool tokenizer_next_token(void); // Returns false on error.

// Consumes the current token if its type matches, otherwise returns false.
bool tokenizer_consume(TokenType expected_type);

char const *tokenizer_get_name_from_type(TokenType t);
