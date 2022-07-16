/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:kdb/tbls.c	1.3"
/* @(#)tables.c	10.2 */

#include	"dis.h"

#define		INVALID	{"",TERM,UNKNOWN,0}

/*
 *	In 16-bit addressing mode:
 *	Register operands may be indicated by a distinguished field.
 *	An '8' bit register is selected if the 'w' bit is equal to 0,
 *	and a '16' bit register is selected if the 'w' bit is equal to
 *	1 and also if there is no 'w' bit.
 */

char	*REG16[16] = {

/* w bit		0		1		*/

/* reg bits */
/* 000	*/		"%al",		"%ax",
/* 001  */		"%cl",		"%cx",
/* 010  */		"%dl",		"%dx",
/* 011	*/		"%bl",		"%bx",
/* 100	*/		"%ah",		"%sp",
/* 101	*/		"%ch",		"%bp",
/* 110	*/		"%dh",		"%si",
/* 111	*/		"%bh",		"%di",

};

/*
 *	In 32-bit addressing mode:
 *	Register operands may be indicated by a distinguished field.
 *	An '8' bit register is selected if the 'w' bit is equal to 0,
 *	and a '32' bit register is selected if the 'w' bit is equal to
 *	1 and also if there is no 'w' bit.
 */

char	*REG32[16] = {

/* w bit		0		1		*/

/* reg bits */
/* 000	*/		"%al",		"%eax",
/* 001  */		"%cl",		"%ecx",
/* 010  */		"%dl",		"%edx",
/* 011	*/		"%bl",		"%ebx",
/* 100	*/		"%ah",		"%esp",
/* 101	*/		"%ch",		"%ebp",
/* 110	*/		"%dh",		"%esi",
/* 111	*/		"%bh",		"%edi",

};


/*
 *	In 16-bit mode:
 *	This initialized array will be indexed by the 'r/m' and 'mod'
 *	fields, to determine the size of the displacement in each mode.
 */

char dispsize16 [8][4] = {
/* mod		00	01	10	11 */
/* r/m */
/* 000 */	0,	1,	2,	0,
/* 001 */	0,	1,	2,	0,
/* 010 */	0,	1,	2,	0,
/* 011 */	0,	1,	2,	0,
/* 100 */	0,	1,	2,	0,
/* 101 */	0,	1,	2,	0,
/* 110 */	2,	1,	2,	0,
/* 111 */	0,	1,	2,	0
};


/*
 *	In 32-bit mode:
 *	This initialized array will be indexed by the 'r/m' and 'mod'
 *	fields, to determine the size of the displacement in this mode.
 */

char dispsize32 [8][4] = {
/* mod		00	01	10	11 */
/* r/m */
/* 000 */	0,	1,	4,	0,
/* 001 */	0,	1,	4,	0,
/* 010 */	0,	1,	4,	0,
/* 011 */	0,	1,	4,	0,
/* 100 */	0,	1,	4,	0,
/* 101 */	4,	1,	4,	0,
/* 110 */	0,	1,	4,	0,
/* 111 */	0,	1,	4,	0
};


/*
 *	When data16 has been specified,
 * the following array specifies the registers for the different addressing modes.
 * Indexed first by mode, then by register number.
 */

char *regname16[4][8] = {
/*reg  000        001        010        011        100    101   110     111 */
/*mod*/
/*00*/ "%bx,%si", "%bx,%di", "%bp,%si", "%bp,%di", "%si", "%di", "",    "%bx",
/*01*/ "%bx,%si", "%bx,%di", "%bp,%si", "%bp,%di", "%si", "%di", "%bp", "%bx",
/*10*/ "%bx,%si", "%bx,%di", "%bp,%si", "%bp,%di", "%si", "%di", "%bp", "%bx",
/*11*/ "%ax",     "%cx",     "%dx",     "%bx",     "%sp", "%bp", "%si", "%di"
};


/*
 *	When data16 has not been specified,
 * 
 *	fields, to determine the addressing mode, and will also provide
 *	strings for printing.
 */

char *regname32[4][8] = {
/*reg   000       001       010       011       100       101       110       111 */
/*mod*/
/*00 */ "%eax", "%ecx", "%edx", "%ebx", "%esp", "",     "%esi", "%edi",
/*01 */ "%eax", "%ecx", "%edx", "%ebx", "%esp", "%ebp", "%esi", "%edi",
/*10 */ "%eax", "%ecx", "%edx", "%ebx", "%esp", "%ebp", "%esi", "%edi",
/*11 */ "%eax", "%ecx", "%edx", "%ebx", "%esp", "%ebp", "%esi", "%edi"
};

