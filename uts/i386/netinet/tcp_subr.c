/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:tcp_subr.c	1.3.1.1"

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
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/strlog.h>
#include <sys/log.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <net/if.h>
#include <sys/errno.h>
#ifdef SYSV
#include <sys/tihdr.h>
#else
#include <nettli/tihdr.h>
#endif SYSV
#include <netinet/nihdr.h>
#ifdef SYSV
#ifdef SYSV
#include <sys/cmn_err.h>
#endif
#endif SYSV

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <net/route.h>
#include <netinet/in_pcb.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_str.h>
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcpip.h>
#include <netinet/insrem.h>
#include <sys/kmem.h>

int	tcp_ttl = TCP_TTL;

extern queue_t *tcp_qbot;

/*
 * Create template to be used to send tcp packets on a connection. Call after
 * host entry created, allocates an mblk and fills in a skeletal tcp/ip
 * header, minimizing the amount of work necessary when the connection is
 * used. 
 */
struct tcpiphdr *
tcp_template(tp)
	struct tcpcb   *tp;
{
	register struct inpcb *inp = tp->t_inpcb;
	register mblk_t *bp;
	register struct tcpiphdr *n;

	if ((n = tp->t_template) == 0) {
		bp = allocb(sizeof(struct tcpiphdr), BPRI_HI);
		if (bp == NULL)
			return (0);
		bp->b_rptr = bp->b_datap->db_lim - sizeof(struct tcpiphdr);
		bp->b_wptr = bp->b_datap->db_lim;
		n = (struct tcpiphdr *) bp->b_rptr;
		tp->t_tmplhdr = bp;
	}
	n->ti_next = 0;
	n->ti_mblk = 0;
	n->ti_x1 = 0;
	n->ti_pr = IPPROTO_TCP;
	n->ti_len = htons(sizeof(struct tcpiphdr) - sizeof(struct ip));
	n->ti_src = inp->inp_laddr;
	n->ti_dst = inp->inp_faddr;
	n->ti_sport = inp->inp_lport;
	n->ti_dport = inp->inp_fport;
	n->ti_seq = 0;
	n->ti_ack = 0;
	n->ti_x2 = 0;
	n->ti_off = 5;
	n->ti_flags = 0;
	n->ti_win = 0;
	n->ti_sum = 0;
	n->ti_urp = 0;
	return (n);
}

/*
 * Send a single message to the TCP at address specified by the given TCP/IP
 * header.  If flags==0, then we make a copy of the tcpiphdr at ti and send
 * directly to the addressed host. This is used to force keep alive messages
 * out using the TCP template for a connection tp->t_template.  If flags are
 * given then we send a message back to the TCP which originated the segment
 * ti, and discard the mbuf containing it and any other attached mblocks. 
 *
 * In any case the ack and sequence number of the transmitted segment are as
 * specified by the parameters. 
 */
