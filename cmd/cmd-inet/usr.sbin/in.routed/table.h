/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/in.routed/table.h	1.1.2.1"

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
 * Routing table management daemon.
 */

/*
 * Routing table structure; differs a bit from kernel tables.
 *
 * Note: the union below must agree in the first 4 members
 * so the ioctl's will work.
 */
struct rthash {
	struct	rt_entry *rt_forw;
	struct	rt_entry *rt_back;
};

struct rt_entry {
	struct	rt_entry *rt_forw;
	struct	rt_entry *rt_back;
	union {
		struct	rtentry rtu_rt;
		struct {
			u_long	rtu_hash;
			struct	sockaddr rtu_dst;
			struct	sockaddr rtu_router;
			short	rtu_flags;
			short	rtu_state;
			int	rtu_timer;
			int	rtu_metric;
			struct	interface *rtu_ifp;
		} rtu_entry;
	} rt_rtu;
};

#define	rt_rt		rt_rtu.rtu_rt			/* pass to ioctl */
#define	rt_hash		rt_rtu.rtu_entry.rtu_hash	/* for net or host */
#define	rt_dst		rt_rtu.rtu_entry.rtu_dst	/* match value */
#define	rt_router	rt_rtu.rtu_entry.rtu_router	/* who to forward to */
#define	rt_flags	rt_rtu.rtu_entry.rtu_flags	/* kernel flags */
#define	rt_timer	rt_rtu.rtu_entry.rtu_timer	/* for invalidation */
#define	rt_state	rt_rtu.rtu_entry.rtu_state	/* see below */
#define	rt_metric	rt_rtu.rtu_entry.rtu_metric	/* cost of route */
#define	rt_ifp		rt_rtu.rtu_entry.rtu_ifp	/* interface to take */

#define	ROUTEHASHSIZ	32		/* must be a power of 2 */
#define	ROUTEHASHMASK	(ROUTEHASHSIZ - 1)

/*
 * "State" of routing table entry.
 */
#define	RTS_CHANGED	0x1		/* route has been altered recently */
#define	RTS_EXTERNAL	0x2		/* extern info, not installed or sent */
#define	RTS_INTERNAL	0x4		/* internal route, not installed */
#define	RTS_PASSIVE	IFF_PASSIVE	/* don't time out route */
#define	RTS_INTERFACE	IFF_INTERFACE	/* route is for network interface */
#define	RTS_REMOTE	IFF_REMOTE	/* route is for ``remote'' entity */
#define	RTS_SUBNET	IFF_SUBNET	/* route is for network subnet */

/*
 * Flags are same as kernel, with this addition for af_rtflags:
 */
#define	RTF_SUBNET	0x8000		/* pseudo: route to subnet */

struct	rthash nethash[ROUTEHASHSIZ];
struct	rthash hosthash[ROUTEHASHSIZ];
struct	rt_entry *rtlookup();
struct	rt_entry *rtfind();