/*
 *	If r/m==100 then the following byte (the s-i-b byte) must be decoded
 */

char *scale_factor[4] = {
	"1",
	"2",
	"4",
	"8"
};

char *indexname[8] = {
	",%eax",
	",%ecx",
	",%edx",
	",%ebx",
	"",
	",%ebp",
	",%esi",
	",%edi"
};

/* For communication to locsympr */
char **regname;

/*
 *	Segment registers are selected by a two or three bit field.
 */

char	*SEGREG[6] = {

/* 000 */	"%es",
/* 001 */	"%cs",
/* 010 */	"%ss",
/* 011 */	"%ds",
/* 100 */	"%fs",
/* 101 */	"%gs",

};

/*
 * Special Registers
 */

char *DEBUGREG[8] = {
	"%db0", "%db1", "%db2", "%db3", "%db4", "%db5", "%db6", "%db7"
};

char *CONTROLREG[8] = {
	"%cr0", "%cr1", "%cr2", "%cr3", "%cr4?", "%cr5?", "%cr6?", "%cr7?"
};

char *TESTREG[8] = {
	"%tr0?", "%tr1?", "%tr2?", "%tr3?", "%tr4?", "%tr5?", "%tr6", "%tr7"
};

/*
 *	Decode table for 0x0F00 opcodes
 */

struct instable op0F00[8] = {

/*  [0]  */	{"sldt",TERM,M,0},	{"str",TERM,M,0},	{"lldt",TERM,M,0},	{"ltr",TERM,M,0},
/*  [4]  */	{"verr",TERM,M,0},	{"verw",TERM,M,0},	INVALID,		INVALID,
};


/*
 *	Decode table for 0x0F01 opcodes
 */

struct instable op0F01[8] = {

/*  [0]  */	{"sgdt",TERM,M,0},	{"sidt",TERM,M,0},	{"lgdt",TERM,M,0},	{"lidt",TERM,M,0},
/*  [4]  */	{"smsw",TERM,M,0},	INVALID,		{"lmsw",TERM,M,0},	INVALID,
};


/*
 *	Decode table for 0x0FBA opcodes
 */

struct instable op0FBA[8] = {

/*  [0]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [4]  */	{"bt",TERM,MIb,1},	{"bts",TERM,MIb,1},	{"btr",TERM,MIb,1},	{"btc",TERM,MIb,1},
};


/*
 *	Decode table for 0x0F opcodes
 */

