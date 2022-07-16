/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	_NETINET_IF_ETHER_H
#define	_NETINET_IF_ETHER_H

#ident	"@(#)kern-inet:if_ether.h	1.3"

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
 * The following include is for compatibility with SunOS 3.x and
 * 4.3bsd.  Newly written programs should include it separately.
 */
#include <net/if_arp.h>

/*
 * Ethernet address - 6 octets
 */
typedef u_char ether_addr_t[6];

/*
 * Structure of a 10Mb/s Ethernet header.
 */
struct	ether_header {
	ether_addr_t ether_dhost;
	ether_addr_t ether_shost;
	u_short	ether_type;
};

#define	ETHERTYPE_PUP		0x0200		/* PUP protocol */
#define	ETHERTYPE_IP		0x0800		/* IP protocol */
#define	ETHERTYPE_ARP		0x0806		/* Addr. resolution protocol */
#define	ETHERTYPE_REVARP	0x8035		/* Reverse ARP */

/*
 * The ETHERTYPE_NTRAILER packet types starting at ETHERTYPE_TRAIL have
 * (type-ETHERTYPE_TRAIL)*512 bytes of data followed
 * by an ETHER type (as given above) and then the (variable-length) header.
 */
#define	ETHERTYPE_TRAIL		0x1000		/* Trailer packet */
#define	ETHERTYPE_NTRAILER	16

#define	ETHERMTU	1500
#define	ETHERMIN	(60-14)

/*
 * Ethernet Address Resolution Protocol.
 *
 * See RFC 826 for protocol description.  Structure below is adapted
 * to resolving internet addresses.  Field names used correspond to
 * RFC 826.
 */
struct	ether_arp {
	struct	arphdr ea_hdr;		/* fixed-size header */
	ether_addr_t arp_sha;		/* sender hardware address */
	u_char	arp_spa[4];		/* sender protocol address */
	ether_addr_t arp_tha;		/* target hardware address */
	u_char	arp_tpa[4];		/* target protocol address */
};
#define	arp_hrd	ea_hdr.ar_hrd
#define	arp_pro	ea_hdr.ar_pro
#define	arp_hln	ea_hdr.ar_hln
#define	arp_pln	ea_hdr.ar_pln
#define	arp_op	ea_hdr.ar_op

/*
 *      multicast address structure
 *
 *      Keep a reference count for each multicast address so
 *      addresses loaded into chip are unique.
 */
struct  mcaddr {
        ether_addr_t mc_enaddr;			/* multicast address */
        u_short mc_count;                       /* reference count */
};
#define MCADDRMAX       64              /* multicast addr table length */
#define MCCOUNTMAX      4096            /* multicast addr max reference count */

/*
 * Structure shared between the ethernet driver modules and
 * the address resolution code.  For example, each ec_softc or il_softc
 * begins with this structure.
 *
 * The structure contains a pointer to an array of multicast addresses.
 * This pointer is NULL until the first successful SIOCADDMULTI ioctl
 * is issued for the interface.
 */
struct	arpcom {
	struct		ifnet ac_if;		/* network-visible interface */
	ether_addr_t	ac_enaddr;		/* ethernet hardware address */
	struct in_addr	ac_ipaddr;		/* copy of ip address- XXX */
	struct mcaddr	*ac_mcaddr;		/* table of multicast addrs */
	u_short		ac_nmcaddr;		/* count of M/C addrs in use */
	struct in_addr	ac_lastip;      	/* cache of last ARP lookup */
	ether_addr_t	ac_lastarp;		/* result of the last ARP */
	int		ac_mintu;		/* minimum transfer unit */
	int		ac_addrlen;		/* length of address */
};

/*
 * Internet to ethernet address resolution table.
 */
struct	arptab {
	struct	in_addr at_iaddr;	/* internet address */
	union {
	    ether_addr_t atu_enaddr;	/* ethernet address */
	    long   atu_tvsec;			/* timestamp if incomplete */
	} 	at_union;
	u_char	at_timer;		/* minutes since last reference */
	u_char	at_flags;		/* flags */
#ifdef STRNET
	mblk_t	*at_hold;	/* last packet until resolved/timeout */
#else
	struct	mbuf *at_hold;	/* last packet until resolved/timeout */
#endif STRNET
};

# define at_enaddr at_union.atu_enaddr
# define at_tvsec at_union.atu_tvsec

/*
 * Compare two Ethernet addresses - assumes that the two given
 * pointers can be referenced as shorts.  On architectures
 * where this is not the case, use bcmp instead.  Note that like
 * bcmp, we return zero if they are the SAME.
 */
#if defined(sun2) || defined(sun3) || defined(sun3x)
/*
 * On 680x0 machines, we can do a longword compare that is NOT
 * longword aligned, as long as it is even aligned.
 */
#define ether_cmp(a,b) ( ((short *)a)[2] != ((short *)b)[2] || \
  *((long *)a) != *((long *)b) )
#endif

/*
 * On a sparc, functions are FAST
 */
#if defined(sparc)
#define ether_cmp(a,b) (sparc_ether_cmp((short *)a, (short *)b))
#endif 

#ifndef ether_cmp
#define ether_cmp(a,b) (bcmp((caddr_t)a,(caddr_t)b, 6))
#endif

/*
 * Copy Ethernet addresses from a to b - assumes that the two given
 * pointers can be referenced as shorts.  On architectures
 * where this is not the case, use bcopy instead.
 */
#if defined(sun2) || defined(sun3) || defined(sun3x)
#define ether_copy(a,b) { ((long *)b)[0]=((long *)a)[0]; \
 ((short *)b)[2]=((short *)a)[2]; }
#endif

#if defined(sparc)
#define ether_copy(a,b) { ((short *)b)[0]=((short *)a)[0]; \
 ((short *)b)[1]=((short *)a)[1]; ((short *)b)[2]=((short *)a)[2]; }
#endif

#ifndef ether_copy
#define ether_copy(a,b) (bcopy((caddr_t)a,(caddr_t)b, 6))
#endif

/*
 * Copy IP addresses from a to b - assumes that the two given
 * pointers can be referenced as shorts.  On architectures
 * where this is not the case, use bcopy instead.
 */
#if defined(sun2) || defined(sun3) || defined(sun3x)
#define ip_copy(a,b) { *((long *)b) = *((long *)a); }
#endif

#if defined(sparc)
#define ip_copy(a,b) { ((short *)b)[0]=((short *)a)[0]; \
 ((short *)b)[1]=((short *)a)[1]; }
#endif

#ifndef ip_copy
#define ip_copy(a,b) (bcopy((caddr_t)a,(caddr_t)b, 4))
#endif

#ifdef	_KERNEL
ether_addr_t etherbroadcastaddr;
struct	arptab *arptnew();
char *ether_sprintf();
#endif	/* _KERNEL */

#endif	/* _NETINET_IF_ETHER_H */
