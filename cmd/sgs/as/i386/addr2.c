/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:i386/addr2.c	1.8"
#include <stdio.h>
#include <storclass.h>
#include <syms.h>
#include <sys/elf_386.h>
#include "symbols.h"
#include "systems.h"
#include "gendefs.h"
#include "section.h"
#include "expr.h"


extern unsigned short 
	line;

extern BYTE
	*longsdi;

extern unsigned int
	relent;


extern short
	transvec,
	filedef;

extern long
	newdot;

extern FILE
	*fdrel,
	*fdcode;

extern symbol
	*dot;

extern SYMENT
	sment;

typedef struct {
	char lobyte;
	char hibyte;
} bytes;

static short
	currbit = 0;

static unsigned short
	bitfield = 0;

extern void aerror();
extern void codgen();
extern void getcode();
extern void yyerror();

static void relpc8();
static void longrel();

static void
newstmt(code)
codebuf *code;
{
	line = (unsigned short) (code->cvalue);
	dot->value = newdot;
}

static void
symrel(rsym,rtype)
	symbol *rsym;
	int rtype;
{
	prelent trelent;
	trelent.relval = newdot;
	trelent.relname = rsym->name;
	trelent.reltype = (short) rtype;
	fwrite((char *)(&trelent),sizeof(prelent),1,fdrel);
	relent++;
}

static void
cjmpopt(code)
	register codebuf *code;
{
	register symbol *sym;
	long	opcd;

	 register SDI    *sdp;
	sdp = Sdi;
	for (;;)
		{
			if (++sdp < Sdibox->b_end)
				if (sdp->sd_itype != IT_LABEL)
					break;
				else
					continue;
			if ((Sdibox = Sdibox->b_link) == 0)
				aerror("no cjmp boxes");
			sdp = Sdibox->b_buf - 1;
		}
	Sdi = sdp;
	opcd = code->cvalue;
	sym = code->csym;		 /* guaranteed a symbol is */
					/* there by shortsdi in pass 1 */
	if (sdp->sd_flags & SDF_BYTE)
		{
			codgen(8,opcd);
			code->cnbits=8;
			code->cvalue = sym->value + sdp->sd_off - (newdot + 1L);
		}
	else {
		codgen(8,0x0fL);
		codgen(8,0x80L|(opcd&0x0fL));
		code->cvalue = sdp->sd_off;
		/* codout (which called this routine) will call codgen	*/
		/* with whatever is left in "code", so must let codout	*/
		/* generate the last field of code			*/
		longrel(code);
		code->cnbits = 32;
	} /* else */
} /* cjmpopt */

static void
ujmpopt(code)
	register codebuf *code;
{
	register symbol *sym;
        register SDI    *sdp;
	long	opcd;

	opcd = code->cvalue;
	sym = code->csym;		 /* guaranteed a symbol is */
					/* there by shortsdi in pass 1 */
	sdp = Sdi;
        for (;;)
        {
                if (++sdp < Sdibox->b_end)
                        if (sdp->sd_itype != IT_LABEL)
                                break;
                        else
                                continue;
                if ((Sdibox = Sdibox->b_link) == 0)
                        aerror("no ujmp boxes");
                sdp = Sdibox->b_buf - 1;
        }
        Sdi = sdp;
	if (sdp->sd_flags & SDF_BYTE)
		{
			codgen(8,opcd);
			code->cnbits=8;
			code->cvalue = sym->value + sdp->sd_off - (newdot + 1L);
		}
	else {
		opcd ^= 0x02L;
		codgen(8,opcd);
		code->cvalue = sdp->sd_off;
		longrel(code);
		codgen(32,code->cvalue);

		/* codout (which called this routine) will call codgen	*/
		/* with whatever is left in "code", so must let codout	*/
		/* generate the last field of code			*/
		/* As a kludge to make "align 4" in text work properly, */
		/* we make sure the difference between the long and	*/
		/* short sizes is 4 bytes! */
		code->cvalue = 0x90L;	/* nop */
		code->cnbits = 8;
	} /* else */
} /* ujmpopt */

