// Own header
#include "parser.h"

// This project's headers
#include "hash_table.h"
#include "tokenizer.h"
#include "types.h"

// Standard headers
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static AstNode *parse_expression(void);


// ### Error handling ###
// 
// When the parser encounters an error it:
// * Stops parsing.
// * Reports the error to the user.
// * Leaves the AST in a consistent state - all AstNodes will be valid, but some
//   will probably have null pointers because of the incomplete parsing.
// * Frees any AstNodes it was in the middle of creating and hadn't been added to
//   the AST.
// 
// Doing this requires the call stack of parser to unwind, with each function
// propagating the error state to its caller.
// 
// Annoyingly the tokenizer reports errors too. So there's a lot of error
// checking to do.
// 
// The rules for writing a parser function are:
// 1. Every time you call another parse function or get next token, check for error.
// 2. On error free everything you allocated and return null.
// 3. Each parser function consumes all its tokens. ie current_token is left
//    holding the token the next parser function will consume.


// ***************************************************************************
// Helper functions
// ***************************************************************************

static void *report_error(char const *msg, Token const *bad_token) {
    fwrite(msg, strlen(msg), 1, stdout);
    printf("'%.*s'. line=%d column=%d'\n", 
        (int)bad_token->lexeme.len, bad_token->lexeme.data, 
        bad_token->line, bad_token->column);
    return NULL;
}

static AstNode *create_ast_node(AstNodeType type) {
    AstNode *node = calloc(1, sizeof(AstNode));
    node->type = type;
    return node;
}


// ***************************************************************************
// Parser functions that correspond to a grammar rule and AstNodeType
// ***************************************************************************

static AstNode *parse_func_call(Token const *name) {
    AstNode *rv = NULL;

    if (strview_cmp_cstr(&name->lexeme, "puts")) {
        if (!tokenizer_next_token()) goto error;

        rv = create_ast_node(NODE_FUNCTION_CALL);
        rv->func_call.func_name = name->lexeme;
        while (current_token.type != TOKEN_RPAREN) {
            AstNode *expr = parse_expression();
            if (!expr) goto error;
            darray_insert(&rv->func_call.parameters, expr);

            if (current_token.type == TOKEN_COMMA) {
                if (!tokenizer_next_token()) goto error;
            }
        }

        tokenizer_next_token();
        return rv;
    }
    else {
        report_error("Unknown function ", name);
        goto error;
    }

error:
    parser_free_ast(rv);
    return NULL;
}

static AstNode *parse_primary(void) {
    AstNode *rv = NULL;

    if (current_token.type == TOKEN_LPAREN) {
        if (!tokenizer_next_token()) goto error;
        rv = parse_expression();
        if (!rv) goto error;
        if (current_token.type != TOKEN_RPAREN) {
            report_error("Expected ). Got ", &current_token);
            goto error;
        }
        if (!tokenizer_next_token()) goto error;
    }
    else if (current_token.type == TOKEN_IDENTIFIER) {
        Token ident_token = current_token;
        if (!tokenizer_next_token()) goto error;

        if (current_token.type == TOKEN_LPAREN) {
            rv = parse_func_call(&ident_token);
        }
        else {
            rv = create_ast_node(NODE_IDENTIFIER);
            rv->identifier_name = ident_token.lexeme;
        }
//         if (!lookup_identifier()) {
//             report_error("Expected 
//         }
    }
    else if (current_token.type == TOKEN_NUMBER) {
        rv = create_ast_node(NODE_NUMBER);
        if (!strview_to_int(&current_token.lexeme, &rv->int_value)) {
            report_error("Expected number. Got ", &current_token);
            goto error;
        }
        if (!tokenizer_next_token()) goto error;
    }
    else if (current_token.type == TOKEN_STRING) {
        rv = create_ast_node(NODE_STRING_LITERAL);
        rv->string_literal = current_token.lexeme;
        if (!tokenizer_next_token()) goto error;
    }
    else {
        return report_error("Expected identifier or literal. Got ", &current_token);
    }

    return rv;

error:
    parser_free_ast(rv);
    return NULL;
}

