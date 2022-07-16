/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/sigsend.c	1.1"

#ifdef __STDC__
	#pragma weak sigsend = _sigsend
#endif
#include "synonyms.h"
#include <sys/types.h>
#include <sys/procset.h>


sigsend(idtype, id, sig)
idtype_t idtype;
id_t	 id;
int	 sig;
{
	procset_t set;
	setprocset(&set, POP_AND, idtype, id, P_ALL, P_MYID);
	return sigsendset(&set, sig);
}
