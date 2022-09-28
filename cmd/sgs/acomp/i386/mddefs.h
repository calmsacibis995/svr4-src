/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:i386/mddefs.h	55.2"
/* i386/mddefs.h */


/* Machine-dependent ANSI C definitions. */


/* These definitions will eventually be in CG's macdefs.h */
#define	SZLDOUBLE	96
#define	ALLDOUBLE	32
#define C_CHSIGN		/* plain chars are signed */
#define	C_SIGNED_RS		/* right shifts of ints are signed */

/* Produce debugging register number corresponding to internal
** register number.
*/
#define	DB_OUTREGNO(i)	(outreg[SY_REGNO(i)])

/* Only want for-loop code tests at bottom. */
#define	FOR_LOOP_CODE	LL_BOT

/* Enable #pragma pack; maximum value is 4 */
#define	PACK_PRAGMA	4

#ifdef	ELF_OBJ

/* HACK:  register numbers to represent ELF debugging.
** Both autos and args are accessed via %ebp.
*/
#define	DB_FRAMEPTR(sid)	5	/* %ebp number */
#define	DB_ARGPTR(sid)		5	/* %ebp number */

/* Enable #pragma weak.  The two strings are for the 1 and 2
** identifier forms of the pragma.
*/
#define	WEAK_PRAGMA "\t.weak\t%1\n", "\t.weak\t%1\n\t.set\t%1,%2\n"
#endif

#define FAT_ACOMP
