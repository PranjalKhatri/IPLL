#pragma once
#include <stddef.h>
#include <stdio.h>

#include "hash.h"
#include "util.h"

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
// Generate Intermediate file from input in phase 1
int GenerateIntermediate(const char* const fname, size_t* capacity,
                         size_t* programLength, size_t* firstInstructionAddress,
                         HashTable* symbolTable);
// Generate Final object file in phase 2
int GenerateObject(const char* fname, const char* out_fname,
                   size_t programLength, size_t FirstInstruction,
                   HashTable* symbolTable);

void SplitInstructions(SourceLine* const sourceLines, size_t numLines);
// Splits a source line into various fields of its struct
void SplitInstruction(SourceLine* const sl);
// Dumps the source lines for debugging in file fname
void DumpLex(const char* const fname, SourceLine* sourceLines, size_t numLines);
// Resolves objectcodeline struct into object code in hex and writes in buf
int ResolveObjectCode(ObjectCodeLine* obcl, HashTable* symbolTable, FILE* outfp,
                      char* buf, size_t buflen);
// Dumps the object code in file fp
void DumpObject(FILE* fp, ObjectCodeLine* ol);
// Dumps Intermediate doe in file fname
void DumpIntermediate(const char* const fname, ObjectCodeLine* objectCodeLines,
                      size_t numLines);

// struct representing mnemonic, its opcode and its string name
typedef struct {
    Mnemonic mnemonic;
    int opcode;
    const char* name;  // string form LDA, ADD..
} MnemonicInfo;
