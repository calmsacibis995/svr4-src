/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:group/groupdel.c	1.3.8.1"



#include <sys/types.h>
#include <stdio.h>
#include <userdefs.h>
#include "messages.h"

/*******************************************************************************
 *  groupdel group
 *
 *	This command deletes groups from the system.  Arguments are:
 *
 *	group - a character string group name
 ******************************************************************************/

char *cmdname = "groupdel";

extern void errmsg(), exit();
extern int del_group();

main(argc, argv)
int argc;
char **argv;
{
	register char *group;			/* group name from command line */
	register retval = 0;
	
	if( argc != 2 ) {
		errmsg( M_DUSAGE );
		exit( EX_SYNTAX );
	}

	group = argv[1];

	switch( retval = del_group( group ) ) {
	case EX_UPDATE:
		errmsg( M_UPDATE, "deleted" );
		break;
		
	case EX_NAME_NOT_EXIST:
		errmsg( M_NO_GROUP, group );
		break;
	}

	exit(retval);
	/*NOTREACHED*/
}
