/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:authdesubr.c	1.3"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)authdesubr.c 1.3 89/03/19 SMI"
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
 * Miscellaneous support routines for kernel implentation of AUTH_DES
 */

/*
 *  rtime - get time from remote machine
 *
 *  sets time, obtaining value from host
 *  on the udp/time socket.  Since timeserver returns
 *  with time of day in seconds since Jan 1, 1900,  must
 *  subtract 86400(365*70 + 17) to get time
 *  since Jan 1, 1970, which is what get/settimeofday
 *  uses.
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socketvar.h>
#include <sys/errno.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/socket.h>
#include <sys/sysmacros.h>
#include <netinet/in.h>
#include <rpc/rpc.h>
#include <sys/stream.h>
#include <sys/strsubr.h>
#include <sys/cred.h>
#include <sys/utsname.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/systeminfo.h>
#include <rpc/rpcb_prot.h>

#define USEC_PER_SEC 1000000
#define TOFFSET ((u_long)86400*(365*70 + (70/4)))
#define WRITTEN ((u_long)86400*(365*86 + (86/4)))

#define NC_INET		"inet"		/* XXX */

rtime(synctp, addrp, calltype, timep, wait)
	dev_t			synctp;
	struct netbuf		*addrp;
	int			calltype;
	struct timeval		*timep;
	struct timeval		*wait;
{
	extern timestruc_t	time;
	int			error;
	int			timo;
	time_t			thetime;
	int			dummy;
	struct t_kunitdata	*unitdata;
	TIUSER			*tiptr;
	struct vnode		*vp;
	int			type;
	int			uderr;
	int			retries;

	retries = 5;
	if (calltype == 0) {
		/* Use old method.
		 */
again:
		RPCLOG(8, "rtime: using old method\n", 0);
		if ((error = t_kopen(NULL, synctp,
					FREAD|FWRITE|FNDELAY, &tiptr)) != 0) {
			RPCLOG(1, "rtime: t_kopen %d\n", error);
			return -1;
		}
	
		if ((error = t_kbind(tiptr, NULL, NULL)) != 0) {
			(void)t_kclose(tiptr, 1);
			RPCLOG(1, "rtime: t_kbind %d\n", error);
			return -1;
		}
	
		if ((error = t_kalloc(tiptr, T_UNITDATA, T_UDATA|T_ADDR,
					 	(char **)&unitdata)) != 0) {
			RPCLOG(1, "rtime: t_kalloc %d\n", error);
			(void)t_kclose(tiptr, 1);
			return -1;
		}
	
		unitdata->addr.len = addrp->len;
		bcopy(addrp->buf, unitdata->addr.buf, unitdata->addr.len);
	
		unitdata->udata.buf = (caddr_t)&dummy;
		unitdata->udata.len = sizeof(dummy);
	
		if ((error = t_ksndudata(tiptr, unitdata, NULL)) != 0) {
			RPCLOG(1, "rtime: t_ksndudata %d\n", error);
			(void)t_kfree(tiptr, (char *)unitdata, T_UNITDATA);
			(void)t_kclose(tiptr, 1);
			return -1;
		}
	
		timo = (int)(wait->tv_sec * HZ +
				(wait->tv_usec * HZ) / USEC_PER_SEC);
		RPCLOG(8, "rtime: timo %x\n", timo);
		if ((error = t_kspoll(tiptr, timo, READWAIT, &type)) != 0) {
			RPCLOG(1, "rtime: t_kspoll %d\n", error);
			(void)t_kfree(tiptr, (char *)unitdata, T_UNITDATA);
			(void)t_kclose(tiptr, 1);
			return -1;
		}
	
		if (type == 0) {
			RPCLOG(1, "rtime: t_kspoll timed out\n", 0);
			(void)t_kfree(tiptr, (char *)unitdata, T_UNITDATA);
			(void)t_kclose(tiptr, 1);
			return -1;
		}
	
		if ((error =t_krcvudata(tiptr, unitdata, &type, &uderr)) != 0) {
			RPCLOG(1, "rtime: t_krcvudata %d\n", error);
			(void)t_kfree(tiptr, (char *)unitdata, T_UNITDATA);
			(void)t_kclose(tiptr, 1);
			return -1;
		}
	
		if (type != T_DATA) {
			RPCLOG(1, "rtime: t_krcvudata rtnd type %d\n", type);
			(void)t_kfree(tiptr, (char *)unitdata, T_UNITDATA);
			(void)t_kclose(tiptr, 1);
			if (retries-- == 0)
				return -1;
			else	goto again;
		}
	
		if (unitdata->udata.len < sizeof(u_long)) {
			RPCLOG(1, "rtime: bad rcvd length %d\n",
						unitdata->udata.len);
			(void)t_kfree(tiptr, (char *)unitdata, T_UNITDATA);
			(void)t_kclose(tiptr, 1);
			if (retries-- == 0)
				return -1;
			else	goto again;
		}
	
		/* LINTED pointer alignment */
		thetime = (time_t) ntohl(*(u_long *)unitdata->udata.buf);

		(void)t_kfree(tiptr, (char *)unitdata, T_UNITDATA);
		(void)t_kclose(tiptr, 1);
	}
	else	{
		CLIENT			*client;
		struct knetconfig	config;
		struct timeval		timeout;

		RPCLOG(1, "rtime: using new method\n", 0);

		/* We talk to rpcbind.
		 */
		config.knc_rdev = synctp;
		config.knc_protofmly = NC_INET;		/* XXX */
		error = clnt_tli_kcreate(&config, addrp, (u_long)RPCBPROG,
	       			(u_long)RPCBVERS, 0, retries,
				 u.u_procp->p_cred, &client);
	 
		if (error != 0) {
			RPCLOG(1, 
			  "key_call: clnt_tli_kcreate rtned error %d", error);
			return -1;
		}
		timeout.tv_sec = 60;
		timeout.tv_usec = 0;
		error = clnt_call(client, RPCBPROC_GETTIME, xdr_void, NULL, 
				xdr_u_long, (caddr_t)&thetime, timeout);
		auth_destroy(client->cl_auth);
		clnt_destroy(client);
		if (error != RPC_SUCCESS) {
			RPCLOG(1, 
			"rtime: time sync clnt_call failed: error %x", error);
			RPCLOG(1, clnt_sperrno(error), 0);
			error = EIO;
			return -1;
		}
	}