struct instable op0F[192] = {

/*  [00]  */	{"",op0F00,TERM,0},	{"",op0F01,TERM,0},	{"lar",TERM,MR,0},	{"lsl",TERM,MR,0},
/*  [04]  */	INVALID,		INVALID,		{"clts",TERM,GO_ON,0},	INVALID,
/*  [08]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [0C]  */	INVALID,		INVALID,		INVALID,		INVALID,

/*  [10]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [14]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [18]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [1C]  */	INVALID,		INVALID,		INVALID,		INVALID,

/*  [20]  */	{"mov",TERM,SREG,1},	{"mov",TERM,SREG,1},	{"mov",TERM,SREG,1},	{"mov",TERM,SREG,1},
/*  [24]  */	{"mov",TERM,SREG,1},	INVALID,		{"mov",TERM,SREG,1},	INVALID,
/*  [28]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [2C]  */	INVALID,		INVALID,		INVALID,		INVALID,

/*  [30]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [34]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [38]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [3C]  */	INVALID,		INVALID,		INVALID,		INVALID,

/*  [40]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [44]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [48]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [4C]  */	INVALID,		INVALID,		INVALID,		INVALID,

/*  [50]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [54]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [58]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [5C]  */	INVALID,		INVALID,		INVALID,		INVALID,

/*  [60]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [64]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [68]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [6C]  */	INVALID,		INVALID,		INVALID,		INVALID,

/*  [70]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [74]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [78]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [7C]  */	INVALID,		INVALID,		INVALID,		INVALID,

/*  [80]  */	{"jo",TERM,D,0},	{"jno",TERM,D,0},	{"jb",TERM,D,0},	{"jae",TERM,D,0},
/*  [84]  */	{"je",TERM,D,0},	{"jne",TERM,D,0},	{"jbe",TERM,D,0},	{"ja",TERM,D,0},
/*  [88]  */	{"js",TERM,D,0},	{"jns",TERM,D,0},	{"jp",TERM,D,0},	{"jnp",TERM,D,0},
/*  [8C]  */	{"jl",TERM,D,0},	{"jge",TERM,D,0},	{"jle",TERM,D,0},	{"jg",TERM,D,0},

/*  [90]  */	{"seto",TERM,M,0},	{"setno",TERM,M,0},	{"setb",TERM,M,0},	{"setae",TERM,M,0},
/*  [94]  */	{"sete",TERM,M,0},	{"setne",TERM,M,0},	{"setbe",TERM,M,0},	{"seta",TERM,M,0},
/*  [98]  */	{"sets",TERM,M,0},	{"setns",TERM,M,0},	{"setp",TERM,M,0},	{"setnp",TERM,M,0},
/*  [9C]  */	{"setl",TERM,M,0},	{"setge",TERM,M,0},	{"setle",TERM,M,0},	{"setg",TERM,M,0},

/*  [A0]  */	{"push",TERM,LSEG,1},	{"pop",TERM,LSEG,1},	INVALID,		{"bt",TERM,RMw,1},
/*  [A4]  */	{"shld",TERM,DSHIFT,1},	{"shld",TERM,DSHIFTcl,1},	INVALID,		INVALID,
/*  [A8]  */	{"push",TERM,LSEG,1},	{"pop",TERM,LSEG,1},	INVALID,		{"bts",TERM,RMw,1},
/*  [AC]  */	{"shrd",TERM,DSHIFT,1},	{"shrd",TERM,DSHIFTcl,1},	INVALID,		{"imul",TERM,MRw,1},

/*  [B0]  */	INVALID,		INVALID,		{"lss",TERM,MR,0},	{"btr",TERM,RMw,1},
/*  [B4]  */	{"lfs",TERM,MR,0},	{"lgs",TERM,MR,0},	{"movzb",TERM,MOVZ,1},	{"movzwl",TERM,MOVZ,0},
/*  [B8]  */	INVALID,		INVALID,		{"",op0FBA,TERM,0},	{"btc",TERM,RMw,1},
/*  [BC]  */	{"bsf",TERM,MRw,1},	{"bsr",TERM,MRw,1},	{"movsb",TERM,MOVZ,1},	{"movswl",TERM,MOVZ,0},
};


/*
 *	Decode table for 0x80 opcodes
 */

struct instable op80[8] = {

/*  [0]  */	{"addb",TERM,IMlw,0},	{"orb",TERM,IMw,0},	{"adcb",TERM,IMlw,0},	{"sbbb",TERM,IMlw,0},
/*  [4]  */	{"andb",TERM,IMw,0},	{"subb",TERM,IMlw,0},	{"xorb",TERM,IMw,0},	{"cmpb",TERM,IMlw,0},
};


/*
 *	Decode table for 0x81 opcodes.
 */

struct instable op81[8] = {

/*  [0]  */	{"add",TERM,IMlw,1},	{"or",TERM,IMw,1},	{"adc",TERM,IMlw,1},	{"sbb",TERM,IMlw,1},
/*  [4]  */	{"and",TERM,IMw,1},	{"sub",TERM,IMlw,1},	{"xor",TERM,IMw,1},	{"cmp",TERM,IMlw,1},
};


/*
 *	Decode table for 0x82 opcodes.
 */

struct instable op82[8] = {

/*  [0]  */	{"addb",TERM,IMlw,0},	INVALID,		{"adcb",TERM,IMlw,0},	{"sbbb",TERM,IMlw,0},
/*  [4]  */	INVALID,		{"subb",TERM,IMlw,0},	INVALID,		{"cmpb",TERM,IMlw,0},
};
/*
 *	Decode table for 0x83 opcodes.
 */

struct instable op83[8] = {

/*  [0]  */	{"add",TERM,IMlw,1},	INVALID,		{"adc",TERM,IMlw,1},	{"sbb",TERM,IMlw,1},
/*  [4]  */	INVALID,		{"sub",TERM,IMlw,1},	INVALID,		{"cmp",TERM,IMlw,1},
};

/*
 *	Decode table for 0xC0 opcodes.
 */

struct instable opC0[8] = {

/*  [0]  */	{"rolb",TERM,MvI,0},	{"rorb",TERM,MvI,0},	{"rclb",TERM,MvI,0},	{"rcrb",TERM,MvI,0},
/*  [4]  */	{"shlb",TERM,MvI,0},	{"shrb",TERM,MvI,0},	INVALID,		{"sarb",TERM,MvI,0},
};

/*
 *	Decode table for 0xD0 opcodes.
 */

