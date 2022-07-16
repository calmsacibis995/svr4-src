#ident	"@(#)bits.c	1.2	92/02/17	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:kdb/bits.c	1.3.1.2"
/* @(#)bits.c	10.3 */

#include "sys/types.h"
#include "../db_as.h"
#include "dis.h"

#define		MAXERRS	10	/* maximum # of errors allowed before	*/
				/* abandoning this disassembly as a	*/
				/* hopeless case			*/

int errlev = 0;		/* to keep track of errors encountered during	*/
			/* the disassembly, probably due to being out	*/
			/* of sync.					*/


#define		OPLEN	35	/* maximum length of a single operand	*/
				/* (will be used for printing)		*/

static	char	operand[3][OPLEN];	/* to store operands as they	*/
					/* are encountered		*/
static	char	symarr[3][OPLEN];
static char *overreg;	/* save the segment override register if any    */
static int data16;	/* 16- or 32-bit data */
static int addr16;	/* 16- or 32-bit addressing */

#define	WBIT(x)	(x & 0x1)		/* to get w bit	*/
#define	REGNO(x) (x & 0x7)		/* to get 3 bit register */
#define	VBIT(x)	((x)>>1 & 0x1)		/* to get 'v' bit */
#define	OPSIZE(data16,wbit) ((wbit) ? ((data16) ? 2:4) : 1 )

#define	REG_ONLY 3	/* mode indicates a single register with	*/
			/* no displacement is an operand		*/
#define	LONGOPERAND 1	/* value of the w-bit indicating a long		*/
			/* operand (2-bytes or 4-bytes)			*/
#define EBP 5 /* from dis.h in dis because same symbol used in reg.h */
#define ESP 4

/*
 *	db_dis(addr, count)
 *
 *	disassemble `count' instructions starting at `addr'
 */

db_dis(addr, count)
	as_addr_t	addr;
	u_int		count;
{
	extern int _start, etext;
	extern char mneu[], *sl_name;
	long l;
	as_addr_t dis_dot();

	errlev = 0;

	while (count--) {
		sprintf(mneu, "0x%x:", addr.a_addr);
		if (addr.a_as == AS_KVIRT) {
			if ((l = adrtoext(addr.a_addr)) == 0)
				sprintf(mneu, "%s:", sl_name);
			else if (l != -1)
				sprintf(mneu, "%s+%x:", sl_name, l);
		}
		dbprintf("%-15s ", mneu);
		addr = dis_dot(addr, 0, 0);
		if (errlev >= MAXERRS) {
			(void) dbprintf("dis: probably not text section\n");
			(void) dbprintf("\tdisassembly terminated\n");
			break;
		}
		dbprintf("%-36s", mneu);
		prassym();
		dbprintf("\n");
	}
}

/*
 *	dis_dot ()
 *
 *	disassemble a text section
 */

