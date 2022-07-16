/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:arp.h	1.1"

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
 * pcb's shared by app module and arp driver
 */

#define	N_ARP	10

struct app_pcb {
	queue_t        *app_q;	/* read pointer for our queue */
	struct arp_pcb *arp_pcb;/* cross pointer to our arp module */
	char            app_uname[IFNAMSIZ];	/* enet unit name */
	struct arpcom   app_ac;	/* common structure for this unit */
};

struct arp_pcb {
	queue_t        *arp_qtop;	/* upstream read queue */
	queue_t        *arp_qbot;	/* downstream write queue */
	int             arp_index;	/* mux index for link */
	struct app_pcb *app_pcb;/* cross pointer to app module */
	char            arp_uname[IFNAMSIZ];	/* enet unit name */
	mblk_t         *arp_saved;	/* saved input request */
	int		arp_flags;	/* flags */
};
