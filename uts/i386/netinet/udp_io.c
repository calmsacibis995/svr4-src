/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:udp_io.c	1.3.1.1"

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

#include <sys/stream.h>
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

#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/errno.h>
#include <net/if.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <net/route.h>
#include <netinet/in_pcb.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/ip_str.h>

/*
 * UDP protocol implementation. Per RFC 768, August, 1980. 
 */

extern queue_t *udp_qbot;

#ifndef	COMPAT_42
int             udpcksum = 1;
#else
int             udpcksum = 0;	/* XXX */
#endif

int udp_ttl = UDP_TTL;

struct sockaddr_in udp_in;
struct udpstat  udpstat;

/*ARGSUSED*/
udp_input(q, bp0)
	queue_t        *q;
	mblk_t         *bp0;
{
	register struct udpiphdr *ui;
	register struct inpcb *inp;
	register mblk_t *bp;
	register struct T_unitdata_ind *ind;
	int             len;
	struct ip       ip;

	/*
	 * Get IP and UDP header together in first message block 
	 */
	bp = bp0->b_cont;
	freeb(bp0);
	if (msgblen(bp) < sizeof(struct udpiphdr) &&
	    (pullupmsg(bp, sizeof(struct udpiphdr))) == 0) {
		freemsg(bp);
		udpstat.udps_hdrops++;
		return;
	}
	ui = (struct udpiphdr *) bp->b_rptr;
	if (((struct ip *) ui)->ip_hl > (sizeof(struct ip) >> 2))
		(void) ip_stripoptions(bp, (mblk_t *) 0);

	/*
	 * Make message data length reflect UDP length. If not enough data to
	 * reflect UDP length, drop. 
	 */
	len = ntohs((u_short) ui->ui_ulen);
	if (((struct ip *) ui)->ip_len != len) {
		if (len > ((struct ip *) ui)->ip_len) {
			udpstat.udps_badlen++;
			goto bad;
		}
		adjmsg(bp, len - ((struct ip *) ui)->ip_len);
		/* ((struct ip *)ui)->ip_len = len; */
	}
	/*
	 * Save a copy of the IP header in case we want restore it for ICMP. 
	 */
	ip = *(struct ip *) ui;

	/*
	 * Checksum extended UDP header and data. 
	 */
	if (udpcksum && ui->ui_sum) {
		ui->ui_next = 0;
		ui->ui_mblk = 0;
		ui->ui_x1 = 0;
		ui->ui_len = ui->ui_ulen;
		if (ui->ui_sum =
		    in_cksum(bp, (int) (len + sizeof(struct ip)))) {
			udpstat.udps_badsum++;
			freemsg(bp);
			return;
		}
	}
	/*
	 * Locate pcb for datagram. 
	 */
	inp = in_pcblookup(&udb,
			 ui->ui_src, ui->ui_sport, ui->ui_dst, ui->ui_dport,
			   INPLOOKUP_WILDCARD);
	if (inp == 0) {
		/* don't send ICMP response for broadcast packet */
		if (in_broadcast(ui->ui_dst))
			goto bad;
		*(struct ip *) ui = ip;
		icmp_error((struct ip *) ui, ICMP_UNREACH, ICMP_UNREACH_PORT,
			   (queue_t *) 0, 0);
		goto bad;
	}
	STRLOG(UDPM_ID, 2, 9, SL_TRACE, "udp_input: src %x dst %x", ui->ui_src, ui->ui_dst);
	STRLOG(UDPM_ID, 2, 9, SL_TRACE, "inp %x sport %x dport %x", inp, ui->ui_sport, ui->ui_dport);

	/*
	 * Construct sockaddr format source address. Stuff source address and
	 * datagram in user buffer. 
	 */

	if (!canput(inp->inp_q->q_next)) {
		goto bad;
	}
	udp_in.sin_family = inp->inp_family;
	udp_in.sin_port = ui->ui_sport;
	udp_in.sin_addr = ui->ui_src;
	bp->b_rptr += sizeof(struct udpiphdr);
	bp0 = allocb(sizeof(struct T_unitdata_ind) +
		     inp->inp_addrlen, BPRI_MED);
	if (bp0 == NULL) {
		goto bad;
	}
	bp0->b_cont = bp;
	bp = bp0;
	bp->b_datap->db_type = M_PROTO;
	bp->b_wptr += sizeof(struct T_unitdata_ind) +
		inp->inp_addrlen;
	ind = (struct T_unitdata_ind *) bp->b_rptr;
	ind->PRIM_type = T_UNITDATA_IND;
	ind->SRC_length = inp->inp_addrlen;
	ind->SRC_offset = sizeof(struct T_unitdata_ind);
	ind->OPT_length = 0;
	ind->OPT_offset = 0;
	bcopy((caddr_t) & udp_in, (caddr_t) bp->b_rptr +
	      sizeof(struct T_unitdata_ind), inp->inp_addrlen);
	STRLOG(UDPM_ID, 2, 9, SL_TRACE, "put to inp_q->q_next %x", inp->inp_q->q_next);
	putnext(inp->inp_q, bp);
	return;
bad:
	freemsg(bp);
}


