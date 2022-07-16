/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:tcp_input.c	1.3"

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
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <net/if.h>
#include <sys/errno.h>
#ifdef SYSV
#include <sys/tihdr.h>
#else
#include <nettli/tihdr.h>
#endif SYSV

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <net/route.h>
#include <netinet/in_pcb.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_debug.h>
#include <netinet/insrem.h>
#include <netinet/ip_str.h>
#ifdef SYSV
#ifdef SYSV
#include <sys/cmn_err.h>
#endif
#endif SYSV

#include <sys/signal.h>

int		tcpdprintf = 0;
int             tcpprintfs = 0;
int             tcpcksum = 1;
int		tcprexmtthresh = 3;
struct tcpiphdr tcp_saveti;
struct tcpstat  tcpstat;
extern          tcpnodelack;
extern int      tcpalldebug;
struct tcpcb   *tcp_newtcpcb();
struct inpcb   *in_pcballoc();
extern void	tcp_calldeq();

/*
 * Insert segment ti into reassembly queue of tcp with control block tp.
 * Return TH_FIN if reassembly now includes a segment with FIN.  The macro
 * form does the common case inline (segment is the next to be received on an
 * established connection, and the queue is empty), avoiding linkage into and
 * removal from the queue and repetition of various conversions. 
 */
#define	TCP_REASS(tp, ti, bp, q, flags) { \
	mblk_t *nbp; \
		     \
	if ((ti)->ti_seq == (tp)->rcv_nxt && \
	    (tp)->seg_next == (struct tcpiphdr *)(tp) && \
	    (tp)->t_state == TCPS_ESTABLISHED && \
	    !((flags) & TH_URG)) { \
		(tp)->rcv_nxt += (ti)->ti_len; \
		(flags) = (ti)->ti_flags & TH_FIN; \
		tcpstat.tcps_rcvpack++; \
                tcpstat.tcps_rcvbyte += (ti)->ti_len; \
		while (bp && !msgblen(bp)) { \
			nbp = bp->b_cont; \
			freeb(bp); \
			bp = nbp; \
		} \
		if (bp) { \
			if ((tp->t_iqsize == 0) && (q) && canput(q->q_next)) { \
				if (nbp = headerize(bp)) { \
					STRLOG(TCPM_ID, 2, 5, SL_TRACE, "tcp sending %d, seq %d, up wq %x", (ti)->ti_len, tp->rcv_nxt%10000, WR(q)); \
					putnext(q, nbp); \
				} else { \
					tcp_enqdata(tp, bp, -1); \
					timeout(tcp_calldeq, q, HZ); \
				} \
			} else { \
				tcp_enqdata(tp, bp, -1); \
			} \
		} \
	} else \
		(flags) = tcp_reass(q, (tp), (ti), (bp)); \
}

mblk_t         *headerize();

tcp_reass(sq, tp, ti, bp)
	queue_t        *sq;
	register struct tcpcb *tp;
	register struct tcpiphdr *ti;
	mblk_t         *bp;
{
	register struct tcpiphdr *q, *qprev;
	mblk_t         *nbp;
	struct inpcb   *inp = tp->t_inpcb;
	int             flags;
	struct tcpiphdr *nti;

	/*
	 * Call with ti==0 after become established to force pre-ESTABLISHED
	 * data up to user socket. 
	 */
	if (bp == 0)
		goto present;


	/*
	 * Find a segment which begins after this one does. 
	 */
	for (qprev = (struct tcpiphdr *)tp, q = tp->seg_next; 
	     q != (struct tcpiphdr *) tp;
	     qprev = q, q = (struct tcpiphdr *) q->ti_next)
		if (SEQ_GT(q->ti_seq, ti->ti_seq))
			break;

	/*
	 * If there is a preceding segment, it may provide some of our data
	 * already.  If so, drop the data from the incoming segment.  If it
	 * provides all of our data, drop us. 
	 */
	if (qprev != (struct tcpiphdr *) tp) {
		register int    i;

		/* conversion to int (in i) handles seq wraparound */
		i = qprev->ti_seq + qprev->ti_len - ti->ti_seq;
		if (i > 0) {
			if (i >= ti->ti_len) {
				tcpstat.tcps_rcvduppack++;
                                tcpstat.tcps_rcvdupbyte += ti->ti_len;
                                goto drop;
                        }
			adjmsg(bp, i);
			ti->ti_len -= i;
			ti->ti_seq += i;
			if (ti->ti_flags & TH_URG) {
				if (ti->ti_urp >= i)
					ti->ti_urp -= i;
				else 
					ti->ti_flags &= ~TH_URG;
			}
		}
	}
        tcpstat.tcps_rcvoopack++;
        tcpstat.tcps_rcvoobyte += ti->ti_len;

	/*
	 * While we overlap succeeding segments trim them or, if they are
	 * completely covered, dequeue them. 
	 */
	while (q != (struct tcpiphdr *) tp) {
		register int    i = (ti->ti_seq + ti->ti_len) - q->ti_seq;

		if (i <= 0)
			break;
		if (i < q->ti_len) {
			q->ti_seq += i;
			q->ti_len -= i;
			adjmsg(q->ti_mblk, i);
			if (q->ti_flags & TH_URG) {
				if (q->ti_urp >= i)
					q->ti_urp -= i;
				else 
					q->ti_flags &= ~TH_URG;
			}
			break;
		}
		dequenxt((struct vq *) qprev);
		freemsg(q->ti_mblk);
		q = (struct tcpiphdr *) qprev->ti_next;
	}

	/*
	 * Stick new segment in its place. 
	 */
	enque((struct vq *) ti, (struct vq *) qprev);

	if (ti->ti_seq != tp->rcv_nxt) {
		STRLOG(TCPM_ID, 2, 4, SL_TRACE, "tcp_reass skip got %d expect %d",
		       ti->ti_seq % 10000, tp->rcv_nxt % 10000);
	}
present:
	/*
	 * Present data to user, advancing rcv_nxt through completed sequence
	 * space. 
	 */
	if (TCPS_HAVERCVDSYN(tp->t_state) == 0)
		return (0);
	ti = tp->seg_next;
	if (ti == (struct tcpiphdr *) tp || ti->ti_seq != tp->rcv_nxt)
		return (0);
	if (tp->t_state == TCPS_SYN_RECEIVED && ti->ti_len)
		return (0);
	do {
		tp->rcv_nxt += ti->ti_len;
		flags = ti->ti_flags & TH_FIN;
		bp = ti->ti_mblk;
		dequenxt((struct vq *) tp);
		nti = (struct tcpiphdr *) ti->ti_next;
		if (inp->inp_state & SS_CANTRCVMORE)
			freemsg(bp);
		else {
			while (bp && !msgblen(bp)) {
				nbp = bp->b_cont;
				freeb(bp);
				bp = nbp;
			}
			if (bp)
				sendup (tp, bp, ti, sq);
		}
		ti = nti;
	} while (ti != (struct tcpiphdr *) tp && ti->ti_seq == tp->rcv_nxt);
	return (flags);
 drop:
	freemsg(bp);
	return (0);
}


