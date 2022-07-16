/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:tcp_debug.c	1.3"

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


#define TCPSTATES
#define	TCPTIMERS
#define	TANAMES
#define PRUREQUESTS
#define TLI_PRIMS

#define STRNET

#ifdef INET
#include <netinet/symredef.h>
#endif INET

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/stream.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <sys/errno.h>
#ifdef SYSV
#include <sys/cmn_err.h>
#endif

#include <net/route.h>
#include <net/if.h>

#include <netinet/in.h>
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

int	tcpconsdebug = 0;	/* send debug printfs to console if set */
int	tcpalldebug = 0;	/* trace all connections if set		*/
int	tcp_debx;

/*
 * Tcp debug routines
 */
/*ARGSUSED*/
tcp_trace(act, ostate, tp, ti, req)
	short act, ostate;
	struct tcpcb *tp;
	struct tcpiphdr *ti;
	int req;
{
	tcp_seq seq, ack;
	int len, flags;

	if (tcp_ndebug > 0 ) {
		struct tcp_debug *td;

		if (tcp_debx == tcp_ndebug)
			tcp_debx = 0;
		td = &tcp_debug[tcp_debx++];
		td->td_time = iptime();
		td->td_act = act;
		td->td_ostate = ostate;
		td->td_tcb = (caddr_t)tp;
		if (tp)
			td->td_cb = *tp;
		else
			bzero((caddr_t)&td->td_cb, sizeof (*tp));
		if (ti)
			td->td_ti = *ti;
		else
			bzero((caddr_t)&td->td_ti, sizeof (*ti));
		td->td_req = req;
	}
	if (tcpconsdebug == 0)
		return;
	if (tp)
#ifdef SYSV
		cmn_err(CE_CONT,"%x %s:", tp, tcpstates[ostate]);
#else
		printf("%x %s:", tp, tcpstates[ostate]);
#endif
	else
#ifdef SYSV
		cmn_err(CE_CONT,"???????? ");
#else
		printf("???????? ");
#endif
#ifdef SYSV
	cmn_err(CE_CONT,"%s ", tanames[act]);
#else
	printf("%s ", tanames[act]);
#endif
	switch (act) {

	case TA_INPUT:
	case TA_OUTPUT:
	case TA_DROP:
		if (ti == 0)
			break;
		seq = ti->ti_seq;
		ack = ti->ti_ack;
		len = ti->ti_len;
		if (act == TA_OUTPUT) {
			seq = ntohl(seq);
			ack = ntohl(ack);
			len = ntohs((u_short)len);
		}
		if (act == TA_OUTPUT)
			len -= sizeof (struct tcphdr);
		if (len)
#ifdef SYSV
			cmn_err(CE_CONT,"[%lx..%lx]", seq, seq + len);
#else
			printf("[%lx..%lx]", seq, seq + len);
#endif
		else
#ifdef SYSV
			cmn_err(CE_CONT,"%lx", seq);
#else
			printf("%lx", seq);
#endif
#ifdef SYSV
		cmn_err(CE_CONT,"@%lx, urp=%lx", ack, ti->ti_urp);
#else
		printf("@%lx, urp=%lx", ack, ti->ti_urp);
#endif
		flags = ti->ti_flags;
		if (flags) {
#ifndef lint
			char           *cp = "<";
#ifdef SYSV
#define pf(f) { if (ti->ti_flags & f) { cmn_err(CE_CONT,"%s%s", cp, "f"); cp = ","; } }
#else
#define pf(f) { if (ti->ti_flags & f) { printf("%s%s", cp, "f"); cp = ","; } }
#endif
			pf(TH_SYN);
			pf(TH_ACK);
			pf(TH_FIN);
			pf(TH_RST);
			pf(TH_PUSH);
			pf(TH_URG);
#endif
#ifdef SYSV
			cmn_err(CE_CONT,">");
#else
			printf(">");
#endif
		}
		break;

	case TA_USER:
#ifdef SYSV
		cmn_err(CE_CONT,"%s", tli_primitives[req & 0xff]);
#else
		printf("%s", tli_primitives[req & 0xff]);
#endif
		break;
	case TA_TIMER:
#ifdef SYSV
		cmn_err(CE_CONT,"<%s>", tcptimers[req]);
#else
		printf("<%s>", tcptimers[req]);
#endif
		break;
	default:
		break;
	}
	if (tp)
		if (tp->t_state > TCP_NSTATES || tp->t_state < 0) {
#ifdef SYSV
			cmn_err(CE_CONT," -> Bad State (%d)", tp->t_state);
#else
			printf(" -> Bad State (%d)", tp->t_state);
#endif
		} else {
#ifdef SYSV
			cmn_err(CE_CONT," -> %s", tcpstates[tp->t_state]);
#else
			printf(" -> %s", tcpstates[tp->t_state]);
#endif
		}
	/* print out internal state of tp !?! */
#ifdef SYSV
	cmn_err(CE_CONT,"\n");
#else
	printf("\n");
#endif
	if (tp == 0)
		return;
#ifdef SYSV
	cmn_err(CE_CONT,"\trcv_(nxt,wnd,up) (%lx,%lx,%x) snd_(una,nxt,max) (%lx,%lx,%lx)\n", tp->rcv_nxt, tp->rcv_wnd, tp->rcv_up, tp->snd_una, tp->snd_nxt, tp->snd_max);
#else
	printf("\trcv_(nxt,wnd,up) (%lx,%lx,%x) snd_(una,nxt,max) (%lx,%lx,%lx)\n", tp->rcv_nxt, tp->rcv_wnd, tp->rcv_up, tp->snd_una, tp->snd_nxt, tp->snd_max);
#endif
#ifdef SYSV
	cmn_err(CE_CONT,"\tsnd_(wl1,wl2,wnd) (%x,%x,%x) snd_cwnd %x\n", tp->snd_wl1, tp->snd_wl2, tp->snd_wnd, tp->snd_cwnd);
#else
	printf("\tsnd_(wl1,wl2,wnd) (%x,%x,%x) snd_cwnd %x\n", tp->snd_wl1, tp->snd_wl2, tp->snd_wnd, tp->snd_cwnd);
#endif
}
