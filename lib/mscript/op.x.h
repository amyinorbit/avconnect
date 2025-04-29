/*===--------------------------------------------------------------------------------------------===
 * mscript_op.x.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef OPCODE
#define OPCODE(name, args, stack)
#define OPCODE_NOTHING
#endif


OPCODE(CONST, 1, 1)

OPCODE(GET_LOCAL, 1, 1)

OPCODE(DUP)
OPCODE(POP)
OPCODE(BLOCK)

OPCODE(NIL, 0, 1)
OPCODE(TRUE, 0, 1)
OPCODE(FALSE, 0, 1)

OPCODE(NEG, 0,  0)
OPCODE(ADD, 0, -1)
OPCODE(SUB, 0, -1)
OPCODE(MUL, 0, -1)
OPCODE(DIV, 0, -1)
OPCODE(POW, 0, -1)

OPCODE(NOT, 0, 0)
OPCODE(LT, 0, -1)
OPCODE(GT, 0, -1)
OPCODE(LTEQ, 0, -1)
OPCODE(GTEQ, 0, -1)
OPCODE(EQ, 0, -1)

OPCODE(JMP, 2, 0)
OPCODE(JMP_FALSE, 2, 0)
    
#ifdef OPCODE_NOTHING
#undef OPCODE
#undef OPCODE_NOTHING
#endif
    