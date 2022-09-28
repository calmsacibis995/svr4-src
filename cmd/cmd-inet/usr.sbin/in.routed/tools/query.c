/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/in.routed/tools/query.c	1.1.2.1"

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


#include <sys/param.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <protocols/routed.h>

#define	WTIME	5		/* Time to wait for responses */

int	s;
int	timedout, timeout();
char	packet[MAXPACKETSIZE];
	/*
	 * Try the following list of commands until one works
	 */		 
int	cmds[] = {RIPCMD_REQUEST,RIPCMD_POLL,0};
int	*cmd = cmds;
extern int errno;

main(argc, argv)
	int argc;
	char *argv[];
{
	int cc, count, bits;
	struct sockaddr from;
	int fromlen = sizeof(from);
	struct timeval notime;
	char **hosts;
	
	if (argc < 2) {
		printf("usage: query hosts...\n");
		exit(1);
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		exit(2);
	}

	argv++, argc--;

	do {
		hosts = argv;
		for (count = argc;count > 0;count--)
			query(*hosts++);

	/*
	 * Listen for returning packets;
	 * may be more than one packet per host.
	 */
		bits = 1 << s;
		bzero(&notime, sizeof(notime));
		timedout = 0;
		signal(SIGALRM, timeout);
		alarm(WTIME);
		count = argc;
		while ((count > 0 && !timedout) ||
		    select(20, &bits, 0, 0, &notime) > 0) {
			cc = recvfrom(s, packet, sizeof (packet), 0,
			  &from, &fromlen);
			if (cc <= 0) {
					if (cc < 0) {
					if (errno == EINTR)
						continue;
				perror("recvfrom");
				(void) close(s);
					exit(1);
				}
				continue;
			}
			rip_input(&from, cc);
			count--;
		}
	  /*
	   * continue trying old commands if we did not receive any response
	   */
	} while (count==argc && *++cmd);

	exit(0);
	/* NOTREACHED */
}

query(host)
	char *host;
{
	struct sockaddr_in router;
	register struct rip *msg = (struct rip *)packet;
	struct hostent *hp;

	bzero((char *)&router, sizeof (router));
	hp = gethostbyname(host);
	if (hp == 0) {
		router.sin_addr.s_addr = inet_addr(host);
		if (router.sin_addr.s_addr == (u_long)-1) {
			printf("%s: unknown\n", host);
			exit(1);
		}
	} else {
		bcopy(hp->h_addr, &router.sin_addr, hp->h_length);
	}
	router.sin_family = AF_INET;
	router.sin_port = IPPORT_ROUTESERVER;
	msg->rip_cmd = *cmd;
	msg->rip_vers = RIPVERSION;
	msg->rip_nets[0].rip_dst.sa_family = htons(AF_UNSPEC);
	msg->rip_nets[0].rip_metric = htonl(HOPCNT_INFINITY);
	if (sendto(s, packet, sizeof (struct rip), 0,
	  &router, sizeof(router)) < 0)
		perror(host);
}

/*
 * Handle an incoming routing packet.
 */
rip_input(from, size)
	struct sockaddr_in *from;
	int size;
{
	register struct rip *msg = (struct rip *)packet;
	struct netinfo *n;
	char *name;
	int lna, net, subnet;
	struct hostent *hp;
	struct netent *np;

	if (msg->rip_cmd != RIPCMD_RESPONSE)
		return;
	hp = gethostbyaddr(&from->sin_addr, sizeof (struct in_addr), AF_INET);
	name = hp == 0 ? "???" : hp->h_name;
	printf("from %s(%s):\n", name, inet_ntoa(from->sin_addr));
	size -= sizeof (int);
	n = msg->rip_nets;
	while (size > 0) {
		register struct sockaddr_in *sin;
		int i;
		static char buf[256];

		if (size < sizeof (struct netinfo))
			break;
		if (msg->rip_vers > 0) {
			n->rip_dst.sa_family =
				ntohs(n->rip_dst.sa_family);
			n->rip_metric = ntohl(n->rip_metric);
		}
		sin = (struct sockaddr_in *)&n->rip_dst;
		if (sin->sin_port) {
		    printf("**Non-zero port (%d) **",
		    	       sin->sin_port & 0xFFFF);
		}
		for (i=6;i<13;i++)
		  if (n->rip_dst.sa_data[i]) {
		    printf("sockaddr = ");
		    for (i=0;i<14;i++)
		      printf("%d ",n->rip_dst.sa_data[i] & 0xFF);
		    break;
		  }
		net = inet_netof(sin->sin_addr);
		subnet = inet_subnetof(sin->sin_addr);
		lna = inet_lnaof(sin->sin_addr);
		name = "???";
		if (lna == INADDR_ANY) {
			np = getnetbyaddr(net, AF_INET);
			if (np)
				name = np->n_name;
			else if (net == 0)
				name = "default";
# ifdef SUBNETS
		} else if ((subnet != net) && ((lna & 0xff) == 0) &&
		    (np = getnetbyaddr(subnet, AF_INET))) {
			struct in_addr subnaddr, inet_makeaddr();

			subnaddr = inet_makeaddr(subnet, INADDR_ANY);
			if (bcmp(&sin->sin_addr, &subnaddr, sizeof(subnaddr)) == 0)
				name = np->n_name;
			else
				goto host;
# endif SUBNETS
		} else {
host:
			hp = gethostbyaddr(&sin->sin_addr,
			    sizeof (struct in_addr), AF_INET);
			if (hp)
				name = hp->h_name;
		}
		printf("\t%s(%s), metric %d\n", name,
			inet_ntoa(sin->sin_addr), n->rip_metric);
		size -= sizeof (struct netinfo), n++;
	}
}

timeout()
{
	timedout = 1;
}

/*
 * Return the possible subnetwork number from an internet address.
 * If the address is of the form of a subnet address (most significant
 * bit of the host part is set), believe the subnet exists.
 * Otherwise, return the network number.
 * SHOULD FIND OUT WHETHER THIS IS A LOCAL NETWORK BEFORE LOOKING
 * INSIDE OF THE HOST PART.  We can only believe this if we have other
 * information (e.g., we can find a name for this number).
 */
inet_subnetof(in)
	struct in_addr in;
{
# ifdef SUBNETS
	register u_long i = ntohl(in.s_addr);

	if (IN_CLASSA(i)) {
		if (IN_SUBNETA(i))
			return ((i & IN_CLASSA_SUBNET) >> IN_CLASSA_SUBNSHIFT);
		else
			return ((i & IN_CLASSA_NET) >> IN_CLASSA_NSHIFT);
	} else if (IN_CLASSB(i)) {
		if (IN_SUBNETB(i))
			return ((i & IN_CLASSB_SUBNET) >> IN_CLASSB_SUBNSHIFT);
		else
			return ((i & IN_CLASSB_NET) >> IN_CLASSB_NSHIFT);
	} else
		return ((i & IN_CLASSC_NET) >> IN_CLASSC_NSHIFT);
# else SUBNETS
	return(0);
# endif SUBNETS
}
