/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:devices/erase/stddef.h	1.1"

/*
	Standard defines for common usage.
*/

/*	Used by argfopen
*/
#define	IGNORE	(-1)
#define	ERROR	(-2)
#define	FATAL	(-3)

/*	Useful string comparison macros
*/
#define	EQ(s1,s2)	(!strcmp(s1, s2))
	/* use !EQ for "not equal" */
#define	EQN(s1,s2,n)	(!strncmp(s1,s2,n))
#define	LT(s1,s2)	(strcmp(s1, s2) < 0)
#define	LE(s1,s2)	(strcmp(s1, s2) <= 0)
#define	GT(s1,s2)	(strcmp(s1, s2) > 0)
#define	GE(s1,s2)	(strcmp(s1, s2) >= 0)

/*	verify that s1 contains only characters in s2
*/
#define	VERIFY(s1,s2)	(strlen(s1) == strspn(s1,s2))

#define	BUMPARG	(argc--, argv++)	/* bump the arg count and arg vector */

#define	DEBUG(x)	fprintf(stderr, x)

#define	ROUND(f)	((int)(f + .5))	/* integer rounded value of float */

/*
	duplicate a string by allocating space and copying it in
*/
extern	char	*Malloc();
#define	strdup(s)	strcpy( Malloc( strlen(s)+1 ), s )


/*
	Definitions of global variables, compiled into stddef.c
*/
#ifndef	EXTERN
#define	EXTERN	extern
#endif


/*
	for argfopen.c
*/
#ifndef	stdin
#include	<stdio.h>
#endif
extern	FILE	*argfopen();
EXTERN	int	argf_dir;


/*
	for bufsplit.c
*/
extern	unsigned	bufsplit();
extern	char	*bsplitchar;		/* initialized in bufsplit.c */

/*
	for copylist.c
*/
extern	char	*copylist();
extern	long	clistcnt();


/*
	for filename.c
*/
EXTERN	char	*file_name;	/* currently open file name */


/*
	for Fopen.c
*/
extern	FILE	*Fopen();


/*
	for pgmname.c
*/
char	*pgm_name;	/* current main program name */

#undef	EXTERN
