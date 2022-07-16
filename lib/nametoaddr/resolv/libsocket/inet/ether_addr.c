/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)nametoaddr:resolv/libsocket/inet/ether_addr.c	1.1.1.1"

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
 * All routines necessary to deal with the file /etc/ethers.  The file
 * contains mappings from 48 bit ethernet addresses to their corresponding
 * hosts name.  The addresses have an ascii representation of the form
 * "x:x:x:x:x:x" where x is a hex number between 0x00 and 0xff;  the
 * bytes are always in network order.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>

#ifdef YP
static char *domain; /* yp domain name */
#endif YP
static char *ethers = "/etc/ethers";

extern	char	*malloc();
extern	char	*calloc();

/*
 * Parses a line from /etc/ethers into its components.  The line has the form
 * 8:0:20:1:17:c8	krypton
 * where the first part is a 48 bit ethernet addrerss and the second is
 * the corresponding hosts name.
 * Returns zero if successful, non-zero otherwise.
 */
_rs_ether_line(s, e, hostname)
	char *s;		/* the string to be parsed */
	ether_addr_t e;	/* ethernet address struct to be filled in */
	char *hostname;		/* hosts name to be set */
{
	register int i;
	unsigned int t[6];
	
	i = sscanf(s, " %x:%x:%x:%x:%x:%x %s",
	    &t[0], &t[1], &t[2], &t[3], &t[4], &t[5], hostname);
	if (i != 7) {
		return (7 - i);
	}
	for (i = 0; i < 6; i++)
		e[i] = (u_char) t[i];
	return (0);
}

/*
 * Converts a 48 bit ethernet number to its string representation.
 */
char *
_rs_ether_ntoa(e)
	ether_addr_t e;
{
	static char *s;

	if (s == 0) {
		s = (char *)malloc((unsigned) 18);
		if (s == 0)
			return (0);
	}
	s[0] = 0;
	(void) sprintf(s, "%x:%x:%x:%x:%x:%x",
		e[0], e[1], e[2], e[3], e[4], e[5]);
	return (s);
}

/*
 * Converts a ethernet address representation back into its 48 bits.
 */
ether_addr_t *
_rs_ether_aton(s)
	char *s;
{
	static ether_addr_t e;
	register int i;
	unsigned int t[6];
	
	i = sscanf(s, " %x:%x:%x:%x:%x:%x",
	    &t[0], &t[1], &t[2], &t[3], &t[4], &t[5]);
	if (i != 6)
	    return ((ether_addr_t *)NULL);
	for (i = 0; i < 6; i++)
		e[i] = (u_char) t[i];
	return (&e);
}

/*
 * Given a host's name, this routine returns its 48 bit ethernet address.
 * Returns zero if successful, non-zero otherwise.
 */
_rs_ether_hostton(host, e)
	char *host;		/* function input */
	ether_addr_t e;	/* function output */
{
	char currenthost[256];
	char buf[512];
	char *val = buf;
	register int reason;
	FILE *f;
#ifdef YP
	int vallen;
	char *map = "ethers.byname";
	
	if (!(reason = yp_match(domain, map, host,
	    strlen(host), &val, &vallen))) {
		reason = _rs_ether_line(val, e, currenthost);
		free(val);  /* yp_match always allocates for us */
		return (reason);
	}
	else if (yp_ismapthere(reason)) return (reason);
	else {
#endif YP
		if ((f = fopen(ethers, "r")) == NULL) {
			return (-1);
		}
		reason = -1;
		while (fscanf(f, "%[^\n] ", val) == 1) {
			if ((_rs_ether_line(val, e, currenthost) == 0) &&
			    (strcmp(currenthost, host) == 0)) {
				reason = 0;
				break;
			}
		}
		(void) fclose(f);
		return (reason);
#ifdef YP
	}
#endif YP
}

/*
 * Given a 48 bit ethernet address, this routine return its host name.
 * Returns zero if successful, non-zero otherwise.
 */
_rs_ether_ntohost(host, e)
	char *host;		/* function output */
	ether_addr_t e;	/* function input */
{
	ether_addr_t currente;
	char buf[512];
	char *val = buf;
	register int reason;
	FILE *f;
#ifdef YP
	char *a = _rs_ether_ntoa(e);
	int vallen;
	char *map = "ethers.byaddr";
	
	if (!(reason = yp_match(domain, map, a,
	    strlen(a), &val, &vallen))) 
	    {
		reason = _rs_ether_line(val, &currente, host);
		free(val);  /* yp_match always allocates for us */
		return (reason);
		}
	else if (yp_ismapthere(reason)) return (reason);
	else
	{
#endif YP
		if ((f = fopen(ethers, "r")) == NULL) {
			return (-1);
		}
		reason = -1;
		while (fscanf(f, "%[^\n] ", val) == 1) {
			if ((_rs_ether_line(val, currente, host) == 0) &&
			    (memcmp((char*) e, (char*) currente, sizeof(currente)) == 0)) {
				reason = 0;
				break;
			}
		}
		(void) fclose(f);
		return (reason);
#ifdef YP
	}
#endif YP
}
