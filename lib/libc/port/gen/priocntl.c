/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/priocntl.c	1.1"
#include	"synonyms.h"
#include	<sys/types.h>
#include	<sys/procset.h>
#include	<sys/priocntl.h>

long
__priocntl(pc_version, idtype, id, cmd, arg)
int		pc_version;
idtype_t	idtype;
id_t		id;
int		cmd;
caddr_t		arg;
{
	procset_t	procset;

	setprocset(&procset, POP_AND, idtype, id, P_ALL, 0);

	return(__priocntlset(pc_version, &procset, cmd, arg));
}