sendup (tp, bp, ti, sq)
	struct tcpcb *tp;
	mblk_t *bp;
	struct tcpiphdr *ti;
	queue_t *sq;
{
	mblk_t *nbp;
	int otlen = ti->ti_len;

	if ((tp->t_iqsize == 0) && sq && canput(sq->q_next)) {
		if ((ti->ti_flags & TH_URG) && 
		    (ti->ti_urp < ti->ti_len)) {
			if (!tcp_passoobup(tp, bp, sq, ti->ti_urp)) 
				tcp_enqdata(tp, bp, ti->ti_urp);
		} else {
			if (nbp = headerize(bp)) {
				STRLOG(TCPM_ID, 2, 5, SL_TRACE,
				       "tcp sending %d, seq %d, up wq %x",
				       otlen, tp->rcv_nxt % 10000, WR(sq));
				putnext(sq, nbp);
			} else {
				tcp_enqdata(tp, bp, -1);
				timeout(tcp_calldeq, sq, HZ);
			}
		}
	} else {
		tcp_enqdata(tp, bp, -1);
	}
}


/*
 * takes one mblk chain with urgent data in it and splits it up into
 * up to three mblk chains and passes these up.  The first is an M_DATA
 * message with data before the urgent mark.  The second is a T_EXDATA_IND
 * containing the urgent byte.  The third is an M_DATA message with the
 * data after the urgent mark.
 */
tcp_passoobup(tp, bp0, q, urp)
struct tcpcb *tp;
mblk_t *bp0;
queue_t *q;
int urp;
{
	/*
	 * bp0 points to the original mblk chain.  bp1 points
	 * to the chain of data before the urgent mark.  bp2 points to the
	 * chain with the urgent byte.  bp3 points to the chain of data
	 * after the urgent mark.  nbp is a temporary.   urp is the offset
	 * to the urgent byte that we are going to extricate.
	 */
	mblk_t *bp1, *bp2, *bp3, *nbp;
	char oobyte;
	struct T_exdata_ind *ind;

	if ((bp1 = dupmsg(bp0)) == (mblk_t *) NULL)
		goto fail;

	/* find mblk with the urgent byte in it */
	for (nbp = bp1 ; urp >= msgblen (nbp); nbp = nbp->b_cont)
		urp -= msgblen (nbp);

	oobyte = *((char *) (nbp->b_rptr + urp));

	if (msgblen(nbp) > urp + 1) {
		/* need to save rest of data for third message */
		bp3 = dupmsg (nbp);	/* can this fail? */
		bp3->b_rptr += (urp + 1);
	} else
		bp3 = nbp->b_cont;

	/* shave off oob byte and data past it */
	nbp->b_wptr -= (msgblen(nbp) - urp);
	nbp->b_cont = NULL;

	/* dump any zero-length mblks */
	while (bp1 && (msgblen (bp1) == 0)) {
		nbp = bp1->b_cont;
		freeb (bp1);
		bp1 = nbp;
	}
	if (bp1) {
		/* finish up first mblk containing data before urgent byte */
		if ((nbp = headerize (bp1)) == (mblk_t *) NULL) {
			freemsg (bp1);
			if (bp3)
				freemsg (bp3);
			goto fail;
		}
		bp1 = nbp;	/* first mblk is done */
	}
	
	if ((bp2 = allocb(sizeof (struct T_exdata_ind), BPRI_HI)) == 
	    (mblk_t *) NULL) {
		if (bp1)
			freemsg (bp1);
		if (bp3)
			freemsg (bp3);
		goto fail;
	}
	bp2->b_datap->db_type = M_PROTO;
	ind = (struct T_exdata_ind *) bp2->b_rptr;
	bp2->b_wptr += sizeof (struct T_exdata_ind);
	ind->PRIM_type = T_EXDATA_IND;
	ind->MORE_flag = 0;
	if ((bp2->b_cont = allocb(1, BPRI_HI)) == (mblk_t *) NULL) {
		if (bp1)
			freemsg(bp1);
		freemsg(bp2);
		if (bp3)
			freemsg(bp3);
		goto fail;
	}
	bp2->b_cont->b_datap->db_type = M_DATA;
	bp2->b_cont->b_wptr += 1;
	*(bp2->b_cont->b_rptr) = oobyte;

	while (bp3 && (msgblen (bp3) == 0)) {
		nbp = bp3->b_cont;
		freeb (bp3);
		bp3 = nbp;
	}
	if (bp3) {
		if ((nbp = headerize (bp3)) == (mblk_t *) NULL) {
			if (bp1)
				freemsg(bp1);
			freemsg(bp2);
			freemsg(bp3);
			goto fail;
		}
		bp3 = nbp;
	}

	/* pass all three up */
	if (bp1) {
		putnext(q, bp1);
	}
	putnext (q, bp2);
	if (bp3) {
		putnext(q, bp3);
	}
	/* success */
	return (1);

 fail:
	return (0);
}



mblk_t         *
headerize(bp)
	register mblk_t *bp;
{
	register mblk_t *bp0;
	extern mblk_t *tcp_dihdr;

	if (!bp)
		return (NULL);
	if (bp0 = dupb(tcp_dihdr))
		bp0->b_cont = bp;
	else
#ifdef SYSV
		cmn_err(CE_WARN, "headerize: dupb failed");
#else
		printf( "headerize: dupb failed");
#endif
	return (bp0);
}


/*
 * TCP input routine, follows pages 65-76 of the protocol specification dated
 * September, 1981 very closely. 
 */
