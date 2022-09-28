/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)optim:i386/optutil.h	1.1.2.5"
/* optutil.h
**
**	Declarations for optimizer utilities
**/

extern boolean isreg();		/* is operand a register? */
extern NODE * insert();		/* insert new instruction node */
extern void chgop();		/* change op code */
extern boolean isdyadic();	/* is instruction dyadic? */
extern boolean is2dyadic();	/* is instruction dyadic? */
extern boolean istriadic();	/* is instruction triadic? */
extern boolean isfp();		/* is floating point instruction? */
extern boolean is_byte_instr();
extern boolean is_legal_op1();  /* check consistency of size of 1st operand */
extern void makelive();		/* make register live */
extern void makedead();		/* make register dead */
extern boolean isindex();	/* operand is an index off a register */
extern boolean ismove();	/* is instruction a move-type? */
extern boolean iszoffset();	/* is operand 0 offset from register? */
extern boolean isnumlit();	/* is operand a numeric literal: &[-]n? */
extern void exchange();		/* exchange node and successor */
extern boolean usesvar();	/* does first operand use second? */
extern boolean isvolatile();	/* is operand volatile? */
extern void ldelin();	
extern void ldelin2();	/* routines to preserve */
extern void lexchin();
extern void lmrgin1();	/* line number info */
/*extern void lmrgin2();*/
extern void lmrgin3();
extern void revbr();	/* reverse sense of a jump node */
extern char * dst();		/* return pointer to destination string */
extern int stype();		/* return implied type of op code */
extern boolean samereg();	/* return true if same register */
extern boolean usesreg();	/* usesreg(a,b) true if reg. b used in a */
extern int uses();		/* return register use bits */
extern int scanreg();		/* determine registers referenced in operand */
extern unsigned int sets();	/* return set register destination bits */
extern boolean isdead();	/* true iff *cp is dead after p */
extern char * getp();		/* return pointer to jump destination operand */
extern char * newlab();		/* generate a new label */
extern void prinst();		/* print instruction */
extern boolean ishlp();		/* return true if a fixed label present */
extern int setreg();		/* return bits corresponding to register */
extern boolean isdead();	/* return true if reg. dead after node */

/* declare functions that are always handy for optimizations */

extern int atoi();
extern long atol();
extern long strtol();


/* declare functions defined in optim.c that are handy here */
extern void wchange();		/* flag a window change */