as_addr_t
dis_dot(ldot, idsp, fmt)
	as_addr_t	ldot;
	int		idsp;
	char		fmt;
{
	/* the following arrays are contained in tables.c	*/
	extern	struct	instable	distable[16][16];
	extern	struct	instable	op0F[12][16];
	extern	struct	instable	opFP1n2[8][8];
	extern	struct	instable	opFP3[8][8];
	extern	struct	instable	opFP4[4][8];
	extern	struct	instable	opFP5[8];

	extern	char	*REG16[8][2];
	extern	char	*REG32[8][2];
	extern	char	*SEGREG[6];
	extern	char	*DEBUGREG[8];
	extern	char	*CONTROLREG[8];
	extern	char	*TESTREG[8];
	/* the following entries are from _extn.c	*/
	extern	as_addr_t	loc;
	extern	char	mneu[];
	extern	unsigned short curbyte;
	extern	unsigned short cur2bytes;
	extern	unsigned long  cur4bytes;
	/* the following routines are in _utls.c	*/
	extern	void	printline();
	extern	void	looklabel();
	extern	void	compoff();
	extern	void	convert();
	extern	short	sect_name();
	extern	void	fetchbyte();
	extern	void	lookbyte();
	/* libc */
	extern	void	exit();
	extern	char 	*strcat();
	extern	char 	*strcpy();
	/* forward */
	void get_modrm_byte();
	void check_override();
	void imm_data();
	void get_opcode();
	void displacement();

	struct instable *dp;
	int wbit, vbit;
	unsigned mode, reg, r_m;
	/* nibbles of the opcode */
	unsigned opcode1, opcode2, opcode3, opcode4, opcode5;
	unsigned short tmpshort;
	short	sect_num;
	long	lngval;
	char *reg_name;
	int got_modrm_byte;
	char mnemonic[OPLEN];
	char	temp[NCPS+1];

	loc = ldot;

	/*
	 * An instruction is disassembled with each iteration of the
	 * following loop.  The loop is terminated upon completion of the
	 * section (loc minus the section's physical address becomes equal
	 * to the section size) or if the number of bad op codes encountered
	 * would indicate this disassembly is hopeless.
	 */

	mnemonic[0] = '\0';
	mneu[0] = '\0';
	operand[0][0] = '\0';
	operand[1][0] = '\0';
	operand[2][0] = '\0';
	symarr[0][0] = '\0';
	symarr[1][0] = '\0';
	symarr[2][0] = '\0';
	overreg = (char *) 0;
	data16 = addr16 = 0;

	/*
	** As long as there is a prefix, the default segment register,
	** addressing-mode, or data-mode in the instruction will be overridden.
	** This may be more general than the chip actually is.
	*/
	for(;;) {
		get_opcode(&opcode1, &opcode2);
		dp = &distable[opcode1][opcode2];

		if ( dp->adr_mode == PREFIX ) strcat(mnemonic, dp->name);
		else if ( dp->adr_mode == AM ) addr16 = !addr16;
		else if ( dp->adr_mode == DM ) data16 = !data16;
		else if ( dp->adr_mode == OVERRIDE ) overreg = dp->name;
		else break;
	}

	/* some 386 instructions have 2 bytes of opcode before the mod_r/m */
	/* byte so we need to perform a table indirection.	      */
	if (dp->indirect == (struct instable *) op0F) {
		get_opcode(&opcode4,&opcode5);
		if (opcode4>11)  {
			sprintf(mneu,"***** Error - bad opcode\n");
			errlev++;
			return loc;
		}
		dp = &op0F[opcode4][opcode5];
	}

	got_modrm_byte = 0;
	if (dp->indirect != TERM) {
		/* This must have been an opcode for which several
		 * instructions exist.  The opcode3 field further decodes
		 * the instruction.
		 */
		got_modrm_byte = 1;
		get_modrm_byte(&mode, &opcode3, &r_m);
		/*
		 * decode 287 instructions (D8-DF) from opcodeN
		 */
		if (opcode1 == 0xD && opcode2 >= 0x8) {
			/* instruction form 5 */
			if (opcode2 == 0xB && mode == 0x3 && opcode3 == 4)
				dp = &opFP5[r_m];
			else if (opcode2 == 0xB && mode == 0x3 && opcode3 > 4) {
				sprintf(mneu,"***** Error - bad opcode\n");
				errlev++;
				return loc;
			}
			/* instruction form 4 */
			else if (opcode2==0x9 && mode==0x3 && opcode3 >= 4)
				dp = &opFP4[opcode3-4][r_m];
			/* instruction form 3 */
			else if (mode == 0x3)
				dp = &opFP3[opcode2-8][opcode3];
			/* instruction form 1 and 2 */
			else dp = &opFP1n2[opcode2-8][opcode3];
		}
		else
			dp = dp -> indirect + opcode3;
				/* now dp points the proper subdecode table entry */
	}

	if (dp->indirect != TERM) {
		sprintf(mneu,"***** Error - bad opcode\n");
		errlev++;
		return loc;
	}

	/* print the mnemonic */
	if ( dp->adr_mode != CBW  && dp->adr_mode != CWD ) {
		(void) strcat(mnemonic, dp -> name);  /* print the mnemonic */
		if (dp->suffix)
			(void) strcat(mnemonic, (data16? "w" : "l") );
		(void) sprintf(mneu, (strlen(mnemonic)<7 ? "%-7s" : "%-7s "),
		               mnemonic);
	}


	/*
	 * Each instruction has a particular instruction syntax format
	 * stored in the disassembly tables.  The assignment of formats
	 * to instructins was made by the author.  Individual formats
	 * are explained as they are encountered in the following
	 * switch construct.
	 */

	switch(dp -> adr_mode){

	/* movsbl movsbw (0x0FBE) or movswl (0x0FBF) */
	/* movzbl movzbw (0x0FB6) or mobzwl (0x0FB7) */
	/* wbit lives in 2nd byte, note that operands are different sized */
	case MOVZ:
		/* Get second operand first so data16 can be destroyed */
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];

		wbit = WBIT(opcode5);
		data16 = 1;
		get_operand(mode, r_m, wbit, 0);
		(void) sprintf(mneu,"%s%s,%s",mneu,operand[0],reg_name);
		return loc;

	/* imul instruction, with either 8-bit or longer immediate */
	case IMUL:
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 1);
		/* opcode 0x6B for byte, sign-extended displacement, 0x69 for word(s)*/
		imm_data( OPSIZE(data16,opcode2 == 0x9), 0);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];
		(void) sprintf(mneu,"%s%s,%s,%s",mneu,operand[0],operand[1],reg_name);
		return loc;

	/* memory or register operand to register, with 'w' bit	*/
	case MRw:
		wbit = WBIT(opcode2);
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, wbit, 0);
		if (data16)
			reg_name = REG16[reg][wbit];
		else
			reg_name = REG32[reg][wbit];
		(void) sprintf(mneu,"%s%s,%s",mneu,operand[0],reg_name);
		return loc;

	/* register to memory or register operand, with 'w' bit	*/
	/* arpl happens to fit here also because it is odd */
	case RMw:
		wbit = WBIT(opcode2);
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, wbit, 0);
		if (data16)
			reg_name = REG16[reg][wbit];
		else
			reg_name = REG32[reg][wbit];
		(void) sprintf(mneu,"%s%s,%s",mneu,reg_name,operand[0]);
		return loc;

	/* Double shift. Has immediate operand specifying the shift. */
	case DSHIFT:
		get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 1);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];
		imm_data(1, 0);
		sprintf(mneu,"%s%s,%s,%s",mneu,operand[0],reg_name,operand[1]);
		return loc;

	/* Double shift. With no immediate operand, specifies using %cl. */
	case DSHIFTcl:
		get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 0);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];
		sprintf(mneu,"%s%s,%s",mneu,reg_name,operand[0]);
		return loc;

	/* immediate to memory or register operand */
	case IMlw:
		wbit = WBIT(opcode2);
		get_operand(mode, r_m, wbit, 1);
		/* A long immediate is expected for opcode 0x81, not 0x80 nor 0x83 */
		imm_data(OPSIZE(data16,opcode2 == 1), 0);
		sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
		return loc;

	/* immediate to memory or register operand with the	*/
	/* 'w' bit present					*/
	case IMw:
		wbit = WBIT(opcode2);
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, wbit, 1);
		imm_data(OPSIZE(data16,wbit), 0);
		sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
		return loc;

	/* immediate to register with register in low 3 bits	*/
	/* of op code						*/
	case IR:
		wbit = opcode2 >>3 & 0x1; /* w-bit here (with regs) is bit 3 */
		reg = REGNO(opcode2);
		imm_data( OPSIZE(data16,wbit), 0);
		if (data16)
			reg_name = REG16[reg][wbit];
		else
			reg_name = REG32[reg][wbit];
		(void) sprintf(mneu,"%s%s,%s",mneu,operand[0],reg_name);
		return loc;

	/* memory operand to accumulator			*/
	case OA:
		wbit = WBIT(opcode2);
		displacement(OPSIZE(addr16,LONGOPERAND), 0,&lngval);
		reg_name = ( data16 ? REG16 : REG32 )[0][wbit];
		(void) sprintf(mneu,"%s%s,%s",mneu,operand[0],reg_name);
		return loc;

	/* accumulator to memory operand			*/
	case AO:
		wbit = WBIT(opcode2);
		
		displacement(OPSIZE(addr16,LONGOPERAND), 0,&lngval);
		reg_name = ( addr16 ? REG16 : REG32 )[0][wbit];
		(void) sprintf(mneu,"%s%s,%s",mneu, reg_name, operand[0]);
		return loc;

	/* memory or register operand to segment register	*/
	case MS:
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 0);
		(void) sprintf(mneu,"%s%s,%s",mneu,operand[0],SEGREG[reg]);
		return loc;

	/* segment register to memory or register operand	*/
	case SM:
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 0);
		(void) sprintf(mneu,"%s%s,%s",mneu,SEGREG[reg],operand[0]);
		return loc;

	/* rotate or shift instrutions, which may shift by 1 or */
	/* consult the cl register, depending on the 'v' bit	*/
	case Mv:
		vbit = VBIT(opcode2);
		wbit = WBIT(opcode2);
		get_operand(mode, r_m, wbit, 0);
		/* When vbit is set, register is an operand, otherwise just $0x1 */
		reg_name = vbit ? "%cl," : "" ;
		(void) sprintf(mneu,"%s%s%s",mneu, reg_name, operand[0]);
		return loc;

	/* immediate rotate or shift instrutions, which may or */
	/* may not consult the cl register, depending on the 'v' bit	*/
	case MvI:
		vbit = VBIT(opcode2);
		wbit = WBIT(opcode2);
		get_operand(mode, r_m, wbit, 0);
		imm_data(1,1);
		/* When vbit is set, register is an operand, otherwise just $0x1 */
		reg_name = vbit ? "%cl," : "" ;
		(void) sprintf(mneu,"%s%s,%s%s",mneu,operand[1], reg_name, operand[0]);
		return loc;

	case MIb:
		get_operand(mode, r_m, LONGOPERAND, 0);
		imm_data(1,1);
		(void) sprintf(mneu,"%s%s,%s",mneu,operand[1], operand[0]);
		return loc;

	/* single memory or register operand with 'w' bit present*/
	case Mw:
		wbit = WBIT(opcode2);
		get_operand(mode, r_m, wbit, 0);
		(void) sprintf(mneu,"%s%s",mneu,operand[0]);
		return loc;

	/* single memory or register operand			*/
	case M:
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 0);
		(void) sprintf(mneu,"%s%s",mneu,operand[0]);
		return loc;

	case SREG: /* special register */
		get_modrm_byte(&mode, &reg, &r_m);
		vbit = 0;
		switch (opcode5) {
		case 2:
			vbit = 1;
			/* fall thru */
		case 0: 
			reg_name = CONTROLREG[reg];
			break;
		case 3:
			vbit = 1;
			/* fall thru */
		case 1:
			reg_name = DEBUGREG[reg];
			break;
		case 6:
			vbit = 1;
			/* fall thru */
		case 4:
			reg_name = TESTREG[reg];
			break;
		}
		strcpy(operand[0], REG32[r_m][1]);

		if (vbit)
		{
			strcpy(operand[0], reg_name);
			reg_name = REG32[r_m][1];
		}
		
		(void) sprintf(mneu, "%s%s,%s",
			mneu, reg_name, operand[0]);
		return loc;

	/* single register operand with register in the low 3	*/
	/* bits of op code					*/
	case R:
		reg = REGNO(opcode2);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];
		(void) sprintf(mneu,"%s%s",mneu,reg_name);
		return loc;

	/* register to accumulator with register in the low 3	*/
	/* bits of op code, xchg instructions                   */
	case RA: {
		char *eprefix;
		reg = REGNO(opcode2);
		if (data16) {
			eprefix = "";
			reg_name = REG16[reg][LONGOPERAND];
		}
		else {
			eprefix = "e";
			reg_name = REG32[reg][LONGOPERAND];
		}
		(void) sprintf(mneu,"%s%s,%%%sax",
			mneu,reg_name,eprefix);
		return loc;
	}

	/* single segment register operand, with register in	*/
	/* bits 3-4 of op code					*/
	case SEG:
		reg = curbyte >> 3 & 0x3; /* segment register */
		(void) sprintf(mneu,"%s%s",mneu,SEGREG[reg]);
		return loc;

	/* single segment register operand, with register in	*/
	/* bits 3-5 of op code					*/
	case LSEG:
		reg = curbyte >> 3 & 0x7; /* long seg reg from opcode */
		(void) sprintf(mneu,"%s%s",mneu,SEGREG[reg]);
		return loc;

	/* memory or register operand to register		*/
	case MR:
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 0);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];
		(void) sprintf(mneu,"%s%s,%s",mneu,operand[0],reg_name);
		return loc;

	/* immediate operand to accumulator			*/
	case IA: {
		int no_bytes = OPSIZE(data16,WBIT(opcode2));
		switch(no_bytes) {
			case 1: reg_name = "%al"; break;
			case 2: reg_name = "%ax"; break;
			case 4: reg_name = "%eax"; break;
		}
		imm_data(no_bytes, 0);
		(void) sprintf(mneu,"%s%s,%s",mneu,operand[0], reg_name) ;
		return loc;
	}
	/* memory or register operand to accumulator		*/
	case MA:
		wbit = WBIT(opcode2);
		get_operand(mode, r_m, wbit, 0);
		reg_name = ( data16 ? REG16 : REG32) [0][wbit];
		(void) sprintf(mneu,"%s%s,%s",mneu, operand[0], reg_name );
		return loc;

	/* si register to di register				*/
	case SD:
		check_override(0);
		(void) sprintf(mneu,"%s%s(%%%ssi),(%%%sdi)",mneu,operand[0],
			addr16? "" : "e" , addr16? "" : "e");
		return loc;

	/* accumulator to di register				*/
	case AD:
		wbit = WBIT(opcode2);
		check_override(0);
		reg_name = (data16 ? REG16 : REG32) [0][wbit] ;
		(void) sprintf(mneu,"%s%s,%s(%%%sdi)",mneu, reg_name, operand[0],
			addr16? "" : "e");
		return loc;

	/* si register to accumulator				*/
	case SA:
		wbit = WBIT(opcode2);
		check_override(0);
		reg_name = (addr16 ? REG16 : REG32) [0][wbit] ;
		(void) sprintf(mneu,"%s%s(%%%ssi),%s",mneu,operand[0],
			addr16? "" : "e", reg_name);
		return loc;

	/* single operand, a 16/32 bit displacement		*/
	/* added to current offset by 'compoff'			*/
	case D:
		displacement(OPSIZE(data16,LONGOPERAND), 0, &lngval);
		compoff(lngval, operand[1]);
		(void) sprintf(mneu,"%s+%s%s",mneu,operand[0],
			(lngval == 0) ? "" : operand[1]);
		return loc;

	/* indirect to memory or register operand		*/
	case INM:
		get_operand(mode, r_m, LONGOPERAND, 0);
		(void) sprintf(mneu,"%s*%s",mneu,operand[0]);
		return loc;

	/* for long jumps and long calls -- a new code segment   */
	/* register and an offset in IP -- stored in object      */
	/* code in reverse order                                 */
	case SO:
		displacement(OPSIZE(addr16,LONGOPERAND), 1,&lngval);
		/* will now get segment operand*/
		displacement(2, 0,&lngval);
		(void) sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
		return loc;

	/* jmp/call. single operand, 8 bit displacement.	*/
	/* added to current EIP in 'compoff'			*/
	case BD:
		displacement(1, 0, &lngval);
		compoff(lngval, operand[1]);
		(void) sprintf(mneu,"%s+%s%s",mneu, operand[0],
			(lngval == 0) ? "" : operand[1]);
		return loc;

	/* single 32/16 bit immediate operand			*/
	case I:
		imm_data(OPSIZE(data16,LONGOPERAND), 0);
		(void) sprintf(mneu,"%s%s",mneu,operand[0]);
		return loc;

	/* single 8 bit immediate operand			*/
	case Ib:
		imm_data(1, 0);
		(void) sprintf(mneu,"%s%s",mneu,operand[0]);
		return loc;

	case ENTER:
		imm_data(2,0);
		imm_data(1,1);
		(void) sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
		return loc;

	/* 16-bit immediate operand */
	case RET:
		imm_data(2,0);
		(void) sprintf(mneu,"%s%s",mneu,operand[0]);
		return loc;

	/* single 8 bit port operand				*/
	case P:
		check_override(0);
		imm_data(1, 0);
		(void) sprintf(mneu,"%s%s",mneu,operand[0]);
		return loc;

	/* single operand, dx register (variable port instruction)*/
	case V:
		check_override(0);
		(void) sprintf(mneu,"%s%s(%%dx)",mneu,operand[0]);
		return loc;

	/* The int instruction, which has two forms: int 3 (breakpoint) or  */
	/* int n, where n is indicated in the subsequent byte (format Ib).  */
	/* The int 3 instruction (opcode 0xCC), where, although the 3 looks */
	/* like an operand, it is implied by the opcode. It must be converted */
	/* to the correct base and output. */
	case INT3:
		convert(3, temp, LEAD);
		(void) sprintf(mneu,"%s$%s",mneu,temp);
		return loc;

	/* an unused byte must be discarded			*/
	case U:
		fetchbyte();
		return loc;

	case CBW:
		if (data16)
			(void) strcat(mneu,"cbtw");
		else
			(void) strcat(mneu,"cwtl");
		return loc;

	case CWD:
		if (data16)
			(void) strcat(mneu,"cwtd");
		else
			(void) strcat(mneu,"cltd");
		return loc;

	/* no disassembly, the mnemonic was all there was	*/
	/* so go on						*/
	case GO_ON:
		return loc;

	/* Special byte indicating a the beginning of a 	*/
	/* jump table has been seen. The jump table addresses	*/
	/* will be printed until the address 0xffff which	*/
	/* indicates the end of the jump table is read.		*/
	case JTAB:
		(void) sprintf(mneu,"***JUMP TABLE BEGINNING***");
		printline();
		lookbyte();
		if (curbyte == FILL) {
			(void) sprintf(mneu,"FILL BYTE FOR ALIGNMENT");
			printline();
			(void) dbprintf("\t");
			lookbyte();
			tmpshort = curbyte;
			lookbyte();
		}
		else {
			tmpshort = curbyte;
			lookbyte();
			(void) dbprintf("\t");
		}
		(void) sprintf(mneu,"");
		while ((curbyte != 0x00ff) || (tmpshort != 0x00ff)) {
			printline();
			(void) dbprintf("\t");
			lookbyte();
			tmpshort = curbyte;
			lookbyte();
		}
		(void) sprintf(mneu,"***JUMP TABLE END***");
		return loc;

	/* float reg */
	case F:
		(void) sprintf(mneu,"%s%%st(%1.1d)",mneu,r_m);
		return loc;

	/* float reg to float reg, with ret bit present */
	case FF:
		if ( opcode2 >> 2 & 0x1 ) {
			/* return result bit for 287 instructions	*/
			/* st -> st(i) */
			(void) sprintf(mneu,"%s%%st,%%st(%1.1d)",mneu,r_m);
		}
		else {
			/* st(i) -> st */
			(void) sprintf(mneu,"%s%%st(%1.1d),%%st",mneu,r_m);
		}
		return loc;

	/* an invalid op code */
	case AM:
	case DM:
	case OVERRIDE:
	case PREFIX:
	case UNKNOWN:
		sprintf(mneu,"***** Error - bad opcode\n");
		errlev++;
		return loc;

	default:
		(void) dbprintf("dis bug: notify implementor:");
		(void) dbprintf(" case from instruction table not found");
		return loc;
	} /* end switch */
}