tcp_respond(bp, tp, ti, ack, seq, flags)
	mblk_t         *bp;
	struct tcpcb   *tp;
	register struct tcpiphdr *ti;
	tcp_seq         ack, seq;
	int             flags;
{
	mblk_t         *bp0;
	int             win = 0, tlen;
	struct route    tcproute, *ro = 0;
	struct ip_unitdata_req *ipreq;

	if (tp) {
		if (tp->t_inpcb->inp_q && !(tp->t_inpcb->inp_state & SS_CANTRCVMORE)) {
			win = tp->t_inpcb->inp_q->q_hiwat - tp->t_iqsize;
			if (win < (long)(tp->rcv_adv - tp->rcv_nxt))
				win = (long)(tp->rcv_adv - tp->rcv_nxt);
		}
		ro = &tp->t_inpcb->inp_route;
	} else {
		ro = &tcproute;
		bzero((caddr_t) ro, sizeof(*ro));
	}
	if (flags == 0) {
		bp = allocb(sizeof(struct tcpiphdr) + 1, BPRI_HI);
		if (bp == NULL)
			return;
#ifdef TCP_COMPAT_42
		tlen = 1;
#else
		tlen = 0;
#endif
		bp->b_wptr += sizeof(struct tcpiphdr) + tlen;
		bcopy((char *) ti, (char *) bp->b_rptr, sizeof(*ti));
		ti = (struct tcpiphdr *) bp->b_rptr;
		flags = TH_ACK;
	} else {
		freemsg(bp->b_cont);
		bp->b_cont = NULL;
		bp->b_rptr = (u_char *) ti;
		tlen = 0;
		bp->b_wptr = bp->b_rptr + sizeof(struct tcpiphdr);
#define xchg(a,b,type) { type t; t=a; a=b; b=t; }
		xchg(ti->ti_dst.s_addr, ti->ti_src.s_addr, u_long);
		xchg(ti->ti_dport, ti->ti_sport, u_short);
#undef xchg
	}
	ti->ti_next = 0;
	ti->ti_mblk = 0;
	ti->ti_x1 = 0;
	ti->ti_len = htons((u_short) (sizeof(struct tcphdr) + tlen));
	ti->ti_seq = htonl(seq);
	ti->ti_ack = htonl(ack);
	ti->ti_x2 = 0;
	ti->ti_off = sizeof(struct tcphdr) >> 2;
	ti->ti_flags = flags;
	ti->ti_win = htons((u_short) win);
	ti->ti_urp = 0;
	ti->ti_sum = 0;
	ti->ti_sum = in_cksum(bp, (int) (sizeof(struct tcpiphdr) + tlen));
	((struct ip *) ti)->ip_len = sizeof(struct tcpiphdr) + tlen;
	((struct ip *) ti)->ip_ttl = tcp_ttl;
	bp0 = allocb(sizeof(struct ip_unitdata_req), BPRI_HI);
	if (bp0 == NULL) {
		freeb(bp);
		return;
	}
	bp0->b_cont = bp;
	bp = bp0;
	bp->b_datap->db_type = M_PROTO;
	bp->b_wptr += sizeof(struct ip_unitdata_req);
	ipreq = (struct ip_unitdata_req *) bp->b_rptr;
	ipreq->dl_primitive = N_UNITDATA_REQ;
	ipreq->options = 0;
	ipreq->route = *ro;
	ipreq->flags = 0;
	ipreq->dl_dest_addr_length = sizeof(ti->ti_dst);
	ipreq->dl_dest_addr_offset = sizeof(struct ip_unitdata_req)
		- sizeof(struct in_addr);
	ipreq->ip_addr = ti->ti_dst;
	if (tcp_qbot) {
		if (ro->ro_rt)
			RT(ro->ro_rt)->rt_refcnt++;
		putnext(tcp_qbot, bp);
	} else {
		freemsg(bp);
	}
}

/*
 * Create a new TCP control block, making an empty reassembly queue and
 * hooking it to the argument protocol control block. 
 */
struct tcpcb   *
tcp_newtcpcb(inp)
	struct inpcb   *inp;
{
	register struct tcpcb *tp;

