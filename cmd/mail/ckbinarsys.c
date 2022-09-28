/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:ckbinarsys.c	1.8.3.1"
/*
    NAME
	ckbinarsys - check system name for binary acceptability

    SYNOPSIS
	ckbinarsys [-S] -s sysname -t msg_type

    DESCRIPTION
	Check that remote system "sysname" can accept messages
	with "msg_type" content. msg_type is usually set
	to check for "binary" (non-text) content. If -S is
	not specified, a message will be printed if the
	message cannot be transmitted.

	Returns:
	    0 ==> OK to send to next hop remote.
	    1 ==> Next hop remote CANNOT support binary content messages.
*/
#include <string.h>
#include <stdio.h>
#include <errno.h>
#ifndef SVR3
#include <unistd.h>
#include <stdlib.h>
#endif
#include "libmail.h"

#define	BINARSYS	"/etc/mail/binarsys"
#define	LSIZE		BUFSIZ

ckbinarsys(remote)
register char	*remote;
{
	char buf[LSIZE];
	register char	*p;
	register FILE	*bsysfp;
	int	defrc = 1;	/* default return code */

	if ((bsysfp = fopen(BINARSYS,"r")) == (FILE *)NULL) {
		int sverrno = errno;
		fprintf(stderr,"Cannot open '%s'\n", BINARSYS);
		errno = sverrno;
		perror("ckbinarsys");
		return(1);
	}
	while (fgets(buf,sizeof(buf),bsysfp) != (char *)NULL) {
		if (buf[0] == '#') {
			/* Skip comments */
			continue;
		}
		if (casncmp(buf,"Default=",8) == 0) {
			p = strchr(buf,'=');
			/* skip past equal sign */
			p++;
			switch (*p) {
			case 'y':
			case 'Y':
				defrc = 0;
				break;
			case 'n':
			case 'N':
				defrc = 1;
				break;
			}
			continue;
		}
		if ((p = strchr(buf,':')) != (char *)NULL) {
			*p = '\0';
			p++;
		}
		if (strcmp(remote,buf) != 0) {
			continue;
		}
		if (p == (char *)NULL) {
			/* No colon found; take the default */
			break;
		}
		fclose (bsysfp);

		/* Found entry for this system. Is it 'y' or 'n'? */
		switch (*p) {
		case 'y':
		case 'Y':
			return(0);
		case 'n':
		case 'N':
			return(1);
		default:
			return(defrc);
		}
	}
	fclose (bsysfp);
	return(defrc);
}

main (argc, argv) 
int	argc;
char	*argv[];
{
	char		*sysname = (char *)NULL;
	char		*msg_type = (char *)NULL;
	extern char	*optarg;	/* for getopt */
	register int 	c;
	int		errflg = 0;
	int		silent = 0;

	while ((c = getopt(argc, argv, "Ss:t:")) != EOF) {
		switch(c) {
		case 'S':
			silent++;
			break;
		case 's':
			sysname = optarg;
			break;
		case 't':
			msg_type = optarg;
			break;
		default:
			errflg++;
			break;
		}
	}
	if ((msg_type == (char *)NULL) || (sysname == (char *)NULL)) {
		errflg++;
	}
	if (errflg) {
		fprintf (stderr,"Usage: %s [-S] -s system_name -t msg_type\n",
					argv[0]);
		exit (1);
	}
	
	if (strcmp(msg_type, "text") == 0) {
		exit (0);
	}
	if (ckbinarsys (sysname) == 0) {
		exit (0);
	} else {
		if (!silent) {
			fprintf(stderr,
				"Cannot send binary content to system '%s'.\n",sysname);
			fprintf(stderr,
				"Permission denied by '%s'\n", BINARSYS);
		}
		exit (1);
	}
	/* NOTREACHED */
}
