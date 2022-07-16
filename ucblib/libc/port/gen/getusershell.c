/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ident	"@(#)ucblibc:port/gen/getusershell.c	1.2.3.1"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>

#define SHELLS "/etc/shells"

/*
 * Do not add local shells here.  They should be added in /etc/shells
 */
static char *okshells[] =
    { "/usr/bin/sh", "/usr/bin/csh", "/usr/bin/ksh", 0 };

static char **shells, *strings;
static char **curshell;
extern char **initshells();

/*
 * Get a list of shells from SHELLS, if it exists.
 */
char *
getusershell()
{
        char *ret;

        if (curshell == NULL)
                curshell = initshells();
        ret = *curshell;
        if (ret != NULL)
                curshell++;
        return (ret);
}

endusershell()
{
        
        if (shells != NULL)
                free((char *)shells);
        shells = NULL;
        if (strings != NULL)
                free(strings);
        strings = NULL;
        curshell = NULL;
}

setusershell()
{

        curshell = initshells();
}

static char **
initshells()
{
        register char **sp, *cp;
        register FILE *fp;
        struct stat statb;
        extern char *malloc(), *calloc();

        if (shells != NULL)
                free((char *)shells);
        shells = NULL;
        if (strings != NULL)
                free(strings);
        strings = NULL;
        if ((fp = fopen(SHELLS, "r")) == (FILE *)0)
                return(okshells);
        if (fstat(fileno(fp), &statb) == -1) {
                (void)fclose(fp);
                return(okshells);
        }
        if ((strings = malloc((unsigned)statb.st_size)) == NULL) {
                (void)fclose(fp);
                return(okshells);
        }
        shells = (char **)calloc((unsigned)statb.st_size / 3, sizeof (char *));
        if (shells == NULL) {
                (void)fclose(fp);
                free(strings);
                strings = NULL;
                return(okshells);
        }
        sp = shells;
        cp = strings;
        while (fgets(cp, MAXPATHLEN + 1, fp) != NULL) {
                while (*cp != '#' && *cp != '/' && *cp != '\0')
                        cp++;
                if (*cp == '#' || *cp == '\0')
                        continue;
                *sp++ = cp;
                while (!isspace(*cp) && *cp != '#' && *cp != '\0')
                        cp++;
                *cp++ = '\0';
        }
        *sp = (char *)0;
        (void)fclose(fp);
        return (shells);
}
