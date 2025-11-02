// A Recursive Descent Parser. Produces an Abstract Syntax Tree from a stream
// of tokens.

#pragma once

#include "tokenizer.h"


typedef enum {
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_ASSIGNMENT,
    NODE_BINARY_OP,
    NODE_COMPARE,
    NODE_UNARY_OP
} AstNodeType;

typedef struct AstNode {
    AstNodeType type;

    union {
        // For NODE_NUMBER
        int int_value;

        // For NODE_IDENTIFIER
        strview_t identifier_name;

        struct {
            struct AstNode *left;
            struct AstNode *right;
        } assignment;

        struct {
            TokenType op; // The operator token type
            struct AstNode *left;
            struct AstNode *right;
        } binary_op;

        struct {
            TokenType op;
            struct AstNode *left;
            struct AstNode *right;
        } compare_op;

        struct {
            struct AstNode *operand;
        } unary_op;
    } data;

    // You could add line/column info here for error reporting
} AstNode;


void parser_free_ast(AstNode *node);
AstNode *parser_parse(char const *source_code);
void parser_print_ast_node(AstNode *node, int indent_level);
