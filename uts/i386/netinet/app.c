/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:app.c	1.3.1.1"

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

/*
 * Ethernet Address Resolution Protocol (ARP).  This program actually
 * consists of a streams driver ARP and module APP.  ARP implements the
 * Address Resolution Protocol itself using broadcasts and direct responses
 * to talk to other ARP implementations on the local net. APP functions
 * as the convergence module between an IP layer and an ethernet driver using
 * the link level interface based on LLC1.  Note that both sides can
 * understand arp ioctls.  It is recommended that users wishing to manipulate
 * the arp tables do so through the ARP side, but ioctls are allowed in
 * APP so that programs based on the sockets paradigm can manipulate
 * these tables through the higher level. 
 *
 * On initialization an ARP driver gets an ethernet interface linked below it.
 * The slink process should also have linked a corresponding APP module
 * between the same ethernet module and a bottom queue of IP.  These two then
 * function in tandem by sharing the arpcom structure.  If arpresolve can't
 * find a destination in the arp table, it will call arpwhohas, which will do
 * the transmission.  When a response comes in, the saved output packet will
 * be transmitted. 
 */

#define STRNET
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/sockio.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/log.h>
#include <sys/strlog.h>
#include <sys/dlpi.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/strioc.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <net/route.h>
#include <netinet/ip_str.h>
#include <netinet/arp.h>

extern int      arpinited;

/*
 * The first section implements the stream interface to app 
 */

int             appopen(), appclose(), appwput(), apprput();

static struct module_info appm_info[MODL_INFO_SZ] = {
	APPM_ID, "app", 0, 8192, 8192, 1024,
	APPM_ID, "app", 0, 8192, 8192, 1024
};

static struct qinit apprinit =
{apprput, NULL, appopen, appclose, NULL, &appm_info[IQP_RQ], NULL};

static struct qinit appwinit =
{appwput, NULL, appopen, appclose, NULL, &appm_info[IQP_WQ], NULL};

struct streamtab appinfo = {&apprinit, &appwinit, NULL, NULL};

struct app_pcb app_pcb[N_ARP];
extern struct arp_pcb arp_pcb[];

/*
 * These are the stream module routines for APP processing 
 */

/* ARGSUSED */
appopen(q, dev, flag, sflag)
	queue_t        *q;
{
	STRLOG(APPM_ID, 0, 9, SL_TRACE, "app open called");
	if (!arpinited)
		arpinit();
	dev = minor(dev);
	if (sflag == MODOPEN) {
		for (dev = 0; dev < N_ARP; dev++) {
			if (app_pcb[dev].app_q == NULL) {
				break;
			}
		}
	}
	if (dev < 0 || dev >= N_ARP) {
		setuerror(ENXIO);
		return (OPENFAIL);
	}

	if (app_pcb[dev].app_q == NULL) {
		app_pcb[dev].app_q = q;
		q->q_ptr = (char *) &app_pcb[dev];
		WR(q)->q_ptr = (char *) &app_pcb[dev];
	} else if (q != app_pcb[dev].app_q) {
		setuerror(EBUSY);
		return (OPENFAIL);
	}
	STRLOG(APPM_ID, 0, 9, SL_TRACE, "open succeeded");
	return (0);
}

appclose(q)
	queue_t        *q;
{
	struct app_pcb *ap;

	STRLOG(APPM_ID, 0, 9, SL_TRACE, "app close called");
	ap = (struct app_pcb *) q->q_ptr;
	ap->app_q = NULL;
	if (ap->arp_pcb) {
		ap->arp_pcb->app_pcb = NULL;
		ap->arp_pcb = NULL;
	}
	ap->app_uname[0] = NULL;
	ap->app_ac.ac_ipaddr.s_addr = 0;
	flushq(WR(q), 1);
	STRLOG(APPM_ID, 0, 9, SL_TRACE, "close succeeded");
	return;
}


