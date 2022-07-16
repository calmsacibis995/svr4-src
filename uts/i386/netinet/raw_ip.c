/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:raw_ip.c	1.3"

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
 * System V STREAMS TCP - Release 3.0 
 *
 * Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI) 
 * All Rights Reserved. 
 *
 * The copyright above and this notice must be preserved in all copies of this
 * source code.  The copyright above does not evidence any actual or intended
 * publication of this source code. 
 *
 * This is unpublished proprietary trade secret source code of Lachman
 * Associates.  This source code may not be copied, disclosed, distributed,
 * demonstrated or licensed except as expressly authorized by Lachman
 * Associates. 
 *
 * System V STREAMS TCP was jointly developed by Lachman Associates and
 * Convergent Technologies. 
 */

#define STRNET

#ifdef INET
#include <netinet/symredef.h>
#endif INET

#include <sys/types.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/debug.h>
#include <sys/strlog.h>
#include <sys/log.h>
#ifdef SYSV
#include <sys/tihdr.h>
#include <sys/tiuser.h>
#else
#include <nettli/tihdr.h>
#include <nettli/tiuser.h>
#endif SYSV
#include <netinet/nihdr.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <netinet/in.h>
#include <net/route.h>
#include <net/if.h>
#include <netinet/in_pcb.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_var.h>
#include <netinet/ip_var.h>
#include <netinet/ip_str.h>

/*
 * Raw interface to IP protocol.
 */

extern struct inpcb rawcb;
extern queue_t *rip_qbot;
extern u_char   ip_protox[];
static struct sockaddr_in ripsrc = {AF_INET};
/*
 * Setup generic address and protocol structures for raw_input routine, then
 * pass them along with mblk chain.
 */
/* ARGSUSED */
rip_input(q, bp0)
	queue_t        *q;
	mblk_t         *bp0;
{
	register struct inpcb *inp;
	register mblk_t *bp;
	register struct T_unitdata_ind *ind;
	struct ip      *ip;
	queue_t        *lastq = NULL;
	struct in_addr  ip_src, ip_dst;
	ushort          ip_p;
	int		addrlen;

	bp = bp0->b_cont;
	freeb(bp0);
	ip = (struct ip *) bp->b_rptr;
	ripsrc.sin_family = AF_INET;
	ripsrc.sin_addr = ip_src = ip->ip_src;
	ip_dst = ip->ip_dst;
	ip_p = ip->ip_p;
	bp0 = allocb(sizeof(struct T_unitdata_ind) + sizeof(struct sockaddr_in),
		     BPRI_HI);
	if (!bp0) {
		freemsg(bp);
		return;
	}
	bp0->b_datap->db_type = M_PROTO;
	bp0->b_wptr += sizeof(struct T_unitdata_ind) + sizeof(struct sockaddr_in);
	ind = (struct T_unitdata_ind *) bp0->b_rptr;
	ind->PRIM_type = T_UNITDATA_IND;
	ind->SRC_length = sizeof(struct sockaddr_in);
	ind->SRC_offset = sizeof(struct T_unitdata_ind);
	ind->OPT_length = 0;
	ind->OPT_offset = 0;
	bcopy((caddr_t) & ripsrc,
	      (caddr_t) bp0->b_rptr + sizeof(struct T_unitdata_ind),
	      sizeof(struct sockaddr_in));
	bp0->b_cont = bp;
	addrlen  = sizeof(struct sockaddr_in);
	for (inp = rawcb.inp_next; inp != &rawcb; inp = inp->inp_next) {
		if (inp->inp_state & SS_CANTRCVMORE ||
		    inp->inp_proto &&
		    inp->inp_proto != ip_p)
			continue;
		/*
		 * We assume the lower level routines have placed the address
		 * in a canonical format suitable for a structure comparison.
		 */
		if (inp->inp_tstate == TS_IDLE &&
		    inp->inp_laddr.s_addr != INADDR_ANY &&
		    inp->inp_laddr.s_addr != ip_dst.s_addr)
			continue;
		if ((inp->inp_state & SS_ISCONNECTED) &&
		    inp->inp_faddr.s_addr != ip_src.s_addr)
			continue;
		if (inp->inp_q && canput(inp->inp_q->q_next)) {
			if (inp->inp_addrlen != addrlen
			    || inp->inp_family != ripsrc.sin_family) {
				if (!(bp = copyb(bp0)))
					continue;
				bp->b_cont = bp0->b_cont;
				freeb(bp0);
				bp0 = bp;
				bp->b_wptr += inp->inp_addrlen - addrlen;
				addrlen = inp->inp_addrlen;
				((struct T_unitdata_ind *) bp->b_rptr)->SRC_length = addrlen;
				ripsrc.sin_family = inp->inp_family;
				((struct sockaddr_in *) (bp->b_rptr + sizeof(struct T_unitdata_ind)))->sin_family = ripsrc.sin_family;
			}
			if (!(bp = dupmsg(bp0)))
				break;
			putnext(inp->inp_q, bp);
			STRLOG(RIPM_ID, 2, 9, SL_TRACE, "put to inp_q %x", inp->inp_q->q_next);
		}
		lastq = inp->inp_q;
	}
	if (lastq && canput(lastq->q_next)) {
		STRLOG(RIPM_ID, 2, 9, SL_TRACE, "put to inp_q %x", lastq->q_next);
		putnext(lastq, bp0);
	} else
		freemsg(bp0);
}

