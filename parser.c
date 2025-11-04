// Own header
#include "parser.h"

// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This project's headers
#include "tokenizer.h"


static bool g_is_valid;

static AstNode *parse_expression(void);


static void *set_error(void) {
    g_is_valid = false;
    return NULL;
}

static void *report_error(char const *msg, strview_t *sv) {
    fwrite(msg, strlen(msg), 1, stdout);
    printf("'%.*s'. line=%d column=%d'\n", (int)sv->len, sv->data, 
        current_token.line, current_token.column);
    return set_error();
}

static AstNode *create_ast_node(AstNodeType type) {
    AstNode *node = calloc(1, sizeof(AstNode));
    node->type = type;
    return node;
}

static AstNode *parse_primary(void) {
    AstNode *rv = NULL;

    if (current_token.type == TOKEN_LPAREN) {
        if (!tokenizer_next_token()) goto error;
        rv = parse_expression();
        if (!rv) goto error;
        if (current_token.type != TOKEN_RPAREN) {
            report_error("Expected ). Got ", &current_token.lexeme);
            goto error;
        }
        if (!tokenizer_next_token()) goto error;
        return rv;
    }

    if (current_token.type == TOKEN_IDENTIFIER) {
        rv = create_ast_node(NODE_IDENTIFIER);
        rv->identifier_name = current_token.lexeme;
        if (!tokenizer_next_token()) goto error;
//         if (!lookup_identifier()) {
//             report_error("Expected 
//         }
        return rv;
    }
    else if (current_token.type == TOKEN_NUMBER) {
        rv = create_ast_node(NODE_NUMBER);
        if (!strview_to_int(&current_token.lexeme, &rv->int_value)) {
            report_error("Expected number. Got ", &current_token.lexeme);
            goto error;
        }
        if (!tokenizer_next_token()) goto error;
        return rv;
    }
    else {
        return report_error("Expected identifier or literal. Got ", &current_token.lexeme);
    }

error:
    parser_free_ast(rv);
    return set_error();
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
    return set_error();
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
    return set_error();
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
    return set_error();
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
    return set_error();
}

static AstNode *parse_expression(void) {
    return parse_assign_expression();
}

static AstNode *parse_expr_statement(void) {
    AstNode *expr = parse_expression();
    if (!expr) return NULL;

    if (current_token.type != TOKEN_SEMICOLON || !tokenizer_next_token()) {
        parser_free_ast(expr);
        return report_error("Expected semicolon. Got ", &current_token.lexeme);
    }

    return expr;
}

AstNode *parser_parse(char const *source_code) {
    g_is_valid = true;
    tokenizer_init(source_code);

    AstNode *root = create_ast_node(NODE_BLOCK);
    while (g_is_valid && current_token.type != TOKEN_EOF) {
        AstNode *statement = parse_expr_statement();
        darray_insert(&root->block.statements, statement);
    }

    return root;
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
    }
}