struct instable opD0[8] = {

/*  [0]  */	{"rolb",TERM,Mv,0},	{"rorb",TERM,Mv,0},	{"rclb",TERM,Mv,0},	{"rcrb",TERM,Mv,0},
/*  [4]  */	{"shlb",TERM,Mv,0},	{"shrb",TERM,Mv,0},	INVALID,		{"sarb",TERM,Mv,0},
};

/*
 *	Decode table for 0xC1 opcodes.
 *	186 instruction set
 */

struct instable opC1[8] = {

/*  [0]  */	{"rol",TERM,MvI,1},	{"ror",TERM,MvI,1},	{"rcl",TERM,MvI,1},	{"rcr",TERM,MvI,1},
/*  [4]  */	{"shl",TERM,MvI,1},	{"shr",TERM,MvI,1},	INVALID,		{"sar",TERM,MvI,1},
};

/*
 *	Decode table for 0xD1 opcodes.
 */

struct instable opD1[8] = {

/*  [0]  */	{"rol",TERM,Mv,1},	{"ror",TERM,Mv,1},	{"rcl",TERM,Mv,1},	{"rcr",TERM,Mv,1},
/*  [4]  */	{"shl",TERM,Mv,1},	{"shr",TERM,Mv,1},	INVALID,		{"sar",TERM,Mv,1},
};


/*
 *	Decode table for 0xD2 opcodes.
 */

struct instable opD2[8] = {

/*  [0]  */	{"rolb",TERM,Mv,0},	{"rorb",TERM,Mv,0},	{"rclb",TERM,Mv,0},	{"rcrb",TERM,Mv,0},
/*  [4]  */	{"shlb",TERM,Mv,0},	{"shrb",TERM,Mv,0},	INVALID,		{"sarb",TERM,Mv,0},
};
/*
 *	Decode table for 0xD3 opcodes.
 */

struct instable opD3[8] = {

/*  [0]  */	{"rol",TERM,Mv,1},	{"ror",TERM,Mv,1},	{"rcl",TERM,Mv,1},	{"rcr",TERM,Mv,1},
/*  [4]  */	{"shl",TERM,Mv,1},	{"shr",TERM,Mv,1},	INVALID,		{"sar",TERM,Mv,1},
};


/*
 *	Decode table for 0xF6 opcodes.
 */

struct instable opF6[8] = {

/*  [0]  */	{"testb",TERM,IMw,0},	INVALID,		{"notb",TERM,Mw,0},	{"negb",TERM,Mw,0},
/*  [4]  */	{"mulb",TERM,MA,0},	{"imulb",TERM,MA,0},	{"divb",TERM,MA,0},	{"idivb",TERM,MA,0},
};


/*
 *	Decode table for 0xF7 opcodes.
 */

struct instable opF7[8] = {

/*  [0]  */	{"test",TERM,IMw,1},	INVALID,		{"not",TERM,Mw,1},	{"neg",TERM,Mw,1},
/*  [4]  */	{"mul",TERM,MA,1},	{"imul",TERM,MA,1},	{"div",TERM,MA,1},	{"idiv",TERM,MA,1},
};


/*
 *	Decode table for 0xFE opcodes.
 */

struct instable opFE[8] = {

/*  [0]  */	{"incb",TERM,Mw,0},	{"decb",TERM,Mw,0},	INVALID,		INVALID,
/*  [4]  */	INVALID,		INVALID,		INVALID,		INVALID,
};
/*
 *	Decode table for 0xFF opcodes.
 */

struct instable opFF[8] = {

/*  [0]  */	{"inc",TERM,Mw,1},	{"dec",TERM,Mw,1},	{"call",TERM,INM,0},	{"lcall",TERM,INM,0},
/*  [4]  */	{"jmp",TERM,INM,0},	{"ljmp",TERM,INM,0},	{"push",TERM,M,1},	INVALID,
};

/* for 287 instructions, which are a mess to decode */

