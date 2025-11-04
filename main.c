#include "parser.h"
#include <stdio.h>


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
//    run_test("x = 10 + 5; x == 2;");
    run_test("x = \"hello\";");
//    run_test("i32 x; x = 3; foo(x);");

    return 0;
}
