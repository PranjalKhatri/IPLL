#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"

int Resolve_LDA(const char* args[3], InstructionHex* out) { return 0; }
int Resolve_ADD(const char* args[3], InstructionHex* out) { return 0; }
int Resolve_RSUB(const char* args[3], InstructionHex* out) { return 0; }
int Resolve_DIR(const char* args[3], InstructionHex* out) { return 0; }

static const MnemonicInfo mnemonicTable[] = {
    // assembler directives
    {MNEMONIC_START, 0, "DUMMY~~~", Resolve_DIR},
    {START, 0, "START", Resolve_DIR},
    {END, 0, "END", Resolve_DIR},
    {BYTE, 0, "BYTE", Resolve_DIR},
    {WORD, 0, "WORD", Resolve_DIR},
    {RESB, 0, "RESB", Resolve_DIR},
    {RESW, 0, "RESW", Resolve_DIR},

    // instructions
    {LDA, 0x00, "LDA", Resolve_LDA},
    {LDX, 0x04, "LDX", Resolve_LDA},
    {LDL, 0x08, "LDL", Resolve_LDA},
    {STA, 0x0c, "STA", Resolve_LDA},
    {STX, 0x10, "STX", Resolve_LDA},
    {STL, 0x14, "STL", Resolve_LDA},
    {LDCH, 0x50, "LDCH", Resolve_LDA},
    {STCH, 0x54, "STCH", Resolve_LDA},

    {ADD, 0x18, "ADD", Resolve_ADD},
    {SUB, 0x1c, "SUB", Resolve_ADD},
    {MUL, 0x20, "MUL", Resolve_ADD},
    {DIV, 0x24, "DIV", Resolve_ADD},
    {COMP, 0x28, "COMP", Resolve_ADD},

    {J, 0x3c, "J", Resolve_ADD},
    {JLT, 0x38, "JLT", Resolve_ADD},
    {JEQ, 0x30, "JEQ", Resolve_ADD},
    {JGT, 0x34, "JGT", Resolve_ADD},
    {JSUB, 0x48, "JSUB", Resolve_ADD},
    {RSUB, 0x4c, "RSUB", Resolve_RSUB},

    {TIX, 0x2c, "TIX", Resolve_ADD},
    {TD, 0xe0, "TD", Resolve_ADD},
    {RD, 0xd8, "RD", Resolve_ADD},
    {WD, 0xdc, "WD", Resolve_ADD},
};
int IsDirective(const char* const str) {
    size_t i;

    if (str == NULL) return 0;

    for (i = 0; i < DIRECTIVE_END; ++i) {
        if (strcmp(str, mnemonicTable[i].name) == 0) return 1;
    }
    return 0;
}
Mnemonic IsMnemonic(const char* const str) {
    size_t i;

    if (str == NULL) return 0;

    for (i = MNEMONIC_START + 1; i < MNEMONIC_COUNT; ++i) {
        if (strcmp(str, mnemonicTable[i].name) == 0) return i;
    }
    return 0;
}

int DirAddressToAdd(Mnemonic mne, int arg0, int isChar) {
    switch (mne) {
        case START:
            return 0;
        case END:
            return 3;
        case BYTE:
            printf("on byte: isChar %d %d",isChar,arg0);
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
    if (obcl->source->args[0]) arg0Converted = parse_constant(obcl->source->args[0],&isChar);
    if (mne < DIRECTIVE_END) {
        retAddr = DirAddressToAdd(mne, arg0Converted, isChar);
    } else
        retAddr = 3;  // for all operands
    return retAddr;
}
