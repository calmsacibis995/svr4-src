/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:i386/gencode.c	1.7"


#include <stdio.h>
#include <symbols.h>
#include "expand.h"
#include "systems.h"
#include "instab.h"
#include "instr.h"
#include "parse.h"
#include "gendefs.h"

/*
 *	"gencode.c" contains the code generation routines for the
 *	first pass of the Basic-16 Assembler. These routines are
 *	called by "yyparse" to determine optimizations, new opcodes,
 *	or confirm proper use of an instruction. Since this is the
 *	only general purpose file in pass one, the flag parsing
 *	routine, "flags", is here.
 */

/*
 *	To understand the generation of the instruction bit pattern
 *	it is necessary to note the instruction format for the 8086.
 *
 *	+------+-+-+--+---+---+--------+--------+--------+--------+
 *	|  op  |d|w|  |reg|r/m| offset | offset | immed  | immed  |
 *	| code | | |  |   |   | addr   | (w==1) |  data  |  data  |
 *	+------+-+-+--+---+---+--------+--------+--------+--------+
 *		    ^
 *		   mod
 *
 *	opcode	This field is used exclusively to represent the opcode
 *		for the instruction. The complete opcode for an instruction
 *		may be spread across other field, i.e. "d", "w", and "reg".
 *
 *	d	This is the direction bit for dyadic instructions. The 
 *		direction means either "to" or "from" a register. In
 *		general, all dyadic instructions must use a register as
 *		one of the operands.
 *
 *	w	This is the word bit, 1 denotes a word oriented instruction
 *		and 0 denotes a byte instruction.
 *
 *	mod	This is the mode field. It specifies how the "r/m" field
 *		is used in locating the operand. For example, mod==10
 *		and r/m==110 means 16-bit offset indirect through the "bp"
 *		base register.
 *
 *	reg	This field contains the register operand in a dyadic
 *		instruction or opcode information in some optimized
 *		instruction forms and monadic instructions.
 *
 *	r/m	This is the register/memory field. This conatins either
 *		the register designation or the base register and/or
 *		the index register designation. The only exception is
 *		mod==00 and r/m==110 which indicates 16-bit external
 *		addressing mode.
 *
 *	The opcode is always stored in the "opcode" field of the
 *	instruction table entry. Note that if the reg field or the
 *	"d" bit is unknown then that field is zero in the "opcode" field of
 *	the instruction. The variable "dbit" is set to the proper
 *	value once there is enough information that its value can
 *	be ascertained. The "mod" (addressing mode), "reg" (register),
 *	and "r/m" (register/memory) fields contain all addressing mode
 *	information for the instruction. This is possible since some
 *	optimizations and immediate addressing mode are represented by
 *	different opcodes. All of the "mod", "reg", and "r/m" information
 *	is kept in "memloc". Therefore to generate a complete opcode
 *	and its associated addressing mode it may be necessary to
 *	logical 'or' the "opcode", "dbit", and "memloc" fields.
 *
 *	Immediate data always is generated last, after the address offset
 *	if it is present.
 *
 */

extern char
#if !ONEPROC
	*xargp[],
#endif
	newname[];

extern short
	argindex,
	spanopt;	/* optimize flag */

extern long
	newdot;		/* up-to-date value of "." */

extern symbol
	*dot;		/* current location counter */

extern short
	nameindx,
	dbit;

extern instr *
	newins;

extern union addrmdtag
	memloc;

extern union sib_byte_tag memsib ;



unsigned short
actreloc(type,deflt)
register short type;
int deflt;
{
	if (type & X86TYPE)
	  return((unsigned short)((type & LO32TYPE) ? LO32BITS : HI12BITS));
	else 
		switch (type) {
			case RELGOT32:
			case RELPLT32:
			case RELGOTOFF:
			  return((unsigned short) type);
			default:
			  return((unsigned short)deflt);
		}
} /* actreloc */



/*--------------------------------------------------------------------------
 * generate 32 bit address information
 *
 *	in most cases ( except eax ) the mode r/m byte contains
 *	the source or destination register in the reg field.
 *	this is encoded into opnd1 by the caller (a production in parse.y )
 *
 *	The direction of operation is encoded into the opcd parameter
 *	by the caller.  Direction means to or from the register.
 *
 *	This is where the optimization for 8 bit displacements takes place.
 *	the productions in parse.y always set the mod field
 *	in the mod r/m byte for a 32 bit displacement.
 *	this routine check to see if the displacement will
 *	fit into a signed byte.  If it will we modify the mod field
 *	for an 8 bit displacement and only put out one byte of
 *	displacement information to the temp file.
 *-------------------------------------------------------------------------*/

