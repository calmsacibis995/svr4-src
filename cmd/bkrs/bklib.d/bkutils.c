/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/bkutils.c	1.4.2.1"

#include	<stdio.h>
#include	<ctype.h>
#include	<errno.h>
#include	<table.h>
#include	<backup.h>
#include	<bkreg.h>

#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif

extern char *optname;
extern char *brcmdname;
extern int nflags;

/*
	Check the combination of flags given on the command line - make sure
	that there are exactly enough and not too many.

	Each option has its own bit. 
*/
int
validcombination( flag, given, f_allowed, f_reqd )
char flag;
unsigned given, f_allowed, f_reqd;
{
	unsigned too_few, too_many;
	register i, offset, error = FALSE;
	too_few = (~given & f_reqd);
	too_many = (given & ~f_allowed);
	for( i = 1, offset = 0; i < (1<<nflags); i <<= 1, offset++ ) {
		if( (i & (too_few)) /* && !(i & f_allowed) */ ) {
			(void) fprintf( stderr, "%s: the -%c option requires the -%c option.\n",
				brcmdname, flag, optname[ offset ] );
			error = TRUE;
		}
		if( i & too_many ) {
			(void) fprintf( stderr,
				"%s: the -%c option does not use the -%c option.\n",
				brcmdname, flag, optname[ offset ] );
			error = TRUE;
		}
	}
	return( !error );
}