	tp = (struct tcpcb *) kmem_alloc(sizeof(struct tcpcb), KM_NOSLEEP);
	if (tp == NULL) {
		STRLOG(TCPM_ID, 1, 2, SL_TRACE, "newtcpcb no mem inp %x",
		       inp);
		return ((struct tcpcb *) 0);
	}
	bzero((char *) tp, sizeof(struct tcpcb));
	tp->seg_next = (struct tcpiphdr *) tp;
	tp->t_q = tp->t_q0 = (struct tcpcb *) NULL;
	tp->t_maxseg = IP_MAXPACKET;	/* large number */
	tp->t_flags = 0;	/* sends options! */
	tp->t_inpcb = inp;
	/*
	 * Init srtt to TCPTV_SRTTBASE (0), so we can tell that we have no
	 * rtt estimate.  Set rttvar so that srtt + 2 * rttvar gives
	 * reasonable initial retransmit time.
	 */
	tp->t_srtt = TCPTV_SRTTBASE;
	tp->t_rttvar = TCPTV_SRTTDFLT << 2;
	TCPT_RANGESET(tp->t_rxtcur, 
	    ((TCPTV_SRTTBASE >> 2) + (TCPTV_SRTTDFLT << 2)) >> 1,
	    TCPTV_MIN, TCPTV_REXMTMAX);
	tp->t_linger = 0;
	tp->t_inq = NULL;
	tp->snd_cwnd = 65535;
	tp->snd_ssthresh = 65535;		/* XXX */
	tp->t_maxwin = (inp->inp_q ? inp->inp_q->q_hiwat : 4096);
	tp->t_iqurp = -1;			/* no urgent data present */
	inp->inp_ppcb = (caddr_t) tp;
	STRLOG(TCPM_ID, 1, 5, SL_TRACE, "newtcpcb tcb %x inp %x", tp, inp);
	return (tp);
}

/*
 * Drop a TCP connection, reporting the specified error.  If connection is
 * synchronized, then send a RST to peer. 
 */

struct tcpcb   *
tcp_drop(tp, errno)
	register struct tcpcb *tp;
	int             errno;
{

	STRLOG(TCPM_ID, 1, 4, SL_TRACE, "tcp_drop tcb %x inp %x err %d.",
	       tp, tp->t_inpcb, errno);

	if (TCPS_HAVERCVDSYN(tp->t_state)) {
		tcp_cancelinger(tp);
		tp->t_state = TCPS_CLOSED;
		tcp_io(tp, TF_NEEDOUT, NULL);
		tcpstat.tcps_drops++;
	} else
		tcpstat.tcps_conndrops++;
	tp->t_inpcb->inp_error = errno;
	return (tcp_close(tp, errno));
}

/*
 * Close a TCP control block: discard all space held by the tcp discard
 * internet protocol block, if not still referenced 
 */
struct tcpcb   *
tcp_close(tp, error)
	register struct tcpcb *tp;
	int             error;
{
	register struct tcpiphdr *t;
	struct inpcb   *inp = tp->t_inpcb;

	STRLOG(TCPM_ID, 1, 5, SL_TRACE, "tcp_close tcb %x err %d",
	       tp, error);

        for (t = tp->seg_next; t != (struct tcpiphdr *)tp; t = tp->seg_next) {
		dequenxt((struct vq *) tp);
		freemsg(t->ti_mblk);
	}
	inpisdisconnected(tp->t_inpcb, error);
	inp->inp_laddr.s_addr = INADDR_ANY;
	inp->inp_lport = 0;
	inp->inp_faddr.s_addr = INADDR_ANY;
	inp->inp_fport = 0;

	tcpstat.tcps_closed++;
	if (inp->inp_state & SS_NOFDREF) {
		tcp_freespc(tp);
		return ((struct tcpcb *) 0);
	}
	return (tp);
}

tcp_freespc(tp)
	struct tcpcb   *tp;
{
	struct inpcb   *inp = tp->t_inpcb;
	mblk_t         *bp, *bp0;
	int 		s;

	STRLOG(TCPM_ID, 1, 9, SL_TRACE, "tcp_freespc pcb %x", inp);
	s = splstr();
	tp->t_flags &= ~(TF_NEEDIN|TF_NEEDOUT);
	splx(s);
	if (tp->t_template)
		(void) freeb(tp->t_tmplhdr);

	bp = tp->t_qfirst;
	while (bp) {
		bp0 = bp->b_next;
		freemsg(bp);
		bp = bp0;
	}