addrgen32(opcd, opnd1)
long	opcd ;				/* the opcode */
register addrmode *opnd1 ;		/* the opcodes operand, mainly
					   contains address info */
{
register long val ;

	val = opnd1->adexpr.expval ;

	if (memloc.addrmd.mod == DISP32 && opnd1->adexpr.symptr == NULLSYM) {
	  if ((val >= -128L) && (val <= 127L)) {
	    memloc.addrmd.mod = DISP8 ;
	    generate(16, NOACTION, (long)(opcd | memloc.addrmdfld), NULLSYM) ;
	    if (memloc.addrmd.mod != 3 && memloc.addrmd.rm == 4)
	      generate (8, NOACTION, (long)(memsib.sib_byte), NULLSYM) ;
	    generate(8, NOACTION, val, NULLSYM) ;
	    return ;
	  }
	}
	  
	generate (16, NOACTION, (long)(opcd | memloc.addrmdfld), NULLSYM) ;
	if (memloc.addrmd.mod != 3 && memloc.addrmd.rm == 4)
	  generate (8, NOACTION, (long)(memsib.sib_byte), NULLSYM) ;
	if (opnd1->admode & EXPRMASK)
	  if (UNEVALUATED(opnd1->adexpr))
	     exp_generate(32, SWAPWB, opnd1->adexpr);
	  else
	     generate (32, actreloc(opnd1->adexpr.reltype, SWAPWB),
		    val, opnd1->adexpr.symptr) ;

}

/*-------------------------------------------------------------------
 * laddrgen32 - long address generator for 32 bit address.
 * Decides weather or not address displacement information can be
 * expressed in 8 or 32 bits. 
 * It assumes the opcodes are 24 bits long.
 * 16 bits of opcode, 8 bits of mode r/m byte, and
 * an sib byte if necessary.
 *------------------------------------------------------------------*/
laddrgen32(opcd, opnd1)
long	opcd ;				/* the opcode */
register addrmode *opnd1 ;		/* the opcodes operand, mainly
					   contains address info */
{
register long val ;

	val = opnd1->adexpr.expval ;

	if (memloc.addrmd.mod == DISP32 && opnd1->adexpr.symptr == NULLSYM) {
	  if ((val >= -128L) && (val <= 127L)) {
	    memloc.addrmd.mod = DISP8 ;
	    generate(24, NOACTION, (long)(opcd | memloc.addrmdfld), NULLSYM) ;
	    if (memloc.addrmd.mod != 3 && memloc.addrmd.rm == 4)
	      generate (8, NOACTION, (long)(memsib.sib_byte), NULLSYM) ;
	    generate(8, NOACTION, val, NULLSYM) ;
	    return ;
	  }
	}
	  
	generate (24, NOACTION, (long)(opcd | memloc.addrmdfld), NULLSYM) ;
	if (memloc.addrmd.mod != 3 && memloc.addrmd.rm == 4)
	  generate (8, NOACTION, (long)(memsib.sib_byte), NULLSYM) ;
	if (opnd1->admode & EXPRMASK)
	  if (UNEVALUATED(opnd1->adexpr))
	     exp_generate(32, SWAPWB, opnd1->adexpr);
	  else
	     generate (32, actreloc(opnd1->adexpr.reltype, SWAPWB),
		    val, opnd1->adexpr.symptr) ;

}


/*---------------------------------------------------------------
 * loop instruction generation routine.
 * This assembler handles the loop instruction as a span
 * dependendt instruction.
 * If the span is short it just generates the loop if it's long
 * it generates:
 *
 *		loop[ze(nz)(ne)] L1
 *		jmps L2
 *	L1:	jmp  (anywhere in segment)
 *	L2:	
 *
 * in essence this is a long form of the loop instruction.
 * this routine is for the 80386, 32 bit address mode.
 * it was addapted from the 16 bit version by just changing the
 * the number of bits to emit for the long displacement.
 *----------------------------------------------------------------*/

