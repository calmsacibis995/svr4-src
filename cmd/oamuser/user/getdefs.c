/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:user/getdefs.c	1.3.7.1"



#include	<sys/types.h>
#include	<stdio.h>
#include	<userdefs.h>
#include	"userdisp.h"

/*******************************************************************************
 *  getusrdefs [-g] [-b] [-r] [-k] [-s] [-f] [-e]
 ******************************************************************************/

static char *usage = "usage:  getusrdefs [-g] [-b] [-r] [-k] [-s] [-f] [-e]\n";

extern void exit();
extern void dispusrdef();
extern int getopt();

main(argc, argv)
int argc;
char **argv;
{
	extern int optind;
	int ch;
	register unsigned int flags = 0;

	while((ch = getopt(argc, argv, "gbrksfe")) != EOF)
		switch(ch) {
		case 'b':
			flags |= D_BASEDIR;
			break;
		case 'e':
			flags |= D_EXPIRE;
			break;
		case 'f':
			flags |= D_INACT;
			break;
		case 'g':
			flags |= D_GROUP;
			break;
		case 'k':
			flags |= D_SKEL;
			break;
		case 'r':
			flags |= D_RID;
			break;
		case 's':
			flags |= D_SHELL;
			break;
		case '?':
			(void) fprintf( stdout, usage );
			exit( EX_SYNTAX );
		}

	if( optind != argc || argc == 1 ) {
		(void) fprintf( stdout, usage );
		exit( EX_SYNTAX );
	}

	dispusrdef( stdout, flags );

	exit( EX_SUCCESS );
	/*NOTREACHED*/
}
