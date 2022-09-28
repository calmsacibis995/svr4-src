/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/systems/putsystem.c	1.3.2.1"
/* LINTLIBRARY */

# include	<sys/types.h>
# include	<stdio.h>
# include	<string.h>
# include	<errno.h>
# include	<stdlib.h>

# include	"lp.h"
# include	"systems.h"


/**
 ** putsystem() - WRITE SYSTEM STRUCTURE TO DISK FILES
 **/


#if	defined(__STDC__)
int putsystem ( const char * name, const SYSTEM * sysbufp )
#else
int putsystem ( name, sysbufp )
char	*name;
SYSTEM	*sysbufp;
#endif
{
    FILE	*fp;

    /*
    **	Validate the arguments.
    **	Must have a name, but not "all".
    **	SYSTEM must have a provider, address, and a protocol.
    */
    if (!name || !*name || STREQU(name, NAME_ALL))
    {
	errno = EINVAL;
	return(-1);
    }

    if (!sysbufp ||
       (sysbufp->protocol != S5_PROTO && sysbufp->protocol != BSD_PROTO))
    {
	errno = EINVAL;
	return(-1);
    }

    /*
    **	Since, this may be an update of an existing entry, delsystem
    **	is called to prevent any duplication.
    */
    (void) delsystem(name);

    if ((fp = open_lpfile(Lp_NetData, "a", MODE_READ)) == NULL)
	return(-1);

    (void) fprintf(fp, "%s:", name);
    (void) fprintf(fp, "%s:", "x");	/* passwd */
    (void) fprintf(fp, "%s:", "-");	/* reserved1 */
    (void) fprintf(fp, "%s:", (sysbufp->protocol == S5_PROTO ?
				NAME_S5PROTO : NAME_BSDPROTO));
    (void) fprintf(fp, "-:");	/* reserved2 */
    if (sysbufp->timeout < 0)
	(void) fprintf(fp, "n:");
    else
	(void) fprintf(fp, "%d:", sysbufp->timeout);
    
    if (sysbufp->retry < 0)
	(void) fprintf(fp, "n:");
    else
    	(void) fprintf(fp, "%d:", sysbufp->retry);

    (void) fprintf(fp, "-:");	/* reserved3 */
    (void) fprintf(fp, "-:");	/* reserved4 */
    if (sysbufp->comment)
    	(void) fprintf(fp, "%s\n", sysbufp->comment);
    else
        (void) fprintf(fp, "\n");
    (void) close_lpfile(fp);
    return(0);
}
