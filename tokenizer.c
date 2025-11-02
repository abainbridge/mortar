// Own header
#include "tokenizer.h"

// Standard headers
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static char const *input_code;
static char const *c;
Token current_token;


static void next_char(void) {
    c++;
    if (*c == '\n')
        current_token.line++;
    current_token.column++;
}

static void skip_whitespace(void) {
    while (isspace(*c)) {
        next_char();
    }
}

void tokenizer_init(char const *source_code) {
    input_code = source_code;
    c = source_code;
    current_token.type = TOKEN_EOF; // Or some initial invalid state
    current_token.lexeme = strview_empty();
    current_token.line = 1;
    current_token.column = 0;

    tokenizer_next_token(); // Get the first token
}

// Gets the next token from the input
bool tokenizer_next_token(void) {
    skip_whitespace();

    current_token.lexeme = strview_empty();

    if (*c == '\0') {
        current_token.type = TOKEN_EOF;
        current_token.lexeme = strview_empty();
        return true;
    }

    if (isdigit(*c)) {
        char const *start_c = c;
        while (isdigit(*c)) {
            next_char();
        }
        int length = (int)(c - start_c);
        current_token.lexeme = strview_create(start_c, length);
        current_token.type = TOKEN_NUMBER;
        return true;
    }

    if (isalpha(*c) || *c == '_') {
        char const *start_c = c;
        while (isalnum(*c) || *c == '_') {
            next_char();
        }
        int length = (int)(c - start_c);
        current_token.lexeme = strview_create(start_c, length);
        current_token.type = TOKEN_IDENTIFIER;
        // Could check for keywords here (e.g., if, for, while)
        return true;
    }

    switch (*c) {
    case ';': current_token.type = TOKEN_SEMICOLON; next_char(); break;
    case '=':
        if (c[1] == '=') {
            current_token.type = TOKEN_EQUALS;
            next_char();
        }
        else {
            current_token.type = TOKEN_ASSIGN;
        }
        next_char();
        break;
    case '+': current_token.type = TOKEN_PLUS; next_char(); break;
    case '-': current_token.type = TOKEN_MINUS; next_char(); break;
    case '*': current_token.type = TOKEN_MULTIPLY; next_char(); break;
    case '/': current_token.type = TOKEN_DIVIDE; next_char(); break;
    case '(': current_token.type = TOKEN_LPAREN; next_char(); break;
    case ')': current_token.type = TOKEN_RPAREN; next_char(); break;
    default:
        printf("Unexpected character '%c' at line %d, column %d\n", 
            *c, current_token.line, current_token.column);
        return false;
    }

    current_token.lexeme = strview_create(c - 1, 1); // For single-char tokens
    return true;
}

bool tokenizer_consume(TokenType expected_type) {
    if (current_token.type == expected_type) {
        return tokenizer_next_token();
    }

    char const *expected = tokenizer_get_name_from_type(expected_type);
    char const *got = tokenizer_get_name_from_type(current_token.type);
    printf("Expected %s, but got %s ('%.*s') at line %d column %d\n",
        expected, got,
        current_token.lexeme.len, current_token.lexeme.data,
        current_token.line, current_token.column);
    return false;
}

char const *tokenizer_get_name_from_type(TokenType t) {
    switch (t) {
    case TOKEN_EOF: return "End of File";
    case TOKEN_IDENTIFIER: return "Identifier";
    case TOKEN_NUMBER: return "Number";
    case TOKEN_ASSIGN: return "Assignment";
    case TOKEN_PLUS: return "+";
    case TOKEN_MINUS: return "-";
    case TOKEN_MULTIPLY: return "*";
    case TOKEN_DIVIDE: return "/";
    case TOKEN_LPAREN: return "(";
    case TOKEN_RPAREN: return ")";
    }
}
