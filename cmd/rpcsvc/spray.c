/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)rpcsvc:spray.c	1.1.1.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)spray.c 1.1 89/03/22 Copyr 1985 Sun Micro";
#endif

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <ctype.h>
#include <rpc/rpc.h>
#include <rpcsvc/spray.h>

enum clnt_stat sprayproc_spray_1nd(/*argp, clnt*/);

#define DEFBYTES 100000		/* default numbers of bytes to send */
#define MAXPACKETLEN 1514
#define SPRAYOVERHEAD 86	/* size of rpc packet when size=0 */

main(argc, argv)
	int argc;
	char **argv;
{
	register CLIENT *clnt;
	int		i;
	int		delay = 0;
	int		psec, bsec;
	int		buf[SPRAYMAX/4];
	char		msgbuf[256];
	int		lnth, cnt;
	sprayarr	arr;	
	spraycumul	cumul;
	spraycumul	*co;
	char		*host;
	char		*type;
	
	if (argc < 2)
		usage();
	
	cnt = -1;
	lnth = SPRAYOVERHEAD;
	type = "netpath";
	while (argc > 1) {
		if (argv[1][0] == '-') {
			switch(argv[1][1]) {
				case 'd':
					delay = atoi(argv[2]);
					argc--;
					argv++;
					break;
				case 'c':
					cnt = atoi(argv[2]);
					argc--;
					argv++;
					break;

				case 't':
					type = (argv[2]);
					argc--;
					argv++;
					break;
				case 'l':
					lnth = atoi(argv[2]);
					argc--;
					argv++;
					break;
				default:
					usage();
			}
		} else {
			host = argv[1];
		}
		argc--;
		argv++;
	}
	if (host == NULL)
		usage();
	clnt = clnt_create(host, SPRAYPROG, SPRAYVERS, type);
	if (clnt == (CLIENT *)NULL) {
		sprintf(msgbuf, "spray: cannot clnt_create %s:%s", host, type);
		clnt_pcreateerror(msgbuf);
		exit(1);
	}
	if (cnt == -1)
		cnt = DEFBYTES/lnth;
	if (lnth < SPRAYOVERHEAD)
		lnth = SPRAYOVERHEAD;
	else if (lnth >= SPRAYMAX)
		lnth = SPRAYMAX;
	if (lnth <= MAXPACKETLEN && lnth % 4 != 2)
		lnth = ((lnth + 5) / 4) * 4 - 2;
	arr.sprayarr_len = lnth - SPRAYOVERHEAD;
	arr.sprayarr_val = (char *) buf;
	printf("sending %d packets of length %d to %s ...", cnt, lnth, host);
	fflush(stdout);
	if (sprayproc_clear_1(NULL, clnt) == NULL) {
		clnt_perror(clnt, "SPRAYPROC_CLEAR ");
		exit(1);
	}
	for (i = 0; i < cnt; i++) {
		sprayproc_spray_1nd(&arr, clnt);
		if (delay > 0)
			slp(delay);
	}
	if ((co = sprayproc_get_1(NULL, clnt)) == NULL) {
		clnt_perror(clnt, "SPRAYPROC_GET ");
		exit(1);
	}
	cumul= *co;
	if (cumul.counter < cnt)
		printf("\n\t%d packets (%.3f%%) dropped by %s\n",
			cnt - cumul.counter,
			100.0 * (cnt - cumul.counter)/cnt, host);
	else
		printf("\n\tno packets dropped by %s\n", host);
	psec = (1000000.0 * cumul.counter)
		/ (1000000.0 * cumul.clock.sec + cumul.clock.usec);
	bsec = (lnth * 1000000.0 * cumul.counter)/
		(1000000.0 * cumul.clock.sec + cumul.clock.usec);
	printf("\t%d packets/sec, %d bytes/sec\n", psec, bsec);
	exit(0);
	/* NOTREACHED */
}

/*
 * A special call, where the TIMEOUT is 0. So, every call times-out.
 */
static struct timeval TIMEOUT = { 0, 0 };

enum clnt_stat
sprayproc_spray_1nd(argp, clnt)
	sprayarr *argp;
	CLIENT *clnt;
{
	return (clnt_call(clnt, SPRAYPROC_SPRAY, xdr_sprayarr, argp, xdr_void,
			NULL, TIMEOUT));
}

/* A cheap microseconds sleep call */
slp(usecs)
{
	struct timeval tv;

	tv.tv_sec = usecs / 1000000;
	tv.tv_usec = usecs % 1000000;
	select(32, 0, 0, 0, &tv);
}

usage()
{
	printf("spray host [-t nettype] [-l lnth] [-c cnt] [-d delay]\n");
	exit(1);
}
