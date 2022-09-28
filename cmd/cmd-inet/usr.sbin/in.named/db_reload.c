/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/in.named/db_reload.c	1.1.3.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <syslog.h>
#include <arpa/nameser.h>
#include "ns.h"
#include "db.h"

extern time_t	resettime;

/*
 * Flush and reload data base.
 */

db_reload()
{
	extern char *bootfile;

#ifdef DEBUG
	if (debug >= 3)
		fprintf(ddt,"reload()\n");
#endif
	syslog(LOG_NOTICE, "reloading nameserver\n");

	qflush();
	sqflush();
	fwdtab_free();
	if (hashtab != NULL)
		db_free(hashtab);
	hashtab = NULL;
	if (fcachetab != NULL)
		db_free(fcachetab);
	fcachetab = NULL;
	db_inv_free();
	fwdtab_free();
	ns_init(bootfile);
	time(&resettime);
}

db_free(htp)
	struct hashbuf *htp;
{
	register struct databuf *dp, *nextdp;
	register struct namebuf *np, *nextnp;
	struct namebuf **npp, **nppend;

	npp = htp->h_tab;
	nppend = npp + htp->h_size;
	while (npp < nppend) {
	    for (np = *npp++; np != NULL; np = nextnp) {
		if (np->n_hash != NULL)
			db_free(np->n_hash);
		(void) free((char *)np->n_dname);
		for (dp = np->n_data; dp != NULL; ) {
			nextdp = dp->d_next;
			(void) free((char *)dp);
			dp = nextdp;
		}
		nextnp = np->n_next;
		free((char *)np);
	    }
	}
	(void) free((char *)htp);
}

db_inv_free()
{
	register struct invbuf *ip;
	register int i, j;

	for (i = 0; i < INVHASHSZ; i++)
		for (ip = invtab[i]; ip != NULL; ip = ip->i_next)
			for (j = 0; j < INVBLKSZ; j++)
				ip->i_dname[j] = NULL;
}

fwdtab_free()
{
	extern	struct fwdinfo *fwdtab;
	struct fwdinfo *fp, *nextfp;

	for (fp = fwdtab; fp != NULL; fp = nextfp) {
		nextfp = fp->next;
		free((char *)fp);
	}
	fwdtab = NULL;
}