static AstNode *parse_unary_expression(void) {
    AstNode *rv = NULL;

    if (current_token.type == TOKEN_EXCLAMATION || current_token.type == TOKEN_MINUS) {
        rv = create_ast_node(NODE_UNARY_OP);
        rv->unary_op.operator = current_token.type;
        if (!tokenizer_next_token()) goto error;
        rv->unary_op.operand = parse_unary_expression();
        return rv;
    }

    rv = parse_primary();
    if (!rv) goto error;
    return rv;

error:
    parser_free_ast(rv);
    return NULL;
}

static AstNode *parse_add_expression(void) {
    AstNode *right = NULL;
    AstNode *left = parse_unary_expression();
    if (!left) goto error;

    if (current_token.type == TOKEN_PLUS || current_token.type == TOKEN_MINUS) {
        AstNode *equals; // VS2013 insists I have to put this up here.

        equals = create_ast_node(NODE_BINARY_OP);
        equals->binary_op.op = current_token.type;
        equals->binary_op.left = left;

        if (!tokenizer_next_token()) goto error;
        right = parse_add_expression();
        if (!right) goto error;

        equals->binary_op.right = right;
        return equals;
    }

    return left;

error:
    parser_free_ast(left);
    parser_free_ast(right);
    return NULL;
}

static AstNode *parse_compare_expression(void) {
    AstNode *right = NULL;
    AstNode *left = parse_add_expression();
    if (!left) goto error;

    if (current_token.type == TOKEN_EQUALS) {
        AstNode *equals; // VS2013 insists I have to put this up here.

        equals = create_ast_node(NODE_COMPARE);
        equals->compare_op.op = TOKEN_EQUALS;
        equals->compare_op.left = left;

        if (!tokenizer_next_token()) goto error;
        right = parse_compare_expression();
        if (!right) goto error;

        equals->compare_op.right = right;
        return equals;
    }

    return left;

error:
    parser_free_ast(left);
    parser_free_ast(right);
    return NULL;
}

static AstNode *parse_assign_expression(void) {
    AstNode *right = NULL;

    AstNode *left = parse_compare_expression();
    if (!left) goto error;

    if (current_token.type == TOKEN_ASSIGN) {
        AstNode *assignment; // VS2013 insists I have to put this up here.
        if (!tokenizer_next_token()) goto error;
        right = parse_assign_expression();
        if (!right) goto error;

        assignment = create_ast_node(NODE_ASSIGNMENT);
        assignment->assignment.left = left;
        assignment->assignment.right = right;
        return assignment;
    }

    return left;

error:
    parser_free_ast(left);
    parser_free_ast(right);
    return NULL;
}

static AstNode *parse_expression(void) {
    return parse_assign_expression();
}

static AstNode *parse_expr_statement(void) {
    AstNode *expr = parse_expression();
    if (!expr) return NULL;

    if (current_token.type != TOKEN_SEMICOLON || !tokenizer_next_token()) {
        parser_free_ast(expr);
        return report_error("Expected semicolon. Got ", &current_token);
    }

    return expr;
}

AstNode *parse_variable_declaration(type_info_t *type_info /* can be NULL */) {  
    AstNode *node = NULL;

    if (!type_info) {
        type_info = hashtab_get(&g_types, &current_token.lexeme);
        if (!type_info) {
            return report_error("Undeclared type ", &current_token);
        }
    }

    if (!tokenizer_next_token()) return NULL;
    if (current_token.type != TOKEN_IDENTIFIER)
        return report_error("Expected identifier. Got ", &current_token);

    node = create_ast_node(NODE_VARIABLE_DECLARATION);
    node->var_decl.type_info = type_info;
    node->var_decl.identifier_name = current_token.lexeme;

    if (!tokenizer_next_token() || current_token.type != TOKEN_SEMICOLON)
        goto error;

    if (!tokenizer_next_token())
        goto error;

    return node;

error:
    parser_free_ast(node);
    return NULL;
}

AstNode *parse_compound_statement(void) {
    AstNode *compound_stmt = create_ast_node(NODE_BLOCK);
    while (current_token.type != TOKEN_EOF) {
        AstNode *node = NULL;

        type_info_t *this_type = hashtab_get(&g_types, &current_token.lexeme);
        if (this_type) {
            // We've found a variable declaration.
            node = parse_variable_declaration(this_type);
        }
        else {
            // We must have an expression.
            node = parse_expr_statement();
        }
        
        if (!node) break;

        darray_insert(&compound_stmt->block.statements, node);
    }

    return compound_stmt;
}


// ***************************************************************************
// Public functions
// ***************************************************************************

