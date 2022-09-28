/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)optim:i386/peep.c	1.1.2.6"
/* peep.c
**
**	Intel 386 peephole improvement driver
**
**
** This module contains the driver for the Intel 386 peephole improver.
*/


#include "optim.h"
#include "optutil.h"

extern boolean w1opt();
extern boolean w2opt();
extern boolean w3opt();
extern boolean w4opt();
extern boolean w6opt();
extern void window();
extern unsigned sets();
extern char *dst();
extern int scanreg();

extern int dflag;			/* non-0 to display live/dead data */
extern int hflag;			/* non-0 to disable peephole entirely */
extern int tflag;			/* non-0 to disable redundant load op */
extern int Tflag;			/* non-0 to display t debugging       */

static void prld();			/* routine to print live/dead data */
static void rdfree();

void peep()
{


    if (hflag == 0)
    {
#ifdef LIVEDEAD
	if (dflag != 0)			/* if enabled */
	    prld();			/* print the world's live/dead data */
#endif

	window(6, w6opt);		/* do 6-instruction sequences */
	window(3, w3opt);		/* do 3-instruction sequences */
	window(1, w1opt);		/* do 1-instruction sequences */
	window(2, w2opt);		/* do 2-instruction sequences */
	window(3, w3opt);		/* do 3-instruction sequences */
	window(4, w4opt);		/* do 4-instruction sequences */

	window(1, w1opt);		/* now repeat to clean up stragglers */
	window(2, w2opt);
	window(1, w1opt);
    }
    return;
}
/* Print live/dead data for all instruction nodes */

#ifdef LIVEDEAD


	static void
prld()
{
    register NODE * p;

    for (ALLN(p))			/* for all instruction nodes... */
    {
	PUTCHAR(CC);			/* write comment char */

	PRINTF("(live: 0x%7.7x)", p->nlive); /* print live/dead data */
	prinst(p);			/* print instruction */
    }
    return;
}

#endif
/*
 * This section of code, pays attention to register loads,
 * and tries to do redundant load removal.  This is done,
 * by watching mov instructions, and keeping track of what
 * values are stored in the actual register.  If the mov
 * is reloading a register with the same contents, then the
 * mov is deleted.
 */

static struct	rld {
	char	*rnm;			/* register name */
	int	rnum;			/* register number    */
	int	rsrc;			/* size of the mov source   */
	int	rdst;			/* size of the mov dest reg */
	char	*ropnd;			/* Actual operand */
} rreg[] = {
	{ "%al",  AL,  0, 0, NULL },
	{ "%ah",  AH,  0, 0, NULL },
	{ "%ax",  EAX, 0, 0, NULL },
	{ "%eax", EAX, 0, 0, NULL },
	{ "%bl",  BL,  0, 0, NULL },
	{ "%bh",  BH,  0, 0, NULL },
	{ "%bx",  EBX, 0, 0, NULL },
	{ "%ebx", EBX, 0, 0, NULL },
	{ "%cl",  CL,  0, 0, NULL },
	{ "%ch",  CH,  0, 0, NULL },
	{ "%cx",  ECX, 0, 0, NULL },
	{ "%ecx", ECX, 0, 0, NULL },
	{ "%dl",  DL,  0, 0, NULL },
	{ "%dh",  DH,  0, 0, NULL },
	{ "%dx",  EDX, 0, 0, NULL },
	{ "%edx", EDX, 0, 0, NULL },
	{ "%si",  ESI, 0, 0, NULL },
	{ "%esi", ESI, 0, 0, NULL },
	{ "%di",  EDI, 0, 0, NULL },
	{ "%edi", EDI, 0, 0, NULL },
	{ NULL, 0, 0, 0, NULL},
};

/*
 * Free all the current status of the registers
 * An argument of -1 frees everthing.
 */
	static void
rdfree(arg)
int arg;
{
	register int i;

	for(i = 0; rreg[i].rnm != (char *)0; i++)
		if ( arg == -1 || arg == i ) {
			rreg[i].rsrc = 0;
			rreg[i].rdst = 0;
			rreg[i].ropnd = NULL;
		}
}

