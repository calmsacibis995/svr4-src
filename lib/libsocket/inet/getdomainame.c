/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libsocket:inet/getdomainame.c	1.1.3.1"


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
 * getdomainname
 * setdomainname
 *	Gets and sets the domain name of the system
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/systeminfo.h>

#ifndef INFO_SRPC_DOMAIN
#define use_file
#endif
#ifdef use_file
char DOMAIN[] = "/etc/domain";
#endif

static char *domainname;
int setdomainname();
extern char *calloc();
#ifdef use_file
static char *
_domainname()
{
	register char *d = domainname;

	if (d == 0) {
		d = (char *)calloc(1, 256);
		domainname = d;
	}
	return (d);
}
#endif

int 
getdomainname(name, namelen)
	char *name;
	int namelen;
{
#ifdef use_file
	char *line = _domainname();
	FILE *domain_fd;

	if (line == NULL)
		return(-1);
	if ((domain_fd = fopen(DOMAIN, "r")) == NULL)
		return (-1);
	
	if (fscanf(domain_fd, "%s", line) == NULL) {
		fclose(domain_fd);
		return (-1);
	}
	fclose(domain_fd);
	(void) strncpy(name, line, namelen);
	return (0);
#else
	int sysinfostatus=sysinfo(INFO_SRPC_DOMAIN,name,namelen);
	if (sysinfostatus<0) return(-1);
	else return(0);
#endif
}

setdomainname(domain, len)
	char *domain;
	int len;
{
#ifdef use_file
	FILE *domain_fd;

	if ((domain_fd = fopen(DOMAIN, "rw")) == NULL)
		return (-1);
	if (fputs(domain, domain_fd) == NULL)
		return (-1);
	fclose(domain_fd);
	return(0);
#else
	int sysinfostatus=sysinfo(INFO_SETSRPC, domain, len+1); /*add null*/
	if (sysinfostatus<0) return(-1);
	else return(0);
#endif
}
