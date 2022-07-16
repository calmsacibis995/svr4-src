/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/str2id.c	1.1"
#ifdef __STDC__
	#pragma weak str2id = _str2id
	#pragma weak id2str = _id2str
#endif
#include "synonyms.h"
#include <sys/types.h>
#include <sys/procset.h>

static struct idname {
	char *idstr;
	idtype_t idtype;
} idnames[] = {
	{ "pid",	P_PID },
	{ "ppid",	P_PPID },
	{ "pgid",	P_PGID },
	{ "sid",	P_SID },
	{ "uid",	P_UID },
	{ "gid",	P_GID },
	{ "cid",	P_CID },
	{ "all",	P_ALL }
};

#define IDCNT (sizeof(idnames)/sizeof(struct idname))

str2id(s,idtypep)
char *s;
idtype_t *idtypep;
{
	register struct idname *ip;

	for (ip = idnames; ip < &idnames[IDCNT]; ip++) {
		if (strcmp(ip->idstr,s) == 0) {
			*idtypep = ip->idtype;
			return (0);
		}
	}
	return(-1);
}

id2str(idtype,s)
idtype_t idtype;
char *s;
{
	register struct idname *ip;

	for (ip = idnames; ip < &idnames[IDCNT]; ip++) {
		if (idtype == ip->idtype) {
			strcpy(s,ip->idstr);
			return (0);
		}
	}
	return(-1);
}