void
rmrdld()
{
	register NODE *p;
	register int i, j;
	char *dop;
	unsigned sreg1, sreg2;
	int src1, dst1;
	boolean lookup_regals();

	if (tflag)
		return;
	rdfree(-1);
	for (ALLN(p))			/* for all instruction nodes... */
	{
		if (Tflag) {
			PRINTF("%c rmrdld: ", CC);
			prinst(p);	/* print instruction */
		}
		if ( isuncbr(p) || islabel(p) ||
		     p->op == ASMS || p->op == CALL || p->op == LCALL )
			rdfree(-1);	/* free up the list between breaks */
		else if ( ismove(p, &src1, &dst1) ) {
			if ( !isreg(p->op2) ) {
				sreg2 = 0;
				dop = p->op2;
				for(i = 0; rreg[i].rnm != (char *)0; i++) {
					if (rreg[i].ropnd && strcmp(dop, rreg[i].ropnd) == 0 ) {
						sreg2 |= rreg[i].rnum;
						rreg[i].rsrc = 0;
						rreg[i].rdst = 0;
						rreg[i].ropnd = NULL;
					}
				}
				for(i = 0; sreg2 && rreg[i].rnm != (char *)0; i++) {
					if ( sreg2 & scanreg(rreg[i].ropnd, true) ||
					     sreg2 & scanreg(rreg[i].ropnd, false) ) {
						rreg[i].rsrc = 0;
						rreg[i].rdst = 0;
						rreg[i].ropnd = NULL;
					}
				}
			} else {
			    for(i = 0; rreg[i].rnm != (char *)0; i++) {
				if (strcmp(p->op2, rreg[i].rnm) == 0) {
					if ( rreg[i].ropnd && strcmp(p->op1,rreg[i].ropnd) == 0
					     && rreg[i].rsrc == src1
					     && rreg[i].rdst == dst1
					     && !is_vol_opnd(p,1)
					     && !(rreg[i].rnum & scanreg(rreg[i].ropnd,false))
					     && (isreg(p->op1) || isnumlit(p->op1) || ((*p->op1 != '*' && (usesreg(p->op1, "%esp") || usesreg(p->op1, "%ebp"))) && lookup_regals(p->op1)))
					   ) {
						/* found a redundant load */
						if (Tflag) {
							PRINTF("%c removed",CC);
							prinst(p);
						}
						wchange();
						ldelin2(p);
						DELNODE(p);
					} else {
						/* time to change the move */
					    sreg2 = rreg[i].rnum;
					    for(j = 0; rreg[j].rnm != (char *)0; j++) {
						/* These next 5 lines may be unnecessary. */
						if ( sreg2 & rreg[j].rnum ) {
							rreg[j].rsrc = 0;
							rreg[j].rdst = 0;
							rreg[j].ropnd = NULL;
						}
						if ( sreg2 & scanreg(rreg[j].ropnd, false) ) {
							rreg[j].rsrc = 0;
							rreg[j].rdst = 0;
							rreg[j].ropnd = NULL;
						}
						/* These next 5 lines may be unnecessary. */
						if ( sreg2 & scanreg(rreg[j].ropnd, true) ) {
							rreg[j].rsrc = 0;
							rreg[j].rdst = 0;
							rreg[j].ropnd = NULL;
						}
					    }
					    rreg[i].rsrc = src1;
					    rreg[i].rdst = dst1;
					    rreg[i].ropnd = p->op1;
					}
					break;
				}
			    }
			}
		} else {
			sreg2 = 0;
			sreg1 = sets(p);
			dop = dst(p);		/* dst doesn't return NULL */
			if (isreg(dop) || dop[0] == '\0')
				dop = NULL;
			for(i = 0; sreg1 && rreg[i].rnm != (char *)0; i++) {
				if ( sreg1 & rreg[i].rnum ) {
					sreg2 |= rreg[i].rnum;
					rreg[i].rsrc = 0;
					rreg[i].rdst = 0;
					rreg[i].ropnd = NULL;
				}
			}
			for(i = 0; sreg2 && rreg[i].rnm != (char *)0; i++) {
				if ( sreg2 & scanreg(rreg[i].ropnd, true) ||
				     sreg2 & scanreg(rreg[i].ropnd, false) ) {
					rreg[i].rsrc = 0;
					rreg[i].rdst = 0;
					rreg[i].ropnd = NULL;
				}
			}
			for(i = 0; dop != NULL &&
				   rreg[i].rnm != (char *)0; i++) {
				if ( rreg[i].ropnd != NULL &&
				     strcmp(dop, rreg[i].ropnd) == 0 ) {
					rreg[i].rsrc = 0;
					rreg[i].rdst = 0;
					rreg[i].ropnd = NULL;
				}
			}
		}
	}
	return;
}
