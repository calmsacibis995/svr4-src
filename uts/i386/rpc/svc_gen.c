/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:svc_gen.c	1.3.1.1"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)svc_generic.c 1.1 88/12/12 SMI"
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
 * svc_generic.c,
 * Server side for RPC in the kernel.
 *
 */

#include <sys/param.h>
#include <sys/types.h>
#include <rpc/types.h>
#include <netinet/in.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <sys/tiuser.h>
#include <sys/t_kuser.h>
#include <rpc/svc.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/stream.h>
#include <sys/tihdr.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
 
int
svc_tli_kcreate(fp, sendsz, nxprt)
	register struct file	*fp;		/* connection end point */
	u_int			sendsz;		/* max sendsize */
	SVCXPRT			**nxprt;
{
	SVCXPRT			*xprt = NULL;		/* service handle */
	TIUSER			*tiptr = NULL;
	int			error;

	error = 0;

	if (fp == NULL || nxprt == NULL)
		return EINVAL;

	if ((error = t_kopen(fp, -1, FREAD|FWRITE|FNDELAY, &tiptr)) != 0) {
                RPCLOG(1, "svc_tli_kcreate: t_kopen: %d\n", error);
         	return error;
	}

	/*
	 * call transport specific function.
	 */
	switch(tiptr->tp_info.servtype) {
		case T_CLTS:
			error = svc_clts_kcreate(tiptr, sendsz, &xprt);
			break;
		default:
                	RPCLOG(1, "svc_tli_kcreate: Bad service type %d\n", 
				tiptr->tp_info.servtype);
			error = EINVAL;
			break;
        }
	if (error != 0)
		goto freedata;

	/*  This is for performance improvements since RPC does not
	 *  use timod.
	 */
	(void)poptimod(tiptr->fp->f_vnode);


	xprt->xp_port = (u_short)-1;	/* To show that it is tli based. Switch */

	*nxprt = xprt;
	return 0;

freedata:
	if (xprt)
		SVC_DESTROY(xprt);
	return error;
}



