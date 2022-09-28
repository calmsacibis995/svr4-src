/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/dfshares/dfshares.c	1.4.3.1"

/*
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
 
*/

/*
 * nfs dfshares
 */
#include <stdio.h>
#include <varargs.h>
#include <string.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <nfs/nfs.h>
#include <rpcsvc/mount.h>

struct timeval timeout = { 25, 0 };

int hflg;
void pr_exports();
void free_ex();
void usage();

main(argc, argv)
	int argc;
	char **argv;
{
	
	char hostbuf[256];
	extern int optind;
	extern char *optarg;
	int i, c;

	while ((c = getopt(argc, argv, "h")) != EOF) {
		switch (c) {
		case 'h':
			hflg++;
			break;
		default:
			usage();
			exit(1);
		}
	}

	if (optind < argc) {
		for (i = optind ; i < argc ; i++)
			pr_exports(argv[i]);
	} else {
		if (gethostname(hostbuf, sizeof(hostbuf)) < 0) {
			perror("nfs dfshares: gethostname");
			exit(1);
		}
		pr_exports(hostbuf);
	}

	exit(0);
}

void
pr_exports(host)
	char *host;
{
	CLIENT *cl;
	struct exports *ex = NULL;
	enum clnt_stat err;

	/*
	 * First try circuit, then drop back to datagram if
	 * circuit is unavailable (an old version of mountd perhaps)
	 * Using circuit is preferred because it can handle
	 * arbitrarily long export lists.
	 */
	cl = clnt_create(host, MOUNTPROG, MOUNTVERS, "circuit_n");
	if (cl == NULL) {
		cl = clnt_create(host, MOUNTPROG, MOUNTVERS, "datagram_n");
		if (cl == NULL) {
			(void) fprintf(stderr, "nfs dfshares:");
			clnt_pcreateerror(host);
			exit(1);
		}
	}

	if (err = clnt_call(cl, MOUNTPROC_EXPORT,
	    xdr_void, 0, xdr_exports, &ex, timeout)) {
		(void) fprintf(stderr, "nfs dfshares: %s\n", clnt_sperrno(err));
		clnt_destroy(cl);
		exit(1);
	}

	if (ex == NULL) {
		clnt_destroy(cl);
		exit(1);
	}

	if (!hflg) {
		printf("%-35s %12s %-8s  %s\n",
			"RESOURCE", "SERVER", "ACCESS", "TRANSPORT");
		hflg++;
	}

	while (ex) {
		printf("%10s:%-24s %12s %-8s  %s\n",
			host, ex->ex_name, host, " -", " -");
		ex = ex->ex_next;
	}
	free_ex(ex);
	clnt_destroy(cl);
}

void
free_ex(ex)
	struct exports *ex;
{
	struct groups *gr, *tmpgr;
	struct exports *tmpex;

	while (ex) {
		free(ex->ex_name);
		gr = ex->ex_groups;
		while (gr) {
			tmpgr = gr->g_next;
			free((char *)gr);
			gr = tmpgr;
		}
		tmpex = ex;
		ex = ex->ex_next;
		free((char *)tmpex);
	}
}

void
usage()
{
	(void) fprintf(stderr, "Usage: dfshares [-h] [host ...]\n");
}
