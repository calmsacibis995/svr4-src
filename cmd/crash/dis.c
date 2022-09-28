/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:dis.c	1.5.9.1"

/*
 * This file contains code for the crash function dis.
 * This code is the ported 386 disasm command.  Refer to the original
 * source for extensive comments.
 */
#include	<sys/types.h>
#include	<a.out.h>
#include	<stdio.h>
#include	<sys/param.h>
#include	<sys/sysmacros.h>

#include	"crash.h"
#include	"dis.h"

#define		RM	0	/* in case RMMR, is the value of the	*/
				/* 'd' bit that indicates an RM rather	*/
				/* than MR instruction			*/
#define		TWOlw	1	/* is value in 'l' and 'w' bits that	*/
				/* indicates a 2 byte immediate operand	*/
#define		TWOw	1	/* value of 'w' bit that indicates a	*/
				/* 2 byte operand			*/
#define		REGv	1	/* value of 'v' bit that indicates a	*/
				/* register needs to be included as an	*/
				/* operand				*/
#define		TYPEv	0	/* in case Iv for an interrupt;value of	*/
				/* 'v' bit which indicates the type of	*/
				/* interrupt				*/
#define		TYPE3	3	/* indicates a type of interrupt	*/
#define		DISP_0	0	/* indicates a 0 byte displacement	*/
#define		DISP_1	8	/* indicates a 1 byte displacement	*/
#define		DISP_2	16	/* indicates a 2 byte displacement	*/
#define		DISP_4	32	/* indicates a 4 byte displacement	*/
#define		REG_ONLY 3	/* indicates a single register with	*/
				/* no displacement is an operand	*/
#define		MAXERRS	5	/* maximum # of errors allowed before	*/
				/* abandoning this disassembly as a	*/
				/* hopeless case			*/

#define MASKw(x)	(x & 0x1)		/* to get w bit	*/
#define MASK3w(x)	( (x >> 3) & 0x1)	/* to get w bit from middle */
						/* where form is  'xxxxwxxx' */
#define MASKlw(x)	(x & 0x3)		/* to get 'lw' bits */
#define MASKreg(x)	(x & 0x7)		/* to get 3 bit register */
#define MASKv(x)	( (x >> 1) & 0x1)	/* to get 'v' bit */
#define MASKd(x)	( (x >> 1) & 0x1)	/* to get 'd' bit */
#define MASKseg(x)	( (x >> 3) & 0x3)	/* get seg reg from op code*/
#define MASKlseg(x)	( (x >> 3) & 0x7)	/* get long seg reg from op code*/

#define R_M(x)		(x & 0x7)		/* get r/m field from byte */
						/* after op code */
#define REG(x)		( (x >> 3) & 0x7)	/* get register field from */
						/* byte after op code      */
#define MODE(x)		( (x >> 6) & 0x3)	/* get mode field from byte */
						/* after op code */
#define	LOW4(x)		( x & 0xf)		/* ----xxxx low 4 bits */
#define	BITS7_4(x)	( (x >> 4) & 0xf)	/* xxxx---- bits 7 to 4 */
#define MASKret(x)	( (x >> 2) & 0x1)	/* return result bit
						for 287 instructions	*/
#define ESP		(unsigned) 0x4		/* three-bit code for %esp register */


#define		NONE	0	/* to indicate register only in addrmod array*/
#define		INVALID	{"",TERM,UNKNOWN,0}

/*
 *	In 16-bit addressing mode:
 */

