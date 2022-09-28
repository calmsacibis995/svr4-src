/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/ldope.h	1.1"

#define RELATIONAL(o)   (ln_dope[o]&RLOP)
#define AOOP(o)		(ln_dope[o]&LOGSHRT)
#define EQOP(o)		(ln_dope[o]&EQUALITY)
#define SHOP(o)		(ln_dope[o]&SHIFT)
#define BIOP(o)		(ln_dope[o]&BITW)
#define LGOP(o)		(ln_dope[o]&LOG)
#define LN_SIDE(o)	(ln_dope[o]&LNSIDE)
#define SIDE		1	/* operator passes EFF to left subtree  */
#define	LOGSHRT		2	/* logical op that "short circuits"	*/
#define EQUALITY	4	/* EQ or NE				*/
#define RLOP		010	/* relational (not including EQ & NE	*/
#define LOG		020	/* logical ops for precedence confusion */
#define BITW		040	/* bitwise operators			*/
#define SHIFT		0100	/* shift operators			*/
#define CPY		0200	/* operators for ln_strcpy		*/
#define LNSIDE		0400	/* operator make side effect at top-lev */

extern long ln_dope[];	/* flags for operators lint is interested in */
extern char *ln_opst[];	/* the operators themselves		     */

extern void ldope();
