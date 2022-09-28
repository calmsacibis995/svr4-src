/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)as:common/codeout.c	1.13"
#include <stdio.h>

#include "systems.h"
#include "symbols.h"
#include <libelf.h>
#include "gendefs.h"
#include "section.h"
#include "expr.h"


extern void aerror();
extern unsigned short	line;
extern Elf *elffile;
extern symbol *dot;
long	newdot;		/* up-to-date value of dot */



static short poscnt = 0;	/* current bit position in outword buffer */
				/* spans from [0 - BITSPOW]	*/
static short bitpos = 0;	/* bit position within a byte in outword buffer	*/
				/* spans from [0 - BITSPBY]	*/

static OUTWTYPE	outword;	/* output buffer - see gendefs.h */
static int mask[] = { 0, 1, 3, 7,
		017, 037, 077,
		0177, 0377, 0777,
		01777, 03777, 07777,
		017777, 037777, 077777,
		0177777 };

static BYTE *new_data;
static int buf_size;
static Elf_Data *data;

void
codgen(nbits, val)
register unsigned nbits;
long val;
{
	register short size;	/* space remaining in outword buffer */
	register long value;	/* working buffer for val */

	if (nbits) {
	    start:	value = val;
			
		/* generate as much of nbits as possible */
		if ((size = BITSPOW - poscnt) > nbits)
			size = (short) nbits;
		value >>= (long) nbits - size;	/* align value to get correct bits */
		value &= mask[size];
		value <<= BITSPOW - poscnt - size;
		outword |= (OUTWTYPE)value;	/* store field in the buffer */
		poscnt += size;
		bitpos += size;
		newdot += bitpos / BITSPBY;
		bitpos %= BITSPBY;
		if (poscnt == BITSPOW) {	/* is buffer full? */
			buf_size++;
			*(new_data + buf_size) = outword;
			poscnt = 0;
			outword = 0;
			if (nbits > size) {	/* more bits? */
				nbits -= size;
				goto start;
			}
		}
	}
}

extern void (*(modes[]))();	/* array of action routine functions */

extern symbol *lookup();


static void exp_eval();

void
codout(sect)
int sect;
{
	register codebuf	*code;
	register void		(*fp)();

	dot = lookup(".", INSTALL);
	dot->value = newdot = 0;
	dot->sectnum =(short) sect;

	
	Sdi = 0;
	if ((Sdibox = sectab[sect].Sdihead) != 0)
		Sdi = Sdibox->b_buf - 1;
	if ((Codebox = sectab[sect].Codehead) == 0)
		return;
	Code = Codebox->c_buf - 1;

	if ((new_data = (BYTE *) calloc((unsigned)sectab[sect].size,
					sizeof(BYTE))) == NULL)
		aerror("cannot alloc data for section");
	buf_size = -1;

#ifndef DCODGEN
	poscnt = 0;
#endif
	/*
	 *	getcode() forces external Code variables
	 */

	for (;;)
	{
		if ((code = ++Code) >= Codebox->c_end)
		{
			if ((Codebox = Codebox->c_link) == 0)
				break;
			Code = code = Codebox->c_buf;
		}

		line = code->errline; 	/* source line */

		if (code->caction & CB_EFLAG)
			exp_eval(code);
		if (code->cline)	/* beginning of new line update dot */
		{
			dot->value = newdot;
		}

		if ((fp = modes[code->caction & CB_AMASK]) != 0)
			(*fp)(code);

		/*	NOACTION or end-of-action processing
		 */

		if (code->cnbits)
			codgen(code->cnbits, code->cvalue);
	}

	dot->value = newdot;
	if ((data = elf_newdata(sectab[sect].s_scn)) == NULL)
		aerror("cannot generate section data");
	ENTER_DATA(new_data,ELF_T_BYTE,buf_size + 1, 0);
	data->d_align = sectab[sect].addralign;

}


void
getcode(code)
	register codebuf	*code;
{

	if (++Code < Codebox->c_end)
	{
		*code = *Code;
		return;
	}
	if ((Codebox = Codebox->c_link) == 0)
		aerror("out of Code entries");
	*code = *(Code = Codebox->c_buf);
	return;
}

/* This function is called to reset the "value" and "csym" fields
of a codebuf struct.  code->csym is assumed to hold a pointer to an
expression tree that needs to be evaluated.  The evaluation is done
and the fields are reset.
*/
static void 
exp_eval(code)
	register codebuf *code;
{
	extern void tree_eval();
	register symbol *sym;
#if DEBUG
	(void) printf("in expr_eval:\n ");
	(void) printf("%d: ",line);
#endif
	if (code->csym) {
		tree_eval((EXPTREE *)(code->csym));
		sym = ((EXPTREE *)(code->csym))->t_leaf.symptr;
		code->cvalue += ((EXPTREE *)(code->csym))->t_leaf.value;
		code->csym = sym;
#if DEBUG
		(void) printf(" ==> ");
		if (sym)
			(void) printf("%s=%d + ",sym->name,sym->value);
		(void) printf("%d\n",code->cvalue);
#endif
	}
}