udp_ctlinput(bp)
	mblk_t         *bp;
{
	struct ip_ctlmsg *ctl;
	extern u_char   inetctlerrmap[];
	int             in_rtchange();
	struct sockaddr_in sin;
	int udp_snduderr();

	ctl = (struct ip_ctlmsg *) bp->b_rptr;
	if ((unsigned) ctl->command > PRC_NCMDS)
		return;
	if (ctl->src_addr.s_addr == INADDR_ANY)
		return;
	sin.sin_addr.s_addr = ctl->src_addr.s_addr;
	sin.sin_port = 0;

	switch (ctl->command) {

	case PRC_QUENCH:
		break;

	case PRC_ROUTEDEAD:
	case PRC_REDIRECT_NET:
	case PRC_REDIRECT_HOST:
	case PRC_REDIRECT_TOSNET:
	case PRC_REDIRECT_TOSHOST:
		in_pcbnotify(&udb, 0, &sin, 0, in_rtchange, 0);
		break;

	default:
		if (inetctlerrmap[ctl->command] == 0)
			return;	/* XXX */
		in_pcbnotify(&udb, 0, &sin,
			 (int) inetctlerrmap[ctl->command], udp_snduderr, 0);
	}
}


udp_output(inp, bp0, inp2)
	register struct inpcb *inp;
	mblk_t         *bp0;
	struct inpcb	*inp2;
{
	register mblk_t *bp;
	register struct udpiphdr *ui;
	register int    len = 0;
	register struct ip_unitdata_req *ipreq;

	/*
	 * Calculate data length and get a message for UDP and IP headers. 
	 */
	len = msgdsize(bp0);
	bp = allocb(sizeof(struct udpiphdr), BPRI_MED);
	if (bp == 0) {
		freemsg(bp0);
		return;
	}
	/*
	 * Fill in mbuf with extended UDP header and addresses and length put
	 * into network format. 
	 */
	bp->b_wptr = bp->b_datap->db_lim;
	bp->b_rptr = bp->b_wptr - sizeof(struct udpiphdr);
	bp->b_cont = bp0;
	ui = (struct udpiphdr *) bp->b_rptr;
	ui->ui_next = 0;
	ui->ui_mblk = 0;
	ui->ui_x1 = 0;
	ui->ui_pr = IPPROTO_UDP;
	ui->ui_len = htons((u_short) len + sizeof(struct udphdr));
	if (!inp2) {
		ui->ui_src = inp->inp_laddr;
		ui->ui_dst = inp->inp_faddr;
		ui->ui_sport = inp->inp_lport;
		ui->ui_dport = inp->inp_fport;
	} else {
		ui->ui_src = inp2->inp_laddr;
		ui->ui_dst = inp2->inp_faddr;
		ui->ui_sport = inp2->inp_lport;
		ui->ui_dport = inp2->inp_fport;
	}
	ui->ui_ulen = ui->ui_len;

	/*
	 * Stuff checksum and output datagram. 
	 */
	ui->ui_sum = 0;
	if (udpcksum) {
		if ((ui->ui_sum =
		     in_cksum(bp, (int) sizeof(struct udpiphdr) + len)) == 0)
			ui->ui_sum = 0xffff;
	}
	((struct ip *) ui)->ip_len = sizeof(struct udpiphdr) + len;
	((struct ip *) ui)->ip_ttl = udp_ttl;

	bp0 = bp;
	bp = allocb(sizeof(struct ip_unitdata_req), BPRI_MED);
	if (bp == NULL) {
		freemsg(bp0);
		return;
	}
	bp->b_cont = bp0;
	bp->b_wptr += sizeof(struct ip_unitdata_req);
	bp->b_datap->db_type = M_PROTO;
	ipreq = (struct ip_unitdata_req *) bp->b_rptr;
	ipreq->dl_primitive = N_UNITDATA_REQ;
	ipreq->options = inp->inp_options;
	ipreq->route = inp->inp_route;
	ipreq->flags = inp->inp_protoopt & (SO_DONTROUTE | SO_BROADCAST);
	ipreq->dl_dest_addr_length = sizeof(ui->ui_dst);
	ipreq->dl_dest_addr_offset = sizeof(struct ip_unitdata_req)
		- sizeof(struct in_addr);
	ipreq->ip_addr = ui->ui_dst;
	if (udp_qbot) {
		if (inp->inp_route.ro_rt)
			RT(inp->inp_route.ro_rt)->rt_refcnt++;
		putnext(udp_qbot, bp);
	} else {
		freemsg(bp);
	}
}
