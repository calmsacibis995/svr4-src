/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-ktli:t_kopen.c	1.1.2.1"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)t_kopen.c 1.2 89/03/19 SMI"
#endif

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
 *
 *	Kernel TLI-like function to initialize a transport
 *	endpoint using the protocol specified.
 *
 *	Returns:
 *		0 on success and "tiptr" is set to a valid transport pointer,
 *		else a positive error code.
 *
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/vnode.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/ioctl.h>
#include <sys/stropts.h>
#include <sys/strsubr.h>
#include <sys/tihdr.h>
#include <sys/timod.h>
#include <sys/tiuser.h>
#include <sys/t_kuser.h>
#include <sys/kmem.h>
#include <sys/cmn_err.h>

static _t_setsize();

int
t_kopen(fp, rdev, flags, tiptr)
	struct file		*fp;
	register int		flags;
	register dev_t		rdev;
	TIUSER			**tiptr;

{
	extern struct vnode	*makespecvp();

	register int		madefp = 0;
	struct T_info_ack	inforeq;
	int			retval;
	struct vnode		*vp;
	struct strioctl		strioc;
	int			error;
	TIUSER			*ntiptr;

	KTLILOG(2, "t_kopen: fp %x, ", fp);
	KTLILOG(2, "rdev %x, ", rdev);
	KTLILOG(2, "flags %x\n", flags);

	error = 0;
	retval = 0;
	if (fp == NULL) {
		int fd;

		if (rdev == 0) {
			KTLILOG(1, "t_kopen: null device\n", 0);
			return EINVAL;
		}
		/* make a vnode.
		 */
		vp = makespecvp(rdev, VCHR);

		/* this will call the streams open
		 * for us.
		 */
		if ((error = VOP_OPEN(&vp, flags, u.u_cred)) != 0) {
			KTLILOG(1, "t_kopen: VOP_OPEN: %d\n", error);
			return error;
		}
		/* allocate a file pointer, but
		 * no file descripter.
		 */
		while ((error = falloc(vp, flags, &fp, &fd)) != 0) {
			KTLILOG(1, "t_kopen: falloc: %d\n", error);
			if (error == EMFILE) {
				return error;
			}
			(void)delay(HZ);
		}
		setf(fd, NULLFP);

		madefp = 1;
	}
	else	vp = (struct vnode *)fp->f_vnode;

	/* allocate a new transport structure
	 */
	ntiptr = (TIUSER *)kmem_alloc((u_int)TIUSERSZ, KM_SLEEP);
	ntiptr->fp = fp;

	KTLILOG(2, "t_kopen: vp %x, ", vp);
	KTLILOG(2, "stp %x\n", vp->v_stream);

	/* see if TIMOD is already pushed
	 */
	error = strioctl(vp, I_FIND, "timod", 0, K_TO_K, u.u_cred, &retval);
	if (error) {
		kmem_free((caddr_t)ntiptr, (u_int)TIUSERSZ);
		if (madefp)
			closef(fp);
		KTLILOG(1, "t_kopen: strioctl(I_FIND, timod): %d\n", error);
		return error;
	}

	if (retval == 0) {
tryagain:
		error = strioctl(vp, I_PUSH, "timod", 0, K_TO_K, u.u_cred,
								 &retval);
		if (error) {
			switch(error) {
			case ENOSPC:
			case EAGAIN:
			case ENOSR:
				/* This probably means the master file
				 * should be tuned.
				 */
				cmn_err(CE_WARN, "t_kopen: I_PUSH of timod failed, error %d\n", error);
				(void)delay(HZ);
				error = 0;
				goto tryagain;

			default:
				kmem_free((caddr_t)ntiptr, (u_int)TIUSERSZ);
				if (madefp)
					closef(fp);
				KTLILOG(1, "t_kopen: I_PUSH (timod): %d", error);
				return error;
			}
		}
	}

	inforeq.PRIM_type = T_INFO_REQ;
	strioc.ic_cmd = TI_GETINFO;
	strioc.ic_timout = 0;
	strioc.ic_dp = (char *)&inforeq;
	strioc.ic_len = sizeof(struct T_info_req);

	error = strdoioctl(vp->v_stream, &strioc, NULL, K_TO_K,
					 (char *)NULL, u.u_cred, &retval);
	if (error) {
		kmem_free((caddr_t)ntiptr, (u_int)TIUSERSZ);
		if (madefp)
			closef(fp);
		KTLILOG(1, "t_kopen: strdoioctl(T_INFO_REQ): %d\n", error);
		return error;
	}

	if (retval) {
		if ((retval & 0xff) == TSYSERR)
			error = (retval >> 8) & 0xff;
		else    error = t_tlitosyserr(retval & 0xff);
		kmem_free((caddr_t)ntiptr, (u_int)TIUSERSZ);
		if (madefp)
			closef(fp);
		KTLILOG(1, "t_kopen: strdoioctl(T_INFO_REQ): retval: 0x%x\n", retval);
		return error;
	}

	if (strioc.ic_len != sizeof(struct T_info_ack)) {
		kmem_free((caddr_t)ntiptr, (u_int)TIUSERSZ);
		if (madefp)
			closef(fp);
		KTLILOG(1, "t_kopen: strioc.ic_len != sizeof (struct T_info_ack): %d\n", strioc.ic_len);
		return EPROTO;
	}

	ntiptr->tp_info.addr = _t_setsize(inforeq.ADDR_size);
	ntiptr->tp_info.options = _t_setsize(inforeq.OPT_size);
	ntiptr->tp_info.tsdu = _t_setsize(inforeq.TSDU_size);
	ntiptr->tp_info.etsdu = _t_setsize(inforeq.ETSDU_size);
	ntiptr->tp_info.connect = _t_setsize(inforeq.CDATA_size);
	ntiptr->tp_info.discon = _t_setsize(inforeq.DDATA_size);
	ntiptr->tp_info.servtype = inforeq.SERV_type;

	*tiptr = ntiptr;

	return (0);
}

#define	DEFSIZE	128
static int
_t_setsize(infosize)
long infosize;
{
        switch(infosize)
        {
                case -1: return(DEFSIZE);
                case -2: return(0);
                default: return(infosize);
        }
}

/******************************************************************************/
