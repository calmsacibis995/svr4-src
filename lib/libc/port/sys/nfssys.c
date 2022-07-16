/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:sys/nfssys.c	1.1"

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

#ifdef __STDC__
	#pragma weak async_daemon = _async_daemon
	#pragma weak exportfs = _exportfs
	#pragma weak nfs_getfh = _nfs_getfh
	#pragma weak nfssvc = _nfssvc
#endif
#include "synonyms.h"
#include "sys/types.h"
#include "rpc/types.h"
#include "sys/vfs.h"
#include "sys/errno.h"
#include "nfs/nfs.h"
#include "nfs/export.h"
#include "nfs/nfssys.h"

int
async_daemon()
{
	return(_nfssys(ASYNC_DAEMON, (_VOID *) 0));
}

int
exportfs(dir, ep)
char	*dir;
struct export	*ep;
{
	struct exportfs_args ea;

	ea.dname = dir;
	ea.uex = ep;
	return (_nfssys(EXPORTFS, &ea));
}

int
nfs_getfh(path, fhp)
char	*path;
fhandle_t	*fhp;
{
	struct nfs_getfh_args nga;

	nga.fname = path;
	nga.fhp = fhp;
	return (_nfssys(NFS_GETFH, &nga));
}

int
nfssvc(fd)
int	fd;
{
	struct nfs_svc_args nsa;

	nsa.fd = fd;
	return(_nfssys(NFS_SVC, &nsa));
}