/*
 * Generate IP header and pass packet to ip_output. Tack on options user may
 * have setup with control call.
 */
rip_output(q, bp0)
	queue_t        *q;
	mblk_t         *bp0;
{
	mblk_t         *bp = bp0;
	register struct ip *ip;
	int             len, error;
	struct inpcb   *inp = qtoinp(q);
	struct ip_unitdata_req *ipreq;

	if (rip_qbot) {
		if (!canput(rip_qbot->q_next))
			return (-1);
	} else {
		error = ENOLINK;
		goto bad;
	}
	/*
	 * Calculate data length and get an mblk for IP header.
	 */
	len = msgdsize(bp0);
	bp0 = allocb(sizeof(struct ip), BPRI_MED);
	if (!bp0) {
		error = ENOSR;
		goto bad;
	}
	/*
	 * Fill in IP header as needed.
	 */
	ip = (struct ip *) bp0->b_rptr =
		(struct ip *) (bp0->b_datap->db_lim - sizeof(struct ip));
	bp0->b_wptr = bp0->b_datap->db_lim;
	bp0->b_cont = bp;
	bp = bp0;
	bp0 = allocb(sizeof(struct ip_unitdata_req), BPRI_HI);
	if (!bp0) {
		error = ENOSR;
		goto bad;
	}
	bp0->b_cont = bp;
	bp0->b_wptr += sizeof(struct ip_unitdata_req);
	bp0->b_datap->db_type = M_PROTO;
	ip->ip_tos = 0;
	ip->ip_off = 0;
	ip->ip_p = inp->inp_proto;
	ip->ip_len = sizeof(struct ip) + len;
	if (inp->inp_tstate == TS_IDLE) {
		ip->ip_src = inp->inp_laddr;
	} else
		ip->ip_src.s_addr = 0;
	ip->ip_dst = inp->inp_faddr;
	ip->ip_ttl = MAXTTL;
	ipreq = (struct ip_unitdata_req *) bp0->b_rptr;
	ipreq->dl_primitive = N_UNITDATA_REQ;
	ipreq->options = inp->inp_options;
	ipreq->route = inp->inp_route;
	ipreq->flags = (inp->inp_protoopt & SO_DONTROUTE) | IP_ALLOWBROADCAST;
	ipreq->dl_dest_addr_length = sizeof(struct in_addr);
	ipreq->dl_dest_addr_offset = sizeof(struct ip_unitdata_req)
		- sizeof(struct in_addr);
	ipreq->ip_addr = ip->ip_dst;
	if (inp->inp_route.ro_rt)
		RT(inp->inp_route.ro_rt)->rt_refcnt++;
	putnext(rip_qbot, bp0);
	return (0);
bad:
	freemsg(bp);
	return (error);
}

/*
 * Raw IP socket option processing.
 */
rip_ctloutput(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	int             in_pcboptmgmt(), ip_options();
	static struct opproc funclist[] = {
		SOL_SOCKET, in_pcboptmgmt,
		IPPROTO_IP, ip_options,
		0, 0
	};

	dooptions(q, bp, funclist);
}
