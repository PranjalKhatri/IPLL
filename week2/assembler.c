#include "assembler.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Usage();

int  main(int argc, char** argv) {
    if (argc != 3) {
        Usage();
        return 1;
    }
    int             error;
    SourceLine*     sourceLines;
    ObjectCodeLine* objectCodeLines;
    size_t          numLines;
    if ((error =
             LexAndInit(argv[1], &sourceLines, &objectCodeLines, &numLines)))
        return error;

    // For debugging
    {
        char dumpFile[512];
        snprintf(dumpFile, sizeof(dumpFile), "%s_lex_dump.txt", argv[1]);
        DumpLex(dumpFile, sourceLines, numLines);
        snprintf(dumpFile, sizeof(dumpFile), "%s_intermediate_dump.txt", argv[1]);
        DumpIntermediate(dumpFile, objectCodeLines, numLines);
    }
}

void Usage() { printf("USAGE: assembler <inputFile> <outputFile>"); }
