/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/systems/getsystem.c	1.4.3.1"
/* LINTLIBRARY */

# include	<stdio.h>
# include	<string.h>
# include	<errno.h>
# include	<stdlib.h>

# include	"lp.h"
# include	"systems.h"

# define	SEPCHARS	":\n"

/**
 ** getsystem() - EXTRACT SYSTEM STRUCTURE FROM DISK FILE
 **/

#if	defined(__STDC__)
SYSTEM * getsystem ( const char * name )
#else
SYSTEM * getsystem ( name )
char	*name;
#endif
{
    static FILE		*fp;
    static int		all = 0;
    static SYSTEM	sysbuf;
    char		*cp;
    char		buf[BUFSIZ];

    if (STREQU(name, NAME_ALL))
    {
	if (all == 0)
	{
	    all = 1;
	    if ((fp = open_lpfile(Lp_NetData, "r", MODE_READ)) == NULL)
		return(NULL);
	}
    }
    else
	if (all == 1)
	{
	    all = 0;
	    (void) rewind(fp);
	}
	else
	    if ((fp = open_lpfile(Lp_NetData, "r", MODE_READ)) == NULL)
		return(NULL);

    (void)	memset (&sysbuf, 0, sizeof (sysbuf));

    while(fgets(buf, BUFSIZ, fp) != NULL)
    {
	if (*buf == '#')
	    continue;

	if ((cp = strtok(buf, SEPCHARS)) == NULL)
	    continue;

	if (!all && STREQU(name, cp) == 0)
	    continue;

	sysbuf.name = Strdup(cp);

	if ((cp = strtok(NULL, SEPCHARS)) == NULL)
	{
	    freesystem(&sysbuf);
	    all = 0;
	    (void) close_lpfile(fp);
	    errno = EBADF;
	    return(NULL);
	}

	sysbuf.passwd = NULL;

	if ((cp = strtok(NULL, SEPCHARS)) == NULL)
	{
	    freesystem(&sysbuf);
	    all = 0;
	    (void) close_lpfile(fp);
	    errno = EBADF;
	    return(NULL);
	}

	sysbuf.reserved1 = NULL;

	if ((cp = strtok(NULL, SEPCHARS)) == NULL)
	{
	    freesystem(&sysbuf);
	    all = 0;
	    (void) close_lpfile(fp);
	    errno = EBADF;
	    return(NULL);
	}

	if (STREQU(NAME_S5PROTO, cp))
	    sysbuf.protocol = S5_PROTO;
	else
	    if (STREQU(NAME_BSDPROTO, cp))
		sysbuf.protocol = BSD_PROTO;

	if ((cp = strtok(NULL, SEPCHARS)) == NULL)
	{
	    freesystem(&sysbuf);
	    errno = EBADF;
	    all = 0;
	    (void) close_lpfile(fp);
	    return(NULL);
	}

	sysbuf.reserved2 = NULL;

	if ((cp = strtok(NULL, SEPCHARS)) == NULL)
	{
	    freesystem(&sysbuf);
	    errno = EBADF;
	    all = 0;
	    (void) close_lpfile(fp);
	    return(NULL);
	}
	if (*cp == 'n')
	    sysbuf.timeout = -1;
	else
	    sysbuf.timeout = atoi(cp);

	if ((cp = strtok(NULL, SEPCHARS)) == NULL)
	{
	    freesystem(&sysbuf);
	    all = 0;
	    (void) close_lpfile(fp);
	    errno = EBADF;
	    return(NULL);
	}

	if (*cp == 'n')
	    sysbuf.retry = -1;
	else
	    sysbuf.retry = atoi(cp);

	if ((cp = strtok(NULL, SEPCHARS)) == NULL)
	{
	    freesystem(&sysbuf);
	    all = 0;
	    (void) close_lpfile(fp);
	    errno = EBADF;
	    return(NULL);
	}

	sysbuf.reserved3 = NULL;

	if ((cp = strtok(NULL, SEPCHARS)) == NULL)
	{
	    freesystem(&sysbuf);
	    all = 0;
	    (void) close_lpfile(fp);
	    errno = EBADF;
	    return(NULL);
	}

	sysbuf.reserved4 = NULL;

	if ((cp = strtok(NULL, SEPCHARS)) == NULL)
		sysbuf.comment = NULL;
	else	
		sysbuf.comment = Strdup(cp);

	if (all == 0)
	    (void) close_lpfile(fp);

	return(&sysbuf);
    }

    (void) close_lpfile(fp);
    all = 0;
    errno = ENOENT;
    return(NULL);
}
