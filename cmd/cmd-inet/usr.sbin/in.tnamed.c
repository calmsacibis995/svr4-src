/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/in.tnamed.c	1.3.2.1"

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
 * This program implements a UDP basic name server as specified in IEN116
 * The extended name server functionality is NOT implemented here (yet).
 * This is generally used in conjunction with MIT's PC/IP software.
 */

#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifdef SYSV
#define	bzero(s,n)	memset((s), 0, (n))
#define	bcopy(a,b,c)	memcpy((b), (a), (c))
#endif /* SYSV */

/*
 * These command codes come from IEN116
 */
#define NAMECODE	1
#define ADDRESSCODE	2
#define ERRORCODE	3
/*
 * These error codes are used to qualify ERRORCODE
 */
#define UNDEFINEDERROR	0
#define NOTFOUNDERROR	1
#define SYNTAXERROR	2
	
#define BUFLEN 2000
static int handler();

main(argc, argv)
	int argc;
	char **argv;
{
	int s;
	struct sockaddr_in client;
	int length, clientlength;
	register struct hostent	*hp;
	char hostname[BUFLEN];
	char iobuf[BUFLEN];
	register char *buffer = iobuf;
	register int replylength;
	int request;
	struct in_addr x;

	if (argc > 1) {
		/* the daemon is run by hand and never exits */
		struct servent temp;
		register struct servent *sp;
		register struct protoent *pp;
		struct sockaddr_in server;

		if((sp = getservbyname("name","udp")) == NULL) {
			fprintf(stderr,
			   "in.tnamed: UDP name server not in /etc/services\n");
			sp = &temp;
			sp->s_port = htons(42);
		}
		if((pp = getprotobyname("udp")) == NULL) {
			fprintf(stderr,
			    "in.tnamed: UDP protocol not in /etc/protocols\n");
			exit(1);
		}
		if((s = socket(AF_INET, SOCK_DGRAM, pp->p_proto)) < 0) {
			perror("in.tnamed: socket error");
			exit(1);
		}
		bzero((char *)&server, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_port = sp->s_port;
		if(bind(s, &server, sizeof(server)) != 0) {
			perror("in.tnamed: bind error");
			exit(1);
		}
		fprintf(stderr, "in.tnamed: UDP name server running\n");
	} else {
		/* daemon forked by inetd and is short lived */
		struct itimerval value, ovalue;

		signal(SIGALRM, handler);
		value.it_value.tv_sec = 5 * 60;
		value.it_value.tv_usec = value.it_interval.tv_usec = 0;
		setitimer(ITIMER_REAL, &value, &ovalue);
		s = 0;  /* by inetd conventions */
	}

	for (;;) {

		clientlength = sizeof(client);
		length = recvfrom(s, buffer, BUFLEN, 0, &client, &clientlength);
		if(length < 0) {
			perror("in.tnamed: recvfrom error. Try in.tnamed -v ?");
			continue;
		}

		request = buffer[0];
		length = buffer[1];
		replylength = length + 2;  /* reply is appended to request */
		if (length < sizeof(hostname)) {
			strncpy(hostname, &buffer[2], length);
			hostname[length]= 0;
		} else {
			hostname[0] = 0;
		}

		if(request != NAMECODE) {
			fprintf(stderr, "in.tnamed: bad request from %s\n",
			    inet_ntoa(client.sin_addr));
			buffer[replylength++] = ERRORCODE;
			buffer[replylength++] = 3;  /* no error msg yet */
			buffer[replylength++] = SYNTAXERROR;
			fprintf(stderr,
			    "in.tnamed: request (%d) not NAMECODE\n", request);
			sleep(5);  /* pause before saying something negative */
			goto sendreply;
		}
		
		if(hostname[0] == '!') {
			/*
			 * !host!net name format is not implemented yet,
			 * only host alone.
			 */
			fprintf(stderr,
			  "in.tnamed: %s using !net!host format name request\n",
			    inet_ntoa(client.sin_addr));
			buffer[replylength++] = ERRORCODE;
			buffer[replylength++] = 0;  /* no error msg yet */
			buffer[replylength++] = UNDEFINEDERROR;
			fprintf(stderr,
			    "in.tnamed: format (%s) not supported\n", hostname);
			sleep(5);  /* pause before saying something negative */
			goto sendreply;
		}

		if((hp = gethostbyname(hostname)) == NULL) {
			buffer[replylength++] = ERRORCODE;
			buffer[replylength++] = 0;  /* no error msg yet */
			buffer[replylength++] = NOTFOUNDERROR;
			fprintf(stderr,
			    "in.tnamed: name (%s) not found\n", hostname);
			sleep(5);  /* pause before saying something negative */
			goto sendreply;
		}

		if(hp->h_addrtype != AF_INET) {
			buffer[replylength++] = ERRORCODE;
			buffer[replylength++] = 0;  /* no error msg yet */
			buffer[replylength++] = UNDEFINEDERROR;
			fprintf(stderr,
			    "in.tnamed: address type (%d) not AF_INET\n",
			    hp->h_addrtype);
			sleep(5);  /* pause before saying something negative */
			goto sendreply;
		}

		fprintf(stderr, "in.tnamed: %s asked for address of %s",
		    inet_ntoa(client.sin_addr), hostname);
		bcopy (hp->h_addr, (char *)&x, sizeof x);
		printf(" - it's %s\n", inet_ntoa(x));
			
		buffer[replylength++] = ADDRESSCODE;
		buffer[replylength++] = hp->h_length;
		bcopy(hp->h_addr, &buffer[replylength], hp->h_length);
		replylength += hp->h_length;

	sendreply:
		if (sendto(s, buffer, replylength, 0, &client, clientlength)
		    != replylength) {
			perror("in.tnamed: sendto error");
			continue;
		}
	}
}

static int
handler()
{

	exit(0);
}



