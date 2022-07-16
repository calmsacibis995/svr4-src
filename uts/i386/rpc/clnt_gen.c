/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:clnt_gen.c	1.3.2.1"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)clnt_gen.c 1.2 89/02/24 SMI"
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
 *  		  All rights reserved.
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
#include <rpc/xdr.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/stream.h>
#include <sys/tihdr.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/sysmacros.h>
#include <sys/errno.h>
#include <sys/cred.h>

#define	NC_INET		"inet"

int	clnt_clts_kcreate();

int
clnt_tli_kcreate(config, svcaddr, prog, vers, sendsz, retrys, cred, ncl)
	register struct knetconfig	*config;
	struct netbuf			*svcaddr;	/* Servers address */
	u_long				prog;		/* Program number */
	u_long				vers;		/* Version number */
	u_int				sendsz;		/* send size */
	int				retrys;
	struct cred			*cred;
	CLIENT				**ncl;
{
	CLIENT				*cl;		/* Client handle */
	TIUSER				*tiptr;
	register struct file		*fp;
	struct cred			*tmpcred;
	struct cred			*savecred;
	int				error;
	int				delaycnt = 300;	/* 5 minute wait */

	error = 0;
	cl = NULL;

	RPCLOG(2, "clnt_tli_kcreate: Entered prog %x\n", prog);

	if (config == NULL || config->knc_protofmly == NULL || ncl == NULL) {
		RPCLOG(1, "clnt_tli_kcreate: bad config or handle\n", 0);
		return EINVAL;
	}

	/* the transport should be opened as root */
	tmpcred = crdup(u.u_cred);
	savecred = u.u_cred;
	u.u_cred = tmpcred;
	u.u_cred->cr_uid = u.u_cred->cr_ruid = 0;
	u.u_uid = u.u_cred->cr_uid;
	fp = NULL;
	error = t_kopen(fp, config->knc_rdev, FREAD|FWRITE|FNDELAY, &tiptr);
	u.u_cred = savecred;
	u.u_uid = u.u_cred->cr_uid;
	crfree(tmpcred);
	if (error) {
		RPCLOG(1, "clnt_tli_kcreate: t_kopen: %d\n", error);
		return error;
	}

	/* must bind the endpoint.
	 */
	if (strcmp(config->knc_protofmly, NC_INET) == 0) {
		while ((error = bindresvport(tiptr)) != 0) {
			RPCLOG(1, "clnt_tli_kcreate: bindresvport failed error %d\n", error);
			(void)delay(HZ);
			if (!--delaycnt)
				return (error);
		}
	}
	else	{
		if ((error = t_kbind(tiptr, NULL, NULL)) != 0) {
			RPCLOG(1, "clnt_tli_kcreate: t_kbind: %d\n", error);
			goto err;
		}
	}

	switch(tiptr->tp_info.servtype) {
		case T_CLTS:
			error = clnt_clts_kcreate(tiptr, config->knc_rdev,
				svcaddr, prog, vers, sendsz, retrys, cred, &cl);
			if (error != 0) {
				RPCLOG(1, "clnt_tli_kcreate: clnt_clts_kcreate failed error %d\n", error);
				goto err;
			}
			break;

		default:
			error = EINVAL;
			RPCLOG(1, "clnt_tli_kcreate: Bad service type %d\n",
					tiptr->tp_info.servtype);
			goto err;
	}
	/* pop TIMOD off the stream just created; NFS doesn't need it! */
	(void)poptimod(tiptr->fp->f_vnode);

	*ncl = cl;
	return 0;
err:
	t_kclose(tiptr, 1);
	return error;
}

/*
 * try to bind to a reserved port
 */
int
bindresvport(tiptr)
	register TIUSER		*tiptr;
{
	struct sockaddr_in	*sin;
	register int		i;
	struct t_bind		*req;
	struct t_bind		*ret;
	int			error;

	error = 0;

#define MAX_PRIV	(IPPORT_RESERVED-1)
#define MIN_PRIV	(IPPORT_RESERVED/2)

	RPCLOG(2, "bindresvport: calling t_kalloc tiptr = %x\n", tiptr);

	if ((error = t_kalloc(tiptr, T_BIND, T_ADDR, (char **)&req)) != 0) {
		RPCLOG(1, "bindresvport: t_kalloc %d\n", error);
		return error;
	}

	RPCLOG(4, "bindresvport: calling t_kalloc tiptr = %x\n", tiptr);

	if ((error = t_kalloc(tiptr, T_BIND, T_ADDR, (char **)&ret)) != 0) {
		RPCLOG(1, "bindresvport: t_kalloc %d\n", error);
		(void)t_kfree(tiptr, req, T_BIND);
		return error;
	}
	/* LINTED pointer alignment */
	sin = (struct sockaddr_in *)req->addr.buf;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = INADDR_ANY;
	req->addr.len = sizeof(struct sockaddr_in);

	error = EADDRINUSE;
	for (i = MAX_PRIV; error == EADDRINUSE && i >= MIN_PRIV; i--) {
		sin->sin_port = htons(i);
		RPCLOG(2, "bindresvport: calling t_kbind tiptr = %x\n", tiptr);
		if ((error = t_kbind(tiptr, req, ret)) != 0)
			RPCLOG(1, "bindresvport: t_kbind: %d\n", error);
		else
		if (bcmp((caddr_t)req->addr.buf, (caddr_t)ret->addr.buf,
						 ret->addr.len) != 0) {
			RPCLOG(1, "bindresvport: bcmp error\n", 0);
			(void)t_kunbind(tiptr);
			error = EADDRINUSE;
		}
		else	error = 0;
	}
	if (error == 0)
		RPCLOG(2, "bindresvport: port assigned %d\n", sin->sin_port);
	(void)t_kfree(tiptr, req, T_BIND);
	(void)t_kfree(tiptr, ret, T_BIND);
	return error;
}
