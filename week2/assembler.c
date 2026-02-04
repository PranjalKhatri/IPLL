#include "assembler.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HashTable* SymbolTable;

void Usage();
int main(int argc, char** argv) {
    if (argc != 3) {
        Usage();
        return 1;
    }
    SymbolTable = ht_create();

    int error;
    size_t numLines, programLength;
    if ((error = GenerateIntermediate(argv[1], &numLines, &programLength,SymbolTable)))
        return error;
    ht_print(SymbolTable);
    ht_destroy(SymbolTable);
    return 0;
}

void Usage() { printf("USAGE: assembler <inputFile> <outputFile>"); }