struct instable opFP1n2[64] = {
/* bit pattern:	1101 1xxx MODxx xR/M */
/*  [0,0] */	{"fadds",TERM,M,0},	{"fmuls",TERM,M,0},	{"fcoms",TERM,M,0},	{"fcomps",TERM,M,0},
/*  [0,4] */	{"fsubs",TERM,M,0},	{"fsubrs",TERM,M,0},	{"fdivs",TERM,M,0},	{"fdivrs",TERM,M,0},
/*  [1,0]  */	{"flds",TERM,M,0},	INVALID,		{"fsts",TERM,M,0},	{"fstps",TERM,M,0},
/*  [1,4]  */	{"fldenv",TERM,M,0},	{"fldcw",TERM,M,0},	{"fnstenv",TERM,M,0},	{"fnstcw",TERM,M,0},
/*  [2,0]  */	{"fiaddl",TERM,M,0},	{"fimull",TERM,M,0},	{"ficoml",TERM,M,0},	{"ficompl",TERM,M,0},
/*  [2,4]  */	{"fisubl",TERM,M,0},	{"fisubrl",TERM,M,0},	{"fidivl",TERM,M,0},	{"fidivrl",TERM,M,0},
/*  [3,0]  */	{"fildl",TERM,M,0},	INVALID,		{"fistl",TERM,M,0},	{"fistpl",TERM,M,0},
/*  [3,4]  */	INVALID,		{"fldt",TERM,M,0},	INVALID,		{"fstpt",TERM,M,0},
/*  [4,0]  */	{"faddl",TERM,M,0},	{"fmull",TERM,M,0},	{"fcoml",TERM,M,0},	{"fcompl",TERM,M,0},
/*  [4,1]  */	{"fsubl",TERM,M,0},	{"fsubrl",TERM,M,0},	{"fdivl",TERM,M,0},	{"fdivrl",TERM,M,0},
/*  [5,0]  */	{"fldl",TERM,M,0},	INVALID,		{"fstl",TERM,M,0},	{"fstpl",TERM,M,0},
/*  [5,4]  */	{"frstor",TERM,M,0},	INVALID,		{"fnsave",TERM,M,0},	{"fnstsw",TERM,M,0},
/*  [6,0]  */	{"fiadd",TERM,M,0},	{"fimul",TERM,M,0},	{"ficom",TERM,M,0},	{"ficomp",TERM,M,0},
/*  [6,4]  */	{"fisub",TERM,M,0},	{"fisubr",TERM,M,0},	{"fidiv",TERM,M,0},	{"fidivr",TERM,M,0},
/*  [7,0]  */	{"fild",TERM,M,0},	INVALID,		{"fist",TERM,M,0},	{"fistp",TERM,M,0},
/*  [7,4]  */	{"fbld",TERM,M,0},	{"fildll",TERM,M,0},	{"fbstp",TERM,M,0},	{"fistpll",TERM,M,0},
};

struct instable opFP3[64] = {
/* bit  pattern:	1101 1xxx 11xx xREG */
/*  [0,0]  */	{"fadd",TERM,FF,0},	{"fmul",TERM,FF,0},	{"fcom",TERM,F,0},	{"fcomp",TERM,F,0},
/*  [0,4]  */	{"fsub",TERM,FF,0},	{"fsubr",TERM,FF,0},	{"fdiv",TERM,FF,0},	{"fdivr",TERM,FF,0},
/*  [1,0]  */	{"fld",TERM,F,0},	{"fxch",TERM,F,0},	{"fnop",TERM,GO_ON,0},	{"fstp",TERM,F,0},
/*  [1,4]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [2,0]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [2,4]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [3,0]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [3,4]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [4,0]  */	{"fadd",TERM,FF,0},	{"fmul",TERM,FF,0},	{"fcom",TERM,F,0},	{"fcomp",TERM,F,0},
/*  [4,4]  */	{"fsub",TERM,FF,0},	{"fsubr",TERM,FF,0},	{"fdiv",TERM,FF,0},	{"fdivr",TERM,FF,0},
/*  [5,0]  */	{"ffree",TERM,F,0},	{"fxch",TERM,F,0},	{"fst",TERM,F,0},	{"fstp",TERM,F,0},
/*  [5,4]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [6,0]  */	{"faddp",TERM,FF,0},	{"fmulp",TERM,FF,0},	{"fcomp",TERM,F,0},	{"fcompp",TERM,GO_ON,0},
/*  [6,4]  */	{"fsubp",TERM,FF,0},	{"fsubrp",TERM,FF,0},	{"fdivp",TERM,FF,0},	{"fdivrp",TERM,FF,0},
/*  [7,0]  */	{"ffree",TERM,F,0},	{"fxch",TERM,F,0},	{"fstp",TERM,F,0},	{"fstp",TERM,F,0},
/*  [7,4]  */	{"fstsw",TERM,M,0},	INVALID,		INVALID,		INVALID,
};

