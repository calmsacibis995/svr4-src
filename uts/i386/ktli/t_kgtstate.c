/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-ktli:t_kgtstate.c	1.1"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)t_kgetstate.c 1.2 89/01/11 SMI"
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
 *	Kernel TLI-like function to get the state of an
 *	endpoint. 
 *
 *	Returns:
 *		0 on success and "state" is set to the current state,
 *		or a positive error code.
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


int
t_kgetstate(tiptr, state)
	register TIUSER		*tiptr;
	register int		*state;
{
	struct T_info_ack	inforeq;
	struct strioctl		strioc;
	int 			retval;
	register struct vnode 	*vp;
	register struct file	*fp;
	int			error;

	error = 0;
	retval = 0;
	fp = tiptr->fp;
	vp = fp->f_vnode;

	if (state == NULL)
		return EINVAL;

	inforeq.PRIM_type = T_INFO_REQ;
	strioc.ic_cmd = TI_GETINFO;
	strioc.ic_timout = 0;
	strioc.ic_dp = (char *)&inforeq;
	strioc.ic_len = sizeof(struct T_info_req);

	error = strdoioctl(vp->v_stream, &strioc, NULL, K_TO_K,
					 (char *)NULL, u.u_cred, &retval);
	if (error) 
		return error;

	if (retval) {
		if ((retval & 0xff) == TSYSERR)
			error = (retval >> 8) & 0xff;
		else    error = t_tlitosyserr(retval & 0xff);
		return error;
	}

	if (strioc.ic_len != sizeof(struct T_info_ack))
		return EPROTO;

	switch (inforeq.CURRENT_state) {
		case TS_UNBND:
			*state = T_UNBND;
			break;

		case TS_IDLE:
			*state = T_IDLE;
			break;

		case TS_WRES_CIND:
			*state = T_INCON;
			break;

		case TS_WCON_CREQ:
			*state = T_OUTCON;
			break;

		case TS_DATA_XFER:
			*state = T_DATAXFER;
			break;

		case TS_WIND_ORDREL:
			*state = T_OUTREL;
			break;

		case TS_WREQ_ORDREL:
			*state = T_INREL;
			break;

		default:
			error = EPROTO;
			break;
	}
	return error;
}

/******************************************************************************/

