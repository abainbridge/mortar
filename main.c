#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

static run_test(char const *source_code) {
    printf("--- Parsing Code: \"%s\" ---\n", source_code);
    AstNode *ast = parser_parse(source_code);

    if (ast) {
        printf("--- Abstract Syntax Tree ---\n");
        parser_print_ast_node(ast, 0);
        parser_free_ast(ast);
    }

    printf("\n");
}

int main() {
    // Test cases
    run_test("x = ;10 + 5 * 2");
    run_test("result = (a + b) / c - 7");
    run_test("my_var = 100");
    run_test("5 * (3 + 2)"); // Pure expression (not an assignment)
    run_test("another_var = x + y * z");

//    run_test("i32 i = 0;");

    return 0;
}
