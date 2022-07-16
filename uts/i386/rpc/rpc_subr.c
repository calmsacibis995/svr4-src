/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:rpc_subr.c	1.3.1.1"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)rpc_subr.c 1.1 89/08/21 SMI"
#endif
/* 
 * Miscellaneous support routines for kernel implementation of RPC.
 *
 */

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/vnode.h>
#include	<sys/stream.h>
#include	<sys/stropts.h>
#include	<sys/strsubr.h>
#include	<sys/cred.h>
#include	<sys/proc.h>
#include	<sys/user.h>
#include	<rpc/types.h>

#ifdef _KERNEL
/*
 * Kernel level debugging aid. The global variable "rpclog" is a bit
 * mask which allows various types of debugging messages to be printed
 * out.
 * 
 *	rpclog & 1 	will cause actual failures to be printed.
 *	rpclog & 2	will cause informational messages to be
 *			printed on the client side of rpc.
 *	rpclog & 4	will cause informational messages to be
 *			printed on the server side of rpc.
 */

int rpclog = 0;

int
rpc_log(level, str, a1)
	ulong		level;
	register char	*str;
	register int	a1;

{
	if (level & rpclog)
		printf(str, a1);
}

/* pop TIMOD off the stream: it appears we don't need it */
void
poptimod(vp)
struct vnode *vp;
{
	int error, isfound, ret;

	error = strioctl(vp, I_FIND, "timod", 0, K_TO_K, u.u_cred, &isfound);
	if (error) {
		RPCLOG(1, "poptimod: I_FIND strioctl error %d\n", error);
		return;
	}
	if (isfound != 0) {
		error = strioctl(vp, I_POP, "timod", 0, K_TO_K, u.u_cred, &ret);
		if (error)
			RPCLOG(1, "poptimod: I_POP strioctl error %d\n", error);
	}
}
#endif _KERNEL
		
