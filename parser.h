// A Recursive Descent Parser. Produces an Abstract Syntax Tree from a stream
// of tokens.

#pragma once

#include "tokenizer.h"


typedef enum {
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_ASSIGNMENT,
    NODE_BINARY_OP,
} AstNodeType;

typedef struct AstNode {
    AstNodeType type;

    union {
        // For NODE_NUMBER
        int int_value;

        // For NODE_IDENTIFIER
        strview_t identifier_name;

        // For NODE_ASSIGNMENT
        struct {
            struct AstNode *identifier; // Left-hand side (LHS)
            struct AstNode *expression; // Right-hand side (RHS)
        } assignment;

        // For NODE_BINARY_OP
        struct {
            TokenType op; // The operator token type
            struct AstNode *left;
            struct AstNode *right;
        } binary_op;
    } data;

    // You could add line/column info here for error reporting
} AstNode;


void parser_free_ast(AstNode *node);
AstNode *parser_parse(char const *source_code);
void parser_print_ast_node(AstNode *node, int indent_level);