loopgen(insptr,opnd1)
register instr *insptr;
register addrmode *opnd1;
{
register symbol *sym;
long val;
SDI *sdp;

	sym = opnd1->adexpr.symptr;
	val = opnd1->adexpr.expval;

	  if (sdp =  (shortsdi(sym,val))) {

	      int bits = LOOP_BSZ * BITSPBY;

	      sdp->sd_itype = IT_LOOP;
	      if (!spanopt)
	      {
			sdp->sd_flags &= ~SDF_BYTE;
			bits = LOOP_WSZ * BITSPBY;
	      }
		
	
	      generate(bits,LOOPOPT,insptr->opcode,sdp->sd_sym);
	      return;
	    
	  }

	/*
	 * we only get here if we know it's going to be a long
	 * loop or jump.
	 */
	generate(insptr->nbits,NOACTION,insptr->opcode,NULLSYM);
	generate(8,NOACTION,0x02L,NULLSYM);
	generate(8,NOACTION,0xebL,NULLSYM);
	generate(8,NOACTION,0x05L,NULLSYM);
	generate(8,NOACTION,0xe9L,NULLSYM);
	generate(32,LONGREL,val,sym);
}
/*------------------------------------------------------------------
 * conditional jump generation routine.
 *
 * treats conditional jumps as span dependent instructions.
 * if they're short or there is a possibility that they will be
 * short it generates a short form of the instruction
 * if they're long it reverses the sense of the jump and jumps
 * around a long jump.
 *
 * The sense reverse and jump around should be changed when the
 * new long conditionals are implemented.
 *
 * This was addapted from the 16 bit version.
 *-----------------------------------------------------------------*/

jmp1opgen(insptr,opnd1)
register instr *insptr;
register addrmode *opnd1;
{
	register symbol *sym;
	long val;
	SDI  *sdp;

	sym = opnd1->adexpr.symptr;
	val = opnd1->adexpr.expval;

	if ((sdp = shortsdi(sym,val)) && spanopt) {
	
		int bits = CJMP_BSZ * BITSPBY;

		sdp->sd_itype = IT_CJMP;

		generate(bits,CJMPOPT,insptr->opcode,sdp->sd_sym);
		return;
	  } /* if (sdp */
	/*
	 * change from short pcrel jcc to long pcrel.
	 */
	generate(8,NOACTION,0x0fL,NULLSYM);
	generate(insptr->nbits,NOACTION,0x80L | (insptr->opcode&0x0fL),NULLSYM);
	generate(32,LONGREL,val,sym);

} /* jmpopgen */

/*---------------------------------------
 * short unconditional jmp 
 * addapted from the 16 version
 *--------------------------------------*/

jmp2opgen(insptr,opnd1)
register instr *insptr;
register addrmode *opnd1;
{
	register symbol *sym;
	long val;
	SDI  *sdp;

	val = opnd1->adexpr.expval;
	sym = opnd1->adexpr.symptr;

	if (spanopt)
		if (sdp = (shortsdi(sym,val)))
		{
			int bits = UJMP_BSZ * BITSPBY;

			sdp->sd_itype = IT_UJMP;

			generate(bits,UJMPOPT, (long)((short)(insptr->opcode) 
				| 0x02), sdp->sd_sym);
			return;
		    /*  L_SDI: fall through */
		}
	  

	generate(insptr->nbits,NOACTION,insptr->opcode,NULLSYM);
	generate(32,LONGREL,val,sym);
}

defgen(iname,opnd1)
	char *iname;
	addrmode *opnd1;
{
	nameindx = copystr(iname,newname);
	newname[nameindx++] = 'I';
	newname[nameindx++] = 'N';
	newname[nameindx] = '\0';
	newins = instlookup(newname);
	if (newins != NULL) {
		addrgen32(newins->opcode,opnd1);
	}
	else
		aerror("Cannot find deferred instr");
}

/*----------------------------------------------------------------
 * 32 bit version of rmongen.
 * this addjustes the opcode for register or regment register
 * operations.
 *----------------------------------------------------------------*/

rmongen32(insptr,opnd1)
instr *insptr;
register addrmode *opnd1;
{

	if ((opnd1->admode & REGMASK)) {
	  nameindx = copystr(insptr->name,newname);
	  newname[nameindx++] = 'R';
	  newname[nameindx] = '\0';
	  newins = instlookup(newname);
	  if (newins != NULL) {
	    generate(newins->nbits,NOACTION,
	      (long)(newins->opcode | opnd1->adreg),NULLSYM);
	  }
  	  else
	    aerror("Cannot find instr");
  	return;
	}

	addrgen32(insptr->opcode,opnd1);

}


