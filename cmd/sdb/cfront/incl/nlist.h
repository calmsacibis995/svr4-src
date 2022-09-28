/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/nlist.h	1.2"
/*ident	"@(#)cfront:incl/nlist.h	1.5"*/

#ifndef NLISTH
#define NLISTH

#if vax | u3b | M32 | u3b15 | u3b5 | u3b2 || i386

/* symbol table entry structure */

struct nlist
{
	char		*n_name;	/* symbol name */
	long		n_value;	/* value of symbol */
	short		n_scnum;	/* section number */
	unsigned short	n_type;		/* type and derived type */
	char		n_sclass;	/* storage class */
	char		n_numaux;	/* number of aux. entries */
};

#else /* pdp11 || u370 */

struct nlist
{
	char		n_name[8];	/* symbol name */
#if pdp11
	int		n_type;		/* type of symbol in .o */
#else
	char		n_type;
	char		n_other;
	short		n_desc;
#endif
	unsigned	n_value;	/* value of symbol */
};

#endif

extern int nlist (const char*, nlist*);

#endif