static void
loopopt(code)
	register codebuf *code;
{
	register symbol *sym;
        register SDI    *sdp;
	long	opcd;

	opcd = code->cvalue;
	sym = code->csym;		 /* guaranteed a symbol is */
					/* there by shortsdi in pass 1 */
#if 0
	getcode(code);
#endif
	sdp = Sdi;
        for (;;)
        {
                if (++sdp < Sdibox->b_end)
                        if (sdp->sd_itype != IT_LABEL)
                                break;
                        else
                                continue;
                if ((Sdibox = Sdibox->b_link) == 0)
                        aerror("no loop  boxes");
                sdp = Sdibox->b_buf - 1;
        }
        Sdi = sdp;
	if (sdp->sd_flags & SDF_BYTE)
		{
			codgen(8,opcd);
			code->cvalue = sdp->sd_off;
			code->cnbits = 8;
			relpc8(code);
		}
	else {
		codgen(8,opcd);
		codgen(8,0x02L);
		codgen(8,0xebL);
		codgen(8,0x05L);
		codgen(8,0xe9L);
		code->cvalue = sdp->sd_off;
		longrel(code);
		codgen(32,code->cvalue);

		/* codout (which called this routine) will call codgen	*/
		/* with whatever is left in "code", so must let codout	*/
		/* generate the last field of code			*/
		/* As a kludge to make "align 4" in text work properly, */
		/* we make sure the difference between the long and	*/
		/* short sizes is a multiple of 4 bytes! */
		code->cvalue = 0x90L;	/* nop */
		code->cnbits = 8;
	} /* else */
} /* loopopt */

static void
resabs(code)
	codebuf *code;
{
	register symbol *sym = code->csym;

	if (sym != NULL) {
	  if (ABS_SYM(sym)) {
	    code->cvalue += sym->value; /* sym must be non-null */
	    return;
	  } else if (UNDEF_SYM(sym)) {
	    yyerror("Undefined symbol in absolute expression");
	    return;
	  } else {
	    yyerror("Relocatable symbol in absolute expression");
	    return;
	  }
	}
} /* resabs */

static void
relpc8(code)
	codebuf *code;
{
	symbol *sym; 
	register short relval;
	long val;

	sym = code->csym;
	val = code->cvalue;
	if (sym != NULLSYM)
		if (sym->sectnum != dot->sectnum && ABS_SYM(sym))
			aerror("relpc8: reference to symbol in another section");
		else
			val += sym->value;
	relval = (short)(val - (newdot + 1L)); /* compute number of bytes */
	code->cvalue = (long)relval;
	if ((relval < -128) || (relval > 127))
		aerror("relpc8: offset out of range");
} /* relpc8 */

static short
swap2(val)
	register unsigned short val;
{
	return (val >> 8) | ((val << 8) & 0xff00);
}

static long
swap4(val)
	register unsigned long val;
{
	register unsigned long temp;

	temp = ((val >> 8) & 0x00ff00ff) | ((val << 8) & 0xff00ff00);
	return (temp >> 16) | ((temp << 16) & 0xffff0000);
}

static void
swapb(code)
	register codebuf *code;
{
	register symbol *sym;
	short val;
	sym = code->csym;
	val = (short)(code->cvalue);
	if (sym != NULLSYM)
		aerror("illegal 16-bit relocation");
/*
 *	This swabbing is necessary due to the way the 8086
 *	expects its memory reference operands (absolutes in b16 lingo)
 */
	val = swap2( (short)val );
	code->cvalue = (long)val;
}

/* Perform 32-bit absolute relocation.  The final value at the reference
 * point should be the absolute value of the location of the target symbol.   
 * - If the target symbol is a temporary, the assembler will generate a 
 *   direct relocation entry with respect to the section symbol of the
 *   target. (Temporaries are local symbols that do not go in the object file 
 *   symbol table: relocation for them can be done with section symbols.)
 * - Otherwise the assembler will generate a direct relocation entry with
 *   respect to the target symbol.
 */ 
static void
swapwb(code)
	register codebuf *code;
{
	register long val;
	register symbol *target, *rsym;
	target = code->csym;

	val = code->cvalue;
	if (target != NULLSYM) {
		if (LOCAL(target) && UNDEF_SYM(target))
		  	aerror("Illegal direct 32-bit relocation");
		else if (strcmp(target->name, "_GLOBAL_OFFSET_TABLE_") == 0) {
			val += (long) newdot - dot->value;
			symrel(target,R_386_GOTPC);
		} else if (ABS_SYM(target)) {
			val += target->value;
		} else if (TEMP_LABEL(target)) {
			val += target->value;
		  	symrel(lookup(sectab[target->sectnum].name),R_386_32);
		} else 
			symrel(target,R_386_32);
	}
/*
 *	This swabbing is necessary due to the way the 8086
 *	expects its memory reference operands (absolutes in b16 lingo)
 */
	val = swap4( val );
	code->cvalue = val;
}


/* Perform 32-bit pc-relative relocation.  The final value at the reference
 * point should be the byte displacement from the location of the
 * NEXT instruction to the location of the target symbol.   
 * - If the location of the reference point and the target symbol are in 
 *   the same section in the file, then the assembler will perform the 
 *   relocation.  
 * - Else if the target symbol is a temporary (in a different
 *   section from the reference point), the assembler will generate a 
 *   PC-relative relocation entry with respect to the section symbol of the
 *   target. (Temporaries are local symbols that do not go in the object file 
 *   symbol table: relocation for them can be done with section symbols.)
 * - Otherwise the assembler will generate a PC-relative relocation entry with
 *   respect to the target symbol.
 */ 
