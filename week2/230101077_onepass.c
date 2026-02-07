#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// number of bytes needed to representt the number n
int bytes_needed_unsigned(unsigned long long n);

// helper: convert hex digit to value, assumes valid hex char
int hex_val(char c);
// parse constants, like decimal, X'',C''(sets is char and returns number of
// char)
long parse_constant(const char* s, int* isChar);
// prime number helps distribution
#define TABLE_SIZE 101

typedef struct BackRef {
    long file_offset;  // where to patch
    int indexed;  // Whether the original instruction used ,X addressing mode
    struct BackRef* next;
} BackRef;

// Uses chaining
typedef struct Entry {
    char* key;           // label name
    int value;           // label address
    int defined;         // 0 = undefined, 1 = defined
    BackRef* backrefs;   // backreference chain
    struct Entry* next;  // hash collision chain
} Entry;

typedef struct HashTable {
    Entry* buckets[TABLE_SIZE];
} HashTable;

// core API
HashTable* ht_create(void);
void ht_put(HashTable* ht, const char* key, int value);
int ht_get(HashTable* ht, const char* key, int* out);
void ht_remove(HashTable* ht, const char* key);
void ht_destroy(HashTable* ht);
void ht_print(HashTable* ht);

// backreference API
void ht_add_backref(HashTable* ht, const char* key, long file_offset,
                    int useIndexed);
BackRef* ht_get_backrefs(HashTable* ht, const char* key);
void ht_clear_backrefs(HashTable* ht, const char* key);

int ht_check_unresolved(HashTable* ht);
Entry* ht_find(HashTable* ht, const char* key);
Entry* ht_find_or_create(HashTable* ht, const char* key);
typedef struct {
    unsigned int instruction;
} InstructionHex;

// struct representing input source line
typedef struct {
    size_t lineNo;
    char* sourceLine;
    char* label;     // optional
    char* mnemonic;  // operation name
    // different instructions can have different operands
    char* args[3];
    int isComment;
} SourceLine;

// struct representing object code line
typedef struct {
    size_t location;
    InstructionHex instruction;
    SourceLine* source;
} ObjectCodeLine;
// from the header section in book
#define PROG_NAME_MAX_LENGTH 6
// mnemonics and directives
typedef enum {
    // assembler directivers sort of but not mnemonic
    // easier to handle this way
    MNEMONIC_START,
    START,
    END,
    BYTE,
    WORD,
    RESB,
    RESW,
    DIRECTIVE_END,

    LDA = DIRECTIVE_END,
    LDX,
    LDL,
    STA,
    STX,
    STL,
    LDCH,
    STCH,
    ADD,
    SUB,
    MUL,
    DIV,
    COMP,
    J,
    JLT,
    JEQ,
    JGT,
    JSUB,
    RSUB,
    TIX,
    TD,
    RD,
    WD,
    MNEMONIC_COUNT
} Mnemonic;

// Initializes instruction hex
void InstructionHex_init(InstructionHex* ih);
// Initializes source line
void SourceLine_init(SourceLine* sl);
// Initializes object code line
void ObjectCodeLine_init(ObjectCodeLine* ocl);
// Check if its amnemonic or not and returns its mnemonic code
Mnemonic IsMnemonic(const char* const str);
// checks if its an assembler directive
int IsDirective(const char* const str);
// Gives how much bytes to add to the location
int AddressToAdd(Mnemonic mne, ObjectCodeLine* obcl);

int Assemble(const char* fname, const char* outname, HashTable* symbolTable);

// Splits a source line into various fields of its struct
void SplitInstruction(SourceLine* const sl);
// Resolves objectcodeline struct into object code in hex and writes in buf
int ResolveObjectCode(ObjectCodeLine* obcl, HashTable* symbolTable, FILE* outfp,
                      char* buf, size_t buflen);
// Dumps the object code in file fp
void DumpObject(FILE* fp, ObjectCodeLine* ol);

// struct representing mnemonic, its opcode and its string name
typedef struct {
    Mnemonic mnemonic;
    int opcode;
    const char* name;  // string form LDA, ADD..
} MnemonicInfo;
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
    retVal     = Assemble(argv[1], outFile, SymbolTable);
    ht_destroy(SymbolTable);
    return retVal;
}

void Usage() { printf("USAGE: assembler <inputFile> [outputFile]"); }

static unsigned hash(const char* s) {
    unsigned h = 0;
    while (*s) h = h * 31 + (unsigned char)*s++;
    return h % TABLE_SIZE;
}

// create hash table
HashTable* ht_create(void) {
    HashTable* ht = malloc(sizeof(HashTable));
    if (!ht) return NULL;

    for (int i = 0; i < TABLE_SIZE; i++) ht->buckets[i] = NULL;

    return ht;
}

