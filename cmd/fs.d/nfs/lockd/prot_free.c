/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/lockd/prot_free.c	1.3.2.1"
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
	/*
	 * prot_freeall.c consists of subroutines that implement the
	 * DOS-compatible file sharing services for PC-NFS
	 */

#include <stdio.h>
/*
#include <sys/file.h>
*/
#include "prot_lock.h"

extern int debug;
extern int grace_period;
extern char *xmalloc();
extern void xfree();
extern void zap_all_locks_for();
extern bool_t obj_cmp();
extern lm_vnode *fh_q;
char *malloc();

void *
proc_nlm_freeall(Rqstp, Transp)
	struct svc_req *Rqstp;
	SVCXPRT *Transp;
{
	nlm_notify	req;
/*
 * Allocate space for arguments and decode them
 */

	req.name = NULL;
	if (!svc_getargs(Transp, xdr_nlm_notify, &req)) {
		svcerr_decode(Transp);
		return;
	}

	if (debug) {
		printf("proc_nlm_freeall from %s\n",
			req.name);
	}
	destroy_client_shares(req.name);
	zap_all_locks_for(req.name);

	free(req.name);
	svc_sendreply(Transp, xdr_void, NULL);
}

void
zap_all_locks_for(client)
	char *client;
{
	reclock *le;
	struct lm_vnode *fp;

	if (debug)
		printf("zap_all_locks_for %s\n", client);

	for (fp = fh_q; fp; fp = fp->next) {
		for (le = fp->reclox; le; le = le->next) {
			if (strcmp(le->alock.clnt_name, client) == 0) {
				if (debug)
					printf("...zapping: le@0x%x\n", le);

				delflck_reclox(&fp->reclox, le);
				le->rel = 1;
				release_reclock(le);
			}
		}
	}
	if (debug)
		printf("DONE zap_all_locks_for %s\n", client);
}
