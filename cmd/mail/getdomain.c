/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:getdomain.c	1.7.3.1"
#include "mail.h"
#ifdef SVR4
# include <sys/utsname.h>
# include <sys/systeminfo.h>
#endif

#define NMLN 512
#ifdef SVR4
# if SYS_NMLN > NMLN
#  undef NMLN
#  define NMLN SYS_NMLN
# endif
#endif

/*
    NAME
	maildomain() - retrieve the domain name

    SYNOPSIS
	char *maildomain()

    DESCRIPTION
	Retrieve the domain name from xgetenv("DOMAIN").
	If that is not set, look in /etc/resolv.conf, /etc/inet/named.boot
	and /etc/named.boot for "^domain[ ]+<domain>".
	If that is not set, use sysinfo(SI_SRPC_DOMAIN) from
	-lnsl. Otherwise, return an empty string.
*/

/* read a file for the domain */
static char *look4domain(file, buf, size)
char *file, *buf;
int size;
{
    char *ret = 0;
    FILE *fp = fopen(file, "r");
    if (!fp)
	return 0;

    while (fgets(buf, size, fp))
	if (strncmp(buf, "domain", 6) == 0)
	    if (isspace(buf[6]))
		{
		char *x = skipspace(buf + 6);
		if (isgraph(*x))
		    {
		    trimnl(x);
		    strmove(buf, x);
		    ret = buf;
		    break;
		    }
		}

    fclose(fp);
    return ret;
}

/* read the domain from the xenvironment or one of the files */
static char *readdomain(buf, size)
char *buf;
int size;
{
    char *ret;

    if ((ret = xgetenv("DOMAIN")) != 0)
	{
	strncpy(buf, ret, size);
	return buf;
	}

    (ret = look4domain("/etc/resolv.conf", buf, size)) ||
    (ret = look4domain("/etc/inet/named.boot", buf, size)) ||
    (ret = look4domain("/etc/named.boot", buf, size));
    if (ret != 0)
	return ret;

#ifdef SVR4
    if (sysinfo(SI_SRPC_DOMAIN, buf, size) >= 0)
	return buf;
#endif

    return 0;
}

char *maildomain()
{
    static char *domain = 0;
    static char dombuf[NMLN+1] = ".";

    /* if we've already been here, return the info */
    if (domain != 0)
	return domain;

    domain = readdomain(dombuf+1, NMLN);

    /* Make certain that the domain begins with a single dot */
    /* and does not have one at the end. */
    if (domain)
	{
	int len;
	domain = dombuf;
	while (*domain && *domain == '.')
	    domain++;
	domain--;
	len = strlen(domain);
	while ((len > 0) && (domain[len-1] == '.'))
	    domain[--len] = '\0';
	}

    /* no domain information */
    else
	domain = "";

    return domain;
}