Entry* ht_find(HashTable* ht, const char* key) {
    unsigned idx = hash(key);
    Entry* e     = ht->buckets[idx];

    while (e) {
        if (strcmp(e->key, key) == 0) return e;
        e = e->next;
    }
    return NULL;
}

Entry* ht_find_or_create(HashTable* ht, const char* key) {
    unsigned idx = hash(key);
    Entry* e     = ht->buckets[idx];

    while (e) {
        if (strcmp(e->key, key) == 0) return e;
        e = e->next;
    }

    // create new entry
    e                = malloc(sizeof(Entry));
    e->key           = strdup(key);
    e->value         = 0;
    e->defined       = 0;
    e->backrefs      = NULL;

    e->next          = ht->buckets[idx];
    ht->buckets[idx] = e;

    return e;
}
// insert or update
void ht_put(HashTable* ht, const char* key, int value) {
    Entry* e   = ht_find_or_create(ht, key);
    e->value   = value;
    e->defined = 1;
}

// lookup
int ht_get(HashTable* ht, const char* key, int* out) {
    Entry* e = ht_find(ht, key);
    if (!e || !e->defined) return 0;

    *out = e->value;
    return 1;
}

void ht_add_backref(HashTable* ht, const char* key, long file_offset,
                    int useIndexed) {
    Entry* e         = ht_find_or_create(ht, key);

    BackRef* ref     = malloc(sizeof(BackRef));
    ref->indexed     = useIndexed;
    ref->file_offset = file_offset;
    ref->next        = e->backrefs;
    e->backrefs      = ref;
}

BackRef* ht_get_backrefs(HashTable* ht, const char* key) {
    Entry* e = ht_find(ht, key);
    return e ? e->backrefs : NULL;
}

void ht_clear_backrefs(HashTable* ht, const char* key) {
    Entry* e = ht_find(ht, key);
    if (!e) return;

    BackRef* ref = e->backrefs;
    while (ref) {
        BackRef* tmp = ref;
        ref          = ref->next;
        free(tmp);
    }
    e->backrefs = NULL;
}
// delete key
void ht_remove(HashTable* ht, const char* key) {
    unsigned idx = hash(key);
    Entry* prev  = NULL;
    Entry* e     = ht->buckets[idx];

    while (e) {
        if (strcmp(e->key, key) == 0) {
            if (prev)
                prev->next = e->next;
            else
                ht->buckets[idx] = e->next;

            ht_clear_backrefs(ht, key);
            free(e->key);
            free(e);
            return;
        }
        prev = e;
        e    = e->next;
    }
}

// free table
void ht_destroy(HashTable* ht) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry* e = ht->buckets[i];
        while (e) {
            Entry* tmp   = e;
            e            = e->next;

            BackRef* ref = tmp->backrefs;
            while (ref) {
                BackRef* r = ref;
                ref        = ref->next;
                free(r);
            }

            free(tmp->key);
            free(tmp);
        }
    }
    free(ht);
}
// debugging
void ht_print(HashTable* ht) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry* e = ht->buckets[i];
        if (!e) continue;

        printf("[%d]:\n", i);
        while (e) {
            printf("  %s = %d (%s)\n", e->key, e->value,
                   e->defined ? "defined" : "undef");

            BackRef* r = e->backrefs;
            while (r) {
                printf("    backref @ %ld\n", r->file_offset);
                r = r->next;
            }
            e = e->next;
        }
    }
}

int ht_check_unresolved(HashTable* ht) {
    int errors = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry* e = ht->buckets[i];
        while (e) {
            if (!e->defined && e->backrefs) {
                fprintf(stderr, "Error: Undefined symbol '%s'\n", e->key);

                BackRef* r = e->backrefs;
                while (r) {
                    fprintf(stderr, "  referenced at file offset %ld\n",
                            r->file_offset);
                    r = r->next;
                }

                errors++;
            }
            e = e->next;
        }
    }

    return errors;  // number of unresolved symbols
}
void InstructionHex_init(InstructionHex* ih) {
    if (ih == NULL) return;
    ih->instruction = 0;
}
void SourceLine_init(SourceLine* sl) {
    if (sl == NULL) return;

    sl->lineNo     = 0;
    sl->label      = NULL;
    sl->mnemonic   = NULL;
    sl->sourceLine = NULL;
    sl->isComment  = 0;
    sl->args[0] = sl->args[1] = sl->args[2] = NULL;
}
void ObjectCodeLine_init(ObjectCodeLine* ocl) {
    if (ocl == NULL) return;

    ocl->location = 0;
    InstructionHex_init(&ocl->instruction);
    ocl->source = NULL;
}