/*--------------------------------------------------------------------------
 * generate a 32 bit move instruction.
 *
 *	in most cases the mode r/m byte contains
 *	the source or destination register in the reg field.
 *	this is encoded into opnd1 by the caller (a production in parse.y )
 *
 *	when eax is either the source or destination register
 *	the 80386 offers a special addressing mode that does not
 *	require the mod r/m byte.  It just requires a 1 byte opcode
 *	and 32 bits of address info.  This address info will be relocated
 *	by the linker.
 *
 *	This is where direct addressing to/from eax is checked.
 *	If this is found to be the case the shorter form of the instruction
 *	is generated.
 *-------------------------------------------------------------------------*/

movgen32(insptr, opnd1)
instr	*insptr ;		/* pointer to the instruction */
register addrmode *opnd1 ;	/* pointer to the memory operand */
{
register long opcd ;

	if (memloc.addrmd.reg == AREGOPCD && opnd1->admode == EXADMD) {
	  nameindx = copystr(insptr->name, newname) ;
	  newname[nameindx++] = 'A' ;
	  newname[nameindx] = '\0' ;
	  newins = instlookup(newname) ;
	  if (newins != NULL) {
	    opcd = newins->opcode ;
	    if (dbit == DBITOFF)
	      opcd |= 0x02L ;
	    generate (newins->nbits, NOACTION, opcd, NULLSYM) ;
            if (opnd1->admode & EXPRMASK)
		  if (UNEVALUATED(opnd1->adexpr))
		     exp_generate(32, SWAPWB, opnd1->adexpr);
            else
		     generate (32, actreloc(opnd1->adexpr.reltype, SWAPWB),
		      opnd1->adexpr.expval, opnd1->adexpr.symptr) ;
	  }
	  else
	    aerror("cannot find mov instruction") ;
	  return ;
	}

	addrgen32((long)(insptr->opcode | dbit), opnd1) ;

}

/*------------------------------------------------------------------------
 *	This routine generates the most optimal opcode and addressing
 *	sequence for the immediate addressing mode.  A routine to
 *	put out the immediate data is called after this one.
 *
 *	The 80386 offers a single byte opcode for immediate data
 *	to a register.
 *	And the 80386 requires a different opcode for immediate
 *	data to memory.
 *------------------------------------------------------------------------*/

mov2gen32 (iname,opnd2)
char	 *iname;
addrmode *opnd2;
{

	nameindx = copystr(iname,newname);
	newname[nameindx++] = 'I';

	if (spanopt && (opnd2->admode & REGMASK)) {
	  newname[nameindx++] = 'R';
	  newname[nameindx] = '\0';
	  newins = instlookup(newname);
	  if (newins != NULL) {
	    generate(newins->nbits,NOACTION,
	      (long)((short)(newins->opcode) | opnd2->adreg),NULLSYM);
	  }
	  else
	    aerror("Cannot find mov instr");
	  return;
	}

	/* general memory reference */

	newname[nameindx++] = 'G';
	newname[nameindx] = '\0';
	newins = instlookup(newname);
	if (newins != NULL) {
	  addrgen32(newins->opcode,opnd2);
	}
	else
	  aerror("Cannot find mov instr");
}


/*----------------------------------------------------
 * immediate with accumulator for dyadic istructions
 *---------------------------------------------------*/

dyad1gen32(iname)
char *iname;
{

	nameindx = copystr(iname,newname);
	newname[nameindx++] = 'I';
	newname[nameindx++] = 'A';
	newname[nameindx] = '\0';
	newins = instlookup(newname);
	if (newins != NULL) {
	  generate(newins->nbits,NOACTION,newins->opcode,NULLSYM);
	}
	else
	  aerror("Cannot find dyadic (acc)");
}

/*-----------------------------------------------------------------------
 * general dyadic instructions with general memory
 * this checks for an immediate with the accumulator and calls dyad1gen32
 * if this is found to be the case.
 *
 * it is the callers responsibility to emit the immediate data
 * after this routine is called.
 *
 * the dbit ( direction bit ) is in the same bit position as the s bit 
 * ( sign extension bit ).  So the dbit is used here to indicate sign
 * extension of a byte into a word or dword.  It's the callers
 * responsibility to set the dbit.
 *------------------------------------------------------------------------*/