/*
 *	void get_modrm_byte (mode, reg, r_m)
 *
 *	Get the byte following the op code and separate it into the
 *	mode, register, and r/m fields.
 * Scale-Index-Bytes have a similar format.
 */

void
get_modrm_byte(mode, reg, r_m)
unsigned	*mode;
unsigned	*reg;
unsigned	*r_m;
{
	extern	void	fetchbyte();	/* in _utls.c */
	extern	unsigned short curbyte;	/* in _extn.c */

	fetchbyte();

	*r_m = curbyte & 0x7; /* r/m field from curbyte */
	*reg = curbyte >> 3 & 0x7; /* register field from curbyte */
	*mode = curbyte >> 6 & 0x3; /* mode field from curbyte */
}

/*
 *	void check_override (opindex)
 *
 *	Check to see if there is a segment override prefix pending.
 *	If so, print it in the current 'operand' location and set
 *	the override flag back to false.
 */

void
check_override(opindex)
int opindex;
{
	if (overreg) {
		(void) sprintf(operand[opindex],"%s",overreg);
		(void) sprintf(symarr[opindex],"%s",overreg);
	}
	overreg = (char *) 0;
}

/*
 *	void displacement (no_bytes, opindex, value)
 *
 *	Get and print in the 'operand' array a one, two or four
 *	byte displacement from a register.
 */

