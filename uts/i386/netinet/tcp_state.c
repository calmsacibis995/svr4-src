/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:tcp_state.c	1.3.2.2"

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

#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <net/if.h>
#include <sys/errno.h>
#include <sys/debug.h>

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
#ifdef SYSV
#ifdef SYSV
#include <sys/cmn_err.h>
#endif
#endif SYSV

#include <netinet/ip_str.h>

/* from timod - this table determines the legal transitions and events */
/*
 * TLI state transition table note that if this changes in timod, it should
 * change here too - and think about what changes do to the code below!!
 */
#define nr	TS_NOSTATES	/* not reachable */

static char     ti_statetbl[TE_NOEVENTS + 2][TS_NOSTATES] = {
	/* STATES */
       /* 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16 */

/* 0*/	{ 1, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
/* 1*/	{nr, nr, nr,  2, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
/* 2*/	{nr, nr, nr,  4, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
/* 3*/	{nr,  3, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
/* 4*/	{nr, nr, nr, nr,  3, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
/* 5*/	{nr,  0,  3, nr,  3,  3, nr, nr,  7, nr, nr, nr,  6,  7,  9, 10, 11},
/* 6*/	{nr, nr,  0, nr, nr,  6, nr, nr, nr, nr, nr, nr,  3, nr,  3,  3,  3},
/* 7*/	{nr, nr, nr, nr, nr, nr, nr, nr,  9, nr, nr, nr, nr,  3, nr, nr, nr},
/* 8*/	{nr, nr, nr, nr, nr, nr, nr, nr,  3, nr, nr, nr, nr,  3, nr, nr, nr},
/* 9*/	{nr, nr, nr, nr, nr, nr, nr, nr,  7, nr, nr, nr, nr,  7, nr, nr, nr},
/*10*/	{nr, nr, nr,  5, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
/*11*/	{nr, nr, nr, nr, nr, nr, nr,  8, nr, nr, nr, nr, nr, nr, nr, nr, nr},
/*12*/	{nr, nr, nr, nr, nr, nr, 12, 13, nr, 14, 15, 16, nr, nr, nr, nr, nr},
/*13*/	{nr, nr, nr, nr, nr, nr, nr, nr, nr,  9, nr, 11, nr, nr, nr, nr, nr},
/*14*/	{nr, nr, nr, nr, nr, nr, nr, nr, nr,  9, nr, 11, nr, nr, nr, nr, nr},
/*15*/	{nr, nr, nr, nr, nr, nr, nr, nr, nr, 10, nr,  3, nr, nr, nr, nr, nr},
/*16*/	{nr, nr, nr,  7, nr, nr, nr,  7, nr, nr, nr, nr, nr, nr, nr, nr, nr},
/*17*/	{nr, nr, nr, nr, nr, nr,  9, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
/*18*/	{nr, nr, nr, nr, nr, nr, nr, nr, nr,  9, 10, nr, nr, nr, nr, nr, nr},
/*19*/	{nr, nr, nr, nr, nr, nr, nr, nr, nr,  9, 10, nr, nr, nr, nr, nr, nr},
/*20*/	{nr, nr, nr, nr, nr, nr, nr, nr, nr, 11,  3, nr, nr, nr, nr, nr, nr},
/*21*/	{nr, nr, nr, nr, nr, nr,  3, nr, nr,  3,  3,  3, nr, nr, nr, nr, nr},
/*22*/	{nr, nr, nr, nr, nr, nr, nr,  3, nr, nr, nr, nr, nr, nr, nr, nr, nr},
/*23*/	{nr, nr, nr, nr, nr, nr, nr,  7, nr, nr, nr, nr, nr, nr, nr, nr, nr},
/*24*/	{nr, nr, nr,  9, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
/*25*/	{nr, nr, nr,  3, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
/*26*/	{nr, nr, nr,  3, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
/*27*/	{nr, nr, nr,  3, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
/*28*/	{ 0,  1,  3,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16},
/*29*/	{nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
};

#ifndef DEBUG
#define NEXTSTATE(X, Y)		ti_statetbl[X][Y]
#else
#define NEXTSTATE(X, Y) ( \
		strlog(TCPM_ID, 3, 5, SL_TRACE, \
			"TLI state %d (%d) -> %d", X, Y, ti_statetbl[X][Y]), \
		ti_statetbl[X][Y] )
#endif				/* OSDEBUG */

#define BADSTATE		nr	/* unreachable state */

/*
 * Mapping of TLI T_primitive types to state machine events
 * 
 * T_INFO_REQ is removed since it doesn't affect state, and T_UNITDATA_REQ is
 * not supported.
 */
#define NPRIM T_ORDREL_REQ+1
#define NONEVENT TE_NOEVENTS
#define BADEVENT TE_NOEVENTS+1

static
int             prim_to_event[NPRIM] = {
	TE_CONN_REQ,		/* 0 - T_CONN_REQ */
	TE_CONN_RES,		/* 1 - T_CONN_RES */
	TE_DISCON_REQ,		/* 2 - T_DISCON_REQ */
	TE_DATA_REQ,		/* 3 - T_DATA_REQ */
	TE_EXDATA_REQ,		/* 4 - T_EXDATA_REQ */
	NONEVENT,		/* 5 - T_INFO_REQ */
	TE_BIND_REQ,		/* 6 - T_BIND_REQ */
	TE_UNBIND_REQ,		/* 7 - T_UNBIND_REQ */
	BADEVENT,		/* 8 - T_UNITDATA_REQ */
	TE_OPTMGMT_REQ,		/* 9 - T_OPTMGMT_REQ */
	TE_ORDREL_REQ,		/* 10 - T_ORDREL_REQ */
};

extern char    *tcpstates[];
extern queue_t *tcp_qbot;
extern int      tcpalldebug;
extern struct inpcb *in_pcballoc();
extern struct tcpcb *tcp_newtcpcb();


mblk_t         *reallocb();

#define CHECKSIZE(bp,size) if (((bp) = reallocb((bp), (size),0)) == NULL) {\
			return;\
			}

/*
 * this is the subfunction of the upper put routine which handles data and
 * protocol packets for us.
 */

tcp_state(q, bp)
	queue_t        *q;
	register mblk_t *bp;
{
	register union T_primitives *t_prim;
	register struct inpcb *inp = qtoinp(q);
	struct inpcb   *newinp;
	struct tcpcb   *tp, *ctp;
	int             error = 0;
	queue_t        *newq;
	mblk_t         *head;
	short           ostate;	/* used for tracing */
	struct sockaddr_in *sin;
	int             otype;
	register int    s;
	short           tstate;
	extern		int tcpopen();

	tp = intotcpcb(inp);
	ostate = tp->t_state;

	/*
	 * check for pending error, or a broken state machine
	 */

	STRLOG(TCPM_ID, 3, 8, SL_TRACE, "tcp_state wq %x inp %x", q, inp);
	if (inp->inp_error != 0) {
		T_errorack(q, bp, TSYSERR, inp->inp_error);
		return;
	}
	if (inp->inp_tstate == BADSTATE) {
		T_errorack(q, bp, TOUTSTATE, 0);
		return;
	}
	/* just send pure data, if we're ready */
	if (bp->b_datap->db_type == M_DATA) {
		if (NEXTSTATE(TE_DATA_REQ, inp->inp_tstate) != BADSTATE) {
			s = splstr();
			tp->t_qsize += msgdsize(bp);
			putq(q, bp);	/* Let service routine take care of
					 * it */
			splx(s);
			if (tp && ((inp->inp_protoopt & SO_DEBUG)
				   || tcpalldebug != 0))
				tcp_trace(TA_USER, ostate, tp,
					  (struct tcpiphdr *) 0, T_DATA_REQ);
		} else {
			CHECKSIZE(bp, sizeof(struct T_error_ack));
			bp->b_datap->db_type = M_PCPROTO;
			t_prim = (union T_primitives *) bp->b_rptr;
			bp->b_wptr = bp->b_rptr + sizeof(struct T_error_ack);
			t_prim->type = T_ERROR_ACK;
			t_prim->error_ack.ERROR_prim = T_DATA_REQ;
			t_prim->error_ack.TLI_error = TOUTSTATE;
			qreply(q, bp);
		}
		return;
	}
	/* if it's not data, it's proto or pcproto */

	t_prim = (union T_primitives *) bp->b_rptr;
	STRLOG(TCPM_ID, 3, 8, SL_TRACE, "DSPM type %d, wq %x inp %x",
	       t_prim->type, q, inp);

	otype = t_prim->type;
	switch (otype) {

	case T_INFO_REQ:
		/* our state doesn't matter here */
		CHECKSIZE(bp, sizeof(struct T_info_ack));
		bp->b_rptr = bp->b_datap->db_base;
		bp->b_wptr = bp->b_rptr + sizeof(struct T_info_ack);
		t_prim = (union T_primitives *) bp->b_rptr;
		t_prim->type = T_INFO_ACK;
		t_prim->info_ack.TSDU_size = 0;
		t_prim->info_ack.ETSDU_size = 1;
		t_prim->info_ack.CDATA_size = -2;	/* ==> not supported */
		t_prim->info_ack.DDATA_size = -2;
		t_prim->info_ack.ADDR_size = sizeof(struct sockaddr_in);
		t_prim->info_ack.OPT_size = -1;
		t_prim->info_ack.TIDU_size = 16 * 1024;
		t_prim->info_ack.SERV_type = T_COTS_ORD;
		t_prim->info_ack.CURRENT_state = inp->inp_tstate;
		t_prim->info_ack.PROVIDER_flag = TP_EXPINLINE;
		bp->b_datap->db_type = M_PCPROTO;	/* make sure */
		qreply(q, bp);
		break;

	case T_BIND_REQ:
		STRLOG(TCPM_ID, 1, 6, SL_TRACE, "BIND_REQ, inp %x", inp);
		if (inp->inp_tstate != TS_UNBND) {
			T_errorack(q, bp, TOUTSTATE, 0);
			break;
		}
		if (t_prim->bind_req.ADDR_length == 0) {
			error = in_pcbbind(inp, NULL);
		} else {
			bp->b_rptr += t_prim->bind_req.ADDR_offset;
			error = in_pcbbind(inp, bp);
			bp->b_rptr -= t_prim->bind_req.ADDR_offset;
		}
		if (error == EACCES) {
			T_errorack(q, bp, TACCES, 0);
			error = 0;
			break;
		}
		else if (error == EINVAL) {
			T_errorack(q, bp, TBADADDR, 0);
			error = 0;
			break;
		}
		else if (error)
			break;
		inp->inp_tstate = TS_IDLE;
		if (t_prim->bind_req.CONIND_number > 0) {
			tp->t_qlimit = t_prim->bind_req.CONIND_number;
			tp->t_state = TCPS_LISTEN;
			inp->inp_protoopt |= SO_ACCEPTCONN;
		}
		if ((bp = reallocb(bp, sizeof(struct T_bind_ack)
				   + inp->inp_addrlen, 1))
		    == NULL) {
			return;
		}
		t_prim = (union T_primitives *) bp->b_rptr;
		t_prim->bind_ack.PRIM_type = T_BIND_ACK;
		t_prim->bind_ack.ADDR_length = inp->inp_addrlen;
		t_prim->bind_ack.ADDR_offset = sizeof(struct T_bind_req);
		sin = (struct sockaddr_in *)
			(bp->b_rptr + sizeof(struct T_bind_ack));
		bp->b_wptr = (unsigned char *)
			(((caddr_t) sin) + inp->inp_addrlen);
		bzero((caddr_t) sin, inp->inp_addrlen);
		sin->sin_family = inp->inp_family;
		sin->sin_addr = inp->inp_laddr;
		sin->sin_port = inp->inp_lport;
		bp->b_datap->db_type = M_PCPROTO;
		qreply(q, bp);
		break;

	case T_UNBIND_REQ:
		STRLOG(TCPM_ID, 1, 8, SL_TRACE, "UNBIND_REQ, inp %x",
		       inp);
		if (inp->inp_tstate != TS_IDLE) {
			T_errorack(q, bp, TOUTSTATE, 0);
			break;
		}
		/*
		 * Note that above state restriction keeps us from having to
		 * worry about connect indications which have arrived but not
		 * been processed.
		 */
		tp = intotcpcb(inp);
		tp->t_state = TCPS_CLOSED;
		tp->t_qlimit = 0;
		in_pcbdisconnect(inp);
		inp->inp_laddr.s_addr = INADDR_ANY;
		inp->inp_lport = 0;
		inp->inp_tstate = TS_UNBND;
		T_okack(q, bp);
		break;

		/*
		 * Initiate connection to peer. Create a template for use in
		 * transmissions on this connection. Enter SYN_SENT state,
		 * and mark socket as connecting. Start keep-alive timer, and
		 * seed output sequence space. Send initial segment on
		 * connection.
		 */
	case T_CONN_REQ:
		STRLOG(TCPM_ID, 1, 6, SL_TRACE, "CONN_REQ, inp %x", inp);
		if (inp->inp_tstate != TS_IDLE) {
			T_errorack(q, bp, TOUTSTATE, 0);
			break;
		}
		bp->b_rptr += t_prim->conn_req.DEST_offset;
		error = in_pcbconnect(inp, bp);
		bp->b_rptr -= t_prim->conn_req.DEST_offset;
		if (error == EINVAL) {
			T_errorack(q, bp, TBADADDR, 0);
			error = 0;
			break;
		}
		else if (error)
			break;
		else if (inp->inp_faddr.s_addr == inp->inp_laddr.s_addr
			&& inp->inp_fport == inp->inp_lport) {
			error = EADDRINUSE;
			break;
		}
		tp->t_template = tcp_template(tp);
		if (tp->t_template == 0) {
			in_pcbdisconnect(inp);
			error = ENOSR;
			break;
		}
		tcpstat.tcps_connattempt++;
		tp->t_state = TCPS_SYN_SENT;
		/* in case we came from the TCPS_LISTEN state */
		inp->inp_protoopt &= ~SO_ACCEPTCONN;
		tp->t_timer[TCPT_KEEP] = TCPTV_KEEP_INIT;
		tp->iss = tcp_iss;
		tcp_iss += TCP_ISSINCR / 2;
		tcp_sendseqinit(tp);
		T_okack(q, bp);
		tcp_io(tp, TF_NEEDOUT, NULL);
		inp->inp_tstate = TS_WCON_CREQ;
		break;

	case T_CONN_RES:
		STRLOG(TCPM_ID, 1, 6, SL_TRACE, "CONN_RES, inp %x", inp);
		if (NEXTSTATE(prim_to_event[T_CONN_RES], inp->inp_tstate)
		    == BADSTATE) {
			inp->inp_tstate =
				NEXTSTATE(TE_ERROR_ACK,
					  inp->inp_tstate);
			T_errorack(q, bp, TOUTSTATE, 0);
			break;
		}
		/*
		 * Don't do this accept if it's not a TCP queue.
		 */

		newq = t_prim->conn_res.QUEUE_ptr;
		if (newq->q_qinfo->qi_qopen != tcpopen) {
			T_errorack(q, bp, TBADF, 0);
			break;
		}

		inp->inp_tstate =
			NEXTSTATE(prim_to_event[t_prim->type], inp->inp_tstate);

		if (newq != RD(q)) {
			newinp = (struct inpcb *) newq->q_ptr;
			ASSERT(newinp);
			if (newinp->inp_tstate != TS_IDLE) {
				inp->inp_tstate =
					NEXTSTATE(TE_ERROR_ACK,
						  inp->inp_tstate);
				T_errorack(q, bp, TBADF, 0);
				break;
			}
		} else {
			newinp = inp;
			if (tp->t_qlen != 1) {
				inp->inp_tstate =
					NEXTSTATE(TE_ERROR_ACK,
						  inp->inp_tstate);
				T_errorack(q, bp, TBADF, 0);
				break;
			}
		}
		for (ctp = tp->t_q; ctp; ctp = ctp->t_q) {
			if ((long) ctp == t_prim->conn_res.SEQ_number) {
				break;
			}
		}
		if (ctp == NULL) {
			inp->inp_tstate =
				NEXTSTATE(TE_ERROR_ACK, inp->inp_tstate);
			T_errorack(q, bp, TBADSEQ, 0);
			break;
		}
		ctp->t_inpcb->inp_minor = newinp->inp_minor;
		ctp->t_inpcb->inp_q = newq;
		newinp->inp_q = NULL;
		newinp->inp_state |= SS_NOFDREF;
		tstate = inp->inp_tstate;
		tcp_close(intotcpcb(newinp), 0);	/* temp or old listener */
		newq->q_ptr = (char *) ctp->t_inpcb;
		WR(newq)->q_ptr = (char *) ctp->t_inpcb;
		if (newq == RD(q))
			inp = (struct inpcb *) q->q_ptr;
		if (tp->t_qlen == 1)
			if (newq == RD(q))
				inp->inp_tstate = NEXTSTATE(TE_OK_ACK2, tstate);
			else
				inp->inp_tstate = NEXTSTATE(TE_OK_ACK3, tstate);
		else
			inp->inp_tstate = NEXTSTATE(TE_OK_ACK4, tstate);
		tpqremque(ctp, 1);
		ctp->t_inpcb->inp_state &= ~SS_NOFDREF;
		T_okack(q, bp);
		qenable(newq);	/* send any early data */
		break;

	case T_ORDREL_REQ:
		STRLOG(TCPM_ID, 1, 6, SL_TRACE, "ORDREL_REQ, inp %x opt %x",
		       inp, inp->inp_protoopt);
		if ((inp->inp_tstate =
		     NEXTSTATE(prim_to_event[otype], inp->inp_tstate))
		    == BADSTATE) {
			inp->inp_tstate =
				NEXTSTATE(TE_ERROR_ACK,
					  inp->inp_tstate);
			T_errorack(q, bp, TOUTSTATE, 0);
			break;
		}
		if (bp)
			freemsg(bp);
		tcp_disconnect(tp);
		break;

	case T_DISCON_REQ:
		STRLOG(TCPM_ID, 1, 6, SL_TRACE, "DISCON_REQ, inp %x",
		       inp);
		if ((inp->inp_tstate =
		     NEXTSTATE(prim_to_event[otype], inp->inp_tstate))
		    == BADSTATE) {
			inp->inp_tstate =
				NEXTSTATE(TE_ERROR_ACK,
					  inp->inp_tstate);
			T_errorack(q, bp, TOUTSTATE, 0);
			break;
		}
		tp = intotcpcb(inp);
		if (inp->inp_tstate == TS_WACK_DREQ7) {
			/* connection indication refused by client */
			ctp = (struct tcpcb *) t_prim->discon_req.SEQ_number;
			if (!tpqremque(ctp, 1)) {
				inp->inp_tstate =
					NEXTSTATE(TE_ERROR_ACK,
						  inp->inp_tstate);
				T_errorack(q, bp, TBADSEQ, 0);
				break;
			}
			tcp_drop(ctp, 0);
			if (tp->t_qlen == 0)
				inp->inp_tstate = NEXTSTATE(TE_OK_ACK2,
							    inp->inp_tstate);
			else
				inp->inp_tstate = NEXTSTATE(TE_OK_ACK4,
							    inp->inp_tstate);
		} else {
			tcp_drop(tp, 0);
			inp->inp_tstate = NEXTSTATE(TE_OK_ACK1,
						    inp->inp_tstate);
		}
		T_okack(q, bp);
		break;

	case T_OPTMGMT_REQ:{
			STRLOG(TCPM_ID, 4, 8, SL_TRACE, "OPTMGMT_REQ, inp %x",
			       inp);
			/* doesn't change the state */
			tcp_ctloutput(q, bp);
			break;
		}

	case T_DATA_REQ:
		STRLOG(TCPM_ID, 2, 8, SL_TRACE, "DATA_REQ, inp %x",
		       inp);
		/* probably should arrange to avoid PUSHing if more set */
		/* sending doesn't change state */
		if (NEXTSTATE(TE_DATA_REQ, inp->inp_tstate) == BADSTATE
		    || bp->b_cont == NULL) {
			freemsg(bp);	/* TLI doesn't want errors here */
			break;
		}
		head = bp;
		bp = bp->b_cont;
		freeb(head);
		if (bp == NULL)
			break;
		s = splstr();
		tp->t_qsize += msgdsize(bp);
		putq(q, bp);	/* Let service routine take care of it */
		splx(s);
		break;

	case T_EXDATA_REQ:
		STRLOG(TCPM_ID, 1, 6, SL_TRACE, "EXDATA_REQ, inp %x",
		       inp);
		if (NEXTSTATE(TE_EXDATA_REQ, inp->inp_tstate) == BADSTATE
		    || bp->b_cont == NULL) {
			T_errorack(q, bp, TOUTSTATE, 0);
			break;
		}
		tp = intotcpcb(inp);
		head = bp;
		bp = bp->b_cont;
		if (bp == NULL) {
			freeb(head);
			break;
		}
		s = splstr();
		tp->t_qsize += msgdsize(bp);
		if (((struct T_exdata_req *)head->b_rptr)->MORE_flag == 0) {
			tp->snd_up = tp->snd_una + tp->t_qsize;
			tp->t_force = 1;
		}
		putq(q, bp);
		splx(s);
		freeb(head);
		break;

	default:
		T_errorack(q, bp, TNOTSUPPORT, 0);
		return;
	}
	if (error)
		T_errorack(q, bp, TSYSERR, error);
	else if (tp && ((inp->inp_protoopt & SO_DEBUG) || tcpalldebug != 0))
		tcp_trace(TA_USER, ostate, tp,
			  (struct tcpiphdr *) 0, (int) otype);
}

/*
 * TCP option processing. Returns 0 if ok, T-error, or negative E-error. A
 * list of options is built in the message mp.
 */
int
tcp_options(q, req, opt, mp)
	queue_t        *q;
	struct T_optmgmt_req *req;
	struct opthdr  *opt;
	mblk_t         *mp;
{
	struct inpcb   *inp = qtoinp(q);
	struct tcpcb   *tp = intotcpcb(inp);

	switch (req->MGMT_flags) {

	case T_NEGOTIATE:
		switch (opt->name) {
		case TCP_NODELAY:
			if (opt->len != OPTLEN(sizeof(int)))
				return TBADOPT;
			if (*(int *) OPTVAL(opt))
				tp->t_flags |= TF_NODELAY;
			else
				tp->t_flags &= ~TF_NODELAY;
			break;

		case TCP_MAXSEG:	/* not yet */
			return TACCES;

		default:
			return TBADOPT;
		}

		/* fall through to retrieve value */

	case T_CHECK:
		switch (opt->name) {
		case TCP_NODELAY:
		case TCP_MAXSEG:{
				int             val;

				switch (opt->name) {
				case TCP_NODELAY:
					val = (tp->t_flags & TF_NODELAY) != 0;
					break;

				case TCP_MAXSEG:
					val = tp->t_maxseg;
					break;
				}
				if (!makeopt(mp, IPPROTO_TCP, opt->name, &val, sizeof(int)))
					return -ENOSR;
				break;
			}

		default:
			req->MGMT_flags = T_FAILURE;
			break;
		}
		break;

	case T_DEFAULT:{
			int             val;

			val = 0;
			if (!makeopt(mp, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(int)))
				return -ENOSR;
			/*
			 * since this can't yet be changed, we can use
			 * current value
			 */
			val = tp->t_maxseg;
			if (!makeopt(mp, IPPROTO_TCP, TCP_MAXSEG, &val, sizeof(int)))
				return -ENOSR;
			break;
		}
	}

	return 0;

}

tcp_ctloutput(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	int             in_pcboptmgmt(), tcp_options(), ip_options();
	static struct opproc funclist[] = {
		SOL_SOCKET, in_pcboptmgmt,
		IPPROTO_TCP, tcp_options,
		IPPROTO_IP, ip_options,
		0, 0
	};

	dooptions(q, bp, funclist);
}


/*
 * Attach TCP protocol to socket, allocating internet protocol control block
 * and tcp control block.
 */
tcp_attach(q)
	queue_t        *q;
{
	register struct tcpcb *tp;
	struct inpcb   *inp;

	inp = in_pcballoc(&tcb);
	if (inp == NULL)
		return (ENOSR);
	tp = tcp_newtcpcb(inp);
	if (tp == 0) {
		in_pcbdetach(inp);
		return (ENOSR);
	}
	q->q_ptr = (char *) inp;
	WR(q)->q_ptr = (char *) inp;
	inp->inp_q = q;
	inp->inp_tstate = TS_UNBND;
	tp->t_state = TCPS_CLOSED;
	return (0);
}

/*
 * Initiate (or continue) disconnect. If embryonic state, just send reset
 * (once). If in ``let data drain'' option and linger null, just drop.
 * Otherwise (hard), mark socket disconnecting and drop current input data;
 * switch states based on user close, and send segment to peer (with FIN).
 */
struct tcpcb   *
tcp_disconnect(tp)
	register struct tcpcb *tp;
{
	struct inpcb   *inp = tp->t_inpcb;

	if (tp->t_state < TCPS_ESTABLISHED)
		tp = tcp_close(tp, 0);
	else if ((inp->inp_protoopt & SO_LINGER) && inp->inp_linger == 0)
		tp = tcp_drop(tp, 0);
	else {
		tp = tcp_usrclosed(tp);
		if (tp)
			tcp_io(tp, TF_NEEDOUT, NULL);
	}
	return (tp);
}

/*
 * User issued close, and wish to trail through shutdown states: if never
 * received SYN, just forget it.  If got a SYN from peer, but haven't sent
 * FIN, then go to FIN_WAIT_1 state to send peer a FIN. If already got a FIN
 * from peer, then almost done; go to LAST_ACK state.  In all other cases,
 * have already sent FIN to peer (e.g. after PRU_SHUTDOWN), and just have to
 * play tedious game waiting for peer to send FIN or not respond to
 * keep-alives, etc. We can let the user exit from the close as soon as the
 * FIN is acked.
 */
struct tcpcb   *
tcp_usrclosed(tp)
	register struct tcpcb *tp;
{
	switch (tp->t_state) {

	case TCPS_CLOSED:
	case TCPS_LISTEN:
	case TCPS_SYN_SENT:
		tp->t_state = TCPS_CLOSED;
		tp = tcp_close(tp, 0);
		break;

	case TCPS_SYN_RECEIVED:
	case TCPS_ESTABLISHED:
		tp->t_state = TCPS_FIN_WAIT_1;
		break;

	case TCPS_CLOSE_WAIT:
		tp->t_state = TCPS_LAST_ACK;
		break;

	default:
		break;
	}
	return (tp);
}

tpqinsque(head, tp, q)
	register struct tcpcb *head, *tp;
	int             q;
{

	tp->t_head = head;
	if (q == 0) {
		head->t_q0len++;
		tp->t_q0 = head->t_q0;
		head->t_q0 = tp;
	} else {
		head->t_qlen++;
		tp->t_q = head->t_q;
		head->t_q = tp;
	}
}

tpqremque(tp, q)
	register struct tcpcb *tp;
	int             q;
{
	register struct tcpcb *head, *prev, *next;

	head = tp->t_head;
	prev = head;
	for (;;) {
		next = q ? prev->t_q : prev->t_q0;
		if (next == tp)
			break;
		if (next == NULL) {
			return (0);
		}
		prev = next;
	}
	if (q == 0) {
		prev->t_q0 = next->t_q0;
		head->t_q0len--;
	} else {
		prev->t_q = next->t_q;
		head->t_qlen--;
	}
	next->t_q0 = next->t_q = 0;
	next->t_head = 0;
	return (1);
}

/*
 * Called by tcp_input when a connection completes, sets the state and sends
 * a connection indication or confirmation upstream.
 */

inpisconnected(inp)
	register struct inpcb *inp;
{
	struct tcpcb   *tp = intotcpcb(inp);
	struct tcpcb   *htp = tp->t_head;
	register struct inpcb *head;
	mblk_t         *bp;
	struct T_conn_ind *conn_ind;
	struct sockaddr_in *sin;
	int             cnt;

	STRLOG(TCPM_ID, 1, 5, SL_TRACE, "inpisconn inp %x tp %x head %x",
	       inp, tp, htp);


	if (htp) {

		head = htp->t_inpcb;
		cnt = sizeof(struct T_conn_ind) + head->inp_addrlen;
		if ((bp = allocb(cnt, BPRI_HI)) == NULL) {
			STRLOG(TCPM_ID, 1, 2, SL_TRACE, "inpisconn alloc fail inp %x", inp);
			return;
		}
		bp->b_wptr += cnt;
		bp->b_datap->db_type = M_PROTO;
		head->inp_tstate = NEXTSTATE(TE_CONN_IND, head->inp_tstate);
		inp->inp_tstate = TS_DATA_XFER;

		if (tpqremque(tp, 0) == 0)
#ifdef SYSV
			cmn_err(CE_PANIC, "inpisconnected: tpqremque failed");
#else
			panic( "inpisconnected: tpqremque failed");
#endif
		tpqinsque(htp, tp, 1);

		conn_ind = (struct T_conn_ind *) bp->b_rptr;
		sin = (struct sockaddr_in *) (bp->b_rptr +
					      sizeof(struct T_conn_ind));
		conn_ind->PRIM_type = T_CONN_IND;
		conn_ind->SRC_length = head->inp_addrlen;
		conn_ind->SRC_offset = sizeof(struct T_conn_ind);
		conn_ind->OPT_length = 0;
		conn_ind->OPT_offset = 0;
		conn_ind->SEQ_number = (long) tp;
	} else {
		struct T_conn_con *conn_con;

		caddr_t         p;
		int             mss;

		/* sizeof (MSS option) == 4 */
		cnt = sizeof(struct T_conn_con) + inp->inp_addrlen + 4;
		if ((bp = allocb(cnt, BPRI_HI)) == NULL) {
			STRLOG(TCPM_ID, 1, 2, SL_TRACE, "inpisconn alloc fail inp %x", inp);
			return;
		}
		bp->b_wptr += cnt;
		bp->b_datap->db_type = M_PROTO;
		inp->inp_tstate = NEXTSTATE(TE_CONN_CON, inp->inp_tstate);

		conn_con = (struct T_conn_con *) bp->b_rptr;
		sin = (struct sockaddr_in *) (bp->b_rptr +
					      sizeof(struct T_conn_con) + 4);
		conn_con->PRIM_type = T_CONN_CON;
		conn_con->RES_length = inp->inp_addrlen;
		conn_con->RES_offset = sizeof(struct T_conn_con) + 4;
		conn_con->OPT_length = 4;
		conn_con->OPT_offset = sizeof(struct T_conn_con);

		/*
		 * * now fill in tcp options (only MSS)
		 */

		p = (caddr_t) (bp->b_rptr + sizeof(struct T_conn_con));
		*p++ = 2;	/* kind 2 */
		*p++ = 4;	/* length 4 */
		mss = htons(tp->t_maxseg);
		*p++ = (mss >> 8) & 0xff;
		*p++ = mss & 0xff;

		head = inp;	/* for the putnext, below */
	}
	bzero((caddr_t) sin, inp->inp_addrlen);
	sin->sin_family = head->inp_family;
	sin->sin_addr = inp->inp_faddr;
	sin->sin_port = inp->inp_fport;
	inp->inp_state &= ~(SS_ISCONNECTING | SS_ISDISCONNECTING);
	inp->inp_state |= SS_ISCONNECTED;
	putnext(head->inp_q, bp);

}

/*
 * inpordrelind sends an orderly release indication up to the user. It gets
 * called when we have an established connection and get a FIN. Returns 0 if
 * allocb failed, 1 otherwise.
 */

int
inpordrelind(inp)
	struct inpcb   *inp;
{
	mblk_t         *bp;
	struct T_ordrel_ind *ind;

	STRLOG(TCPM_ID, 1, 5, SL_TRACE, "inpordrel inp %x", inp);

	if (NEXTSTATE(TE_ORDREL_IND, inp->inp_tstate) == BADSTATE) {
		STRLOG(TCPM_ID, 1, 4, SL_TRACE,
		       "inpordrel inp %x, bad state %d",
		       inp, inp->inp_tstate);
		return 1;
	}
	if ((bp = allocb(sizeof(struct T_ordrel_ind), BPRI_HI)) == NULL)
		return 0;
	inp->inp_tstate = NEXTSTATE(TE_ORDREL_IND, inp->inp_tstate);
	bp->b_datap->db_type = M_PROTO;
	ind = (struct T_ordrel_ind *) bp->b_rptr;
	bp->b_wptr += sizeof(struct T_ordrel_ind);
	ind->PRIM_type = T_ORDREL_IND;
	putnext(inp->inp_q, bp);
	return 1;
}

/*
 * Mark a tcp endpoint disconnected and send the appropriate indication to
 * the user.
 */

inpisdisconnected(inp, error)
	struct inpcb   *inp;
	int             error;
{
	mblk_t         *bp;
	struct T_discon_ind *ind;

	STRLOG(TCPM_ID, 1, 5, SL_TRACE, "inpisdis inp %x error %d", inp, error);

	inp->inp_state &= ~(SS_ISCONNECTING | SS_ISCONNECTED | SS_ISDISCONNECTING);
	inp->inp_state |= (SS_CANTRCVMORE | SS_CANTSENDMORE);
	if (NEXTSTATE(TE_DISCON_IND1, inp->inp_tstate) == BADSTATE) {
		STRLOG(TCPM_ID, 1, 4, SL_TRACE,
		       "inpisdis inp %x error %d, bad state %d",
		       inp, error, inp->inp_tstate);
		return;
	}
	inp->inp_tstate = NEXTSTATE(TE_DISCON_IND1, inp->inp_tstate);

	if (inp->inp_q == NULL || inp->inp_q->q_next == NULL
	    || (inp->inp_state & SS_NOFDREF) != 0) {
		STRLOG(TCPM_ID, 1, 3, SL_TRACE,
		       "inpisdis inp %x error %d, nofdref", inp, error);
		return;
	}
	if ((bp = allocb(sizeof(struct T_discon_ind), BPRI_HI)) == NULL) {
		STRLOG(TCPM_ID, 1, 2, SL_TRACE,
		    "inpisdis inp %x error %d, allocb failure", inp, error);
		return;
	}
	bp->b_datap->db_type = M_PROTO;
	ind = (struct T_discon_ind *) bp->b_rptr;
	bp->b_wptr += sizeof(struct T_discon_ind);
	ind->PRIM_type = T_DISCON_IND;
	ind->DISCON_reason = error;
	ind->SEQ_number = 0;
	putnext(inp->inp_q, bp);
}

/*
 * tcp_errdiscon - calls inpisdisconnected with error code from inp
 */
tcp_errdiscon(inp)
struct inpcb *inp;
{
	inpisdisconnected(inp, inp->inp_error);
}

/*
 * tcp_uderr - process N_UDERROR_IND from IP
 * If the error is not ENOSR and there are endpoints trying to connect
 * to this address, disconnect.
 */
tcp_uderr(bp)
mblk_t *bp;
{
	struct N_uderror_ind *uderr;
	struct sockaddr_in sin;

	uderr = (struct N_uderror_ind *) bp->b_rptr;
	if (uderr->ERROR_type == ENOSR)
		return;
	bzero(&sin,sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr = *(struct in_addr *)(bp->b_rptr + uderr->RA_offset);
	in_pcbnotify(&tcb, 0, &sin, uderr->ERROR_type, tcp_errdiscon, 0);
}
