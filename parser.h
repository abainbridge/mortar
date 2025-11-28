// A Recursive Descent Parser. Produces an Abstract Syntax Tree from a stream
// of tokens.

#pragma once

// Headers from this project
#include "darray.h"
#include "tokenizer.h"

typedef struct _type_info_t type_info_t;

typedef enum {
    NODE_NUMBER = 0,
    NODE_IDENTIFIER,
    NODE_ASSIGNMENT = 2,
    NODE_BINARY_OP,
    NODE_COMPARE = 4,
    NODE_UNARY_OP,
    NODE_BLOCK = 6,
    NODE_STRING_LITERAL,
    NODE_FUNCTION_CALL = 8,
    NODE_VARIABLE_DECLARATION,
    NODE_WHILE = 10
} ast_node_type_t;

typedef struct _ast_node_t {
    ast_node_type_t type;

    union {
        struct {
            int int_value;
        } number;

        struct {
            strview_t name;
        } identifier;

        struct {
            struct _ast_node_t *left;
            struct _ast_node_t *right;
        } assignment;

        struct {
            TokenType op; // The operator token type
            struct _ast_node_t *left;
            struct _ast_node_t *right;
        } binary_op;

        struct {
            TokenType op;
            struct _ast_node_t *left;
            struct _ast_node_t *right;
        } compare_op;

        struct {
            TokenType operator;
            struct _ast_node_t *operand;
        } unary_op;

        struct {
            darray_t statements; // Array stores pointers to AstNodes
        } block;

        struct {
            strview_t val;
        } string_literal;

        struct {
            strview_t func_name;
            darray_t parameters; // Array stores pointers to AstNodes
        } func_call;

        struct {
            type_info_t *type_info;
            strview_t identifier_name;
        } var_decl;

        struct {
            struct _ast_node_t *condition_expr;
            struct _ast_node_t *block;
        } while_loop;
    };

    // You could add line/column info here for error reporting
} ast_node_t;


void parser_free_ast(ast_node_t *node);
ast_node_t *parser_parse(char const *source_code);
void parser_print_ast_node(ast_node_t *node, int indent_level);
