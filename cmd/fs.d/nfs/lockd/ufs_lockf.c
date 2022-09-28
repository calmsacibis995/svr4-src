/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/lockd/ufs_lockf.c	1.4.3.1"
/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
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
#include <sys/types.h>
#include "prot_lock.h"

int				LockID = 0; /* Monotonically increasing id */


/*
 * return a string representation of the lock.
 */
void
print_lock(l)
	struct data_lock	*l;
{

	printf("[ID=%d, pid=%d, base=%d, len=%d, type=%s rsys=%x]",
		l->LockID, l->lld.l_pid, l->lld.l_start, l->lld.l_len,
		(l->lld.l_type == F_WRLCK) ? "EXCL" : "SHRD",
		l->lld.l_sysid);
	return;
}
