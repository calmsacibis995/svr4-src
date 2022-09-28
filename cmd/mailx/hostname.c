/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:hostname.c	1.3.5.1"
/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Code to figure out what host we are on.
 */

#include "rcv.h"
#include "configdefs.h"
#include <sys/utsname.h>

#define	MAILCNFG	"/etc/mail/mailcnfg"

char host[64];
char domain[128];
/*
 * Initialize the network name of the current host.
 */
void
inithost()
{
	register struct netmach *np;
	struct utsname name;
	char *fp;

	xsetenv(MAILCNFG);
	if (fp = xgetenv("CLUSTER")) {
		strncpy(host, fp, sizeof(host));
	} else {
		uname(&name);
		strncpy(host, name.nodename, sizeof host);
	}
	strcpy(domain, host);
	strcat(domain, maildomain());
	for (np = netmach; np->nt_machine != 0; np++)
		if (strcmp(np->nt_machine, EMPTY) == 0)
			break;
	if (np->nt_machine == 0) {
		printf("Cannot find empty slot for dynamic host entry\n");
		exit(1);
	}
	np->nt_machine = host;
	np++;
	np->nt_machine = domain;
	if (debug) fprintf(stderr, "host '%s', domain '%s'\n", host, domain);
}
