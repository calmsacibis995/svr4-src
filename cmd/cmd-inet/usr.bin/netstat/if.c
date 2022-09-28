/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.bin/netstat/if.c	1.1.3.1"

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
 *
 * Copyright 1987, 1988 Lachman Associates, Incorporated (LAI) All Rights Reserved. 
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

#include <sys/types.h>
#include <sys/socket.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_var.h>

#include <stdio.h>

extern int      kmem;
extern int      nflag;
extern char    *interface;
extern int      unit;
extern char    *routename(), *netname();


/*
 * Print a description of the network interfaces. 
 */
intpr(interval, ifnetaddr)
	int             interval;
	off_t           ifnetaddr;
{
	struct ifstats  ifstats;
	union {
		struct ifaddr   ifa;
		struct in_ifaddr in;
	}               ifaddr;
	off_t           ifaddraddr;
	char            name[16];

	if (ifnetaddr == 0) {
		printf("ifstats: symbol not defined\n");
		return;
	}
	if (interval) {
		sidewaysintpr(interval, ifnetaddr);
		return;
	}
	readmem(ifnetaddr, 1, 0, &ifnetaddr, sizeof ifnetaddr,
		"ifstats chain");
	printf("%-6.6s %-5.5s %-10.10s  %-12.12s %-7.7s %-5.5s %-7.7s %-5.5s",
	       "Name", "Mtu", "Network", "Address", "Ipkts", "Ierrs",
	       "Opkts", "Oerrs");
	printf(" %-6.6s", "Collis");
	putchar('\n');
	ifaddraddr = 0;
	while (ifnetaddr || ifaddraddr) {
		struct sockaddr_in *sin;
		register char  *cp;
		int             n;
		extern char	*strchr();
		struct in_addr  in, inet_makeaddr();

		if (ifaddraddr == 0) {
			readmem(ifnetaddr, 1, 0, &ifstats, sizeof ifstats,
				"ifstats element");
			readmem((off_t) ifstats.ifs_name, 1, 0, name, 16,
				"if name");
			name[15] = '\0';
			ifnetaddr = (off_t) ifstats.ifs_next;
			if (interface != 0 &&
			    (strcmp(name, interface) != 0
			     || unit != ifstats.ifs_unit))
				continue;
			cp = strchr(name, '\0');
			sprintf(cp, "%x", ifstats.ifs_unit);
			cp = strchr(name, '\0');
			if (ifstats.ifs_active == 0)
				*cp++ = '*';
			*cp = '\0';
			ifaddraddr = (off_t) ifstats.ifs_addrs;
		}
		printf("%-6.6s %-5d ", name,
		       ifstats.ifs_mtu);
		if (ifaddraddr == 0) {
			printf("%-10.10s  ", "none");
			printf("%-12.12s ", "none");
		} else {
			readmem(ifaddraddr, 1, 0, &ifaddr, sizeof ifaddr,
				"ifaddr chain");
			ifaddraddr = (off_t) ifaddr.ifa.ifa_next;
			switch (ifaddr.ifa.ifa_addr.sa_family) {
			case AF_UNSPEC:
				printf("%-10.10s  ", "none");
				printf("%-12.12s ", "none");
				break;
			case AF_INET:
				sin = (struct sockaddr_in *) & ifaddr.in.ia_ifa.ifa_addr;
				in.s_addr = htonl(ifaddr.in.ia_subnet);
				printf("%-10.10s  ",
				       netname(in,
					       ifaddr.in.ia_subnetmask));
				printf("%-12.12s ", routename(sin->sin_addr));
				break;
			default:
				printf("af%2d: ", ifaddr.ifa.ifa_addr.sa_family);
				for (cp = (char *) &ifaddr.ifa.ifa_addr +
				     sizeof(struct sockaddr) - 1;
				     cp >= ifaddr.ifa.ifa_addr.sa_data; --cp)
					if (*cp != 0)
						break;
				n = cp - (char *) ifaddr.ifa.ifa_addr.sa_data + 1;
				cp = (char *) ifaddr.ifa.ifa_addr.sa_data;
				if (n <= 6)
					while (--n)
						printf("%02d.", *cp++ & 0xff);
				else
					while (--n)
						printf("%02d", *cp++ & 0xff);
				printf("%02d ", *cp & 0xff);
				break;
			}
		}
		printf("%-7d %-5d %-7d %-5d %-6d",
		       ifstats.ifs_ipackets, ifstats.ifs_ierrors,
		       ifstats.ifs_opackets, ifstats.ifs_oerrors,
		       ifstats.ifs_collisions);
		putchar('\n');
	}
}

