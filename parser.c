// Own header
#include "parser.h"

// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This project's headers
#include "tokenizer.h"


bool g_is_valid;


static void *set_error(void) {
    g_is_valid = false;
    return NULL;
}

static void *report_error(char const *msg, strview_t *sv) {
    fwrite(msg, strlen(msg), 1, stdout);
    printf("'%.*s'. line=%d column=%d'\n", sv->len, sv->data, 
        current_token.line, current_token.column);
    return set_error();
}

static AstNode *create_ast_node(AstNodeType type) {
    AstNode *node = calloc(1, sizeof(AstNode));
    node->type = type;
    return node;
}

// Grammar:
// function_def -> 'fn' type '(' [params] ')' '{' statement* '}'
// params       -> param (',' param)*
// param        -> type identifier
// statement    -> assignment | declaration | return ';'
// assignment   -> IDENTIFIER = expression ';'
// declaration  -> type identifier [ = expression ] ';'
// expression   -> term ( ( '+' | '-' ) term )*
// term         -> factor ( ( '*' | '/' ) factor )*
// factor       -> NUMBER | IDENTIFIER | '(' expression ')'

static AstNode *parse_expression();

static AstNode *parse_factor() {
    AstNode *node = NULL;
    if (current_token.type == TOKEN_NUMBER) {
        node = create_ast_node(NODE_NUMBER);
        if (!strview_to_int(&current_token.lexeme, &node->data.int_value)) {
            return report_error("Expected number but got", &current_token.lexeme);
        }
        if (!tokenizer_consume(TOKEN_NUMBER)) return set_error();
    }
    else if (current_token.type == TOKEN_IDENTIFIER) {
        node = create_ast_node(NODE_IDENTIFIER);
        node->data.identifier_name = current_token.lexeme;
        if (!tokenizer_consume(TOKEN_IDENTIFIER)) return set_error();
    }
    else if (current_token.type == TOKEN_LPAREN) {
        if (!tokenizer_consume(TOKEN_LPAREN)) return set_error();
        node = parse_expression();
        if (!tokenizer_consume(TOKEN_RPAREN)) return set_error();
    }
    else {
        return report_error("Expected number, identifier, or '(' but got", 
            &current_token.lexeme);
    }

    return node;
}

static AstNode *parse_term() {
    AstNode *node = parse_factor();
    if (!g_is_valid) return NULL;
    while (current_token.type == TOKEN_MULTIPLY || current_token.type == TOKEN_DIVIDE) {
        AstNode *new_node = create_ast_node(NODE_BINARY_OP);
        new_node->data.binary_op.op = current_token.type;
        if (!tokenizer_consume(current_token.type)) // Consume '*' or '/'
            return set_error();
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = parse_factor();
        if (!new_node->data.binary_op.right) return NULL;
        node = new_node;
    }
    return node;
}

static AstNode *parse_expression() {
    AstNode *node = parse_term();
    if (!g_is_valid) return NULL;
    while (current_token.type == TOKEN_PLUS || current_token.type == TOKEN_MINUS) {
        AstNode *new_node = create_ast_node(NODE_BINARY_OP);
        new_node->data.binary_op.op = current_token.type;
        if (!tokenizer_consume(current_token.type)) // Consume '+' or '-'
            return set_error();
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = parse_term();
        if (!new_node->data.binary_op.right) return NULL;
        node = new_node;
    }
    return node;
}

// assignment   -> IDENTIFIER = expression ';'
static AstNode *parse_assignment() {
    if (current_token.type == TOKEN_IDENTIFIER) {
        Token potential_id = current_token;
        if (!tokenizer_next_token()) { // Look ahead to see if it's an assignment
            return set_error();
        }
        if (current_token.type == TOKEN_ASSIGN) {
            AstNode *assign_node = create_ast_node(NODE_ASSIGNMENT);
            assign_node->data.assignment.identifier = create_ast_node(NODE_IDENTIFIER);
            assign_node->data.assignment.identifier->data.identifier_name = potential_id.lexeme;

            if (!tokenizer_consume(TOKEN_ASSIGN)) {
                return set_error();
            }

            assign_node->data.assignment.expression = parse_expression();
            if (!assign_node->data.assignment.expression) return NULL;
            return assign_node;
        }
        else {
            // It was just an identifier in an expression, not an assignment
            // We need to put the identifier token back for parse_expression to handle
            // (This is a simplified approach, a real parser might backtrack or have more robust lookahead)
            current_token = potential_id; // "Put back" the token
            // A more robust way would be to create a new token struct to store,
            // or pass around a pointer to the current_token and its lexeme.
            // For simplicity here, we'll assume next_token() correctly re-populates current_token.
            // In a real scenario, you'd use a peek() function or a token buffer.
            // For now, we'll just let parse_expression handle it.
            return parse_expression(); // Re-parse from the identifier
        }
    }

    return parse_expression(); // If not an identifier, it must be an expression
}

AstNode *parser_parse(char const *source_code) {
    g_is_valid = true;
    tokenizer_init(source_code);
    tokenizer_next_token(); // Get the first token
    AstNode *root = parse_assignment(); // Try parsing as an assignment first
    if (g_is_valid) {
        tokenizer_consume(TOKEN_EOF); // Ensure we've consumed all input
    }
    return root;
}

void parser_free_ast(AstNode *node) {
    if (!node) {
        return;
    }
    switch (node->type) {
    case NODE_NUMBER:
    case NODE_IDENTIFIER:
        // No dynamic memory
        break;
    case NODE_ASSIGNMENT:
        parser_free_ast(node->data.assignment.identifier);
        parser_free_ast(node->data.assignment.expression);
        break;
    case NODE_BINARY_OP:
        parser_free_ast(node->data.binary_op.left);
        parser_free_ast(node->data.binary_op.right);
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
        printf("NUMBER: %d\n", node->data.int_value);
        break;
    case NODE_IDENTIFIER:
        printf("IDENTIFIER: %.*s\n", node->data.identifier_name.len, node->data.identifier_name.data);
        break;
    case NODE_ASSIGNMENT:
        printf("ASSIGNMENT:\n");
        print_ast_indent(indent_level + 1);
        printf("LHS:\n");
        parser_print_ast_node(node->data.assignment.identifier, indent_level + 2);
        print_ast_indent(indent_level + 1);
        printf("RHS:\n");
        parser_print_ast_node(node->data.assignment.expression, indent_level + 2);
        break;
    case NODE_BINARY_OP:
        printf("BINARY_OP: ");
        switch (node->data.binary_op.op) {
        case TOKEN_PLUS: printf("+\n"); break;
        case TOKEN_MINUS: printf("-\n"); break;
        case TOKEN_MULTIPLY: printf("*\n"); break;
        case TOKEN_DIVIDE: printf("/\n"); break;
        default: printf("UNKNOWN_OP\n"); break;
        }
        print_ast_indent(indent_level + 1);
        printf("Left:\n");
        parser_print_ast_node(node->data.binary_op.left, indent_level + 2);
        print_ast_indent(indent_level + 1);
        printf("Right:\n");
        parser_print_ast_node(node->data.binary_op.right, indent_level + 2);
        break;
    }
}