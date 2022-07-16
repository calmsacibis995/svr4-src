/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:in_transp.c	1.3"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */


/*
 * This module provides generic handler routines for various transparent
 * ioctls.
 */

#define STRNET

#ifdef INET
#include <netinet/symredef.h>
#endif INET

#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/sysmacros.h>
#include <sys/errno.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/strlog.h>
#include <sys/log.h>
#include <netinet/nihdr.h>
#include <sys/dlpi.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <net/strioc.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <net/route.h>
#include <netinet/ip_str.h>
#include <netinet/ip_var.h>
#include <netinet/in_pcb.h>
#ifdef SYSV
#include <sys/cmn_err.h>
#endif SYSV
#include <sys/kmem.h>
#include <sys/timod.h>


inet_doname(q, bp)
	queue_t		*q;
	mblk_t		*bp;
{
	struct inpcb *inp = qtoinp(q);
	struct sockaddr_in localaddr;
	struct sockaddr_in remoteaddr;

	if (inp == (struct inpcb *) 0) {
		/* strlog this */
		return;
	}

	bzero((caddr_t) &localaddr, sizeof(localaddr));
	bzero((caddr_t) &remoteaddr, sizeof(remoteaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr = inp->inp_laddr;
	localaddr.sin_port = inp->inp_lport;
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr = inp->inp_faddr;
	remoteaddr.sin_port = inp->inp_fport;

	switch (bp->b_datap->db_type) {
	case M_IOCTL:
		inp->inp_iocstate = INP_IOCS_DONAME;
		if (ti_doname(q, bp, (caddr_t) &localaddr, sizeof (localaddr),
			      (caddr_t) &remoteaddr, sizeof (remoteaddr))
		    != DONAME_CONT)
			inp->inp_iocstate = 0;
		break;
			

	case M_IOCDATA:
		if (ti_doname(q, bp, (caddr_t) &localaddr, sizeof (localaddr),
			      (caddr_t) &remoteaddr, sizeof (remoteaddr))
		    != DONAME_CONT)
			inp->inp_iocstate = 0;
		break;
	}
}