tcp_linput(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	register struct tcpiphdr *ti;
	struct inpcb   *inp;
	int             len, tlen;
	register struct tcpcb *tp = 0;
	register int    tiflags;

	tcpstat.tcps_rcvtotal++;
	/*
	 * Get IP and TCP header together in first mblk. Note: IP leaves IP
	 * header in first mblk. Also leave room for back pointer to mblk
	 * structure 
	 */
	ti = (struct tcpiphdr *) bp->b_rptr;
	if (((struct ip *) ti)->ip_hl > (sizeof(struct ip) >> 2))
		ip_stripoptions(bp, (mblk_t *) 0);
	if (msgblen(bp) < sizeof(struct tcpiphdr)) {
		if ((pullupmsg(bp, sizeof(struct tcpiphdr))) == 0) {
			if (tcpdprintf)
#ifdef SYSV
				cmn_err(CE_NOTE, "tcp_linput: too short\n");
#else
				printf( "tcp_linput: too short\n");
#endif
			tcpstat.tcps_rcvshort++;
			freemsg(bp);
			return;
		}
		ti = (struct tcpiphdr *) bp->b_rptr;
	}
	bp->b_datap->db_type = M_DATA;	/* we only send data up */

	/*
	 * Checksum extended TCP header and data. 
	 */
	tlen = ((struct ip *) ti)->ip_len;
	len = sizeof(struct ip) + tlen;
	ti->ti_len = (u_short) tlen;
	ti->ti_len = htons((u_short) ti->ti_len);
	if (tcpcksum) {
		ti->ti_next = 0;
		ti->ti_mblk = 0;
		ti->ti_x1 = 0;
		if (ti->ti_sum = in_cksum(bp, len)) {
			if (tcpprintfs)
#ifdef SYSV
				cmn_err(CE_NOTE,"tcp sum: src %lx, sum %x", ntohl(ti->ti_src), ti->ti_sum);
#else
				printf("tcp sum: src %lx, sum %x", ntohl(ti->ti_src), ti->ti_sum);
#endif
			tcpstat.tcps_rcvbadsum++;
			goto drop;
		}
	}
	ti->ti_mblk = bp;
	/*
	 * Convert TCP protocol specific fields to host format. 
	 */
	ti->ti_seq = ntohl(ti->ti_seq);
	ti->ti_ack = ntohl(ti->ti_ack);
	ti->ti_win = ntohs(ti->ti_win);
	ti->ti_urp = ntohs(ti->ti_urp);

	/*
	 * Locate pcb for segment. 
	 */
	inp = in_pcblookup
		(&tcb, ti->ti_src, ti->ti_sport, ti->ti_dst, ti->ti_dport,
		 INPLOOKUP_WILDCARD);

	/*
	 * If the state is CLOSED (i.e., TCB does not exist) then all data in
	 * the incoming segment is discarded. 
         * If the TCB exists but is in CLOSED state, it is embryonic,
         * but should either do a listen or a connect soon.
	 */
	if (inp == 0)
		goto dropwithreset;
	tp = intotcpcb(inp);
	if (tp == 0) {
		STRLOG(TCPM_ID, 3, 3, SL_TRACE,
		       "tcp_input: inp %x but no tp", inp);
		goto dropwithreset;
	}
        if (tp->t_state == TCPS_CLOSED) {
		if (tcpprintfs)
#ifdef SYSV
			cmn_err(CE_NOTE, "tcp_linput: state == CLOSED\n");
#else
			printf( "tcp_linput: state == CLOSED\n");
#endif
                goto drop;
	}
	tcp_io(tp, TF_NEEDIN, bp);
	return;

dropwithreset:
	/*
	 * Generate a RST, dropping incoming segment. Make ACK acceptable to
	 * originator of segment. Don't bother to respond if destination was
	 * broadcast. 
	 */
	if (tcpdprintf)
#ifdef SYSV
		cmn_err(CE_NOTE, "tcp_linput: drop with reset\n");
#else
		printf( "tcp_linput: drop with reset\n");
#endif
        tiflags = ti->ti_flags;
	if ((tiflags & TH_RST) || in_broadcast(ti->ti_dst))
		goto drop;
	if (tiflags & TH_ACK)
		tcp_respond(bp, tp, ti, (tcp_seq) 0, ti->ti_ack, TH_RST);
	else {
		/* adjust ti_len to be length of data */
		ti->ti_len = ntohs(ti->ti_len) - ti->ti_off * 4;
		if (tiflags & TH_SYN)
			ti->ti_len++;
		tcp_respond(bp, tp, ti, ti->ti_seq + ti->ti_len, (tcp_seq) 0,
			    TH_RST | TH_ACK);
	}
	return;

drop:
	if (tcpdprintf)
#ifdef SYSV
		cmn_err(CE_NOTE, "tcp_linput: drop\n");
#else
		printf( "tcp_linput: drop\n");
#endif
	freemsg(bp);
	return;
}

struct tcpcb *
tcp_uinput(tp0)
	struct tcpcb *tp0;
{
	register struct tcpcb *tp = NULL;
	register struct tcpiphdr *ti;
	struct inpcb    *inp;
	int		len;
	int             off;
	register mblk_t *bp;
	mblk_t          *optbp;
	register int    tiflags;
	int             todrop, acked, needoutput, ourfinisacked;
	short           ostate;
	struct in_addr  laddr;
	int             dropsocket;
	int		iss = 0;
	queue_t        *usq;	/* upstream q */
	int		s;

	if (tcpdprintf)
#ifdef SYSV
		cmn_err(CE_NOTE, "tcp_uinput\n");
#else
		printf( "tcp_uinput\n");
#endif
moreinput:
	tp = tp0 ? tp0 : tp;
	tp0 = NULL;

	s = splstr();
	if (!tp || !(bp = tp->t_inq)) {
		if (tp)
			tp->t_flags &= ~TF_NEEDIN;
		splx(s);
		return(tp);
	}
	tp->t_inq = bp->b_next;
	inp = tp->t_inpcb;
	splx(s);

	optbp = (mblk_t *)NULL;
	needoutput = 0;
	dropsocket = 0;
	ti = (struct tcpiphdr *)bp->b_rptr;
	ti->ti_len = ntohs((u_short)ti->ti_len);
        /*
         * Check that TCP offset makes sense, pull out TCP options and adjust
         * length.
         */
	off = ti->ti_off << 2;
	if (off < sizeof(struct tcphdr) || off > ti->ti_len) {
		if (tcpprintfs)
#ifdef SYSV
			cmn_err(CE_NOTE,"tcp off: src %lx off %d", ntohl(ti->ti_src), off);
#else
			printf("tcp off: src %lx off %d", ntohl(ti->ti_src), off);
#endif
		tcpstat.tcps_rcvbadoff++;
		goto drop;
	}
	ti->ti_len -= off;
	if (off > sizeof(struct tcphdr)) {
		if (msgblen(bp) < sizeof(struct ip) + off) {
			if (pullupmsg(bp, (int) sizeof(struct ip) + off)
			    == 0) {
				if (tcpdprintf)
#ifdef SYSV
					cmn_err(CE_NOTE, "tcp_uinput: too short\n");
#else
					printf( "tcp_uinput: too short\n");
#endif
				tcpstat.tcps_rcvshort++;
				goto drop;
			}
			ti = (struct tcpiphdr *) bp->b_rptr;
		}
		optbp = allocb((int) (off - sizeof(struct tcphdr)), BPRI_HI);
		if (optbp == 0) {
			if (tcpdprintf)
#ifdef SYSV
				cmn_err(CE_NOTE, "tcp_uinput: can't allocb opt buf");
#else
				printf( "tcp_uinput: can't allocb opt buf");
#endif
			goto drop;
		}
		optbp->b_wptr += off - sizeof(struct tcphdr);
		{
			caddr_t         op = (caddr_t) bp->b_rptr + sizeof(struct tcpiphdr);
			bcopy(op, (char *) optbp->b_rptr,
			      (unsigned) (off - sizeof(struct tcphdr)));
			bp->b_wptr -= off - sizeof(struct tcphdr);
			bcopy(op + (off - sizeof(struct tcphdr)), op,
			(unsigned) (msgblen(bp) - sizeof(struct tcpiphdr)));
		}
	}
	tiflags = ti->ti_flags;

        /*
         * Drop TCP and IP headers; TCP options were dropped above.
         */
        bp->b_rptr += sizeof(struct tcpiphdr);
	if (tp->t_state == TCPS_CLOSED) {
		STRLOG(TCPM_ID, 3, 1, SL_TRACE,
		       "tcp_input: CLOSED: inp %x tp %x", inp, tp);
		goto dropwithreset;
	}
	if ((inp->inp_protoopt & SO_DEBUG) || tcpalldebug != 0) {
		ostate = tp->t_state;
		tcp_saveti = *ti;
	}
	if (inp->inp_protoopt & SO_ACCEPTCONN) {
		inp = inpnewconn(inp);
		if (inp == 0)
			goto drop;
		/*
		 * This is ugly, but .... 
		 *
		 * Mark pcb as temporary until we're committed to keeping it.
		 * The code at ``drop'' and ``dropwithreset'' check the flag
		 * dropsocket to see if the temporary PCB created here should
		 * be discarded. We mark the PCB as discardable until we're
		 * committed to it below in TCPS_LISTEN. 
		 */
		dropsocket++;
		inp->inp_laddr = ti->ti_dst;
		inp->inp_lport = ti->ti_dport;
		inp->inp_options = ip_srcroute(0);
		tp0 = tp;
		tp = intotcpcb(inp);
		tp->t_state = TCPS_LISTEN;
	}
	/*
	 * Segment received on connection. Reset idle time and keep-alive
	 * timer. 
	 */
	tp->t_idle = 0;
	tp->t_timer[TCPT_KEEP] = tcp_keepidle;