static void
longrel(code)
	codebuf *code;
{
	register symbol *target;
	register long val;

	target = code->csym;
	if (target == NULLSYM || 
	    ABS_SYM(target) || 
	    (LOCAL(target) && UNDEF_SYM(target))
	    )
	  aerror("Illegal PC-relative relocation");

	val = code->cvalue - 4;
			/* code->cvalue holds a user-supplied addend */
			/* The 4 provides an offset to the next instruction */

	if (target->sectnum == dot->sectnum) {
		val += target->value - newdot;
	} else if (TEMP_LABEL(target)) {
		val += target->value;
		symrel(lookup(sectab[target->sectnum].name),R_386_PC32);
	} else {
		symrel(target,R_386_PC32);
	}

	val = swap4(val);
	code->cvalue = val;
	
}

static void
relgot32(code)
	codebuf *code;
{
	code->cvalue = 0;
	if (TEMP_LABEL(code->csym))
		code->csym->flags |= GO_IN_SYMTAB;
	symrel(code->csym, R_386_GOT32);
}

static void
relplt32(code)
	codebuf *code;
{
	code->cvalue -= 4;
	symrel(code->csym,R_386_PLT32);
	code->cvalue = swap4(code->cvalue);
}

static void
relgotoff(code)
	codebuf *code;
{
	register symbol *target;
	
	target = code->csym;
	if (TEMP_LABEL(target)) {
		code->cvalue += target->value;
		symrel(lookup(sectab[target->sectnum].name),R_386_GOTOFF);
	} else
		symrel(target, R_386_GOTOFF);
	code->cvalue = swap4(code->cvalue);
}


static void
pack8(code)
	codebuf *code;
{
	register short numbits,
			val;

	numbits = code->cnbits;
	resabs(code);
	val = (short)(code->cvalue);

	/* now mask out the proper number of bits */
	val &= (1 << numbits) - 1;
	currbit += numbits;
	bitfield |= val << (8 - currbit);	/* save the bits in their proper place */
	code->cnbits = 0;
}

static void
pack16(code)
	codebuf *code;
{
	register short numbits,
			val;

	numbits = code->cnbits;
	resabs(code);
	val = (short)(code->cvalue);

	/* now mask out the proper number of bits */
	val &= (numbits < 16) ? (1 << numbits) - 1 : 0xffff;
	currbit += numbits;
	bitfield |= val << (16 - currbit);	/* save the bits in their proper place */
	code->cnbits = 0;
}

static void
dumpbits(code)
	codebuf *code;
{
	register short numbits;

	numbits = (short)(code->cvalue);
	if (numbits == NBPW) {	/* swap bytes */
		bitfield = swap2( bitfield );
	}
	code->cvalue = (long)bitfield;

	code->cnbits = (BYTE)numbits;
	currbit = 0;	/* re-initialize */
	bitfield = 0;	/* re-initialize */
}

static void
lo32bits(code)
	register codebuf *code;
{
	register symbol *sym;
	register long val;

	sym = code->csym;
	if (sym != NULLSYM) {
		code->cvalue += sym->value & 0xff;
		if (!(ABS_SYM(sym)))
			symrel(sym,R_386_32);
	}
	if (code->cnbits > 8) {
		val = code->cvalue;
		/* swap bytes */
		val = swap4(val);
		code->cvalue = val;
	}
} /* lo32bits */


static void
hi12bits(code)
	register codebuf *code;
{
	register symbol *sym;
	short	val;
	prelent	trelent;

	sym = code->csym;
	if (code->cnbits == 8)
		yyerror("Unable to place 12-bit segment in 8 bits");
	else {
		if (sym != NULLSYM)
			aerror("illegal high 12-bit relocation");
		/* insure low 4 bits are zero */
		val = (short)(code->cvalue >> 4) & 0xfff0;
		val = swap2( (short)val );
		code->cvalue = (long)val;

	}
}

extern void
	define(),
	setval(),
	setscl(),
	endef(),
  	dotzero();

void (*(modes[CB_AMASK+1]))() = {
	0,		/* NOACTION */
	define,
	setval,
	relgot32,
	relplt32,
	relgotoff,
	setscl,
	endef,
	dotzero,
	newstmt,
	resabs,
	relpc8,
	cjmpopt,
	ujmpopt,
	loopopt,
	swapb,
	longrel,
	pack8,
	pack16,
	dumpbits,
	hi12bits,
	swapwb,
	lo32bits,
	
 };