#define	MAXIF	10
struct iftot {
	char            ift_name[16];	/* interface name */
	int             ift_ip;	/* input packets */
	int             ift_ie;	/* input errors */
	int             ift_op;	/* output packets */
	int             ift_oe;	/* output errors */
	int             ift_co;	/* collisions */
}               iftot[MAXIF];

/*
 * Print a running summary of interface statistics. Repeat display every
 * interval seconds, showing statistics collected over that interval.  First
 * line printed at top of screen is always cumulative. 
 */
sidewaysintpr(interval, off)
	int             interval;
	off_t           off;
{
	struct ifstats  ifstats;
	off_t           firstifnet;
	register struct iftot *ip, *total;
	register int    line;
	struct iftot   *lastif, *sum, *interesting;
	extern char	*strchr();

	readmem(off, 1, 0, &firstifnet, sizeof(off_t), "First ifstats");
	lastif = iftot;
	sum = iftot + MAXIF - 1;
	total = sum - 1;
	interesting = iftot;
	for (off = firstifnet, ip = iftot; off;) {
		char           *cp;

		readmem(off, 1, 0, &ifstats, sizeof ifstats, "ifstat chain");
		ip->ift_name[0] = '(';
		readmem(ifstats.ifs_name, 1, 0, ip->ift_name + 1, 15,
			"if name");
		if (interface && strcmp(ip->ift_name + 1, interface) == 0 &&
		    unit == ifstats.ifs_unit)
			interesting = ip;
		ip->ift_name[15] = '\0';
		cp = strchr(ip->ift_name, '\0');
		sprintf(cp, "%x)", ifstats.ifs_unit);
		ip++;
		if (ip >= iftot + MAXIF - 2)
			break;
		off = (off_t) ifstats.ifs_next;
	}
	lastif = ip;
banner:
	printf("    input  %-8.8s   output       ", interesting->ift_name);
	if (lastif - iftot > 0)
		printf("   input  (Total)    output       ");
	for (ip = iftot; ip < iftot + MAXIF; ip++) {
		ip->ift_ip = 0;
		ip->ift_ie = 0;
		ip->ift_op = 0;
		ip->ift_oe = 0;
		ip->ift_co = 0;
	}
	putchar('\n');
	printf("%-7.7s %-5.5s %-7.7s %-5.5s %-5.5s ",
	       "packets", "errs", "packets", "errs", "colls");
	if (lastif - iftot > 0)
		printf("%-7.7s %-5.5s %-7.7s %-5.5s %-5.5s ",
		       "packets", "errs", "packets", "errs", "colls");
	putchar('\n');
	fflush(stdout);
	line = 0;
loop:
	sum->ift_ip = 0;
	sum->ift_ie = 0;
	sum->ift_op = 0;
	sum->ift_oe = 0;
	sum->ift_co = 0;
	for (off = firstifnet, ip = iftot; off && ip < lastif; ip++) {
		readmem(off, 1, 0, &ifstats, sizeof ifstats, "ifstats");
		if (ip == interesting)
			printf("%-7d %-5d %-7d %-5d %-5d ",
			       ifstats.ifs_ipackets - ip->ift_ip,
			       ifstats.ifs_ierrors - ip->ift_ie,
			       ifstats.ifs_opackets - ip->ift_op,
			       ifstats.ifs_oerrors - ip->ift_oe,
			       ifstats.ifs_collisions - ip->ift_co);
		ip->ift_ip = ifstats.ifs_ipackets;
		ip->ift_ie = ifstats.ifs_ierrors;
		ip->ift_op = ifstats.ifs_opackets;
		ip->ift_oe = ifstats.ifs_oerrors;
		ip->ift_co = ifstats.ifs_collisions;
		sum->ift_ip += ip->ift_ip;
		sum->ift_ie += ip->ift_ie;
		sum->ift_op += ip->ift_op;
		sum->ift_oe += ip->ift_oe;
		sum->ift_co += ip->ift_co;
		off = (off_t) ifstats.ifs_next;
	}
	if (lastif - iftot > 0)
		printf("%-7d %-5d %-7d %-5d %-5d\n",
		       sum->ift_ip - total->ift_ip,
		       sum->ift_ie - total->ift_ie,
		       sum->ift_op - total->ift_op,
		       sum->ift_oe - total->ift_oe,
		       sum->ift_co - total->ift_co);
	*total = *sum;
	fflush(stdout);
	line++;
	if (interval)
		sleep(interval);
	if (line == 21)
		goto banner;
	goto loop;
	/* NOTREACHED */
}
