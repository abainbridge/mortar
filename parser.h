// A Recursive Descent Parser. Produces an Abstract Syntax Tree from a stream
// of tokens.

#pragma once

// Headers from this project
#include "darray.h"
#include "tokenizer.h"

typedef struct _type_info_t type_info_t;

typedef enum {
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_ASSIGNMENT,
    NODE_BINARY_OP,
    NODE_COMPARE,
    NODE_UNARY_OP,
    NODE_BLOCK,
    NODE_STRING_LITERAL,
    NODE_FUNCTION_CALL,
    NODE_VARIABLE_DECLARATION
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
            TokenType operator;
            struct AstNode *operand;
        } unary_op;

        struct {
            darray_t statements; // Array stores pointers to AstNodes
        } block;

        strview_t string_literal;

        struct {
            strview_t func_name;
            darray_t parameters; // Array stores pointers to AstNodes
        } func_call;

        struct {
            type_info_t *type_info;
            strview_t identifier_name;
        } var_decl;
    };

    // You could add line/column info here for error reporting
} AstNode;


void parser_free_ast(AstNode *node);
AstNode *parser_parse(char const *source_code);
void parser_print_ast_node(AstNode *node, int indent_level);