struct instable opFP4[32] = {
/* bit pattern:	1101 1001 111x xxxx */
/*  [0,0]  */	{"fchs",TERM,GO_ON,0},	{"fabs",TERM,GO_ON,0},	INVALID,		INVALID,
/*  [0,4]  */	{"ftst",TERM,GO_ON,0},	{"fxam",TERM,GO_ON,0},	INVALID,		INVALID,
/*  [1,0]  */	{"fld1",TERM,GO_ON,0},	{"fldl2t",TERM,GO_ON,0},{"fldl2e",TERM,GO_ON,0},{"fldpi",TERM,GO_ON,0},
/*  [1,4]  */	{"fldlg2",TERM,GO_ON,0},{"fldln2",TERM,GO_ON,0},{"fldz",TERM,GO_ON,0},	INVALID,
/*  [2,0]  */	{"f2xm1",TERM,GO_ON,0},	{"fyl2x",TERM,GO_ON,0},	{"fptan",TERM,GO_ON,0},	{"fpatan",TERM,GO_ON,0},
/*  [2,4]  */	{"fxtract",TERM,GO_ON,0},INVALID,		{"fdecstp",TERM,GO_ON,0},{"fincstp",TERM,GO_ON,0},
/*  [3,0]  */	{"fprem",TERM,GO_ON,0},	{"fyl2xp1",TERM,GO_ON,0},{"fsqrt",TERM,GO_ON,0},INVALID,
/*  [3,4]  */	{"frndint",TERM,GO_ON,0},{"fscale",TERM,GO_ON,0},INVALID,		INVALID,
};

struct instable opFP5[8] = {
/* bit pattern:	1101 1011 1110 0xxx */
/*  [0]  */	INVALID,		INVALID,		{"fnclex",TERM,GO_ON,0},{"fninit",TERM,GO_ON,0},
/*  [4]  */	{"fsetpm",TERM,GO_ON,0},INVALID,		INVALID,		INVALID,
};

/*
 *	Main decode table for the op codes.  The first two nibbles
 *	will be used as an index into the table.  If there is a
 *	a need to further decode an instruction, the array to be
 *	referenced is indicated with the other two entries being
 *	empty.
 */

