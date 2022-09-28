/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/i386/Instr.h	1.5"
#ifndef	Instr_h
#define	Instr_h

#include	"Itype.h"
#include	"Reg.h"

#define 	NLINE	36

class Process;

enum Instrtype {
	NOTINSTR,	// not an instruction
	BREAKPT,	// breakpoint instr
	OTHERINSTR,	// some other instruction type
	SAVEREG,	// save registers
	SUBR,		// jump to subroutine (aka "call", "jsr", etc)
};

struct Instrdata;
enum Opndtype;
struct Modedata;

class Instr {
	Process *	process;
	Iaddr		addr;
	char		opcode;
	unsigned char byte[NLINE]; 
	int		byte_cnt;
	char		get_byte();
	int			make_mnemonic( char *);
	long		lngval;
	unsigned short curbyte;
	unsigned short get_opcode(unsigned *, unsigned *);
	char *		compoff(Iaddr, char *);
	void		getbytes(int, char *, long *);
	void 		imm_data(int, int);
	void		displacement(int, int, long *);
	void		get_modrm_byte(unsigned *, unsigned *, unsigned *);
	void		check_override(int);
	void		get_operand(unsigned, unsigned, int, int);
	int		get_text( Iaddr ); 	// get text from process, as is
	int 		deasm1(Iaddr);
	int		get_text_nobkpt( Iaddr );
				 // get text from process, with no breakpoints
public:

	Instr( Process * );


	Iaddr		retaddr( Iaddr );	// return address if call instr
	Iaddr		next_instr( Iaddr );		// address of next instr
	int		is_bkpt( Iaddr );	// is this a breakpoint instruction?
	char *		deasm( Iaddr, int = 0 ); // This function returns the 
						// assembly language instructions
	Iaddr		adjust_pc();		// adjust pc after breakpoint
	int		nargbytes( Iaddr ); 	// number of argument bytes
	Iaddr		fcn_prolog( Iaddr, short ); // function prolog
	Iaddr		fcn_prolog_sr( Iaddr, short, RegRef[] ); // function prolog
							// with saved registers
	Iaddr           brtbl2fcn( Iaddr );             // branch table to funciton
	Iaddr           fcn2brtbl( Iaddr, int );        // function to branch table
	int		iscall( Iaddr );	// is CALL instruction

	Iaddr		jmp_target( Iaddr );	// target addr if JMP
};
#endif