// Write End record in the fp file
void DumpEndRecord(FILE* fp, size_t FirstInstruction);
// Writes Header record in the fp file.
void DumpHeader(FILE* fp, const char* progName, size_t programLength,
                size_t FirstInstruction);

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

    int retValue        = 0;

    const size_t buflen = 150;
    char buf[150];

    size_t lineNo                  = 0;
    size_t loc                     = 0;
    size_t locStart                = 0;
    size_t programLength           = 0;
    size_t firstInstructionAddress = 0;
    int isFirstMne                 = 1;

    PendingLabel* pending          = NULL;

    /* TEXT record state */
    int textLenBytes               = 0;
    size_t textStartAddr           = 0;
    char textBuf[128];

    char progName[7] = {0};

    SourceLine sl;
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
        obcl.source   = &sl;

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

            loc         = strtol(sl.args[0], NULL, 16);
            locStart    = loc;
            isFirstMne  = 0;

            free(sl.sourceLine);
            continue;
        }

        /* ---------- END ---------- */
        if (mne == END) {
            if (textLenBytes > 0) {
                fprintf(outfp, "T%06zX%02X%s\n", textStartAddr, textLenBytes,
                        textBuf);
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
                printf("Duplicate symbol %s on line %zu\n", sl.label,
                       sl.lineNo);
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
            fprintf(outfp, "T%06zX%02X%s\n", textStartAddr, textLenBytes,
                    textBuf);
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
            fprintf(outfp, "T%06zX%02X%s\n", textStartAddr, textLenBytes,
                    textBuf);
            patch_pending(&pending, symbolTable, outfp);

            textLenBytes  = 0;
            textStartAddr = obcl.location;
            textBuf[0]    = '\0';
        }

        /* temporarily emit prefix so ResolveObjectCode sees correct offset */
        long savePos = ftell(outfp);
        fprintf(outfp, "T%06zX%02X%s", textStartAddr, textLenBytes, textBuf);

        int bytesWritten = ResolveObjectCode(&obcl, symbolTable, outfp, objBuf,
                                             sizeof(objBuf));

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

        loc           += instBytes;
        programLength  = loc - locStart;

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

// OPTAB is one of the two data structures used in assembler
// array is used since it has less elements and hash table constant
// factor takes more time as compared to linear search in such a small data
static const MnemonicInfo OPTAB[] = {
    // assembler directives
    {MNEMONIC_START, 0, "DUMMY~~~"},
    {START, 0, "START"},
    {END, 0, "END"},
    {BYTE, 0, "BYTE"},
    {WORD, 0, "WORD"},
    {RESB, 0, "RESB"},
    {RESW, 0, "RESW"},

    // instructions
    {LDA, 0x00, "LDA"},
    {LDX, 0x04, "LDX"},
    {LDL, 0x08, "LDL"},
    {STA, 0x0c, "STA"},
    {STX, 0x10, "STX"},
    {STL, 0x14, "STL"},
    {LDCH, 0x50, "LDCH"},
    {STCH, 0x54, "STCH"},

    {ADD, 0x18, "ADD"},
    {SUB, 0x1c, "SUB"},
    {MUL, 0x20, "MUL"},
    {DIV, 0x24, "DIV"},
    {COMP, 0x28, "COMP"},

    {J, 0x3c, "J"},
    {JLT, 0x38, "JLT"},
    {JEQ, 0x30, "JEQ"},
    {JGT, 0x34, "JGT"},
    {JSUB, 0x48, "JSUB"},
    {RSUB, 0x4c, "RSUB"},

    {TIX, 0x2c, "TIX"},
    {TD, 0xe0, "TD"},
    {RD, 0xd8, "RD"},
    {WD, 0xdc, "WD"},
};
int IsDirective(const char* const str) {
    size_t i;

    if (str == NULL) return 0;

    for (i = 0; i < DIRECTIVE_END; ++i) {
        if (strcmp(str, OPTAB[i].name) == 0) return 1;
    }
    return 0;
}
Mnemonic IsMnemonic(const char* const str) {
    size_t i;

    if (str == NULL) return 0;

    for (i = MNEMONIC_START + 1; i < MNEMONIC_COUNT; ++i) {
        if (strcmp(str, OPTAB[i].name) == 0) return i;
    }
    return 0;
}

// Assembler directives have different effect on location as compared to
// mnemonics This function resolves them. Start doesnt add anything. END adds 3,
// byte adds number of bytes needed for that argument resw reserves
// wordsize*numwords
int DirAddressToAdd(Mnemonic mne, int arg0, int isChar) {
    switch (mne) {
        case START:
            return 0;
        case END:
            return 3;
        case BYTE:
            return isChar ? arg0 : bytes_needed_unsigned(arg0);
        case RESB:
            return arg0;
        case WORD:
            return 3;
        case RESW:
            return arg0 * 3;
        default:
            assert(0 && "DirAddressToAdd unknown mnemonic encountered");
    }
    return 0;
}
int AddressToAdd(Mnemonic mne, ObjectCodeLine* obcl) {
    assert(obcl != NULL && obcl->source != NULL &&
           "obcl or its  source is null in AddressToAdd!");
    int retAddr       = 0;
    int arg0Converted = 0;
    int isChar        = 0;
    if (obcl->source->args[0])
        arg0Converted = parse_constant(obcl->source->args[0], &isChar);
    if (mne < DIRECTIVE_END) {
        retAddr = DirAddressToAdd(mne, arg0Converted, isChar);
    } else
        retAddr = 3;  // for all operands
    return retAddr;
}

// < 0 -> error
// number of chars written
int ResolveObjectCode(ObjectCodeLine* obcl, HashTable* symbolTable, FILE* outfp,
                      char* buf, size_t buflen) {
    Mnemonic mne = IsMnemonic(obcl->source->mnemonic);
    if (!mne) return -1;

    /* No object code */
    if (mne == RESB || mne == RESW) return 0;

    /* WORD */
    if (mne == WORD) {
        int isConst = 0;
        int num     = parse_constant(obcl->source->args[0], &isConst);

        snprintf(buf, buflen, "%06X", num & 0xFFFFFF);
        return 3;
    }

    /* BYTE */
    if (mne == BYTE) {
        int isChar = 0;
        int val    = parse_constant(obcl->source->args[0], &isChar);

        /* BYTE C'...' */
        if (isChar) {
            const char* s = obcl->source->args[0] + 2; /* skip C' */
            int count     = 0;

            while (*s && *s != '\'') {
                snprintf(buf + count * 2, buflen - count * 2, "%02X",
                         (unsigned char)*s);
                count++;
                s++;
            }
            return count;
        }

        /* BYTE X'...' or numeric */
        int byte_req = bytes_needed_unsigned(val);
        snprintf(buf, buflen, "%0*X", byte_req * 2, val);
        return byte_req;
    }

    /* -------- instructions -------- */

    int address    = 0;
    int useIndexed = 0;
    int needsPatch = 0;

    /* operand label */
    if (obcl->source->args[0] != NULL) {
        if (!ht_get(symbolTable, obcl->source->args[0], &address)) {
            /* forward reference */
            address    = 0;
            needsPatch = 1;
        }
    }

    /* indexed addressing */
    if (obcl->source->args[1] != NULL) {
        if (tolower(obcl->source->args[1][0]) == 'x') {
            useIndexed = 1;
        }
    }

    int opcode = OPTAB[mne].opcode;

    if (useIndexed) address |= 0x8000;

    /* opcode: 1 byte, address: 2 bytes */
    snprintf(buf, buflen, "%02X%04X", opcode & 0xFF, address & 0xFFFF);

    /*
     * Register backreference if needed.
     * Caller has already written TEXT prefix,
     * so ftell(outfp) == start of this object code.
     */
    if (needsPatch) {
        long instrStart  = ftell(outfp);
        long patchOffset = instrStart + 2; /* skip opcode (2 hex chars) */

        ht_add_backref(symbolTable, obcl->source->args[0], patchOffset,
                       useIndexed);
    }

    return 3;
}

int bytes_needed_unsigned(unsigned long long n) {
    int bits = 0;

    if (n == 0) return 1; /* zero still needs 1 byte */

    while (n) {
        bits++;
        n >>= 1;
    }

    return (bits + 7) / 8;
}

// helper: convert hex digit to value, assumes valid hex char
int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1; /* invalid */
}

long parse_constant(const char* s, int* isChar) {
    long result = 0;
    int i;
    *isChar = 0;
    if (s == 0 || *s == '\0') return 0;

    //  Hex constant X'F1'
    if ((s[0] == 'X' || s[0] == 'x') && s[1] == '\'') {
        i = 2;
        while (s[i] && s[i] != '\'') {
            result = result * 16 + hex_val(s[i]);
            i++;
        }
        return result;
    }

    //  Char constant C'EOF
    if ((s[0] == 'C' || s[0] == 'c') && s[1] == '\'') {
        i = 2;
        while (s[i] && s[i] != '\'') i++;
        *isChar = 1;
        return i - 2;  // number of characters
    }

    //  Decimal number
    i = 0;
    while (s[i]) {
        if (!isdigit((unsigned char)s[i])) break;
        result = result * 10 + (s[i] - '0');
        i++;
    }

    return result;
}