	/*
	 * Process options if not in LISTEN state, else do it below (after
	 * getting remote address). 
	 */
	if (optbp && tp->t_state != TCPS_LISTEN) {
		tcp_dooptions(tp, optbp, ti);
		optbp = 0;
	}
	usq = inp->inp_q;	/* find the way upstream */

	/*
	 * Calculate amount of space in receive window, and then do TCP input
	 * processing. Receive window is amount of space in rcv queue, but
	 * not less than advertised window. 
	 */
	{
		int             hiwat, win;

		hiwat = (usq ? usq->q_hiwat : 4096);
		win = hiwat - tp->t_iqsize;
		if (win < 0)
			win = 0;
		if (win > hiwat) {
			win = hiwat;
		}
		tp->rcv_wnd = MAX(win, (int) (tp->rcv_adv - tp->rcv_nxt));
	}

	switch (tp->t_state) {

		/*
		 * If the state is LISTEN then ignore segment if it contains
		 * an RST. If the segment contains an ACK then it is bad and
		 * send a RST. If it does not contain a SYN then it is not
		 * interesting; drop it. Don't bother responding if the
		 * destination was a broadcast. Otherwise initialize
		 * tp->rcv_nxt, and tp->irs, select an initial tp->iss, and
		 * send a segment: <SEQ=ISS><ACK=RCV_NXT><CTL=SYN,ACK> Also
		 * initialize tp->snd_nxt to tp->iss+1 and tp->snd_una to
		 * tp->iss. Fill in remote peer address fields if not
		 * previously specified. Enter SYN_RECEIVED state, and
		 * process any other fields of this segment in this state. 
		 */
	case TCPS_LISTEN:{
			mblk_t         *am;
			register struct sockaddr_in *sin;

			if (tiflags & TH_RST)
				goto drop;
			if (tiflags & TH_ACK)
				goto dropwithreset;
			if ((tiflags & TH_SYN) == 0)
				goto drop;
			if (in_broadcast(ti->ti_dst))
				goto drop;
			am = allocb(sizeof(struct sockaddr_in), BPRI_HI);
			if (am == NULL)
				goto drop;
			am->b_wptr += sizeof(struct sockaddr_in);
			sin = (struct sockaddr_in *) am->b_rptr;
			sin->sin_family = inp->inp_family;
			sin->sin_addr = ti->ti_src;
			sin->sin_port = ti->ti_sport;
			laddr = inp->inp_laddr;
			if (inp->inp_laddr.s_addr == INADDR_ANY)
				inp->inp_laddr = ti->ti_dst;
			if (in_pcbconnect(inp, am)) {
				inp->inp_laddr = laddr;
				(void) freemsg(am);
				goto drop;
			}
			(void) freemsg(am);
			tp->t_template = tcp_template(tp);
			if (tp->t_template == 0) {
				tp = tcp_drop(tp, ENOBUFS);
				dropsocket = 0;	/* socket is already gone */
				goto drop;
			}
			if (optbp) {
				tcp_dooptions(tp, optbp, ti);
				optbp = 0;
			}
			if (iss)
				tp->iss = iss;
			else
				tp->iss = tcp_iss;
			tcp_iss += TCP_ISSINCR / 2;
			tp->irs = ti->ti_seq;
			tcp_sendseqinit(tp);
			tcp_rcvseqinit(tp);
			tp->t_flags |= TF_ACKNOW;
			tp->t_state = TCPS_SYN_RECEIVED;
			tp->t_timer[TCPT_KEEP] = TCPTV_KEEP_INIT;
			dropsocket = 0;	/* committed to socket */
			tcpstat.tcps_accepts++;
			goto trimthenstep6;
		}

		/*
		 * If the state is SYN_SENT: if seg contains an ACK, but not
		 * for our SYN, drop the input. if seg contains a RST, then
		 * drop the connection. if seg does not contain SYN, then
		 * drop it. Otherwise this is an acceptable SYN segment
		 * initialize tp->rcv_nxt and tp->irs if seg contains ack
		 * then advance tp->snd_una if SYN has been acked change to
		 * ESTABLISHED else SYN_RCVD state arrange for segment to be
		 * acked (eventually) continue processing rest of
		 * data/controls, beginning with URG 
		 */
	case TCPS_SYN_SENT:
		if ((tiflags & TH_ACK) &&
		    (SEQ_LEQ(ti->ti_ack, tp->iss) ||
		     SEQ_GT(ti->ti_ack, tp->snd_max)))
			goto dropwithreset;
		if (tiflags & TH_RST) {
			if (tiflags & TH_ACK)
				tp = tcp_drop(tp, ECONNREFUSED);
			goto drop;
		}
		if ((tiflags & TH_SYN) == 0)
			goto drop;
		if (tiflags & TH_ACK) {
			tp->snd_una = ti->ti_ack;
			STRLOG(TCPM_ID, 2, 9, SL_TRACE,
			       "snd_una %x", tp->snd_una);
			if (SEQ_LT(tp->snd_nxt, tp->snd_una))
				tp->snd_nxt = tp->snd_una;
		}
		tp->t_timer[TCPT_REXMT] = 0;
		tp->irs = ti->ti_seq;
		tcp_rcvseqinit(tp);
		tp->t_flags |= TF_ACKNOW;
                if (tiflags & TH_ACK && SEQ_GT(tp->snd_una, tp->iss)) {
			tcpstat.tcps_connects++;
			inpisconnected(inp);
			tp->t_state = TCPS_ESTABLISHED;
			tp->t_maxseg = MIN(tp->t_maxseg, tcp_mss(tp));
			(void) tcp_reass(usq, tp, (struct tcpiphdr *) 0,
					 (mblk_t *) 0);
                        /*
                         * if we didn't have to retransmit the SYN,
                         * use its rtt as our initial srtt & rtt var.
                         */
                        if (tp->t_rtt) {
                                tp->t_srtt = tp->t_rtt << 3;
                                tp->t_rttvar = tp->t_rtt << 1;
                                TCPT_RANGESET(tp->t_rxtcur,
                                    ((tp->t_srtt >> 2) + tp->t_rttvar) >> 1,
                                    TCPTV_MIN, TCPTV_REXMTMAX);
                                tp->t_rtt = 0;
			}
		} else
			tp->t_state = TCPS_SYN_RECEIVED;

trimthenstep6:
		/*
		 * Advance ti->ti_seq to correspond to first data byte. If
		 * data, trim to stay within window, dropping FIN if
		 * necessary. 
		 */
		ti->ti_seq++;
		if (ti->ti_len > tp->rcv_wnd) {
			todrop = ti->ti_len - tp->rcv_wnd;
			adjmsg(bp, -todrop);
			ti->ti_len = tp->rcv_wnd;
			tiflags &= ~TH_FIN;
                        tcpstat.tcps_rcvpackafterwin++;
                        tcpstat.tcps_rcvbyteafterwin += todrop;
		}
		tp->snd_wl1 = ti->ti_seq - 1;
		tp->rcv_up = ti->ti_seq;
		goto step6;

	default:
		break;
	}