	if (tp->t_head) {
		if (!tpqremque(tp, 0) && !tpqremque(tp, 1))
#ifdef SYSV
			cmn_err(CE_PANIC, "tcp_freespc remque");
#else
			panic( "tcp_freespc remque");
#endif
	}
	bp = tp->t_inq;
	while (bp) {
		bp0 = bp->b_next;
		freemsg(bp);
		bp = bp0;
	}
	inp->inp_ppcb = 0;
	(void) kmem_free(tp, sizeof(struct tcpcb));
	in_pcbdetach(inp);
}

tcp_drain()
{

}

tcp_ctlinput(bp)
	mblk_t         *bp;
{
	struct ip_ctlmsg *ctl;
	extern u_char   inetctlerrmap[];
	int             tcp_quench(), in_rtchange();
	struct sockaddr_in src,dst;
	int tcp_errdiscon();

	ctl = (struct ip_ctlmsg *) bp->b_rptr;
	if ((unsigned) ctl->command > PRC_NCMDS)
		return;
	if (ctl->src_addr.s_addr == INADDR_ANY)
		return;
	dst.sin_family = htons(AF_INET);
	dst.sin_addr.s_addr = ctl->src_addr.s_addr;
	dst.sin_port = ctl->src_port;
	src.sin_family = htons(AF_INET);
	src.sin_addr.s_addr = ctl->dst_addr.s_addr;
	src.sin_port = ctl->dst_port;

	switch (ctl->command) {

	case PRC_QUENCH:
		in_pcbnotify(&tcb, &dst, &src, 0, tcp_quench, 0);
		break;

	case PRC_ROUTEDEAD:
	case PRC_REDIRECT_NET:
	case PRC_REDIRECT_HOST:
	case PRC_REDIRECT_TOSNET:
	case PRC_REDIRECT_TOSHOST:
		in_pcbnotify(&tcb, &dst, &src, 0, in_rtchange, 0);
		break;

	default:
		if (inetctlerrmap[ctl->command] == 0)
			return;	/* XXX */
		in_pcbnotify(&tcb, &dst, &src,
			 (int) inetctlerrmap[ctl->command], tcp_errdiscon, 1);
	}
}

/*
 * When a source quench is received, close congestion window
 * to one segment.  We will gradually open it again as we proceed.
 */
tcp_quench(inp)
	struct inpcb *inp;
{
	struct tcpcb *tp = intotcpcb(inp);

	if (tp)
		tp->snd_cwnd = tp->t_maxseg;
}

/*
 * Save data that arrives before the user has accepted the connection (and
 * therefore given us a stream queue).  As well as data which arrives when
 * there is no room upstream.  
 */
tcp_enqdata(tp, bp, urp)
	struct tcpcb   *tp;
	mblk_t         *bp;
	int		urp;	/* pointer to urgent data; -1 if none */
{
	if (!bp)
		return;
	STRLOG(TCPM_ID, 2, 5, SL_TRACE, "tcp_enqdata q %x", tp->t_inpcb->inp_q);
	if (tp->t_qfirst) {
		tp->t_qlast->b_next = bp;
		tp->t_qlast = bp;
	} else {
		tp->t_qfirst = tp->t_qlast = bp;
	}

	if (urp != -1)
		tp->t_iqurp = tp->t_iqsize + urp;
	tp->t_iqsize += msgdsize(bp);
	bp->b_next = NULL;
	return;
}

void
tcp_calldeq(q)
	queue_t		*q;
{
	extern struct streamtab tcpinfo;
	struct tcpcb *tp;

	/*
	 * Make sure this is in fact a tcp queue (since we use bufcall,
	 * it is possible that this routine will be called after the tcp
	 * stream has been closed).  Also make sure this should be done
	 * for this connection, in case the above happens and the queue
	 * has been reused for a new TCP endpoint.
	 */
	if (q->q_qinfo == tcpinfo.st_rdinit
	    && (tp = qtotcpcb(q))
	    && (tp->t_iqsize || TCPS_HAVERCVDFIN(tp->t_state)))
		qenable(q);
}