appwput(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{

	STRLOG(APPM_ID, 0, 9, SL_TRACE,
	       "appuwput: received strbufs from above");

	switch (bp->b_datap->db_type) {

	case M_IOCTL:
		appioctl(q, bp);
		break;

	case M_PROTO:
	case M_PCPROTO:
		STRLOG(APPM_ID, 0, 9, SL_TRACE, "passing data through app");
		app_doproto(q, bp);
		break;

	case M_CTL:
		freemsg(bp);
		break;

	case M_FLUSH:
		if (*bp->b_rptr & FLUSHW)
			flushq(q, FLUSHDATA);
		if (*bp->b_rptr & FLUSHR)
			flushq(RD(q), FLUSHDATA);
		putnext(q, bp);
		break;

	default:
		STRLOG(APPM_ID, 0, 5, SL_ERROR,
		       "app: unexpected type received in wput: %d.\n",
		       bp->b_datap->db_type);
		freemsg(bp);
		break;
	}
}


appioctl(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	struct iocblk  *iocbp;
	struct ifreq   *ifr;
	struct app_pcb *ap;
	int             i;

	ap = (struct app_pcb *) q->q_ptr;
	iocbp = (struct iocblk *) bp->b_rptr;
	if (msgblen(bp) >= sizeof (struct iocblk_in))
		/* It probably came from from IP.  Pass our flags back up. */
		((struct iocblk_in *) iocbp)->ioc_ifflags |= IFF_BROADCAST;

	switch (iocbp->ioc_cmd) {

	case SIOCSIFNAME:
		ifr = (struct ifreq *) bp->b_cont->b_rptr;
		strcpy(ap->app_uname, ifr->ifr_name);
		for (i = 0; i < N_ARP; i++) {
			if (arp_pcb[i].arp_qbot &&
			    !strcmp(arp_pcb[i].arp_uname, ap->app_uname)) {
				arp_pcb[i].app_pcb = ap;
				ap->arp_pcb = &arp_pcb[i];
			}
		}
		break;

	case SIOCSIFFLAGS:
		ifr = (struct ifreq *) bp->b_cont->b_rptr;
		ap->app_ac.ac_if.if_flags = ifr->ifr_flags;
		break;

	case SIOCSIFADDR:
		ifr = (struct ifreq *) bp->b_cont->b_rptr;
		ap->app_ac.ac_ipaddr = *SOCK_INADDR(&ifr->ifr_addr);
		break;

	case SIOCGIFMETRIC:
	case SIOCSIFMETRIC:
	case SIOCGIFADDR:
	case SIOCGIFNETMASK:
	case SIOCSIFNETMASK:
	case SIOCGIFBRDADDR:
	case SIOCSIFBRDADDR:
	case SIOCGIFDSTADDR:
	case SIOCSIFDSTADDR:
	case SIOCIFDETACH:
		break;

		/*
		 * This will be acked from enet level. 
		 */

	case SIOCGIFFLAGS:
	case IF_UNITSEL:
		putnext(q, bp);
		return;

	case SIOCSARP:
	case SIOCDARP:
	case SIOCGARP:
		arpioctl(q, bp);
		return;

	case INITQPARMS:
		if (iocbp->ioc_error = initqparms(bp, appm_info, MODL_INFO_SZ))
			bp->b_datap->db_type = M_IOCNAK;
		else
			bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = 0;
		qreply(q, bp);
		return;

	default:
		/*
		 * Send everything else downstream. 
		 */
		putnext(q, bp);
		return;
	}
	bp->b_datap->db_type = M_IOCACK;
	qreply(q, bp);
}

app_doproto(q, bp)
	queue_t		*q;
	mblk_t		*bp;
{
	union DL_primitives	*prim;
	dl_unitdata_req_t	*req;
	mblk_t         	*tempbp;
	int		temp;
	ether_addr_t	enaddr;
	struct app_pcb 	*ap = (struct app_pcb *) q->q_ptr;
	int		addrlen = ap->app_ac.ac_addrlen;

	prim = (union DL_primitives *) bp->b_rptr;

	switch (prim->dl_primitive) {
	case DL_UNITDATA_REQ:
		req = (dl_unitdata_req_t *) bp->b_rptr;
		if (arpresolve(ap, bp, (char *) &enaddr[0], &temp)
		    == 0) {
			break;
		}
		if (bpsize(bp) < sizeof(dl_unitdata_req_t) +
		    addrlen) {
			tempbp = allocb(sizeof(dl_unitdata_req_t) +
					addrlen, BPRI_HI);
			if (tempbp == NULL) {
				freemsg(bp);
				break;
			}
			tempbp->b_cont = bp->b_cont;
			freeb(bp);
			bp = tempbp;
			bp->b_datap->db_type = M_PROTO;
			req = (dl_unitdata_req_t *) bp->b_rptr;
			req->dl_primitive = DL_UNITDATA_REQ;
		}
		bp->b_wptr = bp->b_rptr +
			sizeof(dl_unitdata_req_t) + addrlen;
		req->dl_dest_addr_length = addrlen;
		req->dl_dest_addr_offset = sizeof(dl_unitdata_req_t);
		bcopy((caddr_t) &enaddr[0],
		      (caddr_t) bp->b_rptr + sizeof(dl_unitdata_req_t),
		      sizeof(enaddr));
		/* if addrlen is 8 (ethernet address length+2), copy in type */
		if (addrlen == 8)
			*((ushort *) (bp->b_wptr - 2)) = htons(IP_SAP);
		/* fall through ... */

	default:
		putnext(q, bp);
		break;
	}
}

/*
 * Upstream put routine. 
 */

apprput(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	struct app_pcb *ap = (struct app_pcb *) q->q_ptr;

	switch (bp->b_datap->db_type) {

	case M_PROTO:
	case M_PCPROTO:
		switch (((union DL_primitives *) bp->b_rptr)->dl_primitive) {
		case DL_BIND_ACK:{
				dl_bind_ack_t *b_ack;

				/* on a bind ack, save the ethernet address */
				b_ack = (dl_bind_ack_t *) bp->b_rptr;
				bcopy((caddr_t) bp->b_rptr + 
				      b_ack->dl_addr_offset,
				      (caddr_t) &ap->app_ac.ac_enaddr,
				      sizeof(ap->app_ac.ac_enaddr));
				break;
			}

		case DL_INFO_ACK: {
			dl_info_ack_t *info_ack =
				(dl_info_ack_t *) bp->b_rptr;
			ap->app_ac.ac_mintu = info_ack->dl_min_sdu;
			ap->app_ac.ac_addrlen = info_ack->dl_addr_length;
			break;
		}
		}
		break;

	case M_FLUSH:
		if (*bp->b_rptr & FLUSHR)
			flushq(q, FLUSHDATA);
		break;
	case M_CTL:
		freemsg(bp);
		return;		/* don't send this message upstream */

	case M_IOCNAK:
		/*
		 * must intercept nak's of SIOCGIFFLAGS
		 * since some drivers may not handle ip-specific ioctls
		 */
		if ( ((struct iocblk *)bp->b_rptr)->ioc_cmd == SIOCGIFFLAGS )
			bp->b_datap->db_type = M_IOCACK;
		break;
	default:
		break;
	}
	STRLOG(ARPM_ID, 0, 9, SL_TRACE, "app passing data up stream");
	putnext(q, bp);
}
