// This project's headers
#include "assembler.h"
#include "code_gen.h"
#include "parser.h"
#include "time.h"

// Standard headers
#include <stdio.h>


typedef int(*two_in_one_out)(int, int);



static void run_test(char const *source_code) {
    printf("--- Parsing Code: \"%s\" ---\n", source_code);
    ast_node_t *ast = parser_parse(source_code);
    if (!ast) return;

    printf("--- Abstract Syntax Tree ---\n");
    parser_print_ast_node(ast, 0);

    code_gen(ast);

    two_in_one_out funcPtr = (two_in_one_out)g_assembler.binary;

    double start = get_time();
    int result = funcPtr(1, 2);
    double duration = get_time() - start;
    printf("%d %.3f\n", result, duration * 1e3);

    parser_free_ast(ast);
    printf("\n");
}

int main() {
//    run_test("{ u8 x; x = 3; u64 y; y = 7; }");

//    run_test("{ u8[] a; }");

    run_test(
        "{"
        "   u64 a; a = 1; u64 b; b = 1; u64 c;"
        "   while (a != 1134903170) {"
//        "       puts(\"hello\");"
        "       c = a + b;"
        "       a = b;"
        "       b = c;"
        "   }"
        "}");

//     run_test(
//         "{"
//         "   u64 a; a = 1;"
//         "   while (a != 1000000000) {"
//         //        "       puts(\"hello\");"
//         "       a = a + 111;"
//         "   }"
//         "}");

//     run_test(
//         "{"
//         "   u64 i; i = 0;"
//         "   while (i != 3) {"
//         "       puts(\"Hello\");"
//         "       i = i + 1;"
//         "   }"
//         "}");
//	  run_test("puts(\"hello\");");

    return 0;
}