	/*
	 * States other than LISTEN or SYN_SENT.
	 * First check that at least some bytes of segment are within 
	 * receive window.  If segment begins before rcv_nxt,
	 * drop leading data (and SYN); if nothing left, just ack.
	 */
	todrop = tp->rcv_nxt - ti->ti_seq;
	if (todrop > 0) {
		if (tiflags & TH_SYN) {
			tiflags &= ~TH_SYN;
			ti->ti_seq++;
			if (ti->ti_urp > 1) 
				ti->ti_urp--;
			else
				tiflags &= ~TH_URG;
			todrop--;
		}
		if (todrop > ti->ti_len ||
		    todrop == ti->ti_len && (tiflags&TH_FIN) == 0) {
			tcpstat.tcps_rcvduppack++;
			tcpstat.tcps_rcvdupbyte += ti->ti_len;
			/*
			 * If segment is just one to the left of the window,
			 * check two special cases:
			 * 1. Don't toss RST in response to 4.2-style keepalive.
			 * 2. If the only thing to drop is a FIN, we can drop
			 *    it, but check the ACK or we will get into FIN
			 *    wars if our FINs crossed (both CLOSING).
			 * In either case, send ACK to resynchronize,
			 * but keep on processing for RST or ACK.
			 */
			if ((tiflags & TH_FIN && todrop == ti->ti_len + 1)
#ifdef TCP_COMPAT_42
			  || (tiflags & TH_RST && ti->ti_seq == tp->rcv_nxt - 1)
#endif
			   ) {
				todrop = ti->ti_len;
				tiflags &= ~TH_FIN;
				tp->t_flags |= TF_ACKNOW;
			} else
				goto dropafterack;
		} else {
			tcpstat.tcps_rcvpartduppack++;
			tcpstat.tcps_rcvpartdupbyte += todrop;
		}
		adjmsg(bp, todrop);
		ti->ti_seq += todrop;
		ti->ti_len -= todrop;
		if (ti->ti_urp > todrop)
			ti->ti_urp -= todrop;
		else {
			tiflags &= ~TH_URG;
			ti->ti_urp = 0;
		}
	}

	/*
	 * If new data are received on a connection after the
	 * user processes are gone, then RST the other end.
	 */
	if ((inp->inp_state & SS_NOFDREF) &&
	    tp->t_state > TCPS_CLOSE_WAIT && ti->ti_len) {
		tp = tcp_close(tp, 0);
		tcpstat.tcps_rcvafterclose++;
		goto dropwithreset;
	}

	/*
	 * If segment ends after window, drop trailing data (and PUSH
	 * and FIN); if nothing left, just ACK. 
	 */
	todrop = (ti->ti_seq + ti->ti_len) - (tp->rcv_nxt + tp->rcv_wnd);
	if (todrop > 0) {
		tcpstat.tcps_rcvpackafterwin++;
		if (todrop >= ti->ti_len) {
			tcpstat.tcps_rcvbyteafterwin += ti->ti_len;
			/*
			 * If window is closed can only take segments at
			 * window edge, and have to drop data and PUSH from
			 * incoming segments.  Continue processing, but
			 * remember to ack.  Otherwise, drop segment
			 * and ack.
			 */
			if (tp->rcv_wnd == 0 && ti->ti_seq == tp->rcv_nxt) {
				tp->t_flags |= TF_ACKNOW;
				tcpstat.tcps_rcvwinprobe++;
			} else
				goto dropafterack;
		} else
			tcpstat.tcps_rcvbyteafterwin += todrop;
		adjmsg(bp, -todrop);
		ti->ti_len -= todrop;
		tiflags &= ~(TH_PUSH | TH_FIN);
	}

	/*
	 * If the RST bit is set examine the state: SYN_RECEIVED STATE: If
	 * passive open, return to LISTEN state. If active open, inform user
	 * that connection was refused. ESTABLISHED, FIN_WAIT_1, FIN_WAIT2,
	 * CLOSE_WAIT STATES: Inform user that connection was reset, and
	 * close tcb. CLOSING, LAST_ACK, TIME_WAIT STATES Close the tcb. 
	 */
	if (tiflags & TH_RST) {
		int error = 0;

		switch (tp->t_state) {

		case TCPS_SYN_RECEIVED:
			error = ECONNREFUSED;
			goto close;

		case TCPS_ESTABLISHED:
		case TCPS_FIN_WAIT_1:
		case TCPS_FIN_WAIT_2:
		case TCPS_CLOSE_WAIT:
			STRLOG(TCPM_ID, 1, 7, SL_TRACE, "rcvd RST tcb %x pcb %x",
			       tp, tp->t_inpcb);
			error = ECONNRESET;
		close:
			tp->t_state = TCPS_CLOSED;
			tcpstat.tcps_drops++;
			tcp_cancelinger(tp);
			tp = tcp_close(tp, error);
			goto drop;

		case TCPS_CLOSING:
		case TCPS_LAST_ACK:
		case TCPS_TIME_WAIT:
			tp->t_state = TCPS_CLOSED;
			tcp_cancelinger(tp);
			tp = tcp_close(tp, 0);
			goto drop;

		default:
			break;
		}
	}

	/*
	 * If a SYN is in the window, then this is an error and we send an
	 * RST and drop the connection. 
	 */
	if (tiflags & TH_SYN) {
		tp = tcp_drop(tp, ECONNRESET);
		goto dropwithreset;
	}
	/*
	 * If the ACK bit is off we drop the segment and return. 
	 */
	if ((tiflags & TH_ACK) == 0)
		goto drop;

