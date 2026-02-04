#include <stdlib.h>

#include "assembler.h"

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
