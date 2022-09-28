/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/systems/delsystem.c	1.3.3.1"
/* LINTLIBRARY */

# include	<string.h>
# include	<stdlib.h>
# include	<errno.h>

# include	"lp.h"
# include	"systems.h"

# define	SEPCHARS	":\n"

/**
 ** delsystem()
 **/

#if	defined(__STDC__)
int delsystem ( const char * name )
#else
int delsystem ( name )
char	*name;
#endif
{
    FILE	*fpi;
    FILE	*fpo;
    char	*cp;
    char	*file;
    char	buf[BUFSIZ];
    char	c;
    int		all = 0;
    int		errtmp;

    if ((file = tempnam(ETCDIR, "lpdat")) == NULL)
    {
	errno = ENOMEM;
	return(-1);
    }

    if ((fpi = open_lpfile(Lp_NetData, "r", MODE_READ)) == NULL)
    {
	Free(file);
	return(-1);
    }

    if ((fpo = open_lpfile(file, "w", MODE_READ)) == NULL)
    {
	errtmp = errno;
	(void) close_lpfile(fpi);
	Free(file);
	errno = errtmp;
	return(-1);
    }

    if (STREQU(NAME_ALL, name))
	all = 1;

    while (fgets(buf, BUFSIZ, fpi) != NULL)
    {
	if (*buf != '#' && *buf != '\n')
	    if ((cp = strpbrk(buf, SEPCHARS)) != NULL)
	    {
		if (all)
		    continue;
		
		c = *cp;
		*cp = '\0';
		if (STREQU(name, buf))
		    continue;
		*cp = c;
	    }

	if (fputs(buf, fpo) == EOF)
	{
	    errtmp = errno;
	    (void) close_lpfile(fpi);
	    (void) close_lpfile(fpo);
	    (void) Unlink(file);
	    Free(file);
	    errno = errtmp;
	    return(-1);
	}
    }

    (void) close_lpfile(fpi);
    (void) close_lpfile(fpo);
    (void) _Rename(file, Lp_NetData);
    Free(file);
    return(0);
}