	/*
	 * Ack processing. 
	 */
	switch (tp->t_state) {

		/*
		 * In SYN_RECEIVED state if the ack ACKs our SYN then enter
		 * ESTABLISHED state and continue processing, otherwise send
		 * an RST. 
		 */
	case TCPS_SYN_RECEIVED:
		if ( ! (SEQ_GT(tp->snd_max, tp->iss)))  /* TLI WRES_CIND */
			goto drop;
		if (SEQ_GT(tp->snd_una, ti->ti_ack) ||
		    SEQ_GT(ti->ti_ack, tp->snd_max))
			goto dropwithreset;
		tcpstat.tcps_connects++;
		inpisconnected(inp);
		tp->t_state = TCPS_ESTABLISHED;
		tp->t_maxseg = MIN(tp->t_maxseg, tcp_mss(tp));
		(void) tcp_reass(usq, tp, (struct tcpiphdr *) 0, (mblk_t *) 0);
		tp->snd_wl1 = ti->ti_seq - 1;
		/* fall into ... */

		/*
		 * In ESTABLISHED state: drop duplicate ACKs; ACK out of
		 * range ACKs.  If the ack is in the range tp->snd_una <
		 * ti->ti_ack <= tp->snd_max then advance tp->snd_una to
		 * ti->ti_ack and drop data from the retransmission queue. If
		 * this ACK reflects more up to date window information we
		 * update our window information. 
		 */
	case TCPS_ESTABLISHED:
	case TCPS_FIN_WAIT_1:
	case TCPS_FIN_WAIT_2:
	case TCPS_CLOSE_WAIT:
	case TCPS_CLOSING:
	case TCPS_LAST_ACK:
	case TCPS_TIME_WAIT:

		if (SEQ_LEQ(ti->ti_ack, tp->snd_una)) {
			if (ti->ti_len == 0 && ti->ti_win == tp->snd_wnd) {
				tcpstat.tcps_rcvdupack++;
				/*
				 * If we have outstanding data (not a
				 * window probe), this is a completely
				 * duplicate ack (ie, window info didn't
				 * change), the ack is the biggest we've
				 * seen and we've seen exactly our rexmt
				 * threshhold of them, assume a packet
				 * has been dropped and retransmit it.
				 * Kludge snd_nxt & the congestion
				 * window so we send only this one
				 * packet.  If this packet fills the
				 * only hole in the receiver's seq.
				 * space, the next real ack will fully
				 * open our window.  This means we
				 * have to do the usual slow-start to
				 * not overwhelm an intermediate gateway
				 * with a burst of packets.  Leave
				 * here with the congestion window set
				 * to allow 2 packets on the next real
				 * ack and the exp-to-linear thresh
				 * set for half the current window
				 * size (since we know we're losing at
				 * the current window size).
				 */
				if (tp->t_timer[TCPT_REXMT] == 0 ||
				    ti->ti_ack != tp->snd_una)
					tp->t_dupacks = 0;
				else if (++tp->t_dupacks == tcprexmtthresh) {
					tcp_seq onxt = tp->snd_nxt;
					u_int win =
					    MIN(tp->snd_wnd, tp->snd_cwnd) / 2 /
						tp->t_maxseg;

					if (win < 2)
						win = 2;
					tp->snd_ssthresh = win * tp->t_maxseg;

					tp->t_timer[TCPT_REXMT] = 0;
					tp->t_rtt = 0;
					tp->snd_nxt = ti->ti_ack;
					tp->snd_cwnd = tp->t_maxseg;
					(void) tcp_output(tp);

					if (SEQ_GT(onxt, tp->snd_nxt))
						tp->snd_nxt = onxt;
					goto drop;
				}
			} else
				tp->t_dupacks = 0;
			break;
		}
		tp->t_dupacks = 0;
		if (SEQ_GT(ti->ti_ack, tp->snd_max)) {
			tcpstat.tcps_rcvacktoomuch++;
			goto dropafterack;
		}
		acked = ti->ti_ack - tp->snd_una;
		tcpstat.tcps_rcvackpack++;
		tcpstat.tcps_rcvackbyte += acked;

		/*
		 * If transmit timer is running and timed sequence
		 * number was acked, update smoothed round trip time.
		 * Since we now have an rtt measurement, cancel the
		 * timer backoff (cf., Phil Karn's retransmit alg.).
		 * Recompute the initial retransmit timer.
		 */
		if (tp->t_rtt && SEQ_GT(ti->ti_ack, tp->t_rtseq)) {
			tcpstat.tcps_rttupdated++;
			if (tp->t_srtt != 0) {
				register short delta;

				/*
				 * srtt is stored as fixed point with 3 bits
				 * after the binary point (i.e., scaled by 8).
				 * The following magic is equivalent
				 * to the smoothing algorithm in rfc793
				 * with an alpha of .875
				 * (srtt = rtt/8 + srtt*7/8 in fixed point).
				 * Adjust t_rtt to origin 0.
				 */
				delta = tp->t_rtt - 1 - (tp->t_srtt >> 3);
				if ((tp->t_srtt += delta) <= 0)
					tp->t_srtt = 1;
				/*
				 * We accumulate a smoothed rtt variance
				 * (actually, a smoothed mean difference),
				 * then set the retransmit timer to smoothed
				 * rtt + 2 times the smoothed variance.
				 * rttvar is stored as fixed point
				 * with 2 bits after the binary point
				 * (scaled by 4).  The following is equivalent
				 * to rfc793 smoothing with an alpha of .75
				 * (rttvar = rttvar*3/4 + |delta| / 4).
				 * This replaces rfc793's wired-in beta.
				 */
				if (delta < 0)
					delta = -delta;
				delta -= (tp->t_rttvar >> 2);
				if ((tp->t_rttvar += delta) <= 0)
					tp->t_rttvar = 1;
			} else {
				/* 
				 * No rtt measurement yet - use the
				 * unsmoothed rtt.  Set the variance
				 * to half the rtt (so our first
				 * retransmit happens at 2*rtt)
				 */
				tp->t_srtt = tp->t_rtt << 3;
				tp->t_rttvar = tp->t_rtt << 1;
			}
			tp->t_rtt = 0;
			tp->t_rxtshift = 0;
			TCPT_RANGESET(tp->t_rxtcur, 
			    ((tp->t_srtt >> 2) + tp->t_rttvar) >> 1,
			    TCPTV_MIN, TCPTV_REXMTMAX);
		}

		/*
		 * If all outstanding data is acked, stop retransmit
		 * timer and remember to restart (more output or persist).
		 * If there is more data to be acked, restart retransmit
		 * timer, using current (possibly backed-off) value.
		 */
		if (ti->ti_ack == tp->snd_max) {
			tp->t_timer[TCPT_REXMT] = 0;
			needoutput = 1;
		} else if (tp->t_timer[TCPT_PERSIST] == 0)
			tp->t_timer[TCPT_REXMT] = tp->t_rxtcur;
		/*
		 * When new data is acked, open the congestion window.
		 * If the window gives us less than ssthresh packets
		 * in flight, open exponentially (maxseg per packet).
		 * Otherwise open linearly (maxseg per window,
		 * or maxseg^2 / cwnd per packet).
		 */
		{
		u_int incr = tp->t_maxseg;
		register tmp;

		if (tp->snd_cwnd > tp->snd_ssthresh)
			incr = MAX(incr * incr / tp->snd_cwnd, 1);

		tmp = (int)tp->snd_cwnd + incr;
		tp->snd_cwnd = MIN(tmp, IP_MAXPACKET); /* XXX */
		}
		ourfinisacked = (tp->t_flags & TF_SENTFIN)
				&& (ti->ti_ack == tp->snd_max);
		if (acked > tp->t_qsize) {
			int             qsize = tp->t_qsize;

			tp->snd_wnd -= qsize;
			tp->t_qsize = 0;
			if (usq)
				tcp_qdrop(WR(usq), qsize);
			STRLOG(TCPM_ID, 2, 9, SL_TRACE,
			       "setting q size to 0");
		} else if (acked) {
			tp->snd_wnd -= acked;
			tp->t_qsize -= acked;
			if (usq)
				tcp_qdrop(WR(usq), acked);
			STRLOG(TCPM_ID, 2, 9, SL_TRACE,
			    "subtracting %d from q size, new value is %d",
			       acked, tp->t_qsize);
			acked = 0;
		}
		tp->snd_una = ti->ti_ack;
		STRLOG(TCPM_ID, 2, 9, SL_TRACE,
		       "snd_una %x", tp->snd_una);
		if (SEQ_LT(tp->snd_nxt, tp->snd_una))
			tp->snd_nxt = tp->snd_una;

		switch (tp->t_state) {

			/*
			 * In FIN_WAIT_1 STATE in addition to the processing
			 * for the ESTABLISHED state if our FIN is now
			 * acknowledged then enter FIN_WAIT_2. 
			 */
		case TCPS_FIN_WAIT_1:
			if (ourfinisacked) {
				/*
				 * If we can't receive any more data, then
				 * closing user can proceed. Starting the
				 * timer is contrary to the specification,
				 * but if we don't get a FIN we'll hang
				 * forever. 
				 */
				tcp_cancelinger(tp);
				if (inp->inp_state & SS_CANTRCVMORE) {
					STRLOG(TCPM_ID, 1, 7, SL_TRACE,
					"FINack, FW1, CANTRCV, inp %x", inp);
					tp->t_timer[TCPT_2MSL] = tcp_maxidle;
				}
				tp->t_state = TCPS_FIN_WAIT_2;
			}
			break;

			/*
			 * In CLOSING STATE in addition to the processing for
			 * the ESTABLISHED state if the ACK acknowledges our
			 * FIN then enter the TIME-WAIT state, otherwise
			 * ignore the segment. 
			 */
		case TCPS_CLOSING:
			if (ourfinisacked) {
				tp->t_state = TCPS_TIME_WAIT;
				tcp_canceltimers(tp);
				tcp_cancelinger(tp);
				tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			}
			break;

			/*
			 * The only thing that can arrive in  LAST_ACK state
			 * is an acknowledgment of our FIN.  If our FIN is
			 * now acknowledged, delete the TCB, enter the closed
			 * state and return. 
			 */
		case TCPS_LAST_ACK:
			if (ourfinisacked) {
				tcp_cancelinger(tp);
				tp->t_state = TCPS_CLOSED;
				tp = tcp_close(tp, 0);
				goto drop;
			}
			break;

			/*
			 * In TIME_WAIT state the only thing that should
			 * arrive is a retransmission of the remote FIN.
			 * Acknowledge it and restart the finack timer. 
			 */
		case TCPS_TIME_WAIT:
			tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			goto dropafterack;
		default:
			break;
		}
	default:
		break;
	}

step6:
	/*
	 * Update window information. Don't look at window if no ACK: TAC's
	 * send garbage on first SYN. 
	 */
	if ((tiflags & TH_ACK) &&
	    (SEQ_LT(tp->snd_wl1, ti->ti_seq) || tp->snd_wl1 == ti->ti_seq &&
	     (SEQ_LT(tp->snd_wl2, ti->ti_ack) ||
	      tp->snd_wl2 == ti->ti_ack && ti->ti_win > tp->snd_wnd))) {
		/* keep track of pure window updates */
		if (ti->ti_len == 0 &&
		    tp->snd_wl2 == ti->ti_ack && ti->ti_win > tp->snd_wnd)
			tcpstat.tcps_rcvwinupd++;
		tp->snd_wnd = ti->ti_win;
		tp->snd_wl1 = ti->ti_seq;
		tp->snd_wl2 = ti->ti_ack;
		if (tp->snd_wnd > tp->max_sndwnd)
			tp->max_sndwnd = tp->snd_wnd;
		needoutput = 1;
	}
	/*
	 * Process segments with URG.
	 */
	if ((tiflags & TH_URG) && ti->ti_urp &&
	    TCPS_HAVERCVDFIN(tp->t_state) == 0) {

		/*
		 * If this segment advances the known urgent pointer, then
		 * mark the data stream.  This should not happen in
		 * CLOSE_WAIT, CLOSING, LAST_ACK or TIME_WAIT STATES since a
		 * FIN has been received from the remote side. In these
		 * states we ignore the URG. 
		 *
		 * XXX - Current TCP interpretations say that the urgent
		 * pointer points to the last octet of urgent data.  For
		 * compatibility with previous releases, we continue to use 
		 * the obsolete interpretation where the urgent pointer 
		 * points to the first octet of data past the urgent section.  
		 * This decision means that we only recognize
		 * segments as urgent when the urgent pointer is > 0,
		 * which may cause problems interoperating with other systems.
		 */
		if (SEQ_GT(ti->ti_seq + ti->ti_urp, tp->rcv_up)) {
			tp->rcv_up = ti->ti_seq + ti->ti_urp;
			tp->t_oobflags &= ~(TCPOOB_HAVEDATA | TCPOOB_HADDATA);
		}

		ti->ti_urp--;		/* XXX - 4.2 BSD compatibility */
	} else
		/*
		 * If no out of band data is expected, pull receive urgent
		 * pointer along with the receive window. 
		 */
		if (SEQ_GT(tp->rcv_nxt, tp->rcv_up))
			tp->rcv_up = tp->rcv_nxt;

