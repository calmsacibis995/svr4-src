/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbsendmail:include/netconfig.h	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
 * netconfig.h
 * For network selection
 */
#ifndef _RPC_NETCONFIG_H
#define _RPC_NETCONFIG_H

#define NETCONFIG "/etc/netconfig"
#define NETPATH   "NETPATH"
#define NETPATHLEN 256

/*
 * Values of nc_semantics
 */
#define NC_TPI_CLTS	1
#define NC_TPI_COTS	2
#define NC_TPI_COTS_ORD	3

/*
 * Values of nc_flag
 */
#define NC_NOFLAG	0x0
#define NC_VISIBLE	0x1
#define NC_BROADCAST	0x2
#define NC_TRANSPORT	0x4
#define NC_SESSION	0x8
#define NC_PRESENTATION	0x10
#define NC_APPLICATION	0x11

/*
 * Values of nc_protofmly
 */
#define NC_NOPROTOFMLY	0
#define NC_LOOPBACK	1
#define NC_INET		2
#define NC_IMPLINK	3
#define NC_PUP		4
#define NC_CHAOS	5
#define NC_NS		6
#define NC_NBS		7
#define NC_ECMA		8
#define NC_DATAKIT	9
#define NC_CCITT	10
#define NC_SNA		11
#define NC_DECNET	12
#define NC_DLI		13
#define NC_LAT		14
#define NC_HYLINK	15
#define NC_APPLETALK	16
#define NC_NIT		17
#define NC_IEEE802	18
#define NC_OSI		19
#define NC_X25		20
#define NC_OSINET	21
#define NC_GOSIP	22

/*
 * Values for nc_proto
 */
#define NC_NOPROTO	0
#define NC_TCP		1
#define NC_UDP		2

struct netconfig {
	char *nc_netid;			/* token name */
	unsigned long nc_semantics;	/* type of transport */
	unsigned long nc_flag;		/* some flags */
	unsigned long nc_protofmly;	/* protocol family */
	unsigned long nc_proto;		/* protocol */
	char         *nc_device;	/* device entry */
	unsigned long nc_nlookups;	/* number of lookup routines */
	char        **nc_lookups;	/* look up routines */
	unsigned long nc_unused[8];
};

/*
 * Sample netconfig file
 * tcp tpi_cots tv inet tcp /dev/tcp mumbojumbo
 * udp tpi_clts tv inet udp /dev/udp mumbojumbo
 */

extern int setnetconfig();
extern struct netconfig *getnetconfig();
extern int endnetconfig();
extern int setnetpath();
extern struct netconfig *getnetpath();
extern struct netconfig *getnetconfigent();
extern int endnetpath();
extern void freenetconfigent();

#endif /* _RPC_NETCONFIG_H */
