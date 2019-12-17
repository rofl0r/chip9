#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ezsdl.h"

static int debug = 1;

static unsigned char *bootrom;
static unsigned lbootrom;
static unsigned char *rom;
static unsigned lrom;
static unsigned char mem[0x10000];
static const unsigned vmem_w = 128;
static const unsigned vmem_h = 64;
static unsigned char vmem[128*64];
static struct {
	unsigned char gpr[7+1];
	unsigned short sp, pc;
} regs;

#define N_A 0
#define N_F 1
#define N_B 2
#define N_C 3
#define N_D 4
#define N_E 5
#define N_H 6
#define N_L 7
#define N_LAST N_L

#define A regs.gpr[N_A]
#define F regs.gpr[N_F]
#define B regs.gpr[N_B]
#define C regs.gpr[N_C]
#define D regs.gpr[N_D]
#define E regs.gpr[N_E]
#define H regs.gpr[N_H]
#define L regs.gpr[N_L]

#define PAIR(X,Y) ((X<<8)|Y)
#define BC PAIR(B, C)
#define DE PAIR(D, E)
#define HL PAIR(H, L)

enum optype {
	OT_ILL = 0,
	OT_NOP = 1,
	OT_RET,
	OT_LDI,
	OT_LDIP,
	OT_LDX,
	OT_PUSH,
	OT_PUSHP,
	OT_PUSHX,
	OT_POP,
	OT_POPP,
	OT_POPX,
	OT_MOV,
	OT_MOVRP,
	OT_MOVPR,
	OT_MOVX,
	OT_CLRF,
	OT_SETF,
	OT_ADD,
	OT_ADDP,
	OT_ADDI,
	OT_ADDX,
	OT_SUB,
	OT_SUBP,
	OT_SUBI,
	OT_INC,
	OT_INCP,
	OT_INX,
	OT_DEC,
	OT_DECP,
	OT_AND,
	OT_ANDP,
	OT_ANDI,
	OT_OR,
	OT_ORP,
	OT_ORI,
	OT_XOR,
	OT_XORP,
	OT_XORI,
	OT_CMP,
	OT_CMPP,
	OT_CMPI,
	OT_CMPS,
	OT_CMPSP,
	OT_SIN,
	OT_SOUT,
	OT_CLRSCR,
	OT_DRAW,
	OT_JMP16,
	OT_JMP16C,
	OT_JMP8,
	OT_JMP8C,
	OT_CALL,
	OT_HALT,
};

static const char* opstr[] = {
	[OT_ILL] = "ILL",
	[OT_NOP] = "nop",
	[OT_RET] = "ret",
	[OT_LDI] = "ldi",
	[OT_LDIP] = "ldip",
	[OT_LDX] = "ldx",
	[OT_PUSH] = "push",
	[OT_PUSHP] = "pushp",
	[OT_PUSHX] = "pushx",
	[OT_POP] = "pop",
	[OT_POPP] = "popp",
	[OT_POPX] = "popx",
	[OT_MOV] = "mov",
	[OT_MOVRP] = "movrp",
	[OT_MOVPR] = "movpr",
	[OT_MOVX] = "movx",
	[OT_CLRF] = "clrf",
	[OT_SETF] = "setf",
	[OT_ADD] = "add",
	[OT_ADDP] = "addp",
	[OT_ADDI] = "addi",
	[OT_ADDX] = "addx",
	[OT_SUB] = "sub",
	[OT_SUBP] = "subp",
	[OT_SUBI] = "subi",
	[OT_INC] = "inc",
	[OT_INCP] = "incp",
	[OT_INX] = "inx",
	[OT_DEC] = "dec",
	[OT_DECP] = "decp",
	[OT_AND] = "and",
	[OT_ANDP] = "andp",
	[OT_ANDI] = "andi",
	[OT_OR] = "or",
	[OT_ORP] = "orp",
	[OT_ORI] = "ori",
	[OT_XOR] = "xor",
	[OT_XORP] = "xorp",
	[OT_XORI] = "xori",
	[OT_CMP] = "cmp",
	[OT_CMPP] = "cmpp",
	[OT_CMPI] = "cmpi",
	[OT_CMPS] = "cmps",
	[OT_CMPSP] = "cmpsp",
	[OT_SIN] = "sin",
	[OT_SOUT] = "sout",
	[OT_CLRSCR] = "clrscr",
	[OT_DRAW] = "draw",
	[OT_JMP16] = "jmpw",
	[OT_JMP16C] = "jmpw",
	[OT_JMP8] = "jmp",
	[OT_JMP8C] = "jmp",
	[OT_CALL] = "call",
	[OT_HALT] = "halt",
};

