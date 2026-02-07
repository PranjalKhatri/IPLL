#include "assembler.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HashTable* SymbolTable;

// prints the sample usage of the program
void Usage();
int main(int argc, char** argv) {
    if (argc > 3 || argc < 2) {
        Usage();
        return 1;
    }
    SymbolTable = ht_create();
    int nameSz=100;
    char outFile[nameSz];
    if (argc == 2) {
        snprintf(outFile, nameSz, "%s.out", argv[1]);
    } else {
        snprintf(outFile, nameSz, "%s", argv[2]);
    }
    int error, retVal = 0;
    size_t numLines, programLength, firstInstruction;
    if ((error = GenerateIntermediate(argv[1], &numLines, &programLength,
                                      &firstInstruction, SymbolTable))) {
        retVal = error;
        goto CleanupAndExit;
    }
    printf("Pass 1 Done. Symbol Table and Intermediate outputFile generated\n");
    printf("-------SymbolTable------\n");
    ht_print(SymbolTable);
    if ((error = GenerateObject(argv[1], outFile, programLength,
                                firstInstruction, SymbolTable))) {
        goto CleanupAndExit;
        retVal = error;
    }
    printf("Pass 2 Done. outputFile %s Created\n", outFile);
CleanupAndExit:
    ht_destroy(SymbolTable);
    return retVal;
}

void Usage() { printf("USAGE: assembler <inputFile> [outputFile]"); }
