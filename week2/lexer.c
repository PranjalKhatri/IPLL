#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"

int LexAndInit(const char* const fname, SourceLine** sourceLines,
               ObjectCodeLine** objectCodeLines, size_t* capacity) {
    FILE* fp = fopen(fname, "r");
    if (fp == NULL) {
        printf("Unable to open file %s\n", fname);
        return 1;
    }

    int          retValue = 0;
    const size_t buflen   = 150;
    char         buf[150];
    size_t       lineNo     = 0;
    size_t       index      = 0;
    size_t       loc        = 0;
    int          isFirstMne = 1;

    /* initial capacity */
    *capacity               = 8;
    *sourceLines            = malloc((*capacity) * sizeof(SourceLine));
    *objectCodeLines        = malloc((*capacity) * sizeof(ObjectCodeLine));
    if (*sourceLines == NULL || *objectCodeLines == NULL) {
        fclose(fp);
        return 3;
    }

    while (fgets(buf, buflen, fp)) {
        size_t len = strlen(buf);

        if (len > 0 && buf[len - 1] == '\n') {
            // end with 0 instead of new line
            buf[len - 1] = '\0';
        } else {
            if (len == buflen - 1) {
                printf("%zu line length exceeds maximum supported line size\n",
                       lineNo + 1);
                retValue = 2;
                goto exit;
            }
        }

        if (index >= *capacity) {
            size_t      newCap = (*capacity) * 2;
            SourceLine* tmp =
                realloc(*sourceLines, newCap * sizeof(SourceLine));
            ObjectCodeLine* tmpo =
                realloc(*objectCodeLines, newCap * sizeof(ObjectCodeLine));
            if (tmp == NULL || tmpo == NULL) {
                retValue = 3;
                goto exit;
            }
            *sourceLines     = tmp;
            *objectCodeLines = tmpo;
            *capacity        = newCap;
        }

        // init source line
        SourceLine* sl = &(*sourceLines)[index];
        SourceLine_init(sl);

        // init objectCode line
        ObjectCodeLine* ocl = &(*objectCodeLines)[index];
        ObjectCodeLine_init(ocl);
        ocl->source    = sl;

        sl->lineNo     = ++lineNo;
        // split later
        sl->sourceLine = malloc(strlen(buf) + 1);
        if (sl->sourceLine == NULL) {
            retValue = 3;
            goto exit;
        }
        strcpy(sl->sourceLine, buf);
        SplitInstruction(sl);
        if (!sl->isComment) {
            Mnemonic mne = IsMnemonic(sl->instruction);
            if (isFirstMne) {
                if (mne != START) {
                    printf("First Mnemonic should be START");
                    exit(5);
                }
                isFirstMne = 0;
                loc        = strtol(sl->args[0], NULL, 16);
            }
            ocl->location  = loc;
            loc           += AddressToAdd(mne, ocl);
        }

        index++;
    }

    *capacity = index;
exit:
    fclose(fp);
    return retValue;
}

void SplitInstruction(SourceLine* const sl) {
    char* p;
    char* tok;
    int   argIndex = 0;

    if (sl == NULL || sl->sourceLine == NULL) return;

    // Skip leading whitespace
    p = sl->sourceLine;
    while (*p && isspace((unsigned char)*p)) p++;

    // Comment line: preserve entire source line
    if (*p == '.') {
        sl->isComment = 1;
        return;
    }

    tok = strtok(sl->sourceLine, " \t,");
    if (tok == NULL) return;

    if (IsMnemonic(tok)) {
        sl->instruction = tok;
    } else {
        sl->label = tok;

        tok       = strtok(NULL, " \t,");
        if (tok == NULL) {
            printf("Label only line not allowed. line no. %zu\n", sl->lineNo);
            return;
        }
        sl->instruction = tok;
    }

    while ((tok = strtok(NULL, " \t,")) != NULL && argIndex < 3) {
        sl->args[argIndex++] = tok;
    }
}

void SplitInstructions(SourceLine* const sourceLines, size_t numLines) {
    size_t i;

    for (i = 0; i < numLines; ++i) {
        SourceLine* sl = &sourceLines[i];
        SplitInstruction(sl);
    }
}

void DumpLex(const char* const fname, SourceLine* sourceLines,
             size_t numLines) {
    if (fname == NULL || sourceLines == NULL) return;

    FILE* fp = fopen(fname, "w");
    if (!fp) {
        perror("Failed to open dump file");
        return;
    }

    for (size_t i = 0; i < numLines; i++) {
        SourceLine sl = sourceLines[i];

        fprintf(fp, "Line %zu\n", sl.lineNo);
        fprintf(fp, "IsComment: %d\n", sl.isComment);

        fprintf(fp, "Label: %s\n", sl.label ? sl.label : "(null)");
        fprintf(fp, "Mnemonic: %s\n",
                sl.instruction ? sl.instruction : "(null)");

        for (int j = 0; j < 3; j++) {
            fprintf(fp, "Arg%d: %s\n", j + 1,
                    sl.args[j] ? sl.args[j] : "(null)");
        }

        fprintf(fp, "\n");
    }

    fclose(fp);
}
void DumpIntermediate(const char* const fname, ObjectCodeLine* objectCodeLines,
                      size_t numLines) {
    if (fname == NULL || objectCodeLines == NULL) return;

    FILE* fp = fopen(fname, "w");
    if (!fp) {
        perror("Failed to open intermediate dump file");
        return;
    }

#define LOC_WIDTH   6
#define LABEL_WIDTH 12
#define INST_WIDTH  8

    for (size_t i = 0; i < numLines; i++) {
        ObjectCodeLine ol = objectCodeLines[i];

        if (ol.source->isComment) {
            fprintf(fp, "      \t%s\n", ol.source->sourceLine);
            continue;
        }

        // location
        fprintf(fp, "%-*zx ", LOC_WIDTH, ol.location);

        // label (or empty)
        fprintf(fp, "%-*s ", LABEL_WIDTH,
                ol.source->label ? ol.source->label : "");

        // instruction
        fprintf(fp, "%-*s ", INST_WIDTH, ol.source->instruction);

        // operands
        for (int j = 0; j < 3 && ol.source->args[j] != NULL; j++) {
            fprintf(fp, "%s", ol.source->args[j]);
            if (j < 2 && ol.source->args[j + 1] != NULL) fprintf(fp, ",");
        }

        fprintf(fp, "\n");
    }

    fclose(fp);
}
