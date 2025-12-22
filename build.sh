#!/bin/bash

set -e

srcs="
    assembler.c
    code_gen.c
    darray.c
    hash_table.c
    lexical_scope.c
    main.c
    parser.c
    stack_frame.c
    strview.c
    time.c
    tokenizer.c
    types.c
"

if [ ! -d obj ]; then
    mkdir obj
fi

for i in $srcs; do
    gcc $i -c -o $i.obj
done
