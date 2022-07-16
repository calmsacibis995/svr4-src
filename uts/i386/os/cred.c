/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

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
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ident	"@(#)kern-os:cred.c	1.3"

#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/vfs.h"
#include "sys/cred.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/errno.h"
#include "sys/immu.h"
#include "sys/tss.h"
#include "sys/kmem.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/conf.h"
#include "sys/debug.h"
#include "sys/proc.h"
#include "sys/acct.h"
#include "sys/fault.h"
#include "sys/syscall.h"
#include "sys/procfs.h"
#include "sys/dl.h"
#include "sys/cmn_err.h"
#include "sys/tuneable.h"
#include "sys/inline.h"

struct credlist {
	union {
		struct cred cru_cred;
		struct credlist *cru_next;
	} cl_U;
#define	cl_cred	cl_U.cru_cred
#define	cl_next	cl_U.cru_next
};

STATIC struct credlist *crfreelist = NULL;
STATIC u_int crsize = 0;
STATIC int cractive = 0;

/*
 * Initialize credentials data structures.  (Could be compile-time
 * initializations except that "mkunix" on the 3B2 builds a unix
 * after the initial values have already been changed.)
 */

void
cred_init()
{
	cractive = 0;
	crsize = 0;
	crfreelist = NULL;
}

/*
 * Allocate a zeroed cred structure and crhold() it.
 */
struct cred *
crget()
{
	register struct cred *cr;

	if (crsize == 0) {
		crsize = sizeof(struct cred) + sizeof(uid_t)*(ngroups_max-1);
		/* Make sure it's word-aligned. */
		crsize = (crsize+sizeof(int)-1) & ~(sizeof(int)-1);
	}
	if (crfreelist) {
		cr = &crfreelist->cl_cred;
		crfreelist = ((struct credlist *)cr)->cl_next;
	} else
		cr = (struct cred *)kmem_alloc(crsize, KM_SLEEP);
	struct_zero((caddr_t)cr, sizeof(*cr));
	crhold(cr);
	cractive++;
	return cr;
}

/*
 * Free a cred structure.  Return it to the freelist when the reference
 * count drops to 0.
 */
void
crfree(cr)
	register struct cred *cr;
{
	register int s = spl6();

	if (--cr->cr_ref != 0) {
		(void) splx(s);
		return;
	}
	((struct credlist *)cr)->cl_next = crfreelist;
	crfreelist = (struct credlist *)cr;
	cractive--;

	(void) splx(s);
}

/*
 * Copy a cred structure to a new one and free the old one.
 */
struct cred *
crcopy(cr)
	register struct cred *cr;
{
	register struct cred *newcr;

	newcr = crget();
	bcopy((caddr_t)cr, (caddr_t)newcr, crsize);
	crfree(cr);
	newcr->cr_ref = 1;
	return newcr;
}

/*
 * Dup a cred struct to a new held one.
 */
struct cred *
crdup(cr)
	struct cred *cr;
{
	register struct cred *newcr;

	newcr = crget();
	bcopy((caddr_t)cr, (caddr_t)newcr, crsize);
	newcr->cr_ref = 1;
	return newcr;
}

/*
 * Return the (held) credentials for the current running process.
 */
struct cred *
crgetcred()
{
	crhold(u.u_cred);
	return u.u_cred;
}

/*
 * Test if the supplied credentials identify the super-user.
 * Distasteful side-effect: set an accounting flag in the
 * caller's u-block if the answer is yes.
 */ 
int
suser(cr)
	register struct cred *cr;
{
	if (cr->cr_uid == 0) {
		u.u_acflag |= ASU;	/* XXX */
		return 1;
	}
	return 0;
}

/*
 * Determine whether the supplied group id is a member of the group
 * described by the supplied credentials.
 */
int
groupmember(gid, cr)
	register gid_t gid;
	register struct cred *cr;
{
	register gid_t *gp, *endgp;

	if (gid == cr->cr_gid)
		return 1;
	endgp = &cr->cr_groups[cr->cr_ngroups];
	for (gp = cr->cr_groups; gp < endgp; gp++)
		if (*gp == gid)
			return 1;
	return 0;
}

/*
 * This function is called to check whether the credentials set
 * "scrp" has permission to act on credentials set "tcrp".  It enforces the
 * permission requirements needed to send a signal to a process.
 * The same requirements are imposed by other system calls, however.
 */

int
hasprocperm(tcrp, scrp)
	register cred_t	*tcrp;
	register cred_t	*scrp;
{
	return scrp->cr_uid == 0
	  || scrp->cr_uid  == tcrp->cr_ruid
	  || scrp->cr_ruid == tcrp->cr_ruid
	  || scrp->cr_uid  == tcrp->cr_suid
	  || scrp->cr_ruid == tcrp->cr_suid;
}
