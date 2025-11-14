#include "parser.h"
#include <stdio.h>
#include <stdint.h>


void *VirtualAlloc(void *address, size_t size, unsigned allocationType, unsigned protect);

enum {
    MEM_COMMIT = 0x1000,
    MEM_RESERVE = 0x2000,
    PAGE_EXECUTE_READWRITE = 0x40
};

typedef int(*two_in_one_out)(int, int);
typedef int(*puts_ptr)(const char* str);

static void foo(char const *s) {
    puts(s);
}

static void code_gen(void) {
//    unsigned char code[] = { 0x8d, 0x04, 0x11, 0xc3 };
    
    char const *msg = "hello";
    uint64_t dodgy = (uint64_t)msg;

    uint8_t *code_start = VirtualAlloc(NULL, 0x1000,
                                 MEM_COMMIT | MEM_RESERVE,
                                 PAGE_EXECUTE_READWRITE);
    uint8_t *c = code_start;

    // Allocate 32-byte shadow space
    // Emit sub rsp, 0x20
    c[0] = 0x48;
    c[1] = 0x83;
    c[2] = 0xEC;
    c[3] = 0x20;
    c += 4;

    // Emit mov rdi, msg
    c[0] = 0x48;
    c[1] = 0xB9;
    c += 2;
    memcpy(c, &dodgy, 8);
    c += 8;
    
    // Emit mov rax, &puts
    uint64_t puts_addr = (uint64_t)puts;
    c[0] = 0x48;
    c[1] = 0xb8;
    c += 2;
    memcpy(c, &puts_addr, 8);
    c += 8;

    // Emit call rax
    c[0] = 0xff;
    c[1] = 0xd0;
    c += 2;

    // Deallocate the shadow space
    // Emit add rsp, 0x20
    c[0] = 0x48;
    c[1] = 0x83;
    c[2] = 0xC4;
    c[3] = 0x20;
    c += 4;

    // Emit ret
    c[0] = 0xc3;

    two_in_one_out funcPtr = (two_in_one_out)code_start;
    int result = funcPtr(1, 2);
    printf("%d\n", result);
}

static void run_test(char const *source_code) {
    printf("--- Parsing Code: \"%s\" ---\n", source_code);
    AstNode *ast = parser_parse(source_code);

    if (!ast) return;

    printf("--- Abstract Syntax Tree ---\n");
    parser_print_ast_node(ast, 0);
    parser_free_ast(ast);
    printf("\n");

    code_gen();
}

int main() {
    code_gen();
//    run_test("x = 10 + 5; x == 2;");
//    run_test("puts(\"hello\");");
//    run_test("i32 x; x = 3; foo(x);");

    return 0;
}