#define RF_A (N_A)
#define RF_F (N_F)
#define RF_B (N_B)
#define RF_C (N_C)
#define RF_D (N_D)
#define RF_E (N_E)
#define RF_H (N_H)
#define RF_L (N_L)
#define RF_SP (N_LAST+1)
#define RF_FIRST(X) (X<<16)

// FLAG REG:
#define FZ (1<<7)
#define FN (1<<6)
#define FH (1<<5)
#define FC (1<<4)
//custom
#define FNOT (1<<0)

#define REG2(R1, R2) (RF_FIRST(R1) | R2)
#define POINTER(R1) (R1)

enum enctype {
	ET_NONE,
	ET_REG,
	ET_2REG,
	ET_FL,
};

static const struct opcode {
	enum optype /*unsigned char*/ optype;
	enum enctype enctype;
	unsigned char argb;
	unsigned opdesc;
} opcodes[256] = {
	[0x20] = { OT_LDI, ET_REG, 1, RF_B, },
	[0x30] = { OT_LDI, ET_REG, 1, RF_C, },
	[0x40] = { OT_LDI, ET_REG, 1, RF_D, },
	[0x50] = { OT_LDI, ET_REG, 1, RF_E, },
	[0x60] = { OT_LDI, ET_REG, 1, RF_H, },
	[0x70] = { OT_LDI, ET_REG, 1, RF_L, },
	[0x80] = { OT_LDIP, ET_REG, 1, POINTER(RF_H), },
	[0x90] = { OT_LDI, ET_REG, 1, RF_A, },

	[0x21] = { OT_LDX, ET_REG, 2, RF_B, },
	[0x31] = { OT_LDX, ET_REG, 2, RF_D, },
	[0x41] = { OT_LDX, ET_REG, 2, RF_H, },
	[0x22] = { OT_LDX, ET_REG, 2, RF_SP, },

	[0x81] = { OT_PUSH, ET_REG, 0, RF_B,},
	[0x91] = { OT_PUSH, ET_REG, 0, RF_C,},
	[0xa1] = { OT_PUSH, ET_REG, 0, RF_D,},
	[0xb1] = { OT_PUSH, ET_REG, 0, RF_E,},
	[0xc1] = { OT_PUSH, ET_REG, 0, RF_H,},
	[0xd1] = { OT_PUSH, ET_REG, 0, RF_L,},
	[0xc0] = { OT_PUSHP, ET_REG, 0, POINTER(RF_H),},
	[0xd0] = { OT_PUSH, ET_REG, 0, RF_A,},
	[0x51] = { OT_PUSHX, ET_REG, 0, RF_B,},
	[0x61] = { OT_PUSHX, ET_REG, 0, RF_D,},
	[0x71] = { OT_PUSHX, ET_REG, 0, RF_H,},
	[0x82] = { OT_POP, ET_REG, 0, RF_B,},
	[0x92] = { OT_POP, ET_REG, 0, RF_C, },
	[0xa2] = { OT_POP, ET_REG, 0, RF_D, },
	[0xb2] = { OT_POP, ET_REG, 0, RF_E, },
	[0xc2] = { OT_POP, ET_REG, 0, RF_H, },
	[0xd2] = { OT_POP, ET_REG, 0, RF_L, },
	[0xc3] = { OT_POPP, ET_REG, 0, POINTER(RF_H), },
	[0xd3] = { OT_POP, ET_REG, 0, RF_A, },
	[0x52] = { OT_POPX, ET_REG, 0, RF_B, },
	[0x62] = { OT_POPX, ET_REG, 0, RF_D,},
	[0x72] = { OT_POPX, ET_REG, 0, RF_H,},
	[0x09] = { OT_MOV, ET_2REG, 0, REG2(RF_B, RF_B),},
	[0x19] = { OT_MOV, ET_2REG, 0, REG2(RF_B, RF_C),},
	[0x29] = { OT_MOV, ET_2REG, 0, REG2(RF_B, RF_D),},
	[0x39] = { OT_MOV, ET_2REG, 0, REG2(RF_B, RF_E),},
	[0x49] = { OT_MOV, ET_2REG, 0, REG2(RF_B, RF_H),},
	[0x59] = { OT_MOV, ET_2REG, 0, REG2(RF_B, RF_L),},
	[0x69] = { OT_MOVRP, ET_2REG, 0, REG2(RF_B, POINTER(RF_H)),},
	[0x79] = { OT_MOV, ET_2REG, 0, REG2(RF_B, RF_A),},
	[0x89] = { OT_MOV, ET_2REG, 0, REG2(RF_C, RF_B),},
	[0x99] = { OT_MOV, ET_2REG, 0, REG2(RF_C, RF_C),},
	[0xa9] = { OT_MOV, ET_2REG, 0, REG2(RF_C, RF_D),},
	[0xb9] = { OT_MOV, ET_2REG, 0, REG2(RF_C, RF_E),},
	[0xc9] = { OT_MOV, ET_2REG, 0, REG2(RF_C, RF_H),},
	[0xd9] = { OT_MOV, ET_2REG, 0, REG2(RF_C, RF_L),},
	[0xe9] = { OT_MOVRP, ET_2REG, 0, REG2(RF_C, POINTER(RF_H)),},
	[0xf9] = { OT_MOV, ET_2REG, 0, REG2(RF_C, RF_A),},
	[0x0a] = { OT_MOV, ET_2REG, 0, REG2(RF_D, RF_B),},
	[0x1a] = { OT_MOV, ET_2REG, 0, REG2(RF_D, RF_C),},
	[0x2a] = { OT_MOV, ET_2REG, 0, REG2(RF_D, RF_D),},
	[0x3a] = { OT_MOV, ET_2REG, 0, REG2(RF_D, RF_E),},
	[0x4a] = { OT_MOV, ET_2REG, 0, REG2(RF_D, RF_H),},
	[0x5a] = { OT_MOV, ET_2REG, 0, REG2(RF_D, RF_L),},
	[0x6a] = { OT_MOVRP, ET_2REG, 0, REG2(RF_D, POINTER(RF_H)),},
	[0x7a] = { OT_MOV, ET_2REG, 0, REG2(RF_D, RF_A),},
	[0x8a] = { OT_MOV, ET_2REG, 0, REG2(RF_E, RF_B),},
	[0x9a] = { OT_MOV, ET_2REG, 0, REG2(RF_E, RF_C),},
	[0xaa] = { OT_MOV, ET_2REG, 0, REG2(RF_E, RF_D), },
	[0xba] = { OT_MOV, ET_2REG, 0, REG2(RF_E, RF_E),},
	[0xca] = { OT_MOV, ET_2REG, 0, REG2(RF_E, RF_H), },
	[0xda] = { OT_MOV, ET_2REG, 0, REG2(RF_E, RF_L), },
	[0xea] = { OT_MOVRP, ET_2REG, 0, REG2(RF_E, POINTER(RF_H)), },
	[0xfa] = { OT_MOV, ET_2REG, 0, REG2(RF_E, RF_A), },
	[0x0b] = { OT_MOV, ET_2REG, 0, REG2(RF_H, RF_B), },
	[0x1b] = { OT_MOV, ET_2REG, 0, REG2(RF_H, RF_C),},
	[0x2b] = { OT_MOV, ET_2REG, 0, REG2(RF_H, RF_D),},
	[0x3b] = { OT_MOV, ET_2REG, 0, REG2(RF_H, RF_E),},
	[0x4b] = { OT_MOV, ET_2REG, 0, REG2(RF_H, RF_H),},
	[0x5b] = { OT_MOV, ET_2REG, 0, REG2(RF_H, RF_L),},
	[0x6b] = { OT_MOVRP, ET_2REG, 0, REG2(RF_H, POINTER(RF_H)),},
	[0x7b] = { OT_MOV, ET_2REG, 0, REG2(RF_H, RF_A),},
	[0x8b] = { OT_MOV, ET_2REG, 0, REG2(RF_L, RF_B),},
	[0x9b] = { OT_MOV, ET_2REG, 0, REG2(RF_L, RF_C),},
	[0xab] = { OT_MOV, ET_2REG, 0, REG2(RF_L, RF_D),},
	[0xbb] = { OT_MOV, ET_2REG, 0, REG2(RF_L, RF_E),},
	[0xcb] = { OT_MOV, ET_2REG, 0, REG2(RF_L, RF_H),},
	[0xdb] = { OT_MOV, ET_2REG, 0, REG2(RF_L, RF_L),},
	[0xeb] = { OT_MOVRP, ET_2REG, 0, REG2(RF_L, POINTER(RF_H)),},
	[0xfb] = { OT_MOV, ET_2REG, 0, REG2(RF_L, RF_A),},
	[0x0c] = { OT_MOVPR, ET_2REG, 0, REG2(RF_H, RF_B),},
	[0x1c] = { OT_MOVPR, ET_2REG, 0, REG2(RF_H, RF_C),},
	[0x2c] = { OT_MOVPR, ET_2REG, 0, REG2(RF_H, RF_D),},
	[0x3c] = { OT_MOVPR, ET_2REG, 0, REG2(RF_H, RF_E),},
	[0x4c] = { OT_MOVPR, ET_2REG, 0, REG2(RF_H, RF_H),},
	[0x5c] = { OT_MOV, ET_2REG, 0, REG2(RF_H, RF_L),},
// halt	[0x6c] = { OT_MOV, 0, },
	[0x7c] = { OT_MOVPR, ET_2REG, 0, REG2(RF_H, RF_A),},
	[0x8c] = { OT_MOV, ET_2REG, 0, REG2(RF_A, RF_B),},
	[0x9c] = { OT_MOV, ET_2REG, 0, REG2(RF_A, RF_C),},
	[0xac] = { OT_MOV, ET_2REG, 0, REG2(RF_A, RF_D),},
	[0xbc] = { OT_MOV, ET_2REG, 0, REG2(RF_A, RF_E),},
	[0xcc] = { OT_MOV, ET_2REG, 0, REG2(RF_A, RF_H),},
	[0xdc] = { OT_MOV, ET_2REG, 0, REG2(RF_A, RF_L),},
	[0xec] = { OT_MOVRP, ET_2REG, 0, REG2(RF_A, POINTER(RF_H)),},
	[0xfc] = { OT_MOV, ET_2REG, 0, REG2(RF_A, RF_A),},
	[0xed] = { OT_MOVX, ET_2REG, 0, REG2(RF_H, RF_B),},
	[0xfd] = { OT_MOVX, ET_2REG, 0, REG2(RF_H, RF_D),},

	[0x08] = { OT_CLRF, ET_NONE, 0, 0,},
	[0x18] = { OT_SETF, ET_FL, 0, FZ, },
	[0x28] = { OT_SETF, ET_FL, 0, FZ|FNOT,},
	[0x38] = { OT_SETF, ET_FL, 0, FN,},
	[0x48] = { OT_SETF, ET_FL, 0, FN|FNOT,},
	[0x58] = { OT_SETF, ET_FL, 0, FH,},
	[0x68] = { OT_SETF, ET_FL, 0, FH|FNOT,},
	[0x78] = { OT_SETF, ET_FL, 0, FC,},
	[0x88] = { OT_SETF, ET_FL, 0, FC|FNOT,},

	[0x04] = { OT_ADD, ET_REG, 0, RF_B, },
	[0x14] = { OT_ADD, ET_REG, 0, RF_C,},
	[0x24] = { OT_ADD, ET_REG, 0, RF_D,},
	[0x34] = { OT_ADD, ET_REG, 0, RF_E,},
	[0x44] = { OT_ADD, ET_REG, 0, RF_H,},
	[0x54] = { OT_ADD, ET_REG, 0, RF_L,},
	[0x64] = { OT_ADDP, ET_REG, 0, POINTER(RF_H),},
	[0x74] = { OT_ADD, ET_REG, 0, RF_A,},
	[0xa7] = { OT_ADDI, ET_REG, 1, RF_A, },
	[0x83] = { OT_ADDX, ET_REG, 0, RF_B,},
	[0x93] = { OT_ADDX, ET_REG, 0, RF_D,},
	[0xa3] = { OT_ADDX, ET_REG, 0, RF_H,},

	[0x84] = { OT_SUB, ET_REG, 0, RF_B,},
	[0x94] = { OT_SUB, ET_REG, 0, RF_C,},
	[0xa4] = { OT_SUB, ET_REG, 0, RF_D,},
	[0xb4] = { OT_SUB, ET_REG, 0, RF_E,},
	[0xc4] = { OT_SUB, ET_REG, 0, RF_H,},
	[0xd4] = { OT_SUB, ET_REG, 0, RF_L,},
	[0xe4] = { OT_SUBP, ET_REG, 0, POINTER(RF_H),},
	[0xf4] = { OT_SUB, ET_REG, 0, RF_A,},
	[0xb7] = { OT_SUBI, ET_REG, 1, RF_A, },

	[0x03] = { OT_INC, ET_REG, 0, RF_B,},
	[0x13] = { OT_INC, ET_REG, 0, RF_C,},
	[0x23] = { OT_INC, ET_REG, 0, RF_D,},
	[0x33] = { OT_INC, ET_REG, 0, RF_E,},
	[0x43] = { OT_INC, ET_REG, 0, RF_H,},
	[0x53] = { OT_INC, ET_REG, 0, RF_L,},
	[0x63] = { OT_INCP, ET_REG, 0, POINTER(RF_H),},
	[0x73] = { OT_INC, ET_REG, 0, RF_A,},

	[0xa8] = { OT_INX, ET_REG, 0, RF_B,},
	[0xb8] = { OT_INX, ET_REG, 0, RF_D,},
	[0xc8] = { OT_INX, ET_REG, 0, RF_H,},

	[0x07] = { OT_DEC, ET_REG, 0, RF_B,},
	[0x17] = { OT_DEC, ET_REG, 0, RF_C,},
	[0x27] = { OT_DEC, ET_REG, 0, RF_D,},
	[0x37] = { OT_DEC, ET_REG, 0, RF_E,},
	[0x47] = { OT_DEC, ET_REG, 0, RF_H,},
	[0x57] = { OT_DEC, ET_REG, 0, RF_L,},
	[0x67] = { OT_DECP, ET_REG, 0, POINTER(RF_H),},
	[0x77] = { OT_DEC, ET_REG, 0, RF_A,},

	[0x05] = { OT_AND, ET_REG, 0, RF_B,},
	[0x15] = { OT_AND, ET_REG, 0, RF_C,},
	[0x25] = { OT_AND, ET_REG, 0, RF_D,},
	[0x35] = { OT_AND, ET_REG, 0, RF_E,},
	[0x45] = { OT_AND, ET_REG, 0, RF_H,},
	[0x55] = { OT_AND, ET_REG, 0, RF_L,},
	[0x65] = { OT_ANDP, ET_REG, 0, POINTER(RF_H),},
	[0x75] = { OT_AND, ET_REG, 0, RF_A,},

	[0xc7] = { OT_ANDI, ET_REG, 1, RF_A,},

	[0x85] = { OT_OR, ET_REG, 0, RF_B,},
	[0x95] = { OT_OR, ET_REG, 0, RF_C,},
	[0xa5] = { OT_OR, ET_REG, 0, RF_D,},
	[0xb5] = { OT_OR, ET_REG, 0, RF_E,},
	[0xc5] = { OT_OR, ET_REG, 0, RF_H,},
	[0xd5] = { OT_OR, ET_REG, 0, RF_L,},
	[0xe5] = { OT_ORP, ET_REG, 0, POINTER(RF_H),},
	[0xf5] = { OT_OR, ET_REG, 0, RF_A,},

	[0xd7] = { OT_ORI, ET_REG, 1, RF_A, },

	[0x06] = { OT_XOR, ET_REG, 0, RF_B,},
	[0x16] = { OT_XOR, ET_REG, 0, RF_C,},
	[0x26] = { OT_XOR, ET_REG, 0, RF_D,},
	[0x36] = { OT_XOR, ET_REG, 0, RF_E,},
	[0x46] = { OT_XOR, ET_REG, 0, RF_H,},
	[0x56] = { OT_XOR, ET_REG, 0, RF_L,},
	[0x66] = { OT_XORP, ET_REG, 0, POINTER(RF_H),},
	[0x76] = { OT_XOR, ET_REG, 0, RF_A,},

	[0xe7] = { OT_XORI, ET_REG, 1, RF_A,},

	[0x86] = { OT_CMP, ET_REG, 0, RF_B,},
	[0x96] = { OT_CMP, ET_REG, 0, RF_C,},
	[0xa6] = { OT_CMP, ET_REG, 0, RF_D,},
	[0xb6] = { OT_CMP, ET_REG, 0, RF_E,},
	[0xc6] = { OT_CMP, ET_REG, 0, RF_H,},
	[0xd6] = { OT_CMP, ET_REG, 0, RF_L,},
	[0xe6] = { OT_CMPP, ET_REG, 0, POINTER(RF_H),},
	[0xf6] = { OT_CMP, ET_REG, 0, RF_A,},

	[0xf7] = { OT_CMPI, ET_REG, 1, RF_A, },

	[0x0d] = { OT_CMPS, ET_REG, 0, RF_B,},
	[0x1d] = { OT_CMPS, ET_REG, 0, RF_C,},
	[0x2d] = { OT_CMPS, ET_REG, 0, RF_D,},
	[0x3d] = { OT_CMPS, ET_REG, 0, RF_E,},
	[0x4d] = { OT_CMPS, ET_REG, 0, RF_H,},
	[0x5d] = { OT_CMPS, ET_REG, 0, RF_L,},
	[0x6d] = { OT_CMPSP, ET_REG, 0, POINTER(RF_H),},
	[0x7d] = { OT_CMPS, ET_REG, 0, RF_A,},

	[0xe0] = { OT_SIN, ET_NONE, 0, 0,},
	[0xe1] = { OT_SOUT, ET_NONE, 0, 0,},
	[0xf0] = { OT_CLRSCR, ET_NONE, 0, 0,},
	[0xf1] = { OT_DRAW, ET_NONE, 0, 0},

	[0x0f] = { OT_JMP16, ET_NONE, 2, 0, },
	[0x1f] = { OT_JMP16C, ET_FL, 2, FZ, },
	[0x2f] = { OT_JMP16C, ET_FL, 2, FZ | FNOT, },
	[0x3f] = { OT_JMP16C, ET_FL, 2, FN, },
	[0x4f] = { OT_JMP16C, ET_FL, 2, FN | FNOT, },
	[0x5f] = { OT_JMP16C, ET_FL, 2, FH, },
	[0x6f] = { OT_JMP16C, ET_FL, 2, FH | FNOT, },
	[0x7f] = { OT_JMP16C, ET_FL, 2, FC, },
	[0x8f] = { OT_JMP16C, ET_FL, 2, FC | FNOT, },

	[0x9f] = { OT_JMP8, ET_NONE, 1, 0, },
	[0xaf] = { OT_JMP8C, ET_FL, 1, FZ, },
	[0xbf] = { OT_JMP8C, ET_FL, 1, FZ | FNOT, },
	[0xcf] = { OT_JMP8C, ET_FL, 1, FN, },
	[0xdf] = { OT_JMP8C, ET_FL, 1, FN | FNOT,},
	[0xef] = { OT_JMP8C, ET_FL, 1, FH, },
	[0xff] = { OT_JMP8C, ET_FL, 1, FH | FNOT, },
	[0xee] = { OT_JMP8C, ET_FL, 1, FC, },
	[0xfe] = { OT_JMP8C, ET_FL, 1, FC | FNOT, },

	[0x1e] = { OT_CALL, ET_NONE, 2, 0},
	[0x0e] = { OT_RET, ET_NONE, 0, 0},
	[0x00] = { OT_NOP, ET_NONE, 0, 0},
	[0x6c] = { OT_HALT, ET_NONE, 0, 0},
};

