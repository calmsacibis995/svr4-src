/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _NETINET_IN_PCB_H
#define _NETINET_IN_PCB_H

#ident	"@(#)kern-inet:in_pcb.h	1.3"

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
 * Common structure pcb for internet protocol implementation.
 * Here are stored pointers to local and foreign host table
 * entries, local and foreign socket numbers, and pointers
 * up (to a socket structure) and down (to a protocol-specific)
 * control block.
 */
#ifdef STRNET
struct inpcb {
	struct inpcb   *inp_next, *inp_prev;
	/* pointers to other pcb's */
	struct inpcb   *inp_head;	/* pointer back to chain of inpcb's
					 * for this protocol */
	short           inp_state;	/* old so_state from sockets */
	short           inp_tstate;	/* TLI state for this endpoint */
	short           inp_error;	/* error on this pcb */
	short           inp_minor;	/* minor device number allocated */
	queue_t        *inp_q;	/* queue for this minor dev */
	struct in_addr  inp_faddr;	/* foreign host table entry */
	struct in_addr  inp_laddr;	/* local host table entry */
	u_short         inp_fport;	/* foreign port */
	u_short         inp_lport;	/* local port */
#define inp_proto	inp_lport       /* overload port field for protocol */
	caddr_t         inp_ppcb;	/* pointer to per-protocol pcb */
	struct route    inp_route;	/* placeholder for routing entry */
	mblk_t         *inp_options;	/* IP options */
	ushort          inp_protoopt;	/* old so_options from sockets */
	ushort          inp_linger;	/* time to linger while closing */
	ushort          inp_protodef;	/* old pr_flags from sockets */
	ushort		inp_iocstate;	/* state for transparent ioctls */
	int		inp_addrlen;	/* address length client likes */
	int		inp_family;	/* address family client likes */
};
/*
 * inp_iocstate tells us which transparent ioctl we are in the process
 * of handling.	 inp_iocstate is usually set when the M_IOCTL message
 * for a transparent ioctl first seen.	It is used to decide what to do
 * when the subsequent associated M_IOCDATA message(s) arrive.
 * inp_iocstate == 0 means we are not currently processing any
 * transparent ioctls.
 */
#define INP_IOCS_DONAME 1

#else
struct inpcb {
	struct	inpcb *inp_next,*inp_prev;
					/* pointers to other pcb's */
	struct	inpcb *inp_head;	/* pointer back to chain of inpcb's
					   for this protocol */
	struct	in_addr inp_faddr;	/* foreign host table entry */
	u_short	inp_fport;		/* foreign port */
	struct	in_addr inp_laddr;	/* local host table entry */
	u_short	inp_lport;		/* local port */
	struct	socket *inp_socket;	/* back pointer to socket */
	caddr_t	inp_ppcb;		/* pointer to per-protocol pcb */
	struct	route inp_route;	/* placeholder for routing entry */
	struct	mbuf *inp_options;	/* IP options */
};
#endif /* STRNET */

#define	INPLOOKUP_WILDCARD	1
#define	INPLOOKUP_SETLOCAL	2

#define	sotoinpcb(so)	((struct inpcb *)(so)->so_pcb)

#ifdef _KERNEL
#ifdef STRNET
#define qtoinp(q) ((struct inpcb *) (q)->q_ptr)
struct inpcb *inpnewconn();
#endif /* STRNET */
struct	inpcb *in_pcblookup();
#endif

#endif	/* _NETINET_IN_PCB_H */
