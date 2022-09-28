/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.bin/netstat/route.c	1.2.3.1"

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
#include <sys/stream.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <net/route.h>
#include <netinet/ip_str.h>

#include <netdb.h>

extern int      kmem;
extern int      nflag;
extern char    *routename(), *netname(), *ns_print();

#define satosin(sa)	((struct sockaddr_in *)(sa))

/*
 * Definitions for showing gateway flags. 
 */
struct bits {
	short           b_mask;
	char            b_val;
}               bits[] = {
	{
		                RTF_UP, 'U'
	}              ,
	{
		                RTF_GATEWAY, 'G'
	}              ,
	{
		                RTF_HOST, 'H'
	}              ,
	{
		                RTF_DYNAMIC, 'D'
	}              ,
	{
		                RTF_TOSWITCH, 'T'
	}              ,
	{
		                RTF_SWITCHED, 'S'
	}              ,
	{
		                RTF_SLAVE, 'V'
	}              ,
	{
		                0
	}
};

/*
 * Print routing tables. 
 */
routepr(hostaddr, netaddr, hashsizeaddr)
	off_t           hostaddr, netaddr, hashsizeaddr;
{
	struct rtentry  rt;
	mblk_t          mb, *m;
	register struct bits *p;
	char            name[16], *flags;
	mblk_t        **routehash;
	struct ip_provider prov;
	int             hashsize;
	int             i, doinghost = 1;

	if (hostaddr == 0) {
		printf("rthost: symbol not in namelist\n");
		return;
	}
	if (netaddr == 0) {
		printf("rtnet: symbol not in namelist\n");
		return;
	}
	if (hashsizeaddr == 0) {
		printf("rthashsize: symbol not in namelist\n");
		return;
	}
	readmem(hashsizeaddr, 1, 0, &hashsize, sizeof(hashsize), "hashsize");
	routehash = (mblk_t **) malloc(hashsize * sizeof(mblk_t *));
	readmem(hostaddr, 1, 0, routehash, hashsize * sizeof(mblk_t *),
		"routehash");
	printf("Routing tables\n");
	printf("%-20.20s %-20.20s %-8.8s %-6.6s %-10.10s %s\n",
	       "Destination", "Gateway",
	       "Flags", "Refcnt", "Use", "Interface");
again:
	for (i = 0; i < hashsize; i++) {
		if (routehash[i] == 0)
			continue;
		m = routehash[i];
		while (m) {
			struct in_addr  in;

			readmem(m, 1, 0, &mb, sizeof(mb), "mblock");
			readmem(mb.b_rptr, 1, 0, &rt, sizeof(rt), "rtentry");
			in = satosin(&rt.rt_dst)->sin_addr;
			printf("%-20.20s ",
			       (in.s_addr == 0) ? "default" :
			       (rt.rt_flags & RTF_HOST) ?
			       routename(in) :
			       netname(in, 0));
			in = satosin(&rt.rt_gateway)->sin_addr;
			printf("%-20.20s ", routename(in));
			for (flags = name, p = bits; p->b_mask; p++)
				if (p->b_mask & rt.rt_flags)
					*flags++ = p->b_val;
			if (rt.rt_flags & RTF_SWITCHED
			    && !(rt.rt_flags & RTF_TOSWITCH))
				*flags++ = (SSS_GETSTATE(&rt) >> 8) + '0';
			*flags = '\0';
			printf("%-8.8s %-6d %-10d ", name,
			       rt.rt_refcnt, rt.rt_use);
			if (rt.rt_prov == 0) {
				putchar('\n');
				m = mb.b_cont;
				continue;
			}
			readmem(rt.rt_prov, 1, 0, &prov, sizeof(prov),
				"provider");
			printf("%s\n", prov.name);
			m = mb.b_cont;
		}
	}
	if (doinghost) {
		readmem(netaddr, 1, 0, routehash, hashsize * sizeof(mblk_t *),
			"routehash");
		doinghost = 0;
		goto again;
	}
	free(routehash);
}

char           *
routename(in)
	struct in_addr  in;
{
	register char  *cp;
	static char     line[MAXHOSTNAMELEN + 1];
	struct hostent *hp;
	static char     domain[MAXHOSTNAMELEN + 1];
	static int      first = 1;
	char           *strchr();