void
displacement(no_bytes, opindex, value)
int no_bytes;
int opindex;
long *value;
{
	char	temp[(NCPS*2)+1];
	void check_override();
	fetchbytes(no_bytes, temp, value);
	check_override(opindex);
	(void) sprintf(operand[opindex],"%s%s",operand[opindex],temp);
}


get_operand(mode, r_m, wbit, opindex)
unsigned mode;
unsigned r_m;
int wbit;
int opindex;
{
	extern	char	dispsize16[8][4];	/* tables.c */
	extern	char	dispsize32[8][4];
	extern	char	*regname16[4][8];
	extern	char	*regname32[4][8];
	extern	char	*indexname[8];
	extern	char 	*REG16[8][2];	  /* in tables.c */
	extern	char 	*REG32[8][2];	  /* in tables.c */
	extern	char	*scale_factor[4];	  /* in tables.c */
	int dispsize;   /* size of displacement in bytes */
	int dispvalue;  /* value of the displacement */
	char *resultreg; /* representation of index(es) */
	char *format;   /* output format of result */
	char *symstring; /* position in symbolic representation, if any */
	int s_i_b;      /* flag presence of scale-index-byte */
	unsigned ss;    /* scale-factor from opcode */
	unsigned index; /* index register number */
	unsigned base;  /* base register number */
	char indexbuffer[16]; /* char representation of index(es) */

	/* if symbolic representation, skip override prefix, if any */
	check_override(opindex);

	/* check for the presence of the s-i-b byte */
	if (r_m==ESP && mode!=REG_ONLY && !addr16) {
		s_i_b = TRUE;
		get_modrm_byte(&ss, &index, &base);
	}
	else
		s_i_b = FALSE;

	if (addr16)
		dispsize = dispsize16[r_m][mode];
	else
		dispsize = dispsize32[r_m][mode];

	if (s_i_b && mode==0 && base==EBP) dispsize = 4;

	if (dispsize != 0)
		displacement(dispsize, opindex, &dispvalue);

	if (s_i_b) {
		register char *basereg = regname32[mode][base];
		(void) sprintf(indexbuffer, "%s%s,%s", basereg,
			indexname[index], scale_factor[ss]);
		resultreg = indexbuffer;
		format = "%s(%s)";
	}
	else { /* no s-i-b */
		if (mode == REG_ONLY) {
			format = "%s%s";
			if (data16)
				resultreg = REG16[r_m][wbit] ;
			else
				resultreg = REG32[r_m][wbit] ;
		}
		else { /* Modes 00, 01, or 10 */
			if (addr16)
				resultreg = regname16[mode][r_m];
			else
				resultreg = regname32[mode][r_m];
			if (r_m ==EBP && mode == 0) { /* displacement only */
				format = "%s";
			}
			else { /* Modes 00, 01, or 10, not displacement only, and no s-i-b */
				format = "%s(%s)";
			}
		}
	}
	(void) sprintf(operand[opindex],format,operand[opindex], resultreg);
}

