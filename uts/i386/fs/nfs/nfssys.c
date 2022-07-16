/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:nfs/nfssys.c	1.3"

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

#include "sys/types.h"
#include "rpc/types.h"
#include "sys/systm.h"
#include "sys/vfs.h"
#include "sys/errno.h"
#include "nfs/nfs.h"
#include "nfs/export.h"
#include "nfs/nfssys.h"

/*ARGSUSED*/
int
nfssys(uap, rvp)
register struct nfssysa *uap;
rval_t *rvp;
{
	union nfssysargs	a;

	switch ((int) uap->opcode) {
		case NFS_SVC:
			/* NFS server daemon */
			{
				struct nfs_svc_args    nsa;

				/* export a file system */
				if (copyin((caddr_t) uap->nfssysarg_svc,
					   (caddr_t) &nsa, sizeof(nsa)))
					return(EFAULT);
				else
					return(nfs_svc(&nsa));
			}

		case ASYNC_DAEMON:
			/* NFS client async daemon */
			return(async_daemon());		/* no args */

		case EXPORTFS:
			/* export a file system */
			{
				struct exportfs_args    ea;

				/* export a file system */
				if (copyin((caddr_t) uap->nfssysarg_exportfs,
					   (caddr_t) &ea, sizeof(ea)))
					return(EFAULT);
				else
					return(exportfs(&ea));
			}

		case NFS_GETFH:
			/* get a file handle */
			{
				struct nfs_getfh_args    nga;

				/* export a file system */
				if (copyin((caddr_t) uap->nfssysarg_getfh,
					   (caddr_t) &nga, sizeof(nga)))
					return(EFAULT);
				else
					return(nfs_getfh(&nga));
			}

		case NFS_CNVT:
			/* open a file referred to by a file handle */
			return (nfs_cnvt(uap->nfssysarg_cnvt));
			
		default:
			return(EINVAL);
	}
}
