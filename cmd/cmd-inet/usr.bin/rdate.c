/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.bin/rdate.c	1.2.2.1"

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
 *  rdate - get date from remote machine
 *
 *     	sets time, obtaining value from host
 *	on the tcp/time socket.  Since timeserver returns
 *	with time of day in seconds since Jan 1, 1900,  must
 *	subtract 86400(365*70 + 17) to get time
 *	since Jan 1, 1970, which is what get/settimeofday
 *	uses.
 */
#define	TOFFSET	(86400*(365*70 + 17))
#define WRITTEN 440199955	/* tv_sec when program was written */

#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#ifdef SYSV
#define	bzero(s,n)	memset((s), 0, (n))
#define	bcopy(a,b,c)	memcpy((b), (a), (c))
#endif /* SYSV */

int timeout();

main(argc, argv)
	int argc;
	char **argv;
{
	struct sockaddr_in server;
	struct sockaddr from;
	int s, i;
	u_long time;
	struct servent *sp;
	struct protoent *pp;
	struct hostent *hp;
	struct timeval timestruct;

	if (argc != 2) {
		fprintf(stderr, "usage: rdate host\n");
		exit(1);
	}
	if ((hp = gethostbyname(argv[1])) == NULL) {
		fprintf(stderr, "Sorry, host %s not in hosts database\n",
		    argv[1]);
		exit(1);
	}
	if ((sp = getservbyname("time", "tcp")) == NULL) {
		fprintf(stderr,
		    "Sorry, time service not in services database\n");
		exit(1);
	}
	if ((pp = getprotobyname("tcp")) == NULL) {
		fprintf(stderr,
		    "Sorry, TCP protocol not in protocols database\n");
		exit(1);
	}
	if ((s = socket(AF_INET, SOCK_STREAM, pp->p_proto)) == 0) {
		perror("rdate: socket");
		exit(1);
	}

	bzero((char *)&server, sizeof (server));
	bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	server.sin_family = hp->h_addrtype;
	server.sin_port = sp->s_port;

	if (connect(s, &server, sizeof (server)) < 0) {
		perror("rdate: connect");
		exit(1);
	}

	alarm(10);			/* perhaps this should be larger? */
	signal(SIGALRM, (void (*)())timeout);

	if (read(s, (char *)&time, sizeof (int)) != sizeof (int)) {
		perror("rdate: read");
		exit(1);
	}
	time = ntohl(time) - TOFFSET;
	/* date must be later than when program was written */
	if (time < WRITTEN) {
		fprintf(stderr, "didn't get plausible time from %s\n",
		    argv[1]);
		exit(1);
	}
	timestruct.tv_usec = 0;
	timestruct.tv_sec = time;
	i = settimeofday(&timestruct, 0);
	if (i == -1)
		perror("couldn't set time of day");
	else
		printf("%s", ctime(&timestruct.tv_sec));
	exit(0);
	/* NOTREACHED */
}

timeout()
{

	fprintf(stderr, "couldn't contact time server\n");
	exit(1);
}