/*
** fetchbytes() reads no_bytes from a file and converts them into destbuf.
** A sign-extended value is placed into destvalue if it is non-null.
*/
fetchbytes(no_bytes, destbuf, destvalue)
int no_bytes;
char *destbuf;
long *destvalue;
{
	extern	void	fetchbyte();	/* from _utls.c */
	extern	unsigned short curbyte;	/* from _extn.c */

	int i;
	unsigned long shiftbuf = 0;
	char *format;
	long value;

	for (i=0; i<no_bytes; i++) {
		fetchbyte();
		shiftbuf |= (long) curbyte << (8*i);
	}

	switch(no_bytes) {
		case 1:
			format = "0x%2lx";
			if (destvalue)
				*destvalue =
					(shiftbuf & 0x80) ?
					shiftbuf | ~0xffL : shiftbuf & 0xffL;
			break;
		case 2:
			format = "0x%4lx";
			if (destvalue) *destvalue = (short) shiftbuf;
			break;
		case 4:
			format = "0x%8lx";
			if (destvalue) *destvalue = shiftbuf;
			break;
	}
	sprintf(destbuf,format,shiftbuf);
}


/*
 *	void imm_data (no_bytes, opindex)
 *
 *	Determine if 1, 2 or 4 bytes of immediate data are needed, then
 *	get and print them.
 */

void
imm_data(no_bytes, opindex)
int no_bytes;
int opindex;
{
	long value;
	int len = strlen(operand[opindex]);

	operand[opindex][len] = '$';
	fetchbytes(no_bytes, &operand[opindex][len+1], &value);
}


/*
 *	get_opcode (high, low)
 *	Get the next byte and separate the op code into the high and
 *	low nibbles.
 */

void
get_opcode(high, low)
unsigned *high;
unsigned *low;		/* low 4 bits of op code   */
{
	extern	unsigned short curbyte;		/* from _extn.c */
	extern	void	fetchbyte();

	fetchbyte();
	*low = curbyte & 0xf;  /* ----xxxx low 4 bits */
	*high = curbyte >> 4 & 0xf;  /* xxxx---- bits 7 to 4 */
}