char	*regster_16[8][2] = {
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
 */

char	*regster_32[8][2] = {
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
 */

struct addr	addrmod_16 [8][4] = {
/* mod		00			01			10			11 */
/* r/m */
/* 000 */	{{0,"(%bx,%si)"},	{8,"(%bx,%si)"},	{16,"(%bx,%si)"},	{NONE,"%ax"}},
/* 001 */	{{0,"(%bx,%di)"},	{8,"(%bx,%di)"},	{16,"(%bx,%di)"},	{NONE,"%cx"}},
/* 010 */	{{0,"(%bp,%si)"},	{8,"(%bp,%si)"},	{16,"(%bp,%si)"},	{NONE,"%dx"}},
/* 011 */	{{0,"(%bp,%di)"},	{8,"(%bp,%di)"},	{16,"(%bp,%di)"},	{NONE,"%bx"}},
/* 100 */	{{0,"(%si)"},		{8,"(%si)"},		{16,"(%si)"},		{NONE,"%sp"}},
/* 101 */	{{0,"(%di)"},		{8,"(%di)"},		{16,"(%di)"},		{NONE,"%bp"}},
/* 110 */	{{16,""},		{8,"(%bp)"},		{16,"(%bp)"},		{NONE,"%si"}},
/* 111 */	{{0,"(%bx)"},		{8,"(%bx)"},		{16,"(%bx)"},		{NONE,"%di"}},
};

/*
 *	In 32-bit mode:
 */

struct addr	addrmod_32 [8][4] = {
/* mod		00			01			10			11 */
/* r/m */
/* 000 */	{{0,"(%eax)"},		{8,"(%eax)"},		{32,"(%eax)"},		{NONE,"%eax"}},
/* 001 */	{{0,"(%ecx)"},		{8,"(%ecx)"},		{32,"(%ecx)"},		{NONE,"%ecx"}},
/* 010 */	{{0,"(%edx)"},		{8,"(%edx)"},		{32,"(%edx)"},		{NONE,"%edx"}},
/* 011 */	{{0,"(%ebx)"},		{8,"(%ebx)"},		{32,"(%ebx)"},		{NONE,"%ebx"}},
/* 100 */	{{0,"(%esp)"},		{8,"(%esp)"},		{32,"(%esp)"},		{NONE,"%esp"}},
/* 101 */	{{32,""},		{8,"(%ebp)"},		{32,"(%ebp)"},		{NONE,"%ebp"}},
/* 110 */	{{0,"(%esi)"},		{8,"(%esi)"},		{32,"(%esi)"},		{NONE,"%esi"}},
/* 111 */	{{0,"(%edi)"},		{8,"(%edi)"},		{32,"(%edi)"},		{NONE,"%edi"}},
};

/*
 *	If r/m==100 then the following byte (the s-i-b byte) must be decoded
 */

char *scale_factor[4] = { ",1", ",2", ",4", ",8" };

char *index_reg[8] = {
	",%eax", ",%ecx", ",%edx", ",%ebx",
	"", ",%ebp", ",%esi", ",%edi" };

/*
 *	Segment registers are selected by a two or three bit field.
 */

char	*segreg[6] = {
/* 000 */	"%es", /* 001 */	"%cs",
/* 010 */	"%ss", /* 011 */	"%ds",
/* 100 */	"%fs", /* 101 */	"%gs",
};

/*
 * Special Registers
 */

char *dregster[8] = {
	"%db0", "%db1", "%db2", "%db3", "%db4", "%db5", "%db6", "%db7" };

char *cregster[8] = {
	"%cr0", "%cr1", "%cr2", "%cr3", "%cr4?", "%cr5?", "%cr6?", "%cr7?" };

char *tregster[8] = {
	"%tr0?", "%tr1?", "%tr2?", "%tr3?", "%tr4?", "%tr5?", "%tr6", "%tr7" };

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
/*  [4]  */	{"bt",TERM,MIb,0},	{"bts",TERM,MIb,0},	{"btr",TERM,MIb,0},	{"btc",TERM,MIb,0},
};


/*
 *	Decode table for 0x0F opcodes
 */

struct instable op0F[12][16] = {
/*  [00]  */	{{"",op0F00,TERM,0},	{"",op0F01,TERM,0},	{"lar",TERM,MR,0},	{"lsl",TERM,MR,0},
/*  [04]  */	INVALID,		INVALID,		{"clts",TERM,GO_ON,0},	INVALID,
/*  [08]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [0C]  */	INVALID,		INVALID,		INVALID,		INVALID},

/*  [10]  */	{INVALID,		INVALID,		INVALID,		INVALID,
/*  [14]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [18]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [1C]  */	INVALID,		INVALID,		INVALID,		INVALID},

/*  [20]  */	{{"mov",TERM,SREG,1},	{"mov",TERM,SREG,1},	{"mov",TERM,SREG,1},	{"mov",TERM,SREG,1},
/*  [24]  */	{"mov",TERM,SREG,1},	INVALID,		{"mov",TERM,SREG,1},	INVALID,
/*  [28]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [2C]  */	INVALID,		INVALID,		INVALID,		INVALID},

/*  [30]  */	{INVALID,		INVALID,		INVALID,		INVALID,
/*  [34]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [38]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [3C]  */	INVALID,		INVALID,		INVALID,		INVALID},

/*  [40]  */	{INVALID,		INVALID,		INVALID,		INVALID,
/*  [44]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [48]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [4C]  */	INVALID,		INVALID,		INVALID,		INVALID},

/*  [50]  */	{INVALID,		INVALID,		INVALID,		INVALID,
/*  [54]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [58]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [5C]  */	INVALID,		INVALID,		INVALID,		INVALID},

/*  [60]  */	{INVALID,		INVALID,		INVALID,		INVALID,
/*  [64]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [68]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [6C]  */	INVALID,		INVALID,		INVALID,		INVALID},

/*  [70]  */	{INVALID,		INVALID,		INVALID,		INVALID,
/*  [74]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [78]  */	INVALID,		INVALID,		INVALID,		INVALID,
/*  [7C]  */	INVALID,		INVALID,		INVALID,		INVALID},

/*  [80]  */	{{"jo",TERM,D,0},	{"jno",TERM,D,0},	{"jb",TERM,D,0},	{"jae",TERM,D,0},
/*  [84]  */	{"je",TERM,D,0},	{"jne",TERM,D,0},	{"jbe",TERM,D,0},	{"ja",TERM,D,0},
/*  [88]  */	{"js",TERM,D,0},	{"jns",TERM,D,0},	{"jp",TERM,D,0},	{"jnp",TERM,D,0},
/*  [8C]  */	{"jl",TERM,D,0},	{"jge",TERM,D,0},	{"jle",TERM,D,0},	{"jg",TERM,D,0}},

/*  [90]  */	{{"seto",TERM,M,0},	{"setno",TERM,M,0},	{"setb",TERM,M,0},	{"setae",TERM,M,0},
/*  [94]  */	{"sete",TERM,M,0},	{"setne",TERM,M,0},	{"setbe",TERM,M,0},	{"seta",TERM,M,0},
/*  [98]  */	{"sets",TERM,M,0},	{"setns",TERM,M,0},	{"setp",TERM,M,0},	{"setnp",TERM,M,0},
/*  [9C]  */	{"setl",TERM,M,0},	{"setge",TERM,M,0},	{"setle",TERM,M,0},	{"setg",TERM,M,0}},

/*  [A0]  */	{{"push",TERM,LSEG,1},	{"pop",TERM,LSEG,1},	INVALID,		{"bt",TERM,RMMR,0},
/*  [A4]  */	{"shld",TERM,IMlw,0},	{"shld",TERM,RMMRI,0},	{"xbts",TERM,IMlw,0},	{"ibts",TERM,IMlw,0},
/*  [A8]  */	{"push",TERM,LSEG,1},	{"pop",TERM,LSEG,1},	INVALID,		{"bts",TERM,RMMR,0},
/*  [AC]  */	{"shrd",TERM,RMMRI,0},	{"shrd",TERM,Mv,0},	INVALID,		{"imul",TERM,RMMR,1}},

/*  [B0]  */	{INVALID,		INVALID,		{"lss",TERM,MR,0},	{"btr",TERM,RMMR,0},
/*  [B4]  */	{"lfs",TERM,MR,0},	{"lgs",TERM,MR,0},	{"movzxb",TERM,MOVX,1},	{"movzxwl",TERM,MOVX,0},
/*  [B8]  */	INVALID,		INVALID,		{"",op0FBA,TERM,0},	{"btc",TERM,RMMR,0},
/*  [BC]  */	{"bsf",TERM,RMMR,0},	{"bsr",TERM,RMMR,0},	{"movsxb",TERM,MOVX,1},	{"movsxwl",TERM,MOVX,0}},
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
 *	186 instruction set
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
/*  [0]  */	{"rol",TERM,MvI,0},	{"ror",TERM,MvI,0},	{"rcl",TERM,MvI,0},	{"rcr",TERM,MvI,0},
/*  [4]  */	{"shl",TERM,MvI,1},	{"shr",TERM,MvI,1},	INVALID,		{"sar",TERM,MvI,1},
};

/*
 *	Decode table for 0xD1 opcodes.
 */

struct instable opD1[8] = {
/*  [0]  */	{"rol",TERM,Mv,0},	{"ror",TERM,Mv,0},	{"rcl",TERM,Mv,0},	{"rcr",TERM,Mv,0},
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
/*  [0]  */	{"rol",TERM,Mv,0},	{"ror",TERM,Mv,0},	{"rcl",TERM,Mv,0},	{"rcr",TERM,Mv,0},
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

struct instable opfp1n2[8][8] = {
/* bit pattern:	1101 1xxx MODxx xR/M */
/*  [0,0] */	{{"fadds",TERM,M,0},	{"fmuls",TERM,M,0},	{"fcoms",TERM,M,0},	{"fcomps",TERM,M,0},
/*  [0,4] */	{"fsubs",TERM,M,0},	{"fsubrs",TERM,M,0},	{"fdivs",TERM,M,0},	{"fdivrs",TERM,M,0}},
/*  [1,0]  */	{{"flds",TERM,M,0},	INVALID,		{"fsts",TERM,M,0},	{"fstps",TERM,M,0},
/*  [1,4]  */	{"fldenv",TERM,M,0},	{"fldcw",TERM,M,0},	{"fnstenv",TERM,M,0},	{"fnstcw",TERM,M,0}},
/*  [2,0]  */	{{"fiaddl",TERM,M,0},	{"fimull",TERM,M,0},	{"ficoml",TERM,M,0},	{"ficompl",TERM,M,0},
/*  [2,4]  */	{"fisubl",TERM,M,0},	{"fisubrl",TERM,M,0},	{"fidivl",TERM,M,0},	{"fidivrl",TERM,M,0}},
/*  [3,0]  */	{{"fildl",TERM,M,0},	INVALID,		{"fistl",TERM,M,0},	{"fistpl",TERM,M,0},
/*  [3,4]  */	INVALID,		{"fldt",TERM,M,0},	INVALID,		{"fstpt",TERM,M,0}},
/*  [4,0]  */	{{"faddl",TERM,M,0},	{"fmull",TERM,M,0},	{"fcoml",TERM,M,0},	{"fcompl",TERM,M,0},
/*  [4,1]  */	{"fsubl",TERM,M,0},	{"fsubrl",TERM,M,0},	{"fdivl",TERM,M,0},	{"fdivrl",TERM,M,0}},
/*  [5,0]  */	{{"fldl",TERM,M,0},	INVALID,		{"fstl",TERM,M,0},	{"fstpl",TERM,M,0},
/*  [5,4]  */	{"frstor",TERM,M,0},	INVALID,		{"fnsave",TERM,M,0},	{"fnstsw",TERM,M,0}},
/*  [6,0]  */	{{"fiadd",TERM,M,0},	{"fimul",TERM,M,0},	{"ficom",TERM,M,0},	{"ficomp",TERM,M,0},
/*  [6,4]  */	{"fisub",TERM,M,0},	{"fisubr",TERM,M,0},	{"fidiv",TERM,M,0},	{"fidivr",TERM,M,0}},
/*  [7,0]  */	{{"fild",TERM,M,0},	INVALID,		{"fist",TERM,M,0},	{"fistp",TERM,M,0},
/*  [7,4]  */	{"fbld",TERM,M,0},	{"fildll",TERM,M,0},	{"fbstp",TERM,M,0},	{"fistpl",TERM,M,0}},
};

struct instable opfp3[8][8] = {
/* bit  pattern:	1101 1xxx 11xx xREG */
/*  [0,0]  */	{{"fadd",TERM,FF,0},	{"fmul",TERM,FF,0},	{"fcom",TERM,F,0},	{"fcomp",TERM,F,0},
/*  [0,4]  */	{"fsub",TERM,FF,0},	{"fsubr",TERM,FF,0},	{"fdiv",TERM,FF,0},	{"fdivr",TERM,FF,0}},
/*  [1,0]  */	{{"fld",TERM,F,0},	{"fxch",TERM,F,0},	{"fnop",TERM,GO_ON,0},	{"fstp",TERM,F,0},
/*  [1,4]  */	INVALID,		INVALID,		INVALID,		INVALID},
/*  [2,0]  */	{INVALID,		INVALID,		INVALID,		INVALID,
/*  [2,4]  */	INVALID,		INVALID,		INVALID,		INVALID},
/*  [3,0]  */	{INVALID,		INVALID,		INVALID,		INVALID,
/*  [3,4]  */	INVALID,		INVALID,		INVALID,		INVALID},
/*  [4,0]  */	{{"fadd",TERM,FF,0},	{"fmul",TERM,FF,0},	{"fcom",TERM,F,0},	{"fcomp",TERM,F,0},
/*  [4,4]  */	{"fsub",TERM,FF,0},	{"fsubr",TERM,FF,0},	{"fdiv",TERM,FF,0},	{"fdivr",TERM,FF,0}},
/*  [5,0]  */	{{"ffree",TERM,F,0},	{"fxch",TERM,F,0},	{"fst",TERM,F,0},	{"fstp",TERM,F,0},
/*  [5,4]  */	INVALID,		INVALID,		INVALID,		INVALID},
/*  [6,0]  */	{{"faddp",TERM,FF,0},	{"fmulp",TERM,FF,0},	{"fcomp",TERM,F,0},	{"fcompp",TERM,GO_ON,0},
/*  [6,4]  */	{"fsubp",TERM,FF,0},	{"fsubrp",TERM,FF,0},	{"fdivp",TERM,FF,0},	{"fdivrp",TERM,FF,0}},
/*  [7,0]  */	{{"ffree",TERM,F,0},	{"fxch",TERM,F,0},	{"fstp",TERM,F,0},	{"fstp",TERM,F,0},
/*  [7,4]  */	{"fstsw",TERM,M,0},	INVALID,		INVALID,		INVALID},
};

struct instable opfp4[4][8] = {
/* bit pattern:	1101 1001 111x xxxx */
/*  [0,0]  */	{{"fchs",TERM,GO_ON,0},	{"fabs",TERM,GO_ON,0},	INVALID,		INVALID,
/*  [0,4]  */	{"ftst",TERM,GO_ON,0},	{"fxam",TERM,GO_ON,0},	INVALID,		INVALID},
/*  [1,0]  */	{{"fld1",TERM,GO_ON,0},	{"fldl2t",TERM,GO_ON,0},{"fldl2e",TERM,GO_ON,0},{"fldpi",TERM,GO_ON,0},
/*  [1,4]  */	{"fldlg2",TERM,GO_ON,0},{"fldln2",TERM,GO_ON,0},{"fldz",TERM,GO_ON,0},	INVALID},
/*  [2,0]  */	{{"f2xm1",TERM,GO_ON,0},	{"fyl2x",TERM,GO_ON,0},	{"fptan",TERM,GO_ON,0},	{"fpatan",TERM,GO_ON,0},
/*  [2,4]  */	{"fxtract",TERM,GO_ON,0},INVALID,		{"fdecstp",TERM,GO_ON,0},{"fincstp",TERM,GO_ON,0}},
/*  [3,0]  */	{{"fprem",TERM,GO_ON,0},	{"fyl2xp1",TERM,GO_ON,0},{"fsqrt",TERM,GO_ON,0},INVALID,
/*  [3,4]  */	{"frndint",TERM,GO_ON,0},{"fscale",TERM,GO_ON,0},INVALID,		INVALID},
};

struct instable opfp5[8] = {
/* bit pattern:	1101 1011 1110 0xxx */
/*  [0]  */	INVALID,		INVALID,		{"fnclex",TERM,GO_ON,0},{"fninit",TERM,GO_ON,0},
/*  [4]  */	{"fsetpm",TERM,GO_ON,0},INVALID,		INVALID,		INVALID,
};

/*
 *	Main decode table for the op codes.
 */

struct instable distable[16][16] = {
/* [0,0] */	{{"addb",TERM,RMMR,0},	{"add",TERM,RMMR,1},	{"addb",TERM,RMMR,0},	{"add",TERM,RMMR,1},
/* [0,4] */	{"addb",TERM,IA,0},	{"add",TERM,IA,1},	{"push",TERM,SEG,1},	{"pop",TERM,SEG,1},
/* [0,8] */	{"orb",TERM,RMMR,0},	{"or",TERM,RMMR,1},	{"orb",TERM,RMMR,0},	{"or",TERM,RMMR,1},
/* [0,C] */	{"orb",TERM,IA,0},	{"or",TERM,IA,1},	{"push",TERM,SEG,1},	{"",&op0F[0][0],TERM,0}},

/* [1,0] */	{{"adcb",TERM,RMMR,0},	{"adc",TERM,RMMR,1},	{"adcb",TERM,RMMR,0},	{"adc",TERM,RMMR,1},
/* [1,4] */	{"adcb",TERM,IA,0},	{"adc",TERM,IA,1},	{"push",TERM,SEG,1},	{"pop",TERM,SEG,1},
/* [1,8] */	{"sbbb",TERM,RMMR,0},	{"sbb",TERM,RMMR,1},	{"sbbb",TERM,RMMR,0},	{"sbb",TERM,RMMR,1},
/* [1,C] */	{"sbbb",TERM,IA,0},	{"sbb",TERM,IA,1},	{"push",TERM,SEG,1},	{"pop",TERM,SEG,1}},

/* [2,0] */	{{"andb",TERM,RMMR,0},	{"and",TERM,RMMR,1},	{"andb",TERM,RMMR,0},	{"and",TERM,RMMR,1},
/* [2,4] */	{"andb",TERM,IA,0},	{"and",TERM,IA,1},	{"%es:",TERM,OVERRIDE,0},{"daa",TERM,GO_ON,0},
/* [2,8] */	{"subb",TERM,RMMR,0},	{"sub",TERM,RMMR,1},	{"subb",TERM,RMMR,0},	{"sub",TERM,RMMR,1},
/* [2,C] */	{"subb",TERM,IA,0},	{"sub",TERM,IA,1},	{"%cs:",TERM,OVERRIDE,0},{"das",TERM,GO_ON,0}},

/* [3,0] */	{{"xorb",TERM,RMMR,0},	{"xor",TERM,RMMR,1},	{"xorb",TERM,RMMR,0},	{"xor",TERM,RMMR,1},
/* [3,4] */	{"xorb",TERM,IA,0},	{"xor",TERM,IA,1},	{"%ss:",TERM,OVERRIDE,0},{"aaa",TERM,GO_ON,0},
/* [3,8] */	{"cmpb",TERM,RMMR,0},	{"cmp",TERM,RMMR,1},	{"cmpb",TERM,RMMR,0},	{"cmp",TERM,RMMR,1},
/* [3,C] */	{"cmpb",TERM,IA,0},	{"cmp",TERM,IA,1},	{"%ds:",TERM,OVERRIDE,0},{"aas",TERM,GO_ON,0}},

/* [4,0] */	{{"inc",TERM,R,1},	{"inc",TERM,R,1},	{"inc",TERM,R,1},	{"inc",TERM,R,1},
/* [4,4] */	{"inc",TERM,R,1},	{"inc",TERM,R,1},	{"inc",TERM,R,1},	{"inc",TERM,R,1},
/* [4,8] */	{"dec",TERM,R,1},	{"dec",TERM,R,1},	{"dec",TERM,R,1},	{"dec",TERM,R,1},
/* [4,C] */	{"dec",TERM,R,1},	{"dec",TERM,R,1},	{"dec",TERM,R,1},	{"dec",TERM,R,1}},

/* [5,0] */	{{"push",TERM,R,1},	{"push",TERM,R,1},	{"push",TERM,R,1},	{"push",TERM,R,1},
/* [5,4] */	{"push",TERM,R,1},	{"push",TERM,R,1},	{"push",TERM,R,1},	{"push",TERM,R,1},
/* [5,8] */	{"pop",TERM,R,1},	{"pop",TERM,R,1},	{"pop",TERM,R,1},	{"pop",TERM,R,1},
/* [5,C] */	{"pop",TERM,R,1},	{"pop",TERM,R,1},	{"pop",TERM,R,1},	{"pop",TERM,R,1}},

/* [6,0] */	{{"pusha",TERM,GO_ON,1},	{"popa",TERM,GO_ON,1},	{"bound",TERM,MR,1},	{"arpl",TERM,RMw,0},
/* [6,4] */	{"%fs:",TERM,OVERRIDE,0},{"%gs:",TERM,OVERRIDE,0},{"data16",TERM,DM,0},	{"addr16",TERM,AM,0},
/* [6,8] */	{"push",TERM,I,1},	{"imul",TERM,RMMRI,1},	{"push",TERM,Ib,1},	{"imul",TERM,RMMRI,1},
/* [6,C] */	{"insb",TERM,GO_ON,0},	{"ins",TERM,GO_ON,1},	{"outsb",TERM,GO_ON,0},	{"outs",TERM,GO_ON,1}},

/* [7,0] */	{{"jo",TERM,BD,0},	{"jno",TERM,BD,0},	{"jb",TERM,BD,0},	{"jae",TERM,BD,0},
/* [7,4] */	{"je",TERM,BD,0},	{"jne",TERM,BD,0},	{"jbe",TERM,BD,0},	{"ja",TERM,BD,0},
/* [7,8] */	{"js",TERM,BD,0},	{"jns",TERM,BD,0},	{"jp",TERM,BD,0},	{"jnp",TERM,BD,0},
/* [7,C] */	{"jl",TERM,BD,0},	{"jge",TERM,BD,0},	{"jle",TERM,BD,0},	{"jg",TERM,BD,0}},

/* [8,0] */	{{"",op80,TERM,0},	{"",op81,TERM,0},	{"",op82,TERM,0},	{"",op83,TERM,0},
/* [8,4] */	{"testb",TERM,MRw,0},	{"test",TERM,MRw,1},	{"xchgb",TERM,MRw,0},	{"xchg",TERM,MRw,1},
/* [8,8] */	{"movb",TERM,RMMR,0},	{"mov",TERM,RMMR,1},	{"movb",TERM,RMMR,0},	{"mov",TERM,RMMR,1},
/* [8,C] */	{"mov",TERM,SM,1},	{"lea",TERM,MR,0},	{"mov",TERM,MS,1},	{"pop",TERM,M,1}},

/* [9,0] */	{{"xchg",TERM,RA,1},	{"xchg",TERM,RA,1},	{"xchg",TERM,RA,1},	{"xchg",TERM,RA,1},
/* [9,4] */	{"xchg",TERM,RA,1},	{"xchg",TERM,RA,1},	{"xchg",TERM,RA,1},	{"xchg",TERM,RA,1},
/* [9,8] */	{"",TERM,CBW,0},	{"",TERM,CWD,0},	{"lcall",TERM,SO,0},	{"wait",TERM,GO_ON,0},
/* [9,C] */	{"pushf",TERM,GO_ON,1},	{"popf",TERM,GO_ON,1},	{"sahf",TERM,GO_ON,0},	{"lahf",TERM,GO_ON,0}},

/* [A,0] */	{{"movb",TERM,OA,0},	{"mov",TERM,OA,1},	{"movb",TERM,AO,0},	{"mov",TERM,AO,1},
/* [A,4] */	{"movsb",TERM,SD,0},	{"movs",TERM,SD,1},	{"cmpsb",TERM,SD,0},	{"cmps",TERM,SD,1},
/* [A,8] */	{"testb",TERM,IA,0},	{"test",TERM,IA,1},	{"stosb",TERM,AD,0},	{"stos",TERM,AD,1},
/* [A,C] */	{"lodsb",TERM,SA,0},	{"lods",TERM,SA,1},	{"scasb",TERM,AD,0},	{"scas",TERM,AD,0}},

/* [B,0] */	{{"movb",TERM,IR,0},	{"movb",TERM,IR,0},	{"movb",TERM,IR,0},	{"movb",TERM,IR,0},
/* [B,4] */	{"movb",TERM,IR,0},	{"movb",TERM,IR,0},	{"movb",TERM,IR,0},	{"movb",TERM,IR,0},
/* [B,8] */	{"mov",TERM,IR,1},	{"mov",TERM,IR,1},	{"mov",TERM,IR,1},	{"mov",TERM,IR,1},
/* [B,C] */	{"mov",TERM,IR,1},	{"mov",TERM,IR,1},	{"mov",TERM,IR,1},	{"mov",TERM,IR,1}},

/* [C,0] */	{{"",opC0,TERM,0},	{"",opC1,TERM,0},	{"ret",TERM,I16,0},	{"ret",TERM,GO_ON,0},
/* [C,4] */	{"les",TERM,MR,0},	{"lds",TERM,MR,0},	{"movb",TERM,IMw,0},	{"mov",TERM,IMw,1},
/* [C,8] */	{"enter",TERM,II,0},	{"leave",TERM,GO_ON,0},	{"lret",TERM,I16,0},	{"lret",TERM,GO_ON,0},
/* [C,C] */	{"int",TERM,Iv,0},	{"int",TERM,Iv,0},	{"into",TERM,GO_ON,0},	{"iret",TERM,GO_ON,0}},

/* [D,0] */	{{"",opD0,TERM,0},	{"",opD1,TERM,0},	{"",opD2,TERM,0},	{"",opD3,TERM,0},
/* [D,4] */	{"aam",TERM,U,0},	{"aad",TERM,U,0},	{"falc",TERM,GO_ON,0},	{"xlat",TERM,GO_ON,0},

/* 287 instructions.  Note that although the indirect field		*/
/* indicates opfp1n2 for further decoding, this is not necessarily	*/
/* the case since the opfp arrays are not partitioned according to key1	*/
/* and key2.  opfp1n2 is given only to indicate that we haven't		*/
/* finished decoding the instruction.					*/
/* [D,8] */	{"",&opfp1n2[0][0],TERM,0},	{"",&opfp1n2[0][0],TERM,0},
		{"",&opfp1n2[0][0],TERM,0},	{"",&opfp1n2[0][0],TERM,0},
/* [D,C] */	{"",&opfp1n2[0][0],TERM,0},	{"",&opfp1n2[0][0],TERM,0},
		{"",&opfp1n2[0][0],TERM,0},	{"",&opfp1n2[0][0],TERM,0}},

/* [E,0] */	{{"loopnz",TERM,BD,0},	{"loopz",TERM,BD,0},	{"loop",TERM,BD,0},	{"jcxz",TERM,BD,0},
/* [E,4] */	{"inb",TERM,P,0},	{"in",TERM,P,1},	{"outb",TERM,P,0},	{"out",TERM,P,1},
/* [E,8] */	{"call",TERM,D,0},	{"jmp",TERM,D,0},	{"ljmp",TERM,SO,0},	{"jmp",TERM,BD,0},
/* [E,C] */	{"inb",TERM,V,0},	{"in",TERM,V,1},	{"outb",TERM,V,0},	{"out",TERM,V,1}},

/* [F,0] */	{{"lock ",TERM,PREFIX,0},	{"",TERM,JTAB,0},	{"repnz ",TERM,PREFIX,0},	{"repz ",TERM,PREFIX,0},
/* [F,4] */	{"halt",TERM,GO_ON,0},	{"cmc",TERM,GO_ON,0},	{"",opF6,TERM,0},	{"",opF7,TERM,0},
/* [F,8] */	{"clc",TERM,GO_ON,0},	{"stc",TERM,GO_ON,0},	{"cli",TERM,GO_ON,0},	{"sti",TERM,GO_ON,0},
/* [F,C] */	{"cld",TERM,GO_ON,0},	{"std",TERM,GO_ON,0},	{"",opFE,TERM,0},	{"",opFF,TERM,0}},
};


opnd_t	dis_opnd[4];		/* to store operands as they	*/
				/* are encountered		*/
char	dis_relopnd;		/* Count of the number of operands for	*/
				/* which a relative form of the value	*/
				/* should be printed.			*/
long	dis_start;		/* start of each instruction	*/
				/* used with jumps		*/
long	dis_loc;		/* Byte location in instruction being	*/
				/* disassembled.  Incremented by the	*/
				/* get1byte routine.			*/
char	dis_line[NLINE];	/* Array to store printable version of	*/
				/* line as it is disassembled.		*/
ushort	cur1byte;		/* The last byte fetched by get1byte.	*/
char	blanks[] = "                                                    ";
long	prevaddr = 0;

int	data_16=FALSE;
int	addr_16=FALSE;
char	*reg_name;
static char *overreg;	/* to save the override register		*/
int dbit, wbit, vbit, lwbits;
#ifndef i386
static	char	operand[4][OPLEN];	/* to store operands as they	*/
					/* are encountered		*/
static	int	dis_relopnd;	/* will index into the 'operand' array	*/
#endif
static	int	no_bytes;	/* length of an immediate */
static	int	w_default=1;
	long	lngval;
	int	temp;

long
compoff(lng, temp)
long	lng;
char	*temp;
{
	lng += dis_loc;
	sprintf(temp, "%s <%lx>", temp, lng);
	return(lng);		/* Return address calculated.	*/
}

void
convert(num, temp, flag)
ulong		num;
char		temp[];
int		flag;
{
	if (flag == NOLEAD) sprintf(temp,"%04lx",num);
	if (flag == LEAD) sprintf(temp,"0x%lx",num);
}

int get1byte()
{
	unsigned char c;
	readmem(dis_loc++,1,-1,&c, 1, "current byte");
	cur1byte = c;
	return(c);
}

void get_opcode(high, low)
unsigned *high;
unsigned *low;		/* low 4 bits of op code   */
{
	unsigned short	byte;

	byte=get1byte();
	*low = LOW4(byte);
	*high = BITS7_4(byte);
}

void get_decode(mode, reg, r_m)
unsigned	*mode, *reg, *r_m;
{
	get1byte();
	*r_m = R_M(cur1byte);
	*reg = REG(cur1byte);
	*mode = MODE(cur1byte);
}

bad_opcode() {error("Bad Opcode\n");}

void prtaddress(mode, r_m, wbit)
unsigned	mode, r_m;
int		wbit;
{
	ck_prt_override();
	addr_disp(mode, r_m);
	addr_print(mode, r_m, wbit);
}

ulong
getval(no_bytes)
enum byte_count no_bytes;
{
	ulong		val;

	val = get1byte();
	if (no_bytes != ONE) {
		val |= get1byte() << 8;
		if (no_bytes != TWO) {
			val |= get1byte() << 16;
			val |= get1byte() << 24;
		}
	}
	return val;
}

void imm_data(no_bytes)
enum byte_count no_bytes;
{
	ulong	val;
	char	temp[12];

	convert(val = getval(no_bytes), temp, LEAD);
	(void) sprintf(dis_opnd[dis_relopnd].opnd_sym, "$%s", temp);
	dis_opnd[dis_relopnd].opnd_val = val;
}

void saving_disp(mode, r_m)
unsigned	mode, r_m;
{
	(void) sprintf(dis_opnd[dis_relopnd].opnd_sym,"");
	dis_opnd[dis_relopnd].opnd_val=0;
	addr_disp(mode, r_m);
}

unsaving_disp(mode, r_m, wbit)
unsigned	mode, r_m, wbit;
{
	dis_relopnd = 3;
	(void) sprintf(dis_opnd[dis_relopnd].opnd_sym,"");
	dis_opnd[dis_relopnd].opnd_val=0;
	addr_print(mode, r_m, wbit);
	(void) sprintf(dis_line,"%s%s,%s%s%s",dis_line,
	               dis_opnd[0].opnd_sym, dis_opnd[1].opnd_sym,
	               dis_opnd[2].opnd_sym, dis_opnd[3].opnd_sym);
}

ck_prt_override()
{
	if (overreg != NULL) {
		(void) sprintf(dis_opnd[dis_relopnd].opnd_sym,"%s",overreg);
		dis_opnd[dis_relopnd].opnd_val=0;
		overreg = NULL;
	}
}

/*	void displacement (no_bytes, ptr)
 *
 *	Get and print in the 'operand' array a one, two or four
 *	byte displacement.
 */

void displacement(no_bytes)
enum byte_count no_bytes;
{
	ulong	val;
	char	temp[12];

	val = getval(no_bytes);
	if (no_bytes != ONE && cur1byte == 0) /* if high byte 0 prt leading 0 */
		(void) sprintf(temp, "0x0%lx", val);
	else
		convert(val, temp, LEAD);

	ck_prt_override();
	strcat(dis_opnd[dis_relopnd].opnd_sym, temp);
	dis_opnd[dis_relopnd].opnd_val = val;
}

static	int s_i_b;
static	unsigned ss, index, base;

addr_disp(mode, r_m)
unsigned mode, r_m;
{
	enum byte_count no_bytes;
	int disp;
	/* check for the presence of the s-i-b byte */
	if ((r_m==ESP) && (mode!=0x3)) {
		s_i_b = TRUE;
		get_decode(&ss, &index, &base);
	} else
	s_i_b = FALSE;
	disp = addr_16 ? addrmod_16[r_m][mode].disp :
			 addrmod_32[r_m][mode].disp;
	if (s_i_b && mode==0 && base==5)
		disp = DISP_4;
	switch (disp) {
	case DISP_0:
		break;
	case DISP_1:
		no_bytes = ONE;
		displacement(no_bytes);
		break;
	case DISP_2:
		no_bytes = TWO;
		displacement(no_bytes);
		break;
	case DISP_4:
		no_bytes = FOUR;
		displacement(no_bytes);
		break;
	}
}

addr_print(mode, r_m, wbit)
unsigned mode, r_m, wbit;
{
	char *reg_name;
	if (s_i_b==FALSE) {
		if (addr_16)
			reg_name = (mode == REG_ONLY) ?
				regster_16[r_m][wbit] :
				addrmod_16[r_m][mode].regs;
		else
			reg_name = (mode == REG_ONLY) ?
				regster_32[r_m][wbit] :
				addrmod_32[r_m][mode].regs;
	} else {
		register char *regs;
		char buffer[16];
		char buf2[16];
		/* get the base register from addrmod_32;
		 * we may have to strip off leading/tailing parens */
		regs = addrmod_32[base][mode].regs;
		if (*regs == '(') {
			strcpy(buf2, regs+1);
			reg_name = buf2;
			reg_name[strlen(reg_name)-1] = '\0';
		} else
			reg_name = regs;
		(void) sprintf(buffer, "(%s%s%s)", reg_name,
			index_reg[index], scale_factor[ss]);
		reg_name = buffer;
	}
	(void) sprintf(	dis_opnd[dis_relopnd].opnd_sym,"%s%s",
			dis_opnd[dis_relopnd].opnd_sym,reg_name);
}

/* get arguments for dis function */
int
getdis()
{
	int c;
	int absflg = 0;
	long addr = 0;
	int count = 1;
	struct syment *sp;

	optind = 1;
	while((c = getopt(argcnt,args,"acw:")) != EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 'a' :	absflg = 1;
					break;
			case 'c' :	addr = prevaddr;
					break;
			default  :	longjmp(syn,0);
		}
	}
	if (!addr) {
		if(!args[optind])
			longjmp(syn,0);
		if(*args[optind] == '(') {
			if((addr = eval(++args[optind])) == -1)
				error("\n");
		}
		else if(sp = symsrch(args[optind]))
			addr = sp->n_value;
		else if(isasymbol(args[optind]))
			error("%s not found in symbol table\n",args[optind]);
		else if((addr = strcon(args[optind],'h')) == -1)
				error("\n");
	} else {
		optind--;
	}
	if(args[++optind]) 
		if((count = strcon(args[optind],'d')) == -1)
			error("\n");
	prdis(addr,count,absflg);
}
	
/* print disassembled code */
int
prdis(addr,count,absflg)
long	addr;
int	count;
int	absflg;
{
	int	i,ii;
	long	nxtaddr;
	struct syment *sp;
	extern short N_TEXT, N_DATA, N_BSS;
	extern struct syment *findsym();
	extern char *strtbl;
	static char addrbuf[256];
	static char temp[21];
	long	val;
	int	outflag; /* 0=no symbols, >0 #omitted symbols, -1 no omitted */
	long	disasmi();

	while(count--){
		nxtaddr = disasmi(addr);
		if(absflg){
			fprintf(fp,"%.8x:  ", addr);
			dis_loc = addr;
			for(ii = 0; ii < 10  ||  addr + ii < nxtaddr; ii++){
				if(addr + ii < nxtaddr){
					get1byte();
					fprintf(fp,"%.2x ",cur1byte);
				}
				else fprintf(fp,"   ");
			}
		}
		else {
			if(!(sp = findsym((unsigned long)addr)))	
				sp = findsym((unsigned long)addr);
			if(!sp)
				sprintf(addrbuf,"%x",addr);
			else {
				if(sp->n_zeroes){
					strncpy(addrbuf,sp->n_name,8);
					addrbuf[8] = '\0';
				}
				else sprintf(addrbuf,"%-12s",strtbl+sp->n_offset);
				sprintf(temp,"+%x",addr-(long)sp->n_value);
				if (addr != (long)sp->n_value)
					strcat(addrbuf,temp);
				else if (count>1) fprintf(fp,"\n");
			}
			ii = 20 - (int)strlen(addrbuf);
			strncat(addrbuf,blanks,ii);
			fprintf(fp,"%s",addrbuf);
		}
		fprintf(fp,"%s",dis_line);
		if(!absflg /* && dis_relopnd */){
			for(i = 0; i < 30 - (int)strlen(dis_line); i++)
				addrbuf[i] = ' ';	
			addrbuf[i] = '\0';
/* for (ii=0;ii<4;ii++) if (dis_opnd[ii].opnd_val != 0) fprintf(fp,"\n%d  %08x  %s",ii,dis_opnd[ii].opnd_val,dis_opnd[ii].opnd_sym);fprintf(fp,"\n"); */
			outflag=0;
			for(ii = 0; /* dis_relopnd */ ii<4 ; ii++) { 
				if(val = dis_opnd[ii].opnd_val){
					if(!(sp = findsym((unsigned long)val)))
						(sp = findsym((unsigned long)val));
					if(!sp) {
						if (outflag<0) strcat(addrbuf,",-");
						else outflag++;
					}
					else {
						if(sp->n_zeroes){
							strncpy(temp,sp->n_name,
								8);
							temp[8] = '\0';
						}
						else sprintf(temp,"%-12s",strtbl
							+sp->n_offset);
						if (outflag>=0) {
							strcat(addrbuf," [");
							while (outflag--) strcat (addrbuf,"-,");
							outflag=(-1);}
						else strcat(addrbuf,",");
						strcat(addrbuf,temp);
						sprintf(temp,"+%x",
							val-(long)sp->n_value);
						if (val != (long)sp->n_value)
							strcat(addrbuf,temp);
					}
				}
			}
			if (outflag<0) fprintf(fp,"%s]",addrbuf);
		}
		fprintf(fp,"\n");
		addr = nxtaddr;
	}
	prevaddr = addr;
}

long
disasmi(addr)
long addr;
{
	struct instable *dp,*submode;
	unsigned key1, key2, key3, key4, key5, mode, reg, r_m;
	ulong temporary;
	int ii;
	unsigned key;		 /* key is the one byte op code		*/
	char mnemonic[NLINE];

	dis_loc = addr;
	dis_start = addr;
	dis_relopnd = 0;

	for(ii = 0  ;  ii < 4  ;  ii++){
		dis_opnd[ii].opnd_sym[0] = '\0';
		dis_opnd[ii].opnd_val = 0;
	}
	addr_16 = data_16 = FALSE;
	overreg = NULL;
	mnemonic[0] = '\0';

	/*
	** As long as there is a prefix, the default segment register,
	** addressing-mode, or data-mode in the instruction will be overridden.
	** This may be more general than the chip actually is.
	*/
	for(;;) {
		get_opcode(&key1, &key2);
		dp = &distable[key1][key2];

		if (dp->adr_mode == PREFIX)  strcat(mnemonic, dp->name);
		else if (dp->adr_mode == AM)  addr_16 = !addr_16;
		else if (dp->adr_mode == DM)  data_16 = !data_16;
		else if (dp->adr_mode == OVERRIDE)  overreg = dp->name;
		else  break;
	}

		/* 386 instructions have 2 bytes of opcode before the mod_r/m */
		/* byte so we need to perform a table indirection.	      */
		if (dp->indirect == &op0F[0][0]) {
			get_opcode(&key4,&key5);
			if ((key4==1) || ((key4>2) && (key4<8)) || (key4>11)) 
				bad_opcode();
			else
				dp = &op0F[key4][key5];
		}
		submode = dp -> indirect;
		if (dp -> indirect != TERM) {
			get_decode(&mode, &key3, &r_m);
			if (key1 == 0xD && key2 >= 0x8) {
				if (key2 == 0xB && mode == 0x3 && key3 == 4)
					dp = &opfp5[r_m];
				else if (key2 == 0xB && mode == 0x3 && key3 > 4)
					bad_opcode();
				else if (key2==0x9 && mode==0x3 && key3 >= 4)
					dp = &opfp4[key3-4][r_m];
				else if (key2 >= 8 && mode == 0x3)
					dp = &opfp3[key2-8][key3];
				else
					dp = &opfp1n2[key2-8][key3];
				} else {
					dp = dp -> indirect + key3;
			}
		}
		/* print the mnemonic */
		if (dp->adr_mode != CBW && dp->adr_mode != CWD)
			strcat(mnemonic, dp->name);
		if (dp->suffix)
			strcat(mnemonic, (data_16 ? "w" : "l"));
		sprintf(dis_line, (int)strlen(mnemonic) < 7 ? "%-7s" : "%s ",
					mnemonic);

		/*
		 * Each instruction has a particular instruction syntax format
		 * stored in the disassembly tables.  
		 */

		switch(dp -> adr_mode){

		/* register to or from a memory or register operand,	*/
		/* based on the 'd' bit					*/
		case RMMR:
			dbit = MASKd(key2);
			wbit = MASKw(key2);
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress( mode, r_m, wbit);
			if (data_16)
				reg_name = regster_16[reg][wbit];
			else
				reg_name = regster_32[reg][wbit];
			if (dbit == RM)
				(void) sprintf(dis_line,"%s%s,%s",dis_line,reg_name,dis_opnd[0].opnd_sym);
			else
				(void) sprintf(dis_line,"%s%s,%s",dis_line,dis_opnd[0].opnd_sym,reg_name);
			break;

		/* movsbl movsbw (0x0FBE) or movswl (0x0FBF) */
		/* movzbl movzbw (0x0FB6) or mobzwl (0x0FB7) */
		/* wbit is in 2nd byte, operands are different sized */
		case MOVX:
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			if (data_16)
				reg_name = regster_16[reg][wbit];
			else
				reg_name = regster_32[reg][wbit];
			wbit = MASKw(key5);
			data_16 = TRUE;
			prtaddress( mode, r_m, wbit);
			sprintf(dis_line,"%s%s,%s",dis_line,dis_opnd[0].opnd_sym,reg_name);
			break;
		/* added for 186 support; March 84; jws */
		case RMMRI:
			lwbits = MASKlw(key2 >> 2);
			if (data_16)
				no_bytes = (lwbits == TWOlw) ? TWO : ONE;
			else
				no_bytes = (lwbits == TWOlw) ? FOUR : ONE;
			dbit = MASKd(key2);
			wbit = MASKw(key2);
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress( mode, r_m, wbit);
			dis_relopnd = 1;
			imm_data(no_bytes);
			dis_relopnd = 0;
			if (data_16)
				reg_name = regster_16[reg][wbit];
			else
				reg_name = regster_32[reg][wbit];
			if (dbit == RM)
				(void) sprintf(dis_line,"%s%s,%s,%s",dis_line,
					dis_opnd[1].opnd_sym,reg_name,
					dis_opnd[0].opnd_sym);
			else
				(void) sprintf(dis_line,"%s%s,%s,%s",dis_line,
					dis_opnd[1].opnd_sym,
					dis_opnd[0].opnd_sym,reg_name);
			break;
		/* memory or register operand to register, with 'w' bit	*/
		case MRw:
			wbit = MASKw(key2);
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress( mode, r_m, wbit);
			if (data_16)
				reg_name = regster_16[reg][wbit];
			else
				reg_name = regster_32[reg][wbit];
			(void) sprintf(dis_line,"%s%s,%s",dis_line,
					dis_opnd[0].opnd_sym,
					reg_name);
			break;
		/* register to memory or register operand, with 'w' bit	*/
		case RMw:
			wbit = MASKw(key2);
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress( mode, r_m, wbit);
			if (data_16)
				reg_name = regster_16[reg][wbit];
			else
				reg_name = regster_32[reg][wbit];
			(void) sprintf(dis_line,"%s%s,%s",dis_line,reg_name,
					dis_opnd[0].opnd_sym);
			break;

		/*
		 *	For the following two cases, one must read the
		 *	displacement (if present) and have it saved for
		 *	printing after the immediate operand.  This is
		 *	done by calling the 'saving_disp' routine.
		 *	For further documention see this routine.
		 */

		/* immediate to memory or register operand with both	*/
		/* 'l' and 'w' bits present				*/

		case IMlw:
			lwbits = MASKlw(key2);
			wbit = MASKw(key2);
			dis_relopnd = 2;
			saving_disp(mode, r_m);
			if (data_16)
				no_bytes = (lwbits == TWOlw) ? TWO : ONE;
			else
				no_bytes = (lwbits == TWOlw) ? FOUR : ONE;
			dis_relopnd = 0;
			imm_data(no_bytes);
			dis_relopnd = 1;
			ck_prt_override();
			unsaving_disp(mode, r_m, wbit);
			break;
		/* immediate to memory or register operand with the	*/
		/* 'w' bit present					*/
		case IMw:
			wbit = MASKw(key2);
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			dis_relopnd = 2;
			saving_disp(mode, r_m);
			if (data_16)
				no_bytes = (wbit == TWOw) ? TWO : ONE;
			else
				no_bytes = (wbit == TWOw) ? FOUR : ONE;
			dis_relopnd = 0;
			imm_data(no_bytes);
			dis_relopnd = 1;
			ck_prt_override();
			unsaving_disp(mode, r_m, wbit);
			break;

		/* immediate to register with register in low 3 bits	*/
		/* of op code						*/
		case IR:
			wbit = (key2 >> 3) & 0x1;
			reg = MASKreg(key2);
			if (data_16)
				no_bytes = (wbit == TWOw) ? TWO : ONE;
			else
				no_bytes = (wbit == TWOw) ? FOUR : ONE;
			imm_data(no_bytes);
			if (data_16)
				reg_name = regster_16[reg][wbit];
			else
				reg_name = regster_32[reg][wbit];
			(void) sprintf(dis_line,"%s%s,%s",dis_line,
				dis_opnd[0].opnd_sym,reg_name);
			break;

		/* memory operand to accumulator			*/
		case OA:
			wbit = MASKw(key2);
			no_bytes = addr_16 ? TWO : FOUR;
			displacement(no_bytes);
			(void) sprintf(dis_line,"%s%s,%s",dis_line,
				dis_opnd[0].opnd_sym,
				addr_16 ? regster_16[0][wbit] : regster_32[0][wbit]);
			break;

		/* accumulator to memory operand			*/
		case AO:
			wbit = MASKw(key2);
			no_bytes = addr_16 ? TWO : FOUR;
			displacement(no_bytes);
			(void) sprintf(dis_line,"%s%s,%s",dis_line, addr_16 ?
				regster_16[0][wbit] : regster_32[0][wbit], dis_opnd[0].opnd_sym);
			break;
		/* memory or register operand to segment register	*/
		case MS:
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress(mode, r_m, w_default);
			(void) sprintf(dis_line,"%s%s,%s",dis_line,
				dis_opnd[0].opnd_sym,segreg[reg]);
			break;

		/* segment register to memory or register operand	*/
		case SM:
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress(mode, r_m, w_default);
			(void) sprintf(dis_line,"%s%s,%s",dis_line,segreg[reg],
				dis_opnd[0].opnd_sym);
			break;

		/* memory or register operand, which may or may not	*/
		/* go to the cl register, dependent on the 'v' bit	*/
		case Mv:
			vbit = MASKv(key2);
			wbit = MASKw(key2);
			prtaddress( mode, r_m, wbit);
			(void) sprintf(dis_line,"%s%s%s",dis_line,(vbit == REGv) ? "%cl,":"",
				dis_opnd[0].opnd_sym);
			break;
		/* added for 186 support; March 84; jws */
		case MvI:
			vbit = MASKv(key2);
			wbit = MASKw(key2);
			prtaddress( mode, r_m, wbit);
			no_bytes = ONE;
			dis_relopnd = 1;
			imm_data(no_bytes);
			(void) sprintf(dis_line,"%s%s,%s%s",dis_line,
				dis_opnd[1].opnd_sym,
				(vbit == REGv) ? "%cl,":"",
				dis_opnd[0].opnd_sym);
			dis_relopnd = 0;
			break;
		/* added for 386 support; April 86; mjn */
		case MIb:
			prtaddress( mode, r_m, w_default);
			no_bytes = ONE;
			dis_relopnd = 1;
			imm_data(no_bytes);
			(void) sprintf(dis_line,"%s%s,%s",dis_line,
				dis_opnd[1].opnd_sym,
				dis_opnd[0].opnd_sym);
			dis_relopnd = 0;
			break;

		/* single memory or register operand with 'w' bit present*/
		case Mw:
			wbit = MASKw(key2);
			prtaddress( mode, r_m, wbit);
			(void) sprintf(dis_line,"%s%s",dis_line,
				dis_opnd[0].opnd_sym);
			break;

		/* single memory or register operand			*/
		case M:
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress(mode, r_m, w_default);
			(void) sprintf(dis_line,"%s%s",dis_line,
				dis_opnd[0].opnd_sym);
			break;

		case SREG:
			get_decode(&mode, &reg, &r_m);
			vbit = 0;
			switch (key5) {
			case 2:
				vbit = 1;
				/* fall thru */
			case 0: 
				reg_name = cregster[reg];
				break;
			case 3:
				vbit = 1;
				/* fall thru */
			case 1:
				reg_name = dregster[reg];
				break;
			case 6:
				vbit = 1;
				/* fall thru */
			case 4:
				reg_name = tregster[reg];
				break;
			}
			strcpy(dis_opnd[0].opnd_sym, regster_32[r_m][1]);

			if (vbit)
			{
				strcpy(dis_opnd[0].opnd_sym, reg_name);
				reg_name = regster_32[r_m][1];
			}
			
			(void) sprintf(dis_line, "%s%s,%s", 
					dis_line, reg_name, dis_opnd[0].opnd_sym);
			break;

		/* single register operand with register in the low 3	*/
		/* bits of op code					*/
		case R:
			reg = MASKreg(key2);
			if (addr_16)
				reg_name = regster_16[reg][w_default];
			else
				reg_name = regster_32[reg][w_default];
			(void) sprintf(dis_line,"%s%s",dis_line,reg_name);
			break;
		/* register to accumulator with register in the low 3	*/
		/* bits of op code					*/
		case RA:
			reg = MASKreg(key2);
			if (addr_16)
				(void) sprintf(dis_line,"%s%s,%%ax",dis_line,regster_16[reg][w_default]);
			else
				(void) sprintf(dis_line,"%s%s,%%eax",dis_line,regster_32[reg][w_default]);
			break;

		/* single segment register operand, with register in	*/
		/* bits 3-4 of op code					*/
		case SEG:
			reg = MASKseg(cur1byte);
			(void) sprintf(dis_line,"%s%s",dis_line,segreg[reg]);
			break;

		/* single segment register operand, with register in	*/
		/* bits 3-5 of op code					*/
		case LSEG:
			reg = MASKlseg(cur1byte);
			(void) sprintf(dis_line,"%s%s",dis_line,segreg[reg]);
			break;

		/* memory or register operand to register		*/
		case MR:
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress(mode, r_m, w_default);
			if (data_16)
				reg_name = regster_16[reg][w_default];
			else
				reg_name = regster_32[reg][w_default];
			(void) sprintf(dis_line,"%s%s,%s",dis_line,
				dis_opnd[0].opnd_sym,reg_name);
			break;

		/* immediate operand to accumulator			*/
		case IA:
			wbit = MASKw(key2);
			if (data_16)
				no_bytes = (wbit == TWOw) ? TWO : ONE;
			else
				no_bytes = (wbit == TWOw) ? FOUR : ONE;
			imm_data(no_bytes);
			(void) sprintf(dis_line,"%s%s,%s",dis_line,
				dis_opnd[0].opnd_sym,
				(no_bytes == ONE) ? "%al" : ((no_bytes == TWO) ? "%ax" : "%eax"));
			break;

		/* memory or register operand to accumulator		*/
		case MA:
			wbit = MASKw(key2);
			prtaddress( mode, r_m, wbit);
			(void) sprintf(dis_line,"%s%s,%s",dis_line,
				dis_opnd[0].opnd_sym,
				data_16 ? regster_16[0][wbit] : regster_32[0][wbit]);
			break;

		/* si register to di register				*/
		case SD:
			ck_prt_override();
			(void) sprintf(dis_line,"%s%s(%%si),(%%di)",dis_line,
				dis_opnd[0].opnd_sym);
			break;

		/* accumulator to di register				*/
		case AD:
			wbit = MASKw(key2);
			ck_prt_override();
			(void) sprintf(dis_line,"%s%s,%s(%%di)",dis_line, addr_16 ?
				regster_16[0][wbit] : regster_32[0][wbit],
					dis_opnd[0].opnd_sym);
			break;

		/* si register to accumulator				*/
		case SA:
			wbit = MASKw(key2);
			ck_prt_override();
			(void) sprintf(dis_line,"%s%s(%%si),%s",dis_line,
				dis_opnd[0].opnd_sym,
				addr_16 ? regster_16[0][wbit] : regster_32[0][wbit]);
			break;

		/* single operand, a 16 bit displacement		*/
		/* added to current offset by 'compoff'			*/
		case D:
			if (addr_16)
				no_bytes = TWO;
			else
				no_bytes = FOUR;
			displacement(no_bytes);
			lngval = dis_opnd[0].opnd_val;
			if (addr_16 && (lngval & 0x8000))
				lngval |= ~0xffffL;
			dis_opnd[0].opnd_val=0;
			dis_opnd[1].opnd_val=compoff(lngval, dis_opnd[1].opnd_sym);
			(void) sprintf(dis_line,"%s%s%s",dis_line,
				dis_opnd[0].opnd_sym,
				(lngval == 0) ? "" : dis_opnd[1].opnd_sym);
			break;

		/* indirect to memory or register operand		*/
		case INM:
			prtaddress(mode, r_m, w_default);
			(void) sprintf(dis_line,"%s*%s",dis_line,
				dis_opnd[0].opnd_sym);
			break;
		/* for long jumps and long calls -- a new code segment	*/
		/* register and an offset in IP -- stored in object	*/
		/* code in reverse order				*/
		case SO:
			no_bytes = addr_16 ? TWO : FOUR;
			dis_relopnd = 1;
			displacement(no_bytes);
			dis_relopnd = 0;
			/* will now get segment operand*/
			no_bytes = TWO;
			displacement(no_bytes);
			(void) sprintf(dis_line,"%s%s,%s",dis_line,
				dis_opnd[0].opnd_sym,
				dis_opnd[1].opnd_sym);
			break;

		/* single operand, 8 bit displacement			*/
		/* added to current offset in 'compoff'			*/
		case BD:
			no_bytes = ONE;
			displacement(no_bytes);
			if ((lngval = dis_opnd[0].opnd_val) & 0x80)
				lngval |= ~0xffL;
			dis_opnd[0].opnd_val=0;
			dis_opnd[1].opnd_val= compoff(lngval, dis_opnd[1].opnd_sym);
			(void) sprintf(dis_line,"%s%s%s",dis_line,
				dis_opnd[0].opnd_sym,
				(lngval == 0) ? "" : dis_opnd[1].opnd_sym);
			break;

		/* single 16 bit immediate operand			*/
		case I16:
			no_bytes = TWO;
			imm_data(no_bytes);
			(void) sprintf(dis_line,"%s%s",dis_line,
				dis_opnd[0].opnd_sym);
			break;

		/* single word size immediate operand			*/
		case I:
			if (data_16)
				no_bytes = TWO;
			else
				no_bytes = FOUR;
			imm_data(no_bytes);
			(void) sprintf(dis_line,"%s%s",dis_line,
				dis_opnd[0].opnd_sym);
			break;

		/* single 8 bit immediate operand			*/
		case Ib:
			no_bytes = ONE;
			imm_data(no_bytes);
			(void) sprintf(dis_line,"%s%s",dis_line,
				dis_opnd[0].opnd_sym);
			break;

		/* added for 186 support; March 84; jws */
		case II:
			if (data_16)
				no_bytes = TWO;
			else
				no_bytes = FOUR;
			imm_data(no_bytes);
			no_bytes = ONE;
			dis_relopnd = 1;
			imm_data(no_bytes);
			dis_relopnd = 0;
			(void) sprintf(dis_line,"%s%s,%s",dis_line,
				dis_opnd[0].opnd_sym, dis_opnd[1].opnd_sym);
			break;

		/* single 8 bit port operand				*/
		case P:
			no_bytes = ONE;
			displacement(no_bytes);
			(void) sprintf(dis_line,"%s%s",dis_line,
				dis_opnd[0].opnd_sym);
			break;

		/* single operand, dx register (variable port instruction)*/
		case V:
			ck_prt_override();
			(void) sprintf(dis_line,"%s%s(%%dx)",dis_line,
				dis_opnd[0].opnd_sym);
			break;
		/* operand is either 3 or else the next 8 bits,		*/
		/* dependent on the 'v' bit (indicates type of interrupt)*/
		case Iv:
			vbit = MASKw(key2);
			if (vbit == TYPEv) {
				temporary = TYPE3;
				convert(temporary, temp, LEAD);
				(void) sprintf(dis_line,"%s$%s",dis_line,temp);
			}
			else {
				no_bytes = ONE;
				imm_data(no_bytes);
				(void) sprintf(dis_line,"%s%s",dis_line,
					dis_opnd[0].opnd_sym);
			}
			break;

		/* an unused byte must be discarded			*/
		case U:
			get1byte();
			break;

		case CBW:
			strcat(mnemonic, (data_16 ? "cbtw" : "cwtl"));
			break;

		case CWD:
			strcat(mnemonic, (data_16 ? "cwtd" : "cltd"));
			break;

		/* no disassembly, the mnemonic was all there was	*/
		/* so go on						*/
		case GO_ON:
			break;

		/* float reg */
		case F:
			(void) sprintf(dis_line,"%s%%st(%1.1d)",dis_line,r_m);
			break;

		/* float reg to float reg, with ret bit present */
		case FF:
			if ( MASKret(key2) )
				/* st -> st(i) */
				(void) sprintf(dis_line,"%s%%st,%%st(%1.1d)",dis_line,r_m);
			else
				/* st(i) -> st */
				(void) sprintf(dis_line,"%s%%st(%1.1d),%%st",dis_line,r_m);
			break;

		/* an invalid op code (there aren't too many)	*/
		case UNKNOWN:
		case OVERRIDE:
		case AM:
		case DM:
		case PREFIX:
		case JTAB:
			bad_opcode();
			break;

		default:
			error ("bug: case from instruction table not found");
	} /* end switch */
	return(dis_loc);
}