	if (first) {
		first = 0;
		if (gethostname(domain, MAXHOSTNAMELEN) == 0 &&
		    (cp = strchr(domain, '.')))
			(void) strcpy(domain, cp + 1);
		else
			domain[0] = 0;
	}
	cp = 0;
	if (!nflag) {
		hp = gethostbyaddr(&in, sizeof(struct in_addr),
				   AF_INET);
		if (hp) {
			if ((cp = strchr(hp->h_name, '.')) &&
			    !strcmp(cp + 1, domain))
				*cp = 0;
			cp = hp->h_name;
		}
	}
	if (cp)
		strcpy(line, cp);
	else {
#define C(x)	((x) & 0xff)
		in.s_addr = ntohl(in.s_addr);
		sprintf(line, "%u.%u.%u.%u", C(in.s_addr >> 24),
			C(in.s_addr >> 16), C(in.s_addr >> 8), C(in.s_addr));
	}
	return (line);
}

/*
 * Return the name of the network whose address is given. The address is
 * assumed to be that of a net or subnet, not a host. 
 */
char           *
netname(in, mask)
	struct in_addr  in;
	u_long          mask;
{
	char           *cp = 0;
	static char     line[50];
	struct netent  *np = 0;
	u_long          net;
	register        i;
	int             subnetshift;

	in.s_addr = ntohl(in.s_addr);
	if (!nflag && in.s_addr) {
		if (mask == 0) {
			i = in.s_addr;
			if (IN_CLASSA(i)) {
				mask = IN_CLASSA_NET;
				subnetshift = 8;
			} else if (IN_CLASSB(i)) {
				mask = IN_CLASSB_NET;
				subnetshift = 8;
			} else {
				mask = IN_CLASSC_NET;
				subnetshift = 4;
			}
			/*
			 * If there are more bits than the standard mask
			 * would suggest, subnets must be in use. Guess at
			 * the subnet mask, assuming reasonable width subnet
			 * fields. 
			 */
			while (in.s_addr & ~mask)
#ifdef SYSV
				/* compiler doesn't sign extend! */
				mask = (mask | ((long) mask >> subnetshift));
#else
				mask = (long) mask >> subnetshift;
#endif /* SYSV */
		}
		net = in.s_addr & mask;
		while ((mask & 1) == 0)
			mask >>= 1, net >>= 1;
		np = getnetbyaddr(net, AF_INET);
		if (np)
			cp = np->n_name;
	}
	if (cp)
		strcpy(line, cp);
	else if ((in.s_addr & 0xffffff) == 0)
		sprintf(line, "%u", C(in.s_addr >> 24));
	else if ((in.s_addr & 0xffff) == 0)
		sprintf(line, "%u.%u", C(in.s_addr >> 24), C(in.s_addr >> 16));
	else if ((in.s_addr & 0xff) == 0)
		sprintf(line, "%u.%u.%u", C(in.s_addr >> 24),
			C(in.s_addr >> 16), C(in.s_addr >> 8));
	else
		sprintf(line, "%u.%u.%u.%u", C(in.s_addr >> 24),
			C(in.s_addr >> 16), C(in.s_addr >> 8), C(in.s_addr));
	return (line);
}

/*
 * Print routing statistics 
 */
rt_stats(off)
	off_t           off;
{
	struct rtstat   rtstat;

	if (off == 0) {
		printf("rtstat: symbol not in namelist\n");
		return;
	}
	readmem(off, 1, 0, &rtstat, sizeof(rtstat), "rtstat");
	printf("routing:\n");
	printf("\t%d bad routing redirect%s\n",
	       rtstat.rts_badredirect, plural(rtstat.rts_badredirect));
	printf("\t%d dynamically created route%s\n",
	       rtstat.rts_dynamic, plural(rtstat.rts_dynamic));
	printf("\t%d new gateway%s due to redirects\n",
	       rtstat.rts_newgateway, plural(rtstat.rts_newgateway));
	printf("\t%d destination%s found unreachable\n",
	       rtstat.rts_unreach, plural(rtstat.rts_unreach));
	printf("\t%d use%s of a wildcard route\n",
	       rtstat.rts_wildcard, plural(rtstat.rts_wildcard));
}
