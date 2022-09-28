/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)i286emu:main.c	1.1"

#include <stdio.h>
#include "i286sys/exec.h"

#define	MAIN
#include "vars.h"

struct exdata exec_data;

main( argc, argv, envp )
	int   argc;
	char *argv[];
	char **envp;
{
	unsigned short TheStack[ 65536/2 ];
	int i;

	pgmname = argv[0];

	for ( i = 0; i < 65536/2; i++ )
		TheStack[i] = 0;

	for ( i = 0; i < MAXDSEGS; i++ ) {
		dsegs[i].base = BAD_ADDR;
		dsegs[i].size = 0;
	}

	stackbase = TheStack;

	setcallgate();

        Siginit();
	i286exec( argv[argc-1], argc, argv, envp );
}