struct instable distable[256] = {

/* [0,0] */	{"addb",TERM,RMw,0},	{"add",TERM,RMw,1},	{"addb",TERM,MRw,0},	{"add",TERM,MRw,1},
/* [0,4] */	{"addb",TERM,IA,0},	{"add",TERM,IA,1},	{"push",TERM,SEG,1},	{"pop",TERM,SEG,1},
/* [0,8] */	{"orb",TERM,RMw,0},	{"or",TERM,RMw,1},	{"orb",TERM,MRw,0},	{"or",TERM,MRw,1},
/* [0,C] */	{"orb",TERM,IA,0},	{"or",TERM,IA,1},	{"push",TERM,SEG,1},	{"",op0F,TERM,0},

/* [1,0] */	{"adcb",TERM,RMw,0},	{"adc",TERM,RMw,1},	{"adcb",TERM,MRw,0},	{"adc",TERM,MRw,1},
/* [1,4] */	{"adcb",TERM,IA,0},	{"adc",TERM,IA,1},	{"push",TERM,SEG,1},	{"pop",TERM,SEG,1},
/* [1,8] */	{"sbbb",TERM,RMw,0},	{"sbb",TERM,RMw,1},	{"sbbb",TERM,MRw,0},	{"sbb",TERM,MRw,1},
/* [1,C] */	{"sbbb",TERM,IA,0},	{"sbb",TERM,IA,1},	{"push",TERM,SEG,1},	{"pop",TERM,SEG,1},

/* [2,0] */	{"andb",TERM,RMw,0},	{"and",TERM,RMw,1},	{"andb",TERM,MRw,0},	{"and",TERM,MRw,1},
/* [2,4] */	{"andb",TERM,IA,0},	{"and",TERM,IA,1},	{"%es:",TERM,OVERRIDE,0},{"daa",TERM,GO_ON,0},
/* [2,8] */	{"subb",TERM,RMw,0},	{"sub",TERM,RMw,1},	{"subb",TERM,MRw,0},	{"sub",TERM,MRw,1},
/* [2,C] */	{"subb",TERM,IA,0},	{"sub",TERM,IA,1},	{"%cs:",TERM,OVERRIDE,0},{"das",TERM,GO_ON,0},

/* [3,0] */	{"xorb",TERM,RMw,0},	{"xor",TERM,RMw,1},	{"xorb",TERM,MRw,0},	{"xor",TERM,MRw,1},
/* [3,4] */	{"xorb",TERM,IA,0},	{"xor",TERM,IA,1},	{"%ss:",TERM,OVERRIDE,0},{"aaa",TERM,GO_ON,0},
/* [3,8] */	{"cmpb",TERM,RMw,0},	{"cmp",TERM,RMw,1},	{"cmpb",TERM,MRw,0},	{"cmp",TERM,MRw,1},
/* [3,C] */	{"cmpb",TERM,IA,0},	{"cmp",TERM,IA,1},	{"%ds:",TERM,OVERRIDE,0},{"aas",TERM,GO_ON,0},

/* [4,0] */	{"inc",TERM,R,1},	{"inc",TERM,R,1},	{"inc",TERM,R,1},	{"inc",TERM,R,1},
/* [4,4] */	{"inc",TERM,R,1},	{"inc",TERM,R,1},	{"inc",TERM,R,1},	{"inc",TERM,R,1},
/* [4,8] */	{"dec",TERM,R,1},	{"dec",TERM,R,1},	{"dec",TERM,R,1},	{"dec",TERM,R,1},
/* [4,C] */	{"dec",TERM,R,1},	{"dec",TERM,R,1},	{"dec",TERM,R,1},	{"dec",TERM,R,1},

/* [5,0] */	{"push",TERM,R,1},	{"push",TERM,R,1},	{"push",TERM,R,1},	{"push",TERM,R,1},
/* [5,4] */	{"push",TERM,R,1},	{"push",TERM,R,1},	{"push",TERM,R,1},	{"push",TERM,R,1},
/* [5,8] */	{"pop",TERM,R,1},	{"pop",TERM,R,1},	{"pop",TERM,R,1},	{"pop",TERM,R,1},
/* [5,C] */	{"pop",TERM,R,1},	{"pop",TERM,R,1},	{"pop",TERM,R,1},	{"pop",TERM,R,1},

/* [6,0] */	{"pusha",TERM,GO_ON,1},	{"popa",TERM,GO_ON,1},	{"bound",TERM,MR,1},	{"arpl",TERM,RMw,0},
/* [6,4] */	{"%fs:",TERM,OVERRIDE,0},{"%gs:",TERM,OVERRIDE,0},{"data16",TERM,DM,0},	{"addr16",TERM,AM,0},
/* [6,8] */	{"push",TERM,I,1},	{"imul",TERM,IMUL,1},	{"push",TERM,Ib,1},	{"imul",TERM,IMUL,1},
/* [6,C] */	{"insb",TERM,GO_ON,0},	{"ins",TERM,GO_ON,1},	{"outsb",TERM,GO_ON,0},	{"outs",TERM,GO_ON,1},

/* [7,0] */	{"jo",TERM,BD,0},	{"jno",TERM,BD,0},	{"jb",TERM,BD,0},	{"jae",TERM,BD,0},
/* [7,4] */	{"je",TERM,BD,0},	{"jne",TERM,BD,0},	{"jbe",TERM,BD,0},	{"ja",TERM,BD,0},
/* [7,8] */	{"js",TERM,BD,0},	{"jns",TERM,BD,0},	{"jp",TERM,BD,0},	{"jnp",TERM,BD,0},
/* [7,C] */	{"jl",TERM,BD,0},	{"jge",TERM,BD,0},	{"jle",TERM,BD,0},	{"jg",TERM,BD,0},
/* [8,0] */	{"",op80,TERM,0},	{"",op81,TERM,0},	{"",op82,TERM,0},	{"",op83,TERM,0},
/* [8,4] */	{"testb",TERM,MRw,0},	{"test",TERM,MRw,1},	{"xchgb",TERM,MRw,0},	{"xchg",TERM,MRw,1},
/* [8,8] */	{"movb",TERM,RMw,0},	{"mov",TERM,RMw,1},	{"movb",TERM,MRw,0},	{"mov",TERM,MRw,1},
/* [8,C] */	{"mov",TERM,SM,1},	{"lea",TERM,MR,1},	{"mov",TERM,MS,1},	{"pop",TERM,M,1},

/* [9,0] */	{"nop",TERM,GO_ON,0},	{"xchg",TERM,RA,1},	{"xchg",TERM,RA,1},	{"xchg",TERM,RA,1},
/* [9,4] */	{"xchg",TERM,RA,1},	{"xchg",TERM,RA,1},	{"xchg",TERM,RA,1},	{"xchg",TERM,RA,1},
/* [9,8] */	{"",TERM,CBW,0},	{"",TERM,CWD,0},	{"lcall",TERM,SO,0},	{"fwait",TERM,GO_ON,0},
/* [9,C] */	{"pushf",TERM,GO_ON,1},	{"popf",TERM,GO_ON,1},	{"sahf",TERM,GO_ON,0},	{"lahf",TERM,GO_ON,0},

/* [A,0] */	{"movb",TERM,OA,0},	{"mov",TERM,OA,1},	{"movb",TERM,AO,0},	{"mov",TERM,AO,1},
/* [A,4] */	{"movsb",TERM,SD,0},	{"movs",TERM,SD,1},	{"cmpsb",TERM,SD,0},	{"cmps",TERM,SD,1},
/* [A,8] */	{"testb",TERM,IA,0},	{"test",TERM,IA,1},	{"stosb",TERM,AD,0},	{"stos",TERM,AD,1},
/* [A,C] */	{"lodsb",TERM,SA,0},	{"lods",TERM,SA,1},	{"scasb",TERM,AD,0},	{"scas",TERM,AD,1},

/* [B,0] */	{"movb",TERM,IR,0},	{"movb",TERM,IR,0},	{"movb",TERM,IR,0},	{"movb",TERM,IR,0},
/* [B,4] */	{"movb",TERM,IR,0},	{"movb",TERM,IR,0},	{"movb",TERM,IR,0},	{"movb",TERM,IR,0},
/* [B,8] */	{"mov",TERM,IR,1},	{"mov",TERM,IR,1},	{"mov",TERM,IR,1},	{"mov",TERM,IR,1},
/* [B,C] */	{"mov",TERM,IR,1},	{"mov",TERM,IR,1},	{"mov",TERM,IR,1},	{"mov",TERM,IR,1},

/* [C,0] */	{"",opC0,TERM,0},	{"",opC1,TERM,0},	{"ret",TERM,RET,0},	{"ret",TERM,GO_ON,0},
/* [C,4] */	{"les",TERM,MR,0},	{"lds",TERM,MR,0},	{"movb",TERM,IMw,0},	{"mov",TERM,IMw,1},
/* [C,8] */	{"enter",TERM,ENTER,0},	{"leave",TERM,GO_ON,0},	{"lret",TERM,RET,0},	{"lret",TERM,GO_ON,0},
/* [C,C] */	{"int",TERM,INT3,0},	{"int",TERM,Ib,0},	{"into",TERM,GO_ON,0},	{"iret",TERM,GO_ON,0},

/* [D,0] */	{"",opD0,TERM,0},	{"",opD1,TERM,0},	{"",opD2,TERM,0},	{"",opD3,TERM,0},
/* [D,4] */	{"aam",TERM,U,0},	{"aad",TERM,U,0},	{"falc",TERM,GO_ON,0},	{"xlat",TERM,GO_ON,0},

/* 287 instructions.  Note that although the indirect field		*/
/* indicates opFP1n2 for further decoding, this is not necessarily	*/
/* the case since the opFP arrays are not partitioned according to key1	*/
/* and key2.  opFP1n2 is given only to indicate that we haven't		*/
/* finished decoding the instruction.					*/
/* [D,8] */	{"",opFP1n2,TERM,0},	{"",opFP1n2,TERM,0},	{"",opFP1n2,TERM,0},	{"",opFP1n2,TERM,0},
/* [D,C] */	{"",opFP1n2,TERM,0},	{"",opFP1n2,TERM,0},	{"",opFP1n2,TERM,0},	{"",opFP1n2,TERM,0},

/* [E,0] */	{"loopnz",TERM,BD,0},	{"loopz",TERM,BD,0},	{"loop",TERM,BD,0},	{"jcxz",TERM,BD,0},
/* [E,4] */	{"inb",TERM,P,0},	{"in",TERM,P,1},	{"outb",TERM,P,0},	{"out",TERM,P,1},
/* [E,8] */	{"call",TERM,D,0},	{"jmp",TERM,D,0},	{"ljmp",TERM,SO,0},	{"jmp",TERM,BD,0},
/* [E,C] */	{"inb",TERM,V,0},	{"in",TERM,V,1},	{"outb",TERM,V,0},	{"out",TERM,V,1},

/* [F,0] */	{"lock ",TERM,PREFIX,0},	{"",TERM,JTAB,0},	{"repnz ",TERM,PREFIX,0},	{"repz ",TERM,PREFIX,0},
/* [F,4] */	{"hlt",TERM,GO_ON,0},	{"cmc",TERM,GO_ON,0},	{"",opF6,TERM,0},	{"",opF7,TERM,0},
/* [F,8] */	{"clc",TERM,GO_ON,0},	{"stc",TERM,GO_ON,0},	{"cli",TERM,GO_ON,0},	{"sti",TERM,GO_ON,0},
/* [F,C] */	{"cld",TERM,GO_ON,0},	{"std",TERM,GO_ON,0},	{"",opFE,TERM,0},	{"",opFF,TERM,0},
};
