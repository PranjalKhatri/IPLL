#include "assembler.h"

#include <stddef.h>
#include <stdio.h>

HashTable* SymbolTable;

// prints the sample usage of the program
void Usage();
int main(int argc, char** argv) {
    if (argc > 3 || argc < 2) {
        Usage();
        return 1;
    }
    SymbolTable = ht_create();
    int nameSz  = 100;
    char outFile[nameSz];
    if (argc == 2) {
        snprintf(outFile, nameSz, "%s.out", argv[1]);
    } else {
        snprintf(outFile, nameSz, "%s", argv[2]);
    }
    int retVal = 0;
    retVal = Assemble(argv[1], outFile, SymbolTable);
    ht_destroy(SymbolTable);
    return retVal;
}

void Usage() { printf("USAGE: assembler <inputFile> [outputFile]"); }