	/*
	 * Process the segment text, merging it into the TCP sequencing
	 * queue, and arranging for acknowledgment of receipt if necessary.
	 * This process logically involves adjusting tp->rcv_wnd as data is
	 * presented to the user (this happens in tcp_usrreq.c, case
	 * PRU_RCVD).  If a FIN has already been received on this connection
	 * then we just ignore the text. 
	 */
	if ((ti->ti_len || (tiflags & TH_FIN)) &&
	    TCPS_HAVERCVDFIN(tp->t_state) == 0) {
		TCP_REASS(tp, ti, bp, usq, tiflags);
		if (tcpnodelack == 0)
			tp->t_flags |= TF_DELACK;
		else
			tp->t_flags |= TF_ACKNOW;
		/*
		 * Note the amount of data that peer has sent into our
		 * window, in order to estimate the sender's buffer size. 
		 */
		if (usq)
			len = usq->q_hiwat - (tp->rcv_adv - tp->rcv_nxt);
		else
			len = 4096 - (tp->rcv_adv - tp->rcv_nxt);
		if (len > tp->max_rcvd)
			tp->max_rcvd = len;
	} else {
		freemsg(bp);
		tiflags &= ~TH_FIN;
	}

	/*
	 * If FIN is received ACK the FIN and let the user know that the
	 * connection is closing. 
	 */
	if (tiflags & TH_FIN) {
		STRLOG(TCPM_ID, 1, 7, SL_TRACE, "rcvd FIN tcb %x pcb %x",
		       tp, tp->t_inpcb);
		if (TCPS_HAVERCVDFIN(tp->t_state) == 0) {
			inp->inp_state |= SS_CANTRCVMORE;
			tp->t_flags |= TF_ACKNOW;
			tp->rcv_nxt++;
		}
		switch (tp->t_state) {


			/*
			 * In SYN_RECEIVED and ESTABLISHED STATES enter the
			 * CLOSE_WAIT state. 
			 */
		case TCPS_SYN_RECEIVED:
		case TCPS_ESTABLISHED:
			tp->t_state = TCPS_CLOSE_WAIT;
			/* tcp_deqdata takes care of sending T_ORDREL_IND */
			if (usq) qenable(usq);
			break;

			/*
			 * If still in FIN_WAIT_1 STATE FIN has not been
			 * acked so enter the CLOSING state. 
			 */
		case TCPS_FIN_WAIT_1:
			tp->t_state = TCPS_CLOSING;
			if (inp->inp_q) 
				qenable(inp->inp_q);
			break;

			/*
			 * In FIN_WAIT_2 state enter the TIME_WAIT state,
			 * starting the time-wait timer, turning off the
			 * other standard timers. 
			 */
		case TCPS_FIN_WAIT_2:
			tp->t_state = TCPS_TIME_WAIT;
			tcp_canceltimers(tp);
			tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			if (inp->inp_q)
				qenable(inp->inp_q);
			break;

			/*
			 * In TIME_WAIT state restart the 2 MSL time_wait
			 * timer. 
			 */
		case TCPS_TIME_WAIT:
			tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			break;

		default:
			break;
		}
	}
	if ((inp->inp_protoopt & SO_DEBUG) || tcpalldebug != 0)
		tcp_trace(TA_INPUT, ostate, tp, &tcp_saveti, 0);

