// Own header
#include "tokenizer.h"

// Standard headers
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// The standard ctype functions are not safe to use on char values. Make ones that are.
bool is_digit(char x) { return x >= '0' && x <= '9'; }
bool is_space(char x) { return x == ' ' || x == '\t' || x == '\n' || x == '\r'; }
bool is_alpha(char x) { x |= 32; return (x >= 'a' && x <= 'z'); }
bool is_alnum(char x) { return is_alpha(x) || is_digit(x); }


static char const *input_code;
static char const *c;
static unsigned line, column;
Token current_token;


static void next_char(void) {
    assert (*c != '\0');
    c++;
    if (*c == '\n') {
        line++;
        column = 1;
    }
    column++;
}

static void skip_whitespace(void) {
    while (is_space(*c)) {
        next_char();
    }
}

void tokenizer_init(char const *source_code) {
    input_code = source_code;
    c = source_code;
    current_token.type = TOKEN_EOF; // Or some initial invalid state
    current_token.lexeme = strview_empty();
    line = 1;
    column = 1;

    tokenizer_next_token(); // Get the first token
}

static bool get_string(void) {
    next_char();
    char const *start_c = c;
    while (*c != '"') {
        if (*c == '\\') {
            next_char();
        }
        if (*c == '\n' || *c == '\0') {
            printf("Unterminated string at line %d, column %d\n",
                line, column);
            return false;
        }
        next_char();
    }
    int len = (int)(c - start_c);
    next_char();
    current_token.lexeme = strview_create(start_c, len);
    current_token.type = TOKEN_STRING;
    return true;
}

// Gets the next token from the input
bool tokenizer_next_token(void) {
    skip_whitespace();

    current_token.lexeme = strview_empty();
    current_token.line = line;
    current_token.column = column;

    if (*c == '\0') {
        current_token.type = TOKEN_EOF;
        return true;
    }

    if (is_digit(*c)) {
        char const *start_c = c;
        next_char();
        while (is_digit(*c)) {
            next_char();
        }
        int length = (int)(c - start_c);
        current_token.lexeme = strview_create(start_c, length);
        current_token.type = TOKEN_NUMBER;
        return true;
    }

    if (is_alpha(*c) || *c == '_') {
        char const *start_c = c;
        next_char();
        while (is_alnum(*c) || *c == '_') {
            next_char();
        }
        int length = (int)(c - start_c);
        current_token.lexeme = strview_create(start_c, length);
        
        if (strview_cmp_cstr(&current_token.lexeme, "while"))
            current_token.type = TOKEN_WHILE;
        else
            current_token.type = TOKEN_IDENTIFIER;
        return true;
    }

    if (*c == '"')
        return get_string();

    if (*c == '!') {
        next_char();
        if (*c == '=') {
            next_char();
            current_token.type = TOKEN_NOT_EQUALS;
            return true;
        }

        current_token.type = TOKEN_EXCLAMATION;
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
    case '{': current_token.type = TOKEN_LBRACE; next_char(); break;
    case '}': current_token.type = TOKEN_RBRACE; next_char(); break;
    case '[': current_token.type = TOKEN_LBRACKET; next_char(); break;
    case ']': current_token.type = TOKEN_RBRACKET; next_char(); break;
    case '<': current_token.type = TOKEN_LESS_THAN; next_char(); break;
    case '>': current_token.type = TOKEN_GREATER_THAN; next_char(); break;
    case ',': current_token.type = TOKEN_COMMA; next_char(); break;
    default:
        printf("Unexpected character '%c' at line %d, column %d\n", 
            *c, line, column);
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
        (int)current_token.lexeme.len, current_token.lexeme.data,
        line, column);
    return false;
}

char const *tokenizer_get_name_from_type(TokenType t) {
    switch (t) {
    case TOKEN_EOF: return "End of File";
    case TOKEN_IDENTIFIER: return "Identifier";
    case TOKEN_NUMBER: return "Number";
    case TOKEN_STRING: return "String";
    case TOKEN_EQUALS: return "==";
    case TOKEN_SEMICOLON: return ";";
    case TOKEN_ASSIGN: return "Assignment";
    case TOKEN_PLUS: return "+";
    case TOKEN_MINUS: return "-";
    case TOKEN_MULTIPLY: return "*";
    case TOKEN_DIVIDE: return "/";
    case TOKEN_EXCLAMATION: return "!";
    case TOKEN_LPAREN: return "(";
    case TOKEN_RPAREN: return ")";
    }

    return "Unknown";
}
