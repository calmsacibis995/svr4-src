/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tsort:erraction.c	1.1"

/*
	Routine called after error message has been printed.
	Dependent upon the return code of errafter.
	Command and library version.
*/

#include	"errmsg.h"
#include	<stdio.h>

void
erraction( action )
int      action;
{


	switch( action ){
	case EABORT:
	     abort();
	     break;
	case EEXIT:
	     exit( Err.exit );
	     break;
	case ERETURN:
	     break;
	}
	return;
}
