#pragma once
#include <stddef.h>

#include "util.h"

typedef struct {
    unsigned int instruction;
} InstructionHex;

typedef struct {
    size_t lineNo;
    char*  sourceLine;
    char*  label;        // optional
    char*  instruction;  // operation name
    // different instructions can have different operands
    char*  args[3];
    int    isComment;
} SourceLine;

typedef struct {
    size_t         location;
    InstructionHex instruction;
    SourceLine*    source;
} ObjectCodeLine;

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

void     InstructionHex_init(InstructionHex* ih);
void     SourceLine_init(SourceLine* sl);
void     ObjectCodeLine_init(ObjectCodeLine* ocl);
Mnemonic IsMnemonic(const char* const str);
int      IsDirective(const char* const str);
int      AddressToAdd(Mnemonic mne, ObjectCodeLine* obcl);
int      LexAndInit(const char* const fname, SourceLine** sourceLines,
                    ObjectCodeLine** objectCodeLines, size_t* capacity);
void     SplitInstructions(SourceLine* const sourceLines, size_t numLines);
void     SplitInstruction(SourceLine* const sl);
void DumpLex(const char* const fname, SourceLine* sourceLines, size_t numLines);
void DumpIntermediate(const char* const fname, ObjectCodeLine* objectCodeLines,
                      size_t numLines);

typedef int (*ResolveFn)(const char* args[3], InstructionHex* out);
typedef struct {
    Mnemonic    mnemonic;
    int         opcode;
    const char* name;     // string form LDA, ADD..
    ResolveFn   resolve;  // function to call to get code
} MnemonicInfo;
