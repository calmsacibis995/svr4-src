/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)truss:systable.h	1.2.3.1"

struct systable {
	CONST char * name;	/* name of system call */
	short	nargs;		/* number of arguments */
	char	rval[2];	/* return value types */
	char	arg[8];		/* argument types */
};

/* the system call table */
extern CONST struct systable systable[];


struct sysalias {
	CONST char * name;	/* alias name of system call */
	int	number;		/* number of system call */
};

extern CONST struct sysalias sysalias[];

/*
 * Function prototypes.
 */

#if	defined(__STDC__)

extern	CONST char *	errname( int );
extern	CONST struct systable *	subsys( int , int );
extern	CONST char *	sysname( int , int );
extern	CONST char *	rawsigname( int );
extern	CONST char *	signame( int );
extern	CONST char *	rawfltname( int );
extern	CONST char *	fltname( int );

#else	/* defined(__STDC__) */

extern	CONST char *	errname();
extern	CONST struct systable *	subsys();
extern	CONST char *	sysname();
extern	CONST char *	rawsigname();
extern	CONST char *	signame();
extern	CONST char *	rawfltname();
extern	CONST char *	fltname();

#endif	/* defined(__STDC__) */
