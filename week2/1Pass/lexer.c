#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "hash.h"

// Write End record in the fp file
void DumpEndRecord(FILE* fp, size_t FirstInstruction);
// Writes Header record in the fp file.
void DumpHeader(FILE* fp, const char* progName, size_t programLength,
                size_t FirstInstruction);
// Strips the location from the line so it becomes sourceline from intermediate
// Line
void StripLocation(char* line);

typedef struct PendingLabel {
    char* name;
    size_t loc;
    struct PendingLabel* next;
} PendingLabel;

static void add_pending(PendingLabel** head, const char* name, size_t loc) {
    PendingLabel* p = malloc(sizeof(PendingLabel));
    p->name         = strdup(name);
    p->loc          = loc;
    p->next         = *head;
    *head           = p;
}

void PatchSymbol(HashTable* symbolTable, const char* symbol, int address,
                 FILE* outfp) {
    Entry* e = ht_find(symbolTable, symbol);  // internal helper
    if (!e || !e->backrefs) return;

    long savedPos = ftell(outfp);

    BackRef* br   = e->backrefs;
    printf("Patching symbol %s\n", symbol);
    while (br) {
        long patchPos   = br->file_offset;

        int patchedAddr = address & 0x7FFF;
        if (br->indexed) {
            patchedAddr |= 0x8000;
        }

        printf("\tfileoffset %ld with %04X\n", br->file_offset,
               patchedAddr & 0xFFFF);
        /* seek to address field */
        fseek(outfp, patchPos, SEEK_SET);

        /* overwrite 4 hex digits */
        fprintf(outfp, "%04X", patchedAddr & 0xFFFF);

        br = br->next;
    }

    /* restore file position */
    fseek(outfp, savedPos, SEEK_SET);

    /* free backrefs */
    ht_clear_backrefs(symbolTable, symbol);
    // br = e->backrefs;
    // while (br) {
    //     BackRef* next = br->next;
    //     free(br);
    //     br = next;
    // }
    //
    // e->backrefs = NULL;
}

static void patch_pending(PendingLabel** head, HashTable* symtab, FILE* outfp) {
    PendingLabel* p = *head;
    while (p) {
        PatchSymbol(symtab, p->name, p->loc, outfp);
        PendingLabel* tmp = p;
        p                 = p->next;
        free(tmp->name);
        free(tmp);
    }
    *head = NULL;
}
int Assemble(const char* fname, const char* outname, HashTable* symbolTable) {
    FILE* fp    = fopen(fname, "r");
    FILE* outfp = fopen(outname, "wb+");

    if (!fp || !outfp) {
        printf("Unable to open input or output file\n");
        if (fp) fclose(fp);
        if (outfp) fclose(outfp);
        return 1;
    }

    int retValue = 0;

    const size_t buflen = 150;
    char buf[150];

    size_t lineNo                  = 0;
    size_t loc                     = 0;
    size_t locStart                = 0;
    size_t programLength           = 0;
    size_t firstInstructionAddress = 0;
    int isFirstMne                 = 1;

    PendingLabel* pending = NULL;

    /* TEXT record state */
    int    textLenBytes   = 0;
    size_t textStartAddr  = 0;
    char   textBuf[128];

    char progName[7] = {0};

    SourceLine     sl;
    ObjectCodeLine obcl;

    SourceLine_init(&sl);
    ObjectCodeLine_init(&obcl);

    /* reserve space for header */
    fseek(outfp, 20, SEEK_SET);

    while (fgets(buf, buflen, fp)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        else if (len == buflen - 1) {
            printf("%zu line too long\n", lineNo + 1);
            retValue = 2;
            goto exit;
        }

        SourceLine_init(&sl);
        ObjectCodeLine_init(&obcl);
        obcl.source = &sl;

        sl.lineNo     = ++lineNo;
        sl.sourceLine = strdup(buf);
        if (!sl.sourceLine) {
            retValue = 3;
            goto exit;
        }

        SplitInstruction(&sl);

        if (sl.isComment) {
            free(sl.sourceLine);
            continue;
        }

        Mnemonic mne = IsMnemonic(sl.mnemonic);

        /* ---------- START ---------- */
        if (isFirstMne) {
            if (mne != START) {
                printf("First mnemonic must be START\n");
                retValue = 5;
                goto exit;
            }

            if (!sl.label || strlen(sl.label) > PROG_NAME_MAX_LENGTH) {
                printf("Invalid program name\n");
                retValue = 6;
                goto exit;
            }

            strncpy(progName, sl.label, 6);
            progName[6] = '\0';

            loc      = strtol(sl.args[0], NULL, 16);
            locStart = loc;
            isFirstMne = 0;

            free(sl.sourceLine);
            continue;
        }

        /* ---------- END ---------- */
        if (mne == END) {
            if (textLenBytes > 0) {
                fprintf(outfp, "T%06zX%02X%s\n",
                        textStartAddr, textLenBytes, textBuf);
                patch_pending(&pending, symbolTable, outfp);
            }

            if (sl.args[0]) {
                int out;
                if (!ht_get(symbolTable, sl.args[0], &out)) {
                    printf("END label %s not found\n", sl.args[0]);
                    retValue = 7;
                    goto exit;
                }
                firstInstructionAddress = out;
            }
            break;
        }

        obcl.location = loc;

        /* ---------- label definition ---------- */
        if (sl.label) {
            int out;
            if (ht_get(symbolTable, sl.label, &out)) {
                printf("Duplicate symbol %s on line %zu\n",
                       sl.label, sl.lineNo);
                retValue = 6;
                goto exit;
            }

            ht_put(symbolTable, sl.label, loc);
            add_pending(&pending, sl.label, loc);
        }

        /* ---------- OBJECT CODE ---------- */

        char objBuf[64];

        /* force flush for RESB / RESW */
        if (textLenBytes > 0 && (mne == RESB || mne == RESW)) {
            fprintf(outfp, "T%06zX%02X%s\n",
                    textStartAddr, textLenBytes, textBuf);
            patch_pending(&pending, symbolTable, outfp);
            textLenBytes = 0;
        }

        if (textLenBytes == 0) {
            textStartAddr = obcl.location;
            textBuf[0]    = '\0';
        }

        /* --------- FIX STARTS HERE --------- */

        int instBytes = AddressToAdd(mne, &obcl);

        /* decide TEXT record BEFORE resolving */
        if (textLenBytes > 0 && textLenBytes + instBytes > 30) {
            fprintf(outfp, "T%06zX%02X%s\n",
                    textStartAddr, textLenBytes, textBuf);
            patch_pending(&pending, symbolTable, outfp);

            textLenBytes  = 0;
            textStartAddr = obcl.location;
            textBuf[0]    = '\0';
        }

        /* temporarily emit prefix so ResolveObjectCode sees correct offset */
        long savePos = ftell(outfp);
        fprintf(outfp, "T%06zX%02X%s",
                textStartAddr, textLenBytes, textBuf);

        int bytesWritten = ResolveObjectCode(
            &obcl, symbolTable, outfp, objBuf, sizeof(objBuf));

        fseek(outfp, savePos, SEEK_SET);

        /* --------- FIX ENDS HERE --------- */

        if (bytesWritten < 0) {
            retValue = bytesWritten;
            goto exit;
        }

        if (bytesWritten > 0) {
            strcat(textBuf, objBuf);
            textLenBytes += bytesWritten;
        }

        loc          += instBytes;
        programLength = loc - locStart;

        free(sl.sourceLine);
    }

    int unresolved = ht_check_unresolved(symbolTable);
    if (unresolved > 0) {
        fprintf(stderr, "Assembly failed: %d unresolved symbol(s)\n",
                unresolved);
        retValue = 8;
        goto exit;
    }

    fseek(outfp, 0, SEEK_SET);
    DumpHeader(outfp, progName, programLength, locStart);

    fseek(outfp, 0, SEEK_END);
    DumpEndRecord(outfp, firstInstructionAddress);

exit:
    fclose(fp);
    fclose(outfp);
    return retValue;
}

