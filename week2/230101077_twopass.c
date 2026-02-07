
#include <ctype.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// prime number helps distribution
#define TABLE_SIZE 101

// Uses chaining
typedef struct Entry {
  char *key;
  int value;
  struct Entry *next;
} Entry;

typedef struct HashTable {
  Entry *buckets[TABLE_SIZE];
} HashTable;

// create hash table
HashTable *ht_create(void);
// insert or update
void ht_put(HashTable *ht, const char *key, int value);
// lookup
int ht_get(HashTable *ht, const char *key, int *out);
// delete key
void ht_remove(HashTable *ht, const char *key);
// free table
void ht_destroy(HashTable *ht);
// used for debugging
void ht_print(HashTable *ht);
typedef struct {
  unsigned int instruction;
} InstructionHex;

// struct representing input source line
typedef struct {
  size_t lineNo;
  char *sourceLine;
  char *label;    // optional
  char *mnemonic; // operation name
  // different instructions can have different operands
  char *args[3];
  int isComment;
} SourceLine;

// struct representing object code line
typedef struct {
  size_t location;
  InstructionHex instruction;
  SourceLine *source;
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
void InstructionHex_init(InstructionHex *ih);
// Initializes source line
void SourceLine_init(SourceLine *sl);
// Initializes object code line
void ObjectCodeLine_init(ObjectCodeLine *ocl);
// Check if its amnemonic or not and returns its mnemonic code
Mnemonic IsMnemonic(const char *const str);
// checks if its an assembler directive
int IsDirective(const char *const str);
// Gives how much bytes to add to the location
int AddressToAdd(Mnemonic mne, ObjectCodeLine *obcl);
// Generate Intermediate file from input in phase 1
int GenerateIntermediate(const char *const fname, size_t *capacity,
                         size_t *programLength, size_t *firstInstructionAddress,
                         HashTable *symbolTable);
// Generate Final object file in phase 2
int GenerateObject(const char *fname, const char *out_fname,
                   size_t programLength, size_t FirstInstruction,
                   HashTable *symbolTable);

void SplitInstructions(SourceLine *const sourceLines, size_t numLines);
// Splits a source line into various fields of its struct
void SplitInstruction(SourceLine *const sl);
// Dumps the source lines for debugging in file fname
void DumpLex(const char *const fname, SourceLine *sourceLines, size_t numLines);
// Resolves objectcodeline struct into object code in hex and writes in buf
int ResolveObjectCode(ObjectCodeLine *obcl, HashTable *symbolTable, char *buf,
                      size_t buflen);
// Dumps the object code in file fp
void DumpObject(FILE *fp, ObjectCodeLine *ol);
// Dumps Intermediate doe in file fname
void DumpIntermediate(const char *const fname, ObjectCodeLine *objectCodeLines,
                      size_t numLines);

// struct representing mnemonic, its opcode and its string name
typedef struct {
  Mnemonic mnemonic;
  int opcode;
  const char *name; // string form LDA, ADD..
} MnemonicInfo;
// number of bytes needed to representt the number n
int bytes_needed_unsigned(unsigned long long n);

// helper: convert hex digit to value, assumes valid hex char
int hex_val(char c);
// parse constants, like decimal, X'',C''(sets is char and returns number of
// char)
long parse_constant(const char *s, int *isChar);

HashTable *SymbolTable;

// prints the sample usage of the program
void Usage();
int main(int argc, char **argv) {
  if (argc > 3 || argc < 2) {
    Usage();
    return 1;
  }
  SymbolTable = ht_create();
  int nameSz = 100;
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
  if ((error = GenerateObject(argv[1], outFile, programLength, firstInstruction,
                              SymbolTable))) {
    goto CleanupAndExit;
    retVal = error;
  }
  printf("Pass 2 Done. outputFile %s Created\n", outFile);
CleanupAndExit:
  ht_destroy(SymbolTable);
  return retVal;
}

void Usage() { printf("USAGE: assembler <inputFile> [outputFile]"); }

// djb2 string hash
unsigned long hash(const char *str) {
  unsigned long h = 5381;
  int c;

  while ((c = *str++))
    h = ((h << 5) + h) + c; /* h * 33 + c */

  return h % TABLE_SIZE;
}

// create hash table
HashTable *ht_create(void) {
  HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
  int i;

  if (!ht)
    return NULL;

  for (i = 0; i < TABLE_SIZE; i++)
    ht->buckets[i] = NULL;

  return ht;
}

// insert or update
void ht_put(HashTable *ht, const char *key, int value) {
  unsigned long idx = hash(key);
  Entry *e = ht->buckets[idx];

  // update if key exists
  while (e) {
    if (strcmp(e->key, key) == 0) {
      e->value = value;
      return;
    }
    e = e->next;
  }

  // insert new entry
  e = (Entry *)malloc(sizeof(Entry));
  if (!e) {
    printf("Out of memory\n");
    exit(2);
  }

  e->key = malloc(strlen(key) + 1); /* +1 for '\0' */
  if (!e->key) {
    printf("Out of memory in hash table put while inserting %s\n", key);
    exit(2);
  }

  strcpy(e->key, key);
  e->value = value;
  e->next = ht->buckets[idx];
  ht->buckets[idx] = e;
}

// lookup
int ht_get(HashTable *ht, const char *key, int *out) {
  unsigned long idx = hash(key);
  Entry *e = ht->buckets[idx];

  while (e) {
    if (strcmp(e->key, key) == 0) {
      *out = e->value;
      return 1; /* found */
    }
    e = e->next;
  }

  return 0; /* not found */
}

// delete key
void ht_remove(HashTable *ht, const char *key) {
  unsigned long idx = hash(key);
  Entry *e = ht->buckets[idx];
  Entry *prev = NULL;

  while (e) {
    if (strcmp(e->key, key) == 0) {
      if (prev)
        prev->next = e->next;
      else
        ht->buckets[idx] = e->next;

      free(e->key);
      free(e);
      return;
    }
    prev = e;
    e = e->next;
  }
}

// free table
void ht_destroy(HashTable *ht) {
  int i;
  Entry *e, *next;

  for (i = 0; i < TABLE_SIZE; i++) {
    e = ht->buckets[i];
    while (e) {
      next = e->next;
      free(e->key);
      free(e);
      e = next;
    }
  }
  free(ht);
}
// debugging
void ht_print(HashTable *ht) {
  int i;
  Entry *e;

  for (i = 0; i < TABLE_SIZE; i++) {
    if (ht->buckets[i] == NULL)
      continue;

    printf("[%d]: ", i);
    e = ht->buckets[i];

    while (e) {
      printf("(%s -> %d)", e->key, e->value);
      if (e->next)
        printf(" -> ");
      e = e->next;
    }
    printf("\n");
  }
}
void InstructionHex_init(InstructionHex *ih) {
  if (ih == NULL)
    return;
  ih->instruction = 0;
}
void SourceLine_init(SourceLine *sl) {
  if (sl == NULL)
    return;

  sl->lineNo = 0;
  sl->label = NULL;
  sl->mnemonic = NULL;
  sl->sourceLine = NULL;
  sl->isComment = 0;
  sl->args[0] = sl->args[1] = sl->args[2] = NULL;
}
void ObjectCodeLine_init(ObjectCodeLine *ocl) {
  if (ocl == NULL)
    return;

  ocl->location = 0;
  InstructionHex_init(&ocl->instruction);
  ocl->source = NULL;
}

// Write End record in the fp file
void DumpEndRecord(FILE* fp, size_t FirstInstruction);
// Writes Header record in the fp file.
void DumpHeader(FILE* fp, const char* progName, size_t programLength,
                size_t FirstInstruction);
// Strips the location from the line so it becomes sourceline from intermediate
// Line
void StripLocation(char* line);

int GenerateIntermediate(const char* const fname, size_t* capacity,
                         size_t* programLength, size_t* firstInstructionAddress,
                         HashTable* symbolTable) {
    char imm_fname[512];
    snprintf(imm_fname, sizeof(imm_fname), "%s_intermediate.txt", fname);
    FILE* fp   = fopen(fname, "r");
    FILE* imfp = fopen(imm_fname, "w");
    if (fp == NULL) {
        printf("Unable to open file %s\n", fname);
        return 1;
    }
    if (imfp == NULL) {
        printf("Unable to open file %s\n", imm_fname);
        return 1;
    }

    int retValue        = 0;
    const size_t buflen = 150;
    char buf[150];
    size_t lineNo   = 0;  // Sourse line no
    size_t index    = 0;
    size_t loc      = 0;  // current location
    size_t locStart = 0;  // START location
    int isFirstMne  = 1;  // first mnemonic should be start

    /* initial capacity */
    *capacity       = 8;
    SourceLine sl;
    SourceLine_init(&sl);
    ObjectCodeLine obcl;
    ObjectCodeLine_init(&obcl);

    // Scan line by line. Not using getline because its not c std.
    // So maximum line size is defined
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

        // init source line
        SourceLine_init(&sl);

        // init objectCode line
        ObjectCodeLine_init(&obcl);
        obcl.source   = &sl;

        sl.lineNo     = ++lineNo;
        // split later
        sl.sourceLine = malloc(strlen(buf) + 1);
        if (sl.sourceLine == NULL) {
            retValue = 3;
            goto exit;
        }
        strcpy(sl.sourceLine, buf);
        // Split source line into fields
        SplitInstruction(&sl);
        // Dump comment directly
        if (!sl.isComment) {
            Mnemonic mne = IsMnemonic(sl.mnemonic);
            if (isFirstMne) {
                // first mnemonic should be start always
                if (mne != START) {
                    printf("First Mnemonic should be START");
                    retValue = 5;
                    goto exit;
                }
                // Header record supports 6 as max prog name
                if (strlen(sl.label) > PROG_NAME_MAX_LENGTH) {
                    printf(
                        "Program name %s exceeds maximum allowed name length "
                        "of %d\n",
                        sl.label, PROG_NAME_MAX_LENGTH);
                    retValue = 6;
                    goto exit;
                }
                isFirstMne = 0;
                loc        = strtol(sl.args[0], NULL, 16);
                locStart   = loc;
            }
            // End also contain first instruction to execute, else 0
            if (mne == END) {
                int out = 0;
                if (!ht_get(symbolTable, sl.args[0], &out)) {
                    printf(
                        "Label defined with END %s not found in the "
                        "symbolTable\n",
                        sl.args[0]);
                    retValue = 7;
                    goto exit;
                }
                *firstInstructionAddress = out;
            }
            obcl.location = loc;
            // put the label in symbolTable.  Check for redefinition
            if (sl.label != NULL) {
                int out = 0;
                if (ht_get(symbolTable, sl.label, &out)) {
                    retValue = 6;
                    printf("Duplicate symbol found %s on line %zu\n", sl.label,
                           sl.lineNo);
                    goto exit;
                } else {
                    ht_put(symbolTable, sl.label, loc);
                }
            }
            *programLength  = loc - locStart;
            loc            += AddressToAdd(mne, &obcl);
        }
        DumpObject(imfp, &obcl);
        index++;
        free(sl.sourceLine);
    }
    *capacity = index;
exit:
    fclose(fp);
    fclose(imfp);
    return retValue;
}
int GenerateObject(const char* fname, const char* out_fname,
                   size_t programLength, size_t FirstInstruction,
                   HashTable* symbolTable) {
    char imm_fname[512];
    snprintf(imm_fname, sizeof(imm_fname), "%s_intermediate.txt", fname);

    FILE* fp    = fopen(imm_fname, "r");
    FILE* outfp = fopen(out_fname, "w");
    if (!fp) {
        printf("Unable to open %s for intermediate reading\n", imm_fname);
        return 1;
    }
    if (!outfp) {
        printf("Unable to open %s for output\n", out_fname);
        fclose(fp);
        return 1;
    }

    char buf[150];
    SourceLine sl;
    ObjectCodeLine obcl;

    char textBuf[70];  // Text section buf
    char objBuf[50];   // resover buf
    int textLenBytes  = 0;
    int textStartAddr = -1;

    int retValue      = 0;

    while (fgets(buf, sizeof(buf), fp)) {
        size_t len = strlen(buf);
        if (len && buf[len - 1] == '\n') buf[len - 1] = '\0';

        SourceLine_init(&sl);
        ObjectCodeLine_init(&obcl);

        sl.sourceLine = buf;
        obcl.source   = &sl;

        // skip comment
        char* p       = buf;
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '.') continue;

        // extract location
        obcl.location = strtol(p, NULL, 16);

        StripLocation(buf);
        SplitInstruction(&sl);

        Mnemonic mne = IsMnemonic(sl.mnemonic);

        if (mne == START) {
            DumpHeader(outfp, sl.label, programLength, FirstInstruction);
            continue;
        }

        if (mne == END) {
            /* flush pending text record */
            if (textLenBytes > 0) {
                fprintf(outfp, "T%06X%02X%s\n", textStartAddr, textLenBytes,
                        textBuf);
            }
            DumpEndRecord(outfp, FirstInstruction);
            break;
        }

        // -------- resolve object code --------

        int bytesWritten =
            ResolveObjectCode(&obcl, symbolTable, objBuf, sizeof(objBuf));
        if (bytesWritten < 0) {
            retValue = bytesWritten;
            goto exitObj;
        }

        /* no object code (RESB / RESW) → force flush */
        if (bytesWritten == 0) {
            if (textLenBytes > 0) {
                fprintf(outfp, "T%06X%02X%s\n", textStartAddr, textLenBytes,
                        textBuf);
                textLenBytes  = 0;
                textStartAddr = -1;
            }
            continue;
        }

        /* start new text record if needed */
        if (textLenBytes == 0) {
            textStartAddr = obcl.location;
            textBuf[0]    = '\0';
        }

        // does it fit? max 30 bytes
        if (textLenBytes + bytesWritten > 30) {
            fprintf(outfp, "T%06X%02X%s\n", textStartAddr, textLenBytes,
                    textBuf);

            textLenBytes  = 0;
            textStartAddr = obcl.location;
            textBuf[0]    = '\0';
        }

        // append object code
        strcat(textBuf, objBuf);
        textLenBytes += bytesWritten;
    }

exitObj:
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
    int  i;
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
