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

    int error, retVal = 0;
    size_t numLines, programLength, firstInstruction;
    if ((error = GenerateIntermediate(argv[1], &numLines, &programLength,
                                      &firstInstruction, SymbolTable))) {
        retVal = error;
        goto CleanupAndExit;
    }
    // ht_print(SymbolTable);
    if ((error = GenerateObject(argv[1], argv[2], programLength,
                                firstInstruction, SymbolTable))) {
        goto CleanupAndExit;
        retVal = error;
    }
CleanupAndExit:
    ht_destroy(SymbolTable);
    return retVal;
}

void Usage() { printf("USAGE: assembler <inputFile> <outputFile>"); }
