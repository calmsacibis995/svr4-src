/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/addr1.c	1.7"

#include <stdio.h>
#include <string.h>
#include <libelf.h>
#include "systems.h"
#include "gendefs.h"
#include "symbols.h"
#include "codeout.h"

/*
 *
 *	"addr1.c" is a file containing routines for implementing the
 *	various addressing modes from the intermediate representation.
 *	The array "modes" is initialized to contain the addresses
 *	of the functions that implement the various addressing modes.
 *	Indexing this array with the addressing mode will give the
 *	correct routine for implementing that mode.
 *
 *	The following functions implement obsolete COFF-based debugging
 *	directives.  For the most part, these directives can be
 *	ignored.  However, in the special case of function symbols,
 * 	some minimal processing is needed to set the symbol type and
 *	size.
 */




extern void codgen();


static symbol	*savsym;
static short 	save_val;



void define();
void setscl();
void endef();





/*
 *
 *	"define" is a function that handles obsolete COFF-based
 *	".def <symbol>" directives.  It saves the symbol for further
 *      processing.
 *
 */

void
define(code)
codebuf *code;
{

	savsym = code->csym;
}

/*
 *
 *	"setval" is a function that sets the value field of a symbol
 *	table entry. The arguments are evaluated and the result is
 *	stored into the value field of the symbol table entry, and
 *	the section number is set according to the type of the result.
 *
 */

void
setval(code)
	register codebuf *code;
{
	register symbol *sym = code->csym;

#if DEBUG
	printf("in setval code->cvalue %d\n", code->cvalue);
	printf("in setval sym->value %d\n", sym->value);
#endif
	if (sym != NULLSYM) 
		code->cvalue += sym->value;

	save_val = code->cvalue;
}


/*
 *
 *	"setscl" is a function that processes an obsolete COFF-based
 *	.scl directive.  If the storage class is 2, the symbol has 
 * 	global binding.  If the storage class is -1 (physical end of 
 *	a function in COFF) type field of the saved symbol is set to 
 *	STT_FUNCTION. Otherwise, the directive is ignored.
 *
 */

void
setscl(code)
register codebuf *code;
{
	if (code->cvalue == 2)  
		savsym->binding = STB_GLOBAL;
	else if (code->cvalue == -1)
		savsym->type = STT_FUNC;
	/* else ignore it */
}



/*
 *
 *	"endef" is a function that completes processing of a symbol
 *	if the saved symbol is a function symbol, it sets the size
 *	field to be the saved value - the value of the saved symbol.
 *
 */

/*ARGSUSED*/
void
endef(code)
codebuf *code;
{
	if (savsym->type == STT_FUNC)
		savsym->size = save_val - savsym->value;
}


/*	
*
*	dotzero is a function used to generate zeroes in the data section of a file
*
*/

void
dotzero(code)
	register codebuf *code;
{
	while (code->cvalue >= sizeof(OUTWTYPE)) {
		codgen(sizeof(OUTWTYPE) * BITSPBY, 0L);
		code->cvalue -= sizeof(OUTWTYPE);
	}
	if (code->cvalue)
		codgen((unsigned short) (code->cvalue * BITSPBY), 0L);
}