AstNode *parser_parse(char const *source_code) {
    types_init();
    tokenizer_init(source_code);

    return parse_compound_statement();
}

void parser_free_ast(AstNode *node) {
    if (!node) return;

    switch (node->type) {
    case NODE_NUMBER:
    case NODE_IDENTIFIER:
        // No dynamic memory
        break;
    case NODE_ASSIGNMENT:
        parser_free_ast(node->assignment.left);
        parser_free_ast(node->assignment.right);
        break;
    case NODE_BINARY_OP:
        parser_free_ast(node->binary_op.left);
        parser_free_ast(node->binary_op.right);
        break;
    case NODE_COMPARE:
        parser_free_ast(node->compare_op.left);
        parser_free_ast(node->compare_op.right);
        break;
    case NODE_UNARY_OP:
        parser_free_ast(node->unary_op.operand);
        break;
    case NODE_BLOCK: {
            for (unsigned i = 0; i < node->block.statements.size; i++) {
                parser_free_ast(node->block.statements.data[i]);
            }
            darray_free(&node->block.statements);
            break;
        }
    case NODE_FUNCTION_CALL:
        darray_free(&node->func_call.parameters);
        break;
    }

    free(node);
}

static void print_ast_indent(int indent_level) {
    for (int i = 0; i < indent_level; i++) {
        printf("  ");
    }
}

void parser_print_ast_node(AstNode *node, int indent_level) {
    if (!node) {
        return;
    }

    print_ast_indent(indent_level);
    switch (node->type) {
    case NODE_NUMBER:
        printf("NUMBER: %d\n", node->int_value);
        break;
    case NODE_IDENTIFIER:
        printf("IDENTIFIER: %.*s\n", (int)node->identifier_name.len, node->identifier_name.data);
        break;
    case NODE_ASSIGNMENT:
        printf("ASSIGNMENT:\n");
        print_ast_indent(indent_level + 1);
        printf("LHS:\n");
        parser_print_ast_node(node->assignment.left, indent_level + 2);
        print_ast_indent(indent_level + 1);
        printf("RHS:\n");
        parser_print_ast_node(node->assignment.right, indent_level + 2);
        break;
    case NODE_BINARY_OP:
        printf("BINARY_OP: ");
        switch (node->binary_op.op) {
        case TOKEN_PLUS: printf("+\n"); break;
        case TOKEN_MINUS: printf("-\n"); break;
        case TOKEN_MULTIPLY: printf("*\n"); break;
        case TOKEN_DIVIDE: printf("/\n"); break;
        default: printf("UNKNOWN_OP\n"); break;
        }
        print_ast_indent(indent_level + 1);
        printf("Left:\n");
        parser_print_ast_node(node->binary_op.left, indent_level + 2);
        print_ast_indent(indent_level + 1);
        printf("Right:\n");
        parser_print_ast_node(node->binary_op.right, indent_level + 2);
        break;
    case NODE_COMPARE:
        printf("COMPARE: ");
        switch (node->compare_op.op) {
        case TOKEN_EQUALS: printf("==\n"); break;
        default: printf("UNKNOWN_OP\n"); break;
        }
        print_ast_indent(indent_level + 1);
        printf("Left:\n");
        parser_print_ast_node(node->binary_op.left, indent_level + 2);
        print_ast_indent(indent_level + 1);
        printf("Right:\n");
        parser_print_ast_node(node->binary_op.right, indent_level + 2);
        break;
    case NODE_UNARY_OP:
        printf("UNARY_OP: %c\n", node->unary_op.operator);
        parser_print_ast_node(node->unary_op.operand, indent_level + 2);
        break;
    case NODE_BLOCK: {
            printf("Block of %d statements\n", node->block.statements.size);
            for (unsigned i = 0; i < node->block.statements.size; i++) {
                parser_print_ast_node(node->block.statements.data[i], indent_level + 2);
            }
            break;
        }
    case NODE_STRING_LITERAL:
        printf("STRING: %.*s\n", node->string_literal.len, node->string_literal.data);
        break;
    case NODE_FUNCTION_CALL:
        printf("FUNCTION_CALL: %.*s\n", node->func_call.func_name.len, node->func_call.func_name.data);
        for (unsigned i = 0; i < node->func_call.parameters.size; i++)
            parser_print_ast_node(node->func_call.parameters.data[i], indent_level + 2);
        break;

    default:
        assert(0);
    }
}