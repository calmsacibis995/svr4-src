/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/in.routed/af.c	1.1.2.1"

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


#include "defs.h"

/*
 * Address family support routines
 */
int	inet_hash(), inet_netmatch(), inet_output(),
	inet_portmatch(), inet_portcheck(),
	inet_checkhost(), inet_rtflags(), inet_sendsubnet(), inet_canon();
char	*inet_format();

#define NIL	{ 0 }
#define	INET \
	{ inet_hash,		inet_netmatch,		inet_output, \
	  inet_portmatch,	inet_portcheck,		inet_checkhost, \
	  inet_rtflags,		inet_sendsubnet,	inet_canon, \
	  inet_format \
	}

struct afswitch afswitch[AF_MAX] = {
	NIL,		/* 0- unused */
	NIL,		/* 1- Unix domain, unused */
	INET,		/* Internet */
};

int af_max = sizeof(afswitch) / sizeof(afswitch[0]);

struct sockaddr_in inet_default = { AF_INET, INADDR_ANY };

inet_hash(sin, hp)
	register struct sockaddr_in *sin;
	struct afhash *hp;
{
	register u_long n;

	n = inet_netof(sin->sin_addr);
	if (n)
	    while ((n & 0xff) == 0)
		n >>= 8;
	hp->afh_nethash = n;
	hp->afh_hosthash = ntohl(sin->sin_addr.s_addr);
	hp->afh_hosthash &= 0x7fffffff;
}

inet_netmatch(sin1, sin2)
	struct sockaddr_in *sin1, *sin2;
{

	return (inet_netof(sin1->sin_addr) == inet_netof(sin2->sin_addr));
}

/*
 * Verify the message is from the right port.
 */
inet_portmatch(sin)
	register struct sockaddr_in *sin;
{
	
	return (sin->sin_port == htons(IPPORT_ROUTESERVER));
}

/*
 * Verify the message is from a "trusted" port.
 */
inet_portcheck(sin)
	struct sockaddr_in *sin;
{

	return (ntohs(sin->sin_port) <= IPPORT_RESERVED);
}

/*
 * Internet output routine.
 */
inet_output(s, flags, sin, size)
	int s, flags;
	struct sockaddr_in *sin;
	int size;
{
	struct sockaddr_in dst;

	dst = *sin;
	sin = &dst;
	if (sin->sin_port == 0)
		sin->sin_port = htons(IPPORT_ROUTESERVER);
	if (sendto(s, packet, size, flags, sin, sizeof (*sin)) < 0)
		perror("sendto");
}

/*
 * Return 1 if the address is believed
 * for an Internet host -- THIS IS A KLUDGE.
 */
inet_checkhost(sin)
	register struct sockaddr_in *sin;
{
	register u_long i = ntohl(sin->sin_addr.s_addr);

	if (IN_BADCLASS(i) || sin->sin_port != 0)
		return (0);
	if (i != 0 && (i & 0xff000000) == 0)
		return (0);
	for (i = 0; i < sizeof(sin->sin_zero)/sizeof(sin->sin_zero[0]); i++)
		if (sin->sin_zero[i])
			return (0);
	return (1);
}

inet_canon(sin)
	struct sockaddr_in *sin;
{
	register int i;

	sin->sin_port = 0;
	for (i=0; i < sizeof(sin->sin_zero); i++)
		sin->sin_zero[i] = 0;
}

char *
inet_format(sin)
	struct sockaddr_in *sin;
{
	char *inet_ntoa();

	return (inet_ntoa(sin->sin_addr));
}