#define IP mem[regs.pc]
#define IPVB mem[regs.pc+1]
#define IPVW *((unsigned short*)(mem+regs.pc+1))
#define MREADB(OFF) mem[OFF]
#define MREADW(OFF) *((unsigned short*)(mem+OFF))
//#define RREADB(R) regs.gpr[R]
//#define RWRITEB(R, V) regs.gpr[R] = V
//#define RWRITEW(R, V) *((unsigned short*)(&regs.gpr[0] + R) = V
//#define MWRITEB(OFF, V) mem[OFF] = V
#define REGNO(OPDESC) (OPDESC)
#define REGPTR(OPDESC) (&regs.gpr[REGNO(OPDESC)])
#define REGPTRW(OPDESC) ((unsigned short*)REGPTR(OPDESC))
#define HLPTR (&mem[*REGPTRW(RF_H)])
#define STACKPTR (mem + regs.sp)
#define STACKPTRW (unsigned short*)(STACKPTR)
#define GETREG1(OPDESC) (OPDESC>>16)
#define GETREG2(OPDESC) (OPDESC&0xffff)

static void ezdraw() {
	unsigned *ptr = (void *) ezsdl_get_vram();
        unsigned pitch = ezsdl_get_pitch()/4;
	unsigned x, y, yl;
	for(y=0; y<vmem_h; ++y) {
		yl = y*pitch;
		for(x=0; x<vmem_w; ++x) {
			ptr[yl + x] = vmem[y*vmem_w+x]*0x10101;
		}
	}
	ezsdl_refresh();
}

