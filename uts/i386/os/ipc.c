/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:ipc.c	1.3"
/*
 * Common Inter-Process Communication routines.
 */

#include	"sys/errno.h"
#include	"sys/types.h"
#include	"sys/param.h"
#include	"sys/cred.h"
#include	"sys/signal.h"
#include	"sys/immu.h"
#include	"sys/proc.h"
#include	"sys/user.h"
#include	"sys/ipc.h"

/*
 * Check message, semaphore, or shared memory access permissions.
 *
 * This routine verifies the requested access permission for the current
 * process.  The super-user is always given permission.  Otherwise, the
 * appropriate bits are checked corresponding to owner, group (including
 * the list of supplementary groups), or everyone.  Zero is returned on
 * success.  On failure, a non-zero errno (EACCES) is returned.
 *
 * The arguments must be set up as follows:
 * 	p - Pointer to permission structure to verify
 * 	mode - Desired access permissions
 */
int
ipcaccess(p, mode, cr)
	register struct ipc_perm	*p;
	register int			mode;
	struct cred			*cr;
{
	if (cr->cr_uid == 0)
		return 0;
	if (cr->cr_uid != p->uid && cr->cr_uid != p->cuid) {
		mode >>= 3;
		if (!groupmember(p->gid, cr) && !groupmember(p->cgid, cr))
			mode >>= 3;
	}
	if (mode & p->mode)
		return 0;
	return EACCES;
}

/*
 * Get message, semaphore, or shared memory structure.
 *
 * This routine searches for a matching key based on the given flags
 * and returns, in *ipcpp, a pointer to the appropriate entry. 
 * A structure is allocated if the key doesn't exist and the flags call
 * for it.  The arguments must be set up as follows:
 * 	key - Key to be used
 * 	flag - Creation flags and access modes
 * 	base - Base address of appropriate facility structure array
 * 	cnt - # of entries in facility structure array
 * 	size - sizeof(facility structure)
 * 	status - Pointer to status word: set on successful completion
 * 		only:	0 => existing entry found
 * 			1 => new entry created
 * Ipcget returns 0 on success, or an appropriate non-zero errno on failure.
 */
int
ipcget(key, flag, base, cnt, size, status, ipcpp)
	key_t				key;
	int				flag;
	register struct ipc_perm	*base;
	int				cnt;
	int				size;
	int				*status;
	struct ipc_perm			**ipcpp;
{
	register struct ipc_perm	*a;	/* ptr to available entry */
	register int			i;	/* loop control */

	if (key == IPC_PRIVATE) {
		for (i = 0; i++ < cnt;
		  /* LINTED pointer alignment */
		  base = (struct ipc_perm *)(((char *)base) + size)) {
			if (base->mode & IPC_ALLOC)
				continue;
			goto init;
		}
		return ENOSPC;
	} else {
		for (i = 0, a = NULL; i++ < cnt;
		  /* LINTED pointer alignment */
		  base = (struct ipc_perm *)(((char *)base) + size)) {
			if (base->mode & IPC_ALLOC) {
				if (base->key == key) {
					if ((flag & (IPC_CREAT | IPC_EXCL)) ==
					  (IPC_CREAT | IPC_EXCL)) {
						return EEXIST;
					}
					if ((flag & 0777) & ~base->mode)
						return EACCES;
					*status = 0;
					*ipcpp = base;
					return 0;
				}
				continue;
			}
			if (a == NULL)
				a = base;
		}
		if (!(flag & IPC_CREAT))
			return ENOENT;
		if (a == NULL)
			return ENOSPC;
		base = a;
	}
init:
	*status = 1;
	base->mode = IPC_ALLOC | (flag & 0777);
	base->key = key;
	base->cuid = base->uid = u.u_cred->cr_uid;
	base->cgid = base->gid = u.u_cred->cr_gid;
	*ipcpp = base;
	return 0;
}