	/*
	 * Return any desired output. 
	 */
	if (needoutput || (tp->t_flags & TF_ACKNOW))
		(void) tcp_output(tp);		/* Already got the IO lock */
	goto moreinput;

dropafterack:
	/*
	 * Generate an ACK dropping incoming segment if it occupies sequence
	 * space, where the ACK reflects our state. 
	 */
	if (tiflags & TH_RST)
		goto drop;
	freemsg(bp);
	tp->t_flags |= TF_ACKNOW;
	(void) tcp_output(tp);
	goto moreinput;

dropwithreset:
	if (optbp) {
		(void) freemsg(optbp);
		optbp = 0;
	}
	/*
	 * Generate a RST, dropping incoming segment. Make ACK acceptable to
	 * originator of segment. Don't bother to respond if destination was
	 * broadcast. 
	 */
	if ((tiflags & TH_RST) || in_broadcast(ti->ti_dst))
		goto drop;
	if (tiflags & TH_ACK)
		tcp_respond(bp, tp, ti, (tcp_seq) 0, ti->ti_ack, TH_RST);
	else {
		if (tiflags & TH_SYN)
			ti->ti_len++;
		tcp_respond(bp, tp, ti, ti->ti_seq + ti->ti_len, (tcp_seq) 0,
			    TH_RST | TH_ACK);
	}
	/* destroy temporarily created socket */
	if (dropsocket)
		tp = tcp_drop(tp, ECONNABORTED);
	goto moreinput;

drop:
	if (tcpdprintf)
#ifdef SYSV
		cmn_err(CE_NOTE, "tcp_uinput: drop\n");
#else
		printf( "tcp_uinput: drop\n");
#endif
	if (optbp)
		(void) freemsg(optbp);
	/*
	 * Drop space held by incoming segment and return. 
	 */
	if (tp && ((tp->t_inpcb->inp_protoopt & SO_DEBUG) || tcpalldebug != 0))
		tcp_trace(TA_DROP, ostate, tp, &tcp_saveti, 0);
	freemsg(bp);
	/* destroy temporarily created socket */
	if (dropsocket)
		tp = tcp_drop(tp, ECONNABORTED);
	goto moreinput;
}

tcp_dooptions(tp, optbp, ti)
	struct tcpcb   *tp;
	mblk_t         *optbp;
	struct tcpiphdr *ti;
{
	register u_char *cp;
	int             opt, optlen, cnt;

	cp = (u_char *) optbp->b_rptr;
	cnt = msgblen(optbp);
	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[0];
		if (opt == TCPOPT_EOL)
			break;
		if (opt == TCPOPT_NOP)
			optlen = 1;
		else {
			optlen = cp[1];
			if (optlen <= 0)
				break;
		}
		switch (opt) {

		default:
			break;

		case TCPOPT_MAXSEG:
			if (optlen != 4)
				continue;
			if (!(ti->ti_flags & TH_SYN))
				continue;
			tp->t_maxseg = *(u_short *) (cp + 2);
			tp->t_maxseg = ntohs((u_short) tp->t_maxseg);
			tp->t_maxseg = MIN(tp->t_maxseg, tcp_mss(tp));
			break;
		}
	}
	(void) freemsg(optbp);
}

/*
 * Determine a reasonable value for maxseg size. If the route is known, use
 * one that can be handled on the given interface without forcing IP to
 * fragment. If interface pointer is unavailable, or the destination isn't
 * local, use a conservative size (512 or the default IP max size, but no
 * more than the maxtu of the interface through which we route), as we can't
 * discover anything about intervening gateways or networks. 
 *
 * This is ugly, and doesn't belong at this level, but has to happen somehow. 
 */
#define satosin(x) ((struct sockaddr_in *) (x))
tcp_mss(tp)
	register struct tcpcb *tp;
{
	struct route   *ro;
	struct ip_provider *prov;
	int             mss;
	struct inpcb   *inp;
	int             rtret = RT_OK;

	inp = tp->t_inpcb;
	ro = &inp->inp_route;
	if ((ro->ro_rt == (mblk_t *) 0) ||
	    (prov = RT(ro->ro_rt)->rt_prov) == (struct ip_provider *) 0) {
		if (ro->ro_rt)
			rtfree(ro->ro_rt);
		/* No route yet, so try to acquire one */
		if (inp->inp_faddr.s_addr != INADDR_ANY) {
			satosin(&ro->ro_dst)->sin_addr.s_addr = 
				inp->inp_faddr.s_addr;
			rtret = rtalloc(ro, 0);	/* no dial */
		}
		if (rtret == RT_DEFER) {
			rtfree(ro->ro_rt);
			ro->ro_rt = 0;
		}
		if ((ro->ro_rt == 0) || (prov = RT(ro->ro_rt)->rt_prov) == 0)
			return (TCP_MSS);
		if (rtret == RT_SWITCHED) {
			rtfree(ro->ro_rt);
			ro->ro_rt = 0;
		}
	}
	mss = prov->if_maxtu - sizeof(struct tcpiphdr);
	if (mss > 1024)
		mss = (mss / 1024) * 1024;
	if (in_localaddr(inp->inp_faddr))
		return (mss);
	mss = MIN(mss, TCP_MSS);
	tp->snd_cwnd = mss;
	return (mss);
}

/*
 * When an attempt at a new connection is noted on a TCP endpoint which
 * accepts connections, inpnewconn is called.  If the connection is possible
 * (subject to space constraints, etc.) then we allocate a new structure,
 * properly linked into the data structure of the original tcpcb, and return
 * this. 
 */
struct inpcb   *
inpnewconn(head)
	register struct inpcb *head;
{
	register struct inpcb *inp;
	struct tcpcb   *htp, *tp;

	htp = intotcpcb(head);
	if (htp->t_qlen + htp->t_q0len > 3 * htp->t_qlimit / 2)
		goto bad;

	inp = in_pcballoc(&tcb);
	if (inp == NULL)
		goto bad;
	tp = tcp_newtcpcb(inp);
	if (tp == 0) {
		in_pcbdetach(inp);
		goto bad;
	}
	inp->inp_q = (queue_t *) NULL;
	inp->inp_tstate = TS_DATA_XFER;

	inp->inp_protoopt = head->inp_protoopt & ~SO_ACCEPTCONN;
	inp->inp_linger = head->inp_linger;
	inp->inp_state = head->inp_state | SS_NOFDREF;
	tpqinsque(htp, tp, 0);
	STRLOG(TCPM_ID, 1, 5, SL_TRACE, "inpnewconn head %x inp %x", head, inp);
	return (inp);
bad:
	STRLOG(TCPM_ID, 1, 3, SL_TRACE, "inpnewconn failed head %x", head);
	return ((struct inpcb *) 0);
}

/*
 * tcp_qdrop trims data from the front of the queue.  It is used when acks
 * for the data come in. 
 */

tcp_qdrop(q, len)
	register queue_t *q;
	register int    len;
{
	register mblk_t *bp = NULL, *obp;
	int		n;

	STRLOG(TCPM_ID, 2, 9, SL_TRACE, "tcp_qdrop q %x len %d", q, len);

	while (len && (bp = getq(q))) {
		do {
                	if (len >= (n=msgblen(bp))) {
                        	obp = bp;
                        	bp = bp->b_cont;
                        	freeb(obp);
                        	len -= n;
                	} else {
                        	bp->b_rptr += len;
				goto out;
                	}
        	} while (bp);
	}
out:
	if (bp)
		putbq(q, bp);
	q->q_flag |= QWANTR;
}