static void draw(signed char x, signed char y, unsigned char what) {
	if(x <= -8 || y < 0) return;
	unsigned i;
	for(i=0; i<8; ++i) {
		if(x+i<0) continue;
		if(x+i>vmem_w) break;
		vmem[y*vmem_w+x+i] = (what & (1<<(7-i)))*255;
	}
	ezdraw();
}
static void clrscr(void) {
	memset(vmem, 0, sizeof vmem);
	ezsdl_clear();
	ezsdl_refresh();
}

static void setbit(unsigned mask, unsigned one) {
	if(one) F |= mask;
	else F &= ~mask;
}
static void pushw(unsigned short val) {
	*STACKPTRW = val;
	regs.sp -= 2;
}
static unsigned short popw(void) {
	regs.sp += 2;
	return *STACKPTRW;
}
static char* fmtb(char *buf, unsigned val) { sprintf(buf, "%02x", val); return buf; }
static char* fmtw(char *buf, unsigned val) { sprintf(buf, "%04x", val); return buf; }
static char* fmtbin(char *buf, unsigned char *pc, int bytes) {
	unsigned char *p = buf;
	if(bytes-- > 0) { sprintf(p, "%02x", *(pc++)); p+= 2;};
	if(bytes-- > 0) { sprintf(p, "%02x", *(pc++)); p+= 2;}
	if(bytes-- > 0) { sprintf(p, "%02x", *(pc++)); p+= 2;}
	return buf;
}
static char *flagstr(unsigned char desc) {
	switch(desc & ~FNOT) {
		case FZ: return "z";
		case FN: return "n";
		case FH: return "h";
		case FC: return "c";
		default: abort();
	}
}
static char *fmtins(char *buf) {
	static const char *regnames[] = {"B", "C", "D", "E", "H", "L", "A", "FL", "SP"};
	const struct opcode *oi = &opcodes[IP];
	char *p = buf + sprintf(buf, "%s", opstr[oi->optype]);
	switch(oi->enctype) {
		case ET_REG: p += sprintf(p, " %s", regnames[oi->opdesc]); break;
		case ET_2REG: p += sprintf(p, " %s, %s", regnames[GETREG1(oi->opdesc)], regnames[GETREG2(oi->opdesc)]); break;
		case ET_FL: p += sprintf(p, "%s%s", oi->opdesc & FNOT ? "n" : "", flagstr(oi->opdesc)); break;
	}
	if(oi->argb == 2) sprintf(p, " %04x", IPVW);
	else if(oi->argb == 1) sprintf(p, " %02x", IPVB);
	return buf;
}
static char *fmtflags(char* buf) {
	char* p = buf;
	if(F & FZ) *(p++) = 'Z';
	if(F & FN) *(p++) = 'N';
	if(F & FH) *(p++) = 'H';
	if(F & FC) *(p++) = 'C';
	*p = 0;
	return buf;
}
static void state() {
	char buf[8];
	if(debug) dprintf(2, "pc:%02x sp:%02x a:%02x b:%02x c:%02x d:%02x e:%02x h:%02x l:%02x  fl:%s\n",
	regs.pc, regs.sp, A, B, C, D, E, H, L, fmtflags(buf));
}
static void alu_set(unsigned char *old, unsigned short new) {
	setbit(FZ, new == 0);
	setbit(FN, new & (1<<7));
	setbit(FC, new & (1<<8));
	*old = new;
}
static void alu_add(unsigned char* old, unsigned char val) {
	unsigned short result = *old + val;
	setbit(FZ, !!(result == 0));
	setbit(FN, !!(result & (1<<7)));
	setbit(FC, !!(result & (1<<8)));
	setbit(FH, !!(((*old&0xf) + (val&0xf))&0x10));
	*old = result;
}
static void setflags(unsigned char result) {
	setbit(FZ, result == 0);
	setbit(FN, result & (1<<7));
}
static void setflagsw(unsigned short result) {
	setbit(FZ, result == 0);
	setbit(FN, result & (1<<15));
}
static int step() {
	const struct opcode *oi = &opcodes[IP];
	unsigned int inc = 1 + oi->argb;
	unsigned short us;
	unsigned tmp;
	int itmp;
	char buf[32], buf2[16];
	if(debug) dprintf(2, "BEF: "); state();
	if(debug) dprintf(2, "(%s) %s\n",
		fmtbin(buf2, &IP, oi->argb+1),
		fmtins(buf));
	switch(oi->optype) {
	case OT_HALT: sleep(10); return 0;
	case OT_ILL:  dprintf(2, "SIGILL\n"); return 0;
	case OT_NOP:  break;
	case OT_LDI:  *REGPTR(oi->opdesc) = IPVB; break;
	case OT_LDIP: *HLPTR = IPVB; break;
	case OT_LDX:
		*REGPTRW(oi->opdesc) = IPVW;
		break;
	case OT_PUSH:
		mem[regs.sp] = *REGPTR(oi->opdesc);
		regs.sp -= 2;
		break;
	case OT_PUSHP:
		*STACKPTRW = *HLPTR;
		regs.sp -= 2;
	case OT_PUSHX:
		*STACKPTRW = *REGPTRW(oi->opdesc);
		regs.sp -= 2;
		break;
	case OT_POP:
		regs.sp += 2;
		*REGPTRW(oi->opdesc) = *STACKPTR;
		break;
	case OT_POPP:
		regs.sp += 2;
		*HLPTR = *STACKPTR;
		break;
	case OT_POPX:
		regs.sp += 2;
		*REGPTRW(oi->opdesc) = *STACKPTRW;
		break;
	case OT_MOV: *REGPTR(GETREG1(oi->opdesc)) = *REGPTR(GETREG2(oi->opdesc)); break;
	case OT_MOVRP: *REGPTR(GETREG1(oi->opdesc)) = *HLPTR; break;
	case OT_MOVX: *REGPTRW(GETREG1(oi->opdesc)) = *REGPTRW(GETREG2(oi->opdesc)); break;
	case OT_CLRF: F = 0; break;
	case OT_SETF: setbit(oi->opdesc & ~FNOT, !(oi->opdesc & FNOT)); break;
	case OT_ADD:  alu_add(REGPTR(oi->opdesc), A); break;
	case OT_ADDP: alu_add(HLPTR, A); break;
	case OT_ADDI: alu_add(REGPTR(oi->opdesc), IPVB); break;
	case OT_ADDX: F = 0xff; *REGPTRW(oi->opdesc) += A; /*setflagsw(*REGPTRW(oi->opdesc));*/ break;
	case OT_SUB:  us = *REGPTR(oi->opdesc); us -= A; alu_set(REGPTR(oi->opdesc), us); break;
	case OT_SUBP: us = *HLPTR; us -= A; alu_set(HLPTR, us); break;
	case OT_SUBI: us = *REGPTR(oi->opdesc); us -= IPVB; alu_set(REGPTR(oi->opdesc), us); break;
	case OT_INC:  alu_add(REGPTR(oi->opdesc), 1); break;
	case OT_INCP: alu_add(HLPTR, 1); break;
	/* inx SHALL NOT set flags */
	case OT_INX:  *REGPTRW(oi->opdesc) += 1; break;
	case OT_DEC:  us = *REGPTR(oi->opdesc); --us; alu_set(REGPTR(oi->opdesc), us); break;
	case OT_AND:  F = 0; *REGPTR(oi->opdesc) &= A; setflags(*REGPTR(oi->opdesc)); break;
	case OT_ANDI: F = 0; *REGPTR(oi->opdesc) &= IPVB; setflags(*REGPTR(oi->opdesc)); break;
	case OT_OR:   F = 0; *REGPTR(oi->opdesc) |= A; setflags(*REGPTR(oi->opdesc)); break;
	case OT_ORI:  F = 0; *REGPTR(oi->opdesc) |= IPVB; setflags(*REGPTR(oi->opdesc)); break;
	case OT_XOR:  F = 0; *REGPTR(oi->opdesc) ^= A; setflags(*REGPTR(oi->opdesc)); break;
	case OT_XORI: F = 0; *REGPTR(oi->opdesc) ^= IPVB; setflags(*REGPTR(oi->opdesc)); break;
	case OT_CMP:  itmp = *REGPTR(oi->opdesc) - A; setbit(FZ, itmp == 0); setbit(FN, itmp < 0); break;
	case OT_CMPP: itmp = *HLPTR - A; setbit(FZ, itmp == 0); setbit(FN, itmp < 0); break;
	case OT_CMPI: itmp = A - IPVB; setbit(FZ, itmp == 0); setbit(FN, itmp < 0); break;
	case OT_CMPS: setbit(FZ, A==*REGPTR(oi->opdesc)); setbit(FN, *(signed char*)REGPTR(oi->opdesc) < (signed char)A); break;
	case OT_CMPSP:setbit(FZ, A==*HLPTR); setbit(FN, *(signed char*)HLPTR < (signed char)A); break;
	case OT_SIN:  A = fgetc(stdin); break;
	case OT_SOUT:
	fputc(A, stdout);
dprintf(2, ">>> "); fputc(A, stderr); dprintf(2, "\n"); fflush(stdout); break;
	case OT_CLRSCR: clrscr(); break;
	case OT_DRAW: ; draw(C, B, A); break;
	case OT_JMP16:  regs.pc = IPVW; inc = 0; break;
	case OT_JMP16C:
		tmp = oi->opdesc & ~FNOT;
		if(((oi->opdesc & FNOT) && !(F & tmp))
		|| (!(oi->opdesc & FNOT) && (F & tmp))) {
			regs.pc = IPVW;
			inc = 0;
		}
		break;
	case OT_JMP8: regs.pc += (signed char)IPVB; break;
	case OT_JMP8C:
		tmp = oi->opdesc & ~FNOT;
		if(((oi->opdesc & FNOT) && !(F & tmp))
		|| (!(oi->opdesc & FNOT) && (F & tmp)))
			regs.pc += (signed char)IPVB;
		break;
	case OT_CALL: pushw(regs.pc+inc); regs.pc = IPVW; inc = 0; break;
	case OT_RET: regs.pc = popw(); inc = 0; break;
	default: abort();
	}
	regs.pc += inc;
	if(debug) dprintf(2, "AFT: "); state();
	return 1;
}

static void run() {
	unsigned int rnd = 0x65, i;
	for(i=0;i<7;++i) regs.gpr[i] = rnd ^ i;
	//state();
	while(step());
}

char *slurp(FILE *f, unsigned *len) {
	fseek(f, 0, SEEK_END);
	*len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *mem = malloc(*len+1);
	fread(mem, 1, *len, f);
	mem[*len] = 0;
	fclose(f);
	return mem;
}
int main() {
	FILE *f;
	f = fopen("bootrom", "r");
	bootrom = slurp(f, &lbootrom);
	f = fopen("rom", "r");
	rom = slurp(f, &lrom);
	memcpy(mem, bootrom, lbootrom);
	memcpy(mem + 0x597, rom, lrom);
	ezsdl_init(vmem_w, vmem_h, SDL_HWPALETTE);
	run();
}
