#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "util.h"

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
int ResolveObjectCode(ObjectCodeLine* obcl, HashTable* symbolTable,
                      long textOffset, char* buf, size_t buflen) {
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
        long patchOffset =  2; /* skip opcode (2 hex chars) */

        ht_add_backref(symbolTable, obcl->source->args[0],textOffset,patchOffset,
                       useIndexed);
    }

    return 3;
}
