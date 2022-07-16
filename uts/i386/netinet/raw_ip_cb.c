/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:raw_ip_cb.c	1.3"

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
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <netinet/in.h>
#include <net/route.h>
#include <netinet/in_pcb.h>
#include <sys/kmem.h>

#if defined(u3b2) || defined(i386)
#include <netinet/insrem.h>
#endif /* u3b2 */

/*
 * Routines to manage the raw protocol control blocks. 
 *
 * TODO:
 *	hash lookups by protocol family/protocol + address family
 *	take care of unique address problems per AF?
 *	redo address binding to allow wildcards
 */

extern struct	inpcb	rawcb;
/*
 * Allocate a control block and a nominal amount
 * of buffer space for the socket.
 */
rip_attach(q)
	queue_t      *q;
{
	register struct inpcb *inp;

	if (!(inp = (struct inpcb *)kmem_alloc(sizeof(struct inpcb), KM_NOSLEEP)))
		return(ENOSR);
	bzero((char *)inp, sizeof(struct inpcb));
	inp->inp_addrlen = sizeof(struct sockaddr_in);
	inp->inp_family = AF_INET;
	q->q_ptr = WR(q)->q_ptr = (caddr_t)inp;
	insque((struct vq *)inp, (struct vq *)&rawcb);
	return (0);
}

/*
 * Detach the raw connection block and discard
 * socket resources.
 */
rip_detach(inp)
	register struct inpcb *inp;
{
	if (inp->inp_route.ro_rt)
		rtfree(inp->inp_route.ro_rt);
	remque((struct vq *)inp);
	if (inp->inp_options)
		freeb(inp->inp_options);
	kmem_free((char *)inp, sizeof(struct inpcb));
}

/*
 * Disconnect and possibly release resources.
 */
rip_disconnect(inp)
	struct inpcb *inp;
{
	inp->inp_faddr.s_addr = INADDR_ANY;
	inp->inp_fport = 0;

	if (inp->inp_state & SS_NOFDREF)
		rip_detach(inp);
}

rip_bind(inp, addr)
	register struct inpcb *inp;
	struct sockaddr_in *addr;
{

/* BEGIN DUBIOUS */
	/*
	 * Should we verify address not already in use?
	 * Some say yes, others no.
	 */
	if (addr->sin_family == AF_INET || addr->sin_family == htons(AF_INET)) {
		if (addr->sin_addr.s_addr && !prov_withaddr(addr->sin_addr))
			return (EADDRNOTAVAIL);
	} else {
		return (EAFNOSUPPORT);
	}
/* END DUBIOUS */
	inp->inp_laddr = addr->sin_addr;
	inp->inp_family = addr->sin_family;
	return (0);
}

/*
 * Associate a peer's address with a
 * raw connection block.
 */
rip_connaddr(inp, addr)
	struct inpcb *inp;
	struct sockaddr_in *addr;
{
	if (addr->sin_family != AF_INET && addr->sin_family != htons(AF_INET))
		return(EAFNOSUPPORT);
	inp->inp_faddr = addr->sin_addr;
	inp->inp_family = addr->sin_family;
	return(0);
}
