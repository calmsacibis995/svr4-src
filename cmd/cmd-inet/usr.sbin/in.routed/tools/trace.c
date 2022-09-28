/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/in.routed/tools/trace.c	1.1.3.1"

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
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <protocols/routed.h>

struct	sockaddr_in myaddr = { AF_INET, IPPORT_RESERVED-1 };
char	packet[MAXPACKETSIZE];

main(argc, argv)
	int argc;
	char *argv[];
{
	int size, s;
	struct sockaddr from;
	struct sockaddr_in router;
	register struct rip *msg = (struct rip *)packet;
	struct hostent *hp;
	struct servent *sp;
	
	if (argc < 3) {
usage:
		printf("usage: trace cmd machines,\n");
		printf("cmd either \"on filename\", or \"off\"\n");
		exit(1);
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		exit(2);
	}
#ifdef vax || pdp11 || i386
	myaddr.sin_port = htons(myaddr.sin_port);
#endif
	if (bind(s, &myaddr, sizeof(myaddr)) < 0) {
		perror("bind");
		exit(2);
	}

	argv++, argc--;
	msg->rip_cmd = strcmp(*argv, "on") == 0 ?
		RIPCMD_TRACEON : RIPCMD_TRACEOFF;
	msg->rip_vers = RIPVERSION;
	argv++, argc--;
	size = sizeof (int);
	if (msg->rip_cmd == RIPCMD_TRACEON) {
		strcpy(msg->rip_tracefile, *argv);
		size += strlen(*argv);
		argv++, argc--;
	}
	if (argc == 0)
		goto usage;
	bzero((char *)&router, sizeof (router));
	router.sin_family = AF_INET;
	sp = getservbyname("router", "udp");
	if (sp == 0) {
		printf("udp/router: service unknown\n");
		exit(1);
	}
	router.sin_port = sp->s_port;
	while (argc > 0) {
		hp = gethostbyname(*argv);
		if (hp == 0) {
			printf("%s: unknown\n", *argv);
			continue;
		}
		bcopy(hp->h_addr, &router.sin_addr, hp->h_length);
		if (sendto(s, packet, size, 0, &router, sizeof(router)) < 0)
			perror(*argv);
		argv++, argc--;
	}
	exit(0);
	/* NOTREACHED */
}