void DumpHeader(FILE* fp, const char* progName, size_t programLength,
                size_t FirstInstruction) {
    // Dump in format given in book
    fprintf(fp, "H%-6.6s%06zX%06zX\n", progName ? progName : "",
            FirstInstruction, programLength);
}
void DumpEndRecord(FILE* fp, size_t FirstInstruction) {
    // Dump in format given in book
    fprintf(fp, "E%06zx\n", FirstInstruction);
}

void SplitInstruction(SourceLine* const sl) {
    char* p;
    char* tok;
    int argIndex = 0;

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

    // if first word is not mnemonic than its a label
    if (IsMnemonic(tok)) {
        sl->mnemonic = tok;
    } else {
        sl->label = tok;

        tok       = strtok(NULL, " \t,");
        if (tok == NULL) {
            printf("Label only line not allowed. line no. %zu\n", sl->lineNo);
            return;
        }
        sl->mnemonic = tok;
    }

    while ((tok = strtok(NULL, " \t,")) != NULL && argIndex < 3) {
        sl->args[argIndex++] = tok;
    }
}
void StripLocation(char* line) {
    char* p = line;

    // Skip leading whitespace
    while (*p && isspace((unsigned char)*p)) p++;

    // Skip location token (hex number)
    while (*p && !isspace((unsigned char)*p)) p++;

    // Skip whitespace after location
    while (*p && isspace((unsigned char)*p)) p++;

    // Shift remainder to the front
    memmove(line, p, strlen(p) + 1);
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
        fprintf(fp, "Mnemonic: %s\n", sl.mnemonic ? sl.mnemonic : "(null)");

        for (int j = 0; j < 3; j++) {
            fprintf(fp, "Arg%d: %s\n", j + 1,
                    sl.args[j] ? sl.args[j] : "(null)");
        }

        fprintf(fp, "\n");
    }

    fclose(fp);
}
void DumpObject(FILE* fp, ObjectCodeLine* ol) {
    if (ol == NULL || fp == NULL) return;
#define LOC_WIDTH   6
#define LABEL_WIDTH 12
#define INST_WIDTH  8
    if (ol->source->isComment) {
        fprintf(fp, "      \t%s\n", ol->source->sourceLine);
        return;
    }

    // location
    fprintf(fp, "%-*zx ", LOC_WIDTH, ol->location);

    // label (or empty)
    fprintf(fp, "%-*s ", LABEL_WIDTH,
            ol->source->label ? ol->source->label : "");

    // instruction
    fprintf(fp, "%-*s ", INST_WIDTH, ol->source->mnemonic);

    // operands
    for (int j = 0; j < 3 && ol->source->args[j] != NULL; j++) {
        fprintf(fp, "%s", ol->source->args[j]);
        if (j < 2 && ol->source->args[j + 1] != NULL) fprintf(fp, ",");
    }

    fprintf(fp, "\n");
}
void DumpIntermediate(const char* const fname, ObjectCodeLine* objectCodeLines,
                      size_t numLines) {
    if (fname == NULL || objectCodeLines == NULL) return;

    FILE* fp = fopen(fname, "w");
    if (!fp) {
        perror("Failed to open intermediate dump file");
        return;
    }

    for (size_t i = 0; i < numLines; i++) {
        DumpObject(fp, &objectCodeLines[i]);
    }

    fclose(fp);
}
