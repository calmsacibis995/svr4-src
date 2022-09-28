/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dis:i386/dis.h	1.2"

/*
 *	This is the header file for the iapx disassembler.
 *	The information contained in the first part of the file
 *	is common to each version, while the last part is dependent
 *	on the particular machine architecture being used.
 */

#define		NCPS	8	/* number of chars per symbol	*/
#define		NHEX	40	/* max # chars in object per line	*/
#define		NLINE	36	/* max # chars in mnemonic per line	*/
#define		FAIL	0
#define		TRUE	1
#define		FALSE	0
#define		LEAD	1
#define		NOLEAD	0
#define		NOLEADSHORT 2
#define		TERM 0		/* used in _tbls.c to indicate		*/
				/* that the 'indirect' field of the	*/
				/* 'instable' terminates - no pointer.	*/
				/* Is also checked in 'dis_text()' in	*/
				/* _bits.c.				*/

#define	LNNOBLKSZ	1024	/* size of blocks of line numbers	  */
#define	SYMBLKSZ	1024	/* size if blocks of symbol table entries */
#define	STRNGEQ		0	/* used in string compare operation	  */

/*
 *	This is the structure that will be used for storing all the
 *	op code information.  The structure values themselves are
 *	in '_tbls.c'.
 */

struct	instable {
	char		name[NCPS];
	struct instable *indirect;	/* for decode op codes */
	unsigned	adr_mode;
	int		suffix;		/* for instructions which may
					   have a 'w' or 'l' suffix */
};

/*	NOTE:	the following information in this file must be changed
 *		between the different versions of the disassembler.
 *
 *	This structure is used to determine the displacements and registers
 *	used in the addressing modes.  The values are in 'tables.c'.
 */
struct addr {
	int	disp;
	char	regs[9];
};

/*
 *	The register that contains the frame pointer is required for
 *	symbolic disassembly. The *86 do not have an argument pointer.
 *	Both are %[E]BX.
 */
#define APNO 5
#define FPNO 5
/*
 *	These are the instruction formats as they appear in
 *	'tables.c'.  Here they are given numerical values
 *	for use in the actual disassembly of an object file.
 */
#define UNKNOWN	0
#define MRw	2
#define IMlw	3
#define IMw	4
#define IR	5
#define OA	6
#define AO	7
#define MS	8
#define SM	9
#define Mv	10
#define Mw	11
#define M	12
#define R	13
#define RA	14
#define SEG	15
#define MR	16
#define IA	17
#define MA	18
#define SD	19
#define AD	20
#define SA	21
#define D	22
#define INM	23
#define SO	24
#define BD	25
#define I	26
#define P	27
#define V	28
#define DSHIFT	29 /* for double shift that has an 8-bit immediate */
#define U	30
#define OVERRIDE 31
#define GO_ON	32
#define	O	33	/* for call	*/
#define JTAB	34	/* jump table 	*/
#define IMUL	35	/* for 186 iimul instr  */
#define CBW 36 /* so that data16 can be evaluated for cbw and its variants */
#define MvI	37	/* for 186 logicals */
#define	ENTER	38	/* for 186 enter instr  */
#define RMw	39	/* for 286 arpl instr */
#define Ib	40	/* for push immediate byte */
#define	F	41	/* for 287 instructions */
#define	FF	42	/* for 287 instructions */
#define DM	43	/* 16-bit data */
#define AM	44	/* 16-bit addr */
#define LSEG	45	/* for 3-bit seg reg encoding */
#define	MIb	46	/* for 386 logicals */
#define	SREG	47	/* for 386 special registers */
#define PREFIX 48 /* an instruction prefix like REP, LOCK */
#define INT3 49   /* The int 3 instruction, which has a fake operand */
#define DSHIFTcl 50 /* for double shift that implicitly uses %cl */
#define CWD 51    /* so that data16 can be evaluated for cwd and variants */
#define RET 52    /* single immediate 16-bit operand */
#define MOVZ 53   /* for movs and movz, with different size operands */

#define	FILL	0x90	/* Fill byte used for alignment (nop)	*/
/*
** Register numbers for the i386
*/
#define EAX 0
#define ECX 1
#define EDX 2
#define EBX 3
#define ESP 4
#define EBP 5
#define ESI 6
#define EDI 7
