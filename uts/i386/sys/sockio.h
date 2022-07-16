/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	_SYS_SOCKIO_H
#define	_SYS_SOCKIO_H

#ident	"@(#)head.sys:sys/sockio.h	1.8.2.1"

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
 * General socket ioctl definitions.
 */

#include <sys/ioccom.h>

/* socket i/o controls */
#define	SIOCSHIWAT	_IOW('s',  0, int)		/* set high watermark */
#define	SIOCGHIWAT	_IOR('s',  1, int)		/* get high watermark */
#define	SIOCSLOWAT	_IOW('s',  2, int)		/* set low watermark */
#define	SIOCGLOWAT	_IOR('s',  3, int)		/* get low watermark */
#define	SIOCATMARK	_IOR('s',  7, int)		/* at oob mark? */
#define	SIOCSPGRP	_IOW('s',  8, int)		/* set process group */
#define	SIOCGPGRP	_IOR('s',  9, int)		/* get process group */

#define	SIOCADDRT	_IOW('r', 10, struct rtentry)	/* add route */
#define	SIOCDELRT	_IOW('r', 11, struct rtentry)	/* delete route */

#define	SIOCSIFADDR	_IOW('i', 12, struct ifreq)	/* set ifnet address */
#define	SIOCGIFADDR	_IOWR('i',13, struct ifreq)	/* get ifnet address */
#define	SIOCSIFDSTADDR	_IOW('i', 14, struct ifreq)	/* set p-p address */
#define	SIOCGIFDSTADDR	_IOWR('i',15, struct ifreq)	/* get p-p address */
#define	SIOCSIFFLAGS	_IOW('i', 16, struct ifreq)	/* set ifnet flags */
#define	SIOCGIFFLAGS	_IOWR('i',17, struct ifreq)	/* get ifnet flags */
#define	SIOCSIFMEM	_IOW('i', 18, struct ifreq)	/* set interface mem */
#define	SIOCGIFMEM	_IOWR('i',19, struct ifreq)	/* get interface mem */
#define	SIOCGIFCONF	_IOWR('i',20, struct ifconf)	/* get ifnet list */
#define	SIOCSIFMTU	_IOW('i', 21, struct ifreq)	/* set if_mtu */
#define	SIOCGIFMTU	_IOWR('i',22, struct ifreq)	/* get if_mtu */

	/* from 4.3BSD */
#define	SIOCGIFBRDADDR	_IOWR('i',23, struct ifreq)	/* get broadcast addr */
#define	SIOCSIFBRDADDR	_IOW('i',24, struct ifreq)	/* set broadcast addr */
#define	SIOCGIFNETMASK	_IOWR('i',25, struct ifreq)	/* get net addr mask */
#define	SIOCSIFNETMASK	_IOW('i',26, struct ifreq)	/* set net addr mask */
#define	SIOCGIFMETRIC	_IOWR('i',27, struct ifreq)	/* get IF metric */
#define	SIOCSIFMETRIC	_IOW('i',28, struct ifreq)	/* set IF metric */

#define	SIOCSARP	_IOW('i', 30, struct arpreq)	/* set arp entry */
#define	SIOCGARP	_IOWR('i',31, struct arpreq)	/* get arp entry */
#define	SIOCDARP	_IOW('i', 32, struct arpreq)	/* delete arp entry */
#define	SIOCUPPER       _IOW('i', 40, struct ifreq)       /* attach upper layer */
#define	SIOCLOWER       _IOW('i', 41, struct ifreq)       /* attach lower layer */
#define	SIOCSETSYNC	_IOW('i',  44, struct ifreq)	/* set syncmode */
#define	SIOCGETSYNC	_IOWR('i', 45, struct ifreq)	/* get syncmode */
#define	SIOCSSDSTATS	_IOWR('i', 46, struct ifreq)	/* sync data stats */
#define	SIOCSSESTATS	_IOWR('i', 47, struct ifreq)	/* sync error stats */

#define	SIOCSPROMISC	_IOW('i', 48, int)		/* request promisc mode
							   on/off */
#define	SIOCADDMULTI	_IOW('i', 49, struct ifreq)	/* set m/c address */
#define	SIOCDELMULTI	_IOW('i', 50, struct ifreq)	/* clr m/c address */

/* protocol i/o controls */
#define	SIOCSNIT	_IOW('p',  0, struct nit_ioc)	/* set nit modes */
#define	SIOCGNIT	_IOWR('p', 1, struct nit_ioc)	/* get nit modes */

/* STREAMS based socket emulation */

#define SIOCPROTO	_IOW('s', 51, struct socknewproto)	/* link proto */
#define SIOCGETNAME	_IOR('s', 52, struct sockaddr)	/* getsockname */
#define SIOCGETPEER	_IOR('s', 53, struct sockaddr)	/* getpeername */
#define IF_UNITSEL	_IOW('s', 54, int)	/* set unit number */
#define SIOCXPROTO	_IO('s', 55)	/* empty proto table */

#define SIOCIFDETACH	_IOW('i', 56, struct ifreq)	/* detach interface */
#define SIOCGENPSTATS	_IOWR('i', 57, struct ifreq)	/* get ENP stats */
#define SIOCX25XMT	_IOWR('i', 59, struct ifreq)	/* start a slp proc in
							 * x25if */
#define SIOCX25RCV	_IOWR('i', 60, struct ifreq)	/* start a slp proc in
							 * x25if */
#define SIOCX25TBL	_IOWR('i', 61, struct ifreq)	/* xfer lun table to
							 * kernel */
#define SIOCSLGETREQ	_IOWR('i', 71, struct ifreq)	/* wait for switched
							 * SLIP request */
#define SIOCSLSTAT	_IOW('i', 72, struct ifreq)	/* pass SLIP info to
							 * kernel */
#define SIOCSIFNAME	_IOW('i', 73, struct ifreq)	/* set interface name */
#define SIOCGENADDR	_IOWR('i', 85, struct ifreq)	/* Get ethernet addr */
#define SIOCSOCKSYS	_IOW('i', 86, struct socksysreq)	/* Pseudo socket syscall */

#endif	/* _SYS_SOCKIO_H */
