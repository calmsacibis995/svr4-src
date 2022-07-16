/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/swapctl.c	1.1"
#ifdef __STDC__
	#pragma weak swapctl = _swapctl
#endif
#include "synonyms.h"
#include	"sys/uadmin.h"

swapctl(cmd, arg)
int cmd;
int arg;
{
	return uadmin(A_SWAPCTL, cmd, arg);
}