dyad2gen32(iname,opnd2)
char *iname;
addrmode *opnd2;
{

	if (spanopt && (opnd2->admode & AREGMASK)) {
	  /* optimize for accumulator */
	  dyad1gen32(iname);
	  dbit = DBITOFF;
	  return;
	}

	nameindx = copystr(iname,newname);
	newname[nameindx++] = 'I';
	newname[nameindx++] = 'G';
	newname[nameindx] = '\0';
	newins = instlookup(newname);

	if (newins != NULL) {
	  addrgen32((long)(newins->opcode | dbit),opnd2);
	}
	else
	  aerror("Cannot find dyad");
}


/* xchg register with accumulator */

xchgopt(iname,reg)
	char *iname;
	char reg;
{
	nameindx = copystr(iname,newname);
	newname[nameindx++] = 'A';
	newname[nameindx] = '\0';
	newins = instlookup(newname);
	if (newins != NULL) {
		generate(8,NOACTION, (newins->opcode | (long)reg),NULLSYM);
	}
	else
		aerror("Cannot find xchg instr");
}


/*-------------------------------------------------------
 * register/memory with immediate test generation code,
 * with 32 bit address.
 * It's up to the caller to call immedgen to put out the
 * immediate data after this is called.  This just generates
 * the opcode and addressing information.
 *
 * if the register is the accumulator, generate the short
 * form of the iinstruction.
 *-------------------------------------------------------*/

testgen32(iname,opnd2)
char *iname;
addrmode *opnd2;
{
	if (spanopt && (opnd2->admode & AREGMASK)) {
	  /* optimize for accumulator */
	  dyad1gen32(iname);
	  return;
	}

	nameindx = copystr(iname,newname);
	newname[nameindx++] = 'I';
	newname[nameindx++] = 'G';
	newname[nameindx] = '\0';

	newins = instlookup(newname);
	if (newins != NULL) {
	  addrgen32(newins->opcode,opnd2);
	}
	else
	  aerror("Cannot find test");
}



/*-----------------------------------------------------------
 * generate immediate data for 32bit immediate data mode.
 * This means the data will either be 8 or 32 bits.
 *----------------------------------------------------------*/

immedgen32 (nbits, immed)
register short nbits;
register addrmode *immed;
{
register short type ;

	if (type = immed->adexpr.reltype & X86TYPE) {
	  if ((nbits == 8) && (type & HI12TYPE))
	    yyerror("Cannot request 12 bits in 8 bit expr");
	  generate(nbits, 
		   (unsigned short)((type & LO32TYPE) ? LO32BITS : HI12BITS),
		   immed->adexpr.expval, immed->adexpr.symptr);
	  return;
	}

	if (UNEVALUATED(immed->adexpr))
		exp_generate(nbits, SWAPWB, immed->adexpr);

	else
		{ unsigned short action;

		  action = (nbits == 8) ? RESABS : actreloc(immed->adexpr.reltype, SWAPWB),
			
		generate(nbits, action,immed->adexpr.expval, immed->adexpr.symptr);
		}
}



immedgen(nbits,immed)
	register short nbits;
	register addrmode *immed;
{
	register short type;

	if (type = immed->adexpr.reltype & X86TYPE) {
		if ((nbits == 8) && (type & HI12TYPE))
			yyerror("Cannot request 12 bits in 8 bit expr");
		generate(nbits,
			 (unsigned short)((type & LO32TYPE) ? LO32BITS : HI12BITS),
			 immed->adexpr.expval, immed->adexpr.symptr);
		return;
	}
	if (UNEVALUATED(immed->adexpr))
		exp_generate(nbits, SWAPWB, immed->adexpr);

	else
		generate(nbits, (unsigned short)((nbits == 8) ?
			RESABS : SWAPB),
			immed->adexpr.expval, immed->adexpr.symptr);
}

flags(ch)
	register char ch;
{
	char errmsg[28];

	if (ch == 'M')
		{}
	else {
		sprintf(errmsg,"Illegal flag (%c) - ignored",ch);
		werror(errmsg);
	}
}

copystr(str1,str2)
	register char *str1, *str2;
{
	register int n = 0;

	while (*str2++ = *str1++) ++n;
	return(n);
}


