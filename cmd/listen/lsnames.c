/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)listen:lsnames.c	1.2.3.1"

#include <string.h>
#include <ctype.h>
#include <sys/utsname.h>

#include "lsparam.h"		/* listener parameters		*/
#include "lserror.h"

/*
 * getnodename:	return "my" nodename in a char string.
 */

static struct utsname myname;
static char _nodename[sizeof(myname.nodename) + 1];

char *
getnodename()
{
	register struct utsname *up = &myname;

	DEBUG((9,"in getnodename, sizeof(_nodename) = %d", sizeof(_nodename)));

	if ( uname(up) )
		sys_error(E_UNAME, EXIT);

	/* will be null terminated by default */
	strncpy(_nodename,up->nodename,sizeof(up->nodename));
	return(_nodename);
}

