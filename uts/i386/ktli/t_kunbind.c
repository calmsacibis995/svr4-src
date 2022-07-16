/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-ktli:t_kunbind.c	1.1"
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
 *	Kernel TLI-like function to unbind a transport endpoint
 *	to an address.
 *
 *	Returns 0 on success and ret is set if non-NULL,
 *	else positive error code.
 *
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/strsubr.h>
#include <sys/ioctl.h>
#include <sys/stropts.h>
#include <sys/vnode.h>
#include <sys/tihdr.h>
#include <sys/timod.h>
#include <sys/tiuser.h>
#include <sys/t_kuser.h>
#include <sys/kmem.h>

int 
t_kunbind(tiptr)
	register TIUSER			*tiptr;
{
	register struct T_unbind_req	*unbind_req;
	register struct T_ok_ack	*ok_ack;
	register int			unbindsz;
	register struct vnode		*vp;
	register struct file		*fp;
	register char			*buf;
	struct strioctl			strioc;
	int				retval;
	int				error;

	error = 0;
	retval = 0;
	fp = tiptr->fp;
	vp = fp->f_vnode;

	/* send the ioctl request and wait
	 * for a reply.
	 */
	unbindsz = max(TUNBINDREQSZ, TOKACKSZ);
	buf = (char *)kmem_alloc(unbindsz, KM_SLEEP);
	/* LINTED pointer alignment */
	unbind_req = (struct T_unbind_req *)buf;
	unbind_req->PRIM_type = T_UNBIND_REQ;

	strioc.ic_cmd = TI_BIND;
	strioc.ic_timout = 0;
	strioc.ic_dp = buf;
	strioc.ic_len = TUNBINDREQSZ;

	error = strdoioctl(vp->v_stream, &strioc, NULL, K_TO_K,
					 (char *)NULL, u.u_cred, &retval);
	if (error)
		goto badbind;

	if (retval) {
		if ((retval & 0xff) == TSYSERR)
			error = (retval >> 8) & 0xff;
		else    error = t_tlitosyserr(retval & 0xff);
		goto badbind;
	}

	/* LINTED pointer alignment */
	ok_ack = (struct T_ok_ack *)strioc.ic_dp;
	if (strioc.ic_len < TOKACKSZ || ok_ack->PRIM_type != T_UNBIND)
		error = EIO;

badbind:
	kmem_free(buf, (u_int)unbindsz);
	return error;
}

/******************************************************************************/

