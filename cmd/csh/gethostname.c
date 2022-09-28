/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)csh:gethostname.c	1.3.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 


#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/systeminfo.h>
#include <sys/errno.h>

extern	int	errno;

gethostname(name, namelen)
	char	*name;
	int	namelen;
{
	int	error;

	error = sysinfo(SI_HOSTNAME, name, namelen);
	/*
	 * error > 0 ==> number of bytes to hold name
	 * and is discarded since gethostname only
	 * cares if it succeeded or failed
	 */
	return (error == -1 ? -1 : 0);
}


sethostname(name, namelen)
	char	*name;
	int	namelen;
{

	int	error;

	/*
	 * Check if superuser
	 */
	if (getuid()) {
		errno = EPERM;
		return (-1);
	}
	error = sysinfo(SI_SET_HOSTNAME, name, namelen);
	return (error == -1 ? -1 : 0);
}