tcp_deqdata(q)
	queue_t        *q;
{
	register struct tcpcb *tp;
	register mblk_t *bp, *bp2;
	extern mblk_t  *headerize();
	int             win;

	if (q->q_ptr == NULL) {
#ifdef SYSV
		cmn_err(CE_WARN, "tcp_deqdata: null q_ptr wq %x", WR(q));
#else
		printf( "tcp_deqdata: null q_ptr wq %x", WR(q));
#endif
		return;
	}

	tp = qtotcpcb(q);

	while ((bp = tp->t_qfirst) && canput(q->q_next)) {

		if ((tp->t_iqurp >= 0) && (tp->t_iqurp < msgdsize(bp))) {
			if (tcp_passoobup(tp, bp, q, tp->t_iqurp)) {
				tp->t_iqsize -= msgdsize(bp);
				tp->t_iqurp = -1;
			} else {
				timeout(tcp_calldeq,q,HZ);
				break;
			}
		} else {
			if (bp2 = headerize(bp)) {
				STRLOG(TCPM_ID, 2, 5, SL_TRACE, "tcp_deq up q %x", q);
				tp->t_qfirst = bp->b_next;
				bp->b_next = NULL;
				if (tp->t_qfirst == NULL) {
					tp->t_qlast = NULL;
				}
				tp->t_iqsize -= msgdsize(bp);
				if (tp->t_iqurp >= 0)
					tp->t_iqurp -= msgdsize(bp);
				putnext(q, bp2);
			}
			else {
				timeout(tcp_calldeq,q,HZ);
				break;
			}
		}
	}

	/*
	 * If we have received a FIN and queue is empty, send orderly
	 * release indication.  Note: if the T_ORDREL_IND has already
	 * been sent, inpordrelind will not send another one.
	 */
	if (TCPS_HAVERCVDFIN(tp->t_state) && !tp->t_qfirst) {
		if (!inpordrelind((struct inpcb *) q->q_ptr)) {
			if (!bufcall(sizeof(struct T_ordrel_ind),BPRI_HI,tcp_calldeq,q))
				timeout(tcp_calldeq,q,HZ);
		}
	}

	/*
	 * If window just opened up, call tcp_output to send window update. 
	 * If we've already received a FIN, no need to do this.
	 */
	if (!TCPS_HAVERCVDFIN(tp->t_state)) {
		int	hiwat;

		hiwat = (q ? q->q_hiwat : 4096);
		win = hiwat - tp->t_iqsize;
		if (win > 0) {
			int adv = win - (tp->rcv_adv - tp->rcv_nxt);

			if ((tp->t_iqsize == 0 && adv >= 2 * tp->t_maxseg)
			    || (100 * adv / hiwat >= 35))
				tcp_io(tp, TF_NEEDOUT, 0);
		}
	}
}

struct tcpcb *tcp_output(), *tcp_uinput(), *tcp_dotimers();

tcp_io(tp, flag, bp)
	register struct tcpcb *tp;
	int flag;
	mblk_t *bp;
{
	int s;
	int oflag, tflag;
	struct tcpcb *(*func)();
	mblk_t **bpp;

	s = splstr();
	if (flag == TF_NEEDIN) {
		for (bpp = &tp->t_inq; *bpp; bpp = &(*bpp)->b_next)
			;
		*bpp = bp;
		bp->b_next = NULL;
	}
	tp->t_flags |= flag;
	if (!(tp->t_flags & TF_IOLOCK)) {
                tp->t_flags |= TF_IOLOCK;
                for (;;) {
			/* TF_NEED* flags cleared by service routines */
			if (tp->t_flags & TF_NEEDTIMER)
				func = tcp_dotimers;
			else if (tp->t_flags & TF_NEEDOUT)
				func = tcp_output;
			else if (tp->t_flags & TF_NEEDIN)
				func = tcp_uinput;
			else {
				tp->t_flags &= ~TF_IOLOCK;
				break;
			}
			splx(s);
			if (!(*func)(tp))
				return;
			s = splstr();
		}
	}
	splx(s);
}
