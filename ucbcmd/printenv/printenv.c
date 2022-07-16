/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright (c) 1983, 1984, 1985, 1986, 1987, 1988 Sun Microsystems, Inc. 
 * All Rights Reserved.
 */
    
/*
 * Derived from UNIX(R) and Berkeley 4.2 BSD licensed from AT&T
 * Information Systems, Inc. and The Regents of the University of
 * California, respectively.
 */


#ident	"@(#)ucbprintenv:printenv.c	1.1.1.1"

/*
 * printenv
*/

extern	char **environ;

main(argc, argv)
	int argc;
	char *argv[];
{
	register char **ep;
	int found = 0;

	argc--, argv++;
	if (environ)
		for (ep = environ; *ep; ep++)
			if (argc == 0 || prefix(argv[0], *ep)) {
				register char *cp = *ep;

				found++;
				if (argc) {
					while (*cp && *cp != '=')
						cp++;
					if (*cp == '=')
						cp++;
				}
				(void)printf("%s\n", cp);
			}
	exit (!found);
	/* NOTREACHED */
}

prefix(cp, dp)
	char *cp, *dp;
{

	while (*cp && *dp && *cp == *dp)
		cp++, dp++;
	if (*cp == 0)
		return (*dp == '=');
	return (0);
}