	if (calltype != 0)
		thetime += TOFFSET;

	RPCLOG(8, "rtime: thetime = %x\n", thetime);

	if (thetime < WRITTEN) {
		RPCLOG(1, "rtime: time returned is too far in past %x",
						thetime);
		RPCLOG(1, "rtime: WRITTEN %x", WRITTEN);
		return -1;
	}
	thetime -= TOFFSET;

	timep->tv_sec = thetime;
	RPCLOG(8, "rtime: timep->tv_sec = %x\n", timep->tv_sec);
	RPCLOG(8, "rtime: machine time  = %x\n", hrestime.tv_sec);
	timep->tv_usec = 0;
	RPCLOG(8, "rtime: returning success\n", 0);
	return 0;
}


/*
 * Short to ascii conversion
 */
static char *
sitoa(s, i)
	char *s;
	short i;
{
	char *p;
	char *end;
	char c;

	if (i < 0) {
		*s++ = '-';		
		i = -i;
	} else if (i == 0) {
		*s++ = '0';
	}

	/*
	 * format in reverse order
	 */
	for (p = s; i > 0; i /= 10) {	
		*p++ = (i % 10) + '0';
	}
	*(end = p) = 0; 

	/*
	 * reverse
	 */
	while (p > s) {
		c = *--p;
		*p = *s;
		*s++ = c;
	}
	return(end);
}

static char *
atoa(dst, src)
	char *dst;	
	char *src;
{
	while (*dst++ = *src++)
		;
	return(dst-1);
}

/*
 * What is my network name?
 * WARNING: this gets the network name in sun unix format. 
 * Other operating systems (non-unix) are free to put something else
 * here.
 */

void
getnetname(netname)
	char *netname;
{
	char *p;

	p = atoa(netname, "unix.");
	if (u.u_cred->cr_uid == 0) {
		p = atoa(p, utsname.nodename);
	} else {
		p = sitoa(p, (short)u.u_cred->cr_uid);
	}
	*p++ = '@';
	p = atoa(p, srpc_domain);
}
