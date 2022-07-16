/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libadm:pkgnmchk.c	1.2.3.1"

#include <string.h>
#include <ctype.h>

#ifdef PRESVR4
#include <stdio.h>
#endif

static char	*rsvrd[] = {
	"all",
	"install",
	"new",
	NULL
};

#define NMBRK	".*"
#define WILD1	".*"
#define WILD2	"*"
#define WILD3	".name"

static int
valname(pkg, wild, presvr4flg)
char	*pkg;
int	wild;
int presvr4flg;
{
	int	count, i, n;
	char	*pt;

	/* wild == 1	allow wildcard specification as a name */
	if(wild && !strcmp(pkg, "all"))
		return(0);

	/* check for reserved package names */
	for(i=0; rsvrd[i]; i++) {
		n = strlen(rsvrd[i]);
		if(!strncmp(pkg, rsvrd[i], n) && 
		  (!pkg[n] || strchr(NMBRK, pkg[n])))
			return(1);
	}

	/* check for valid extensions; we must do this
	 * first since we need to look for SVR3 ".name"
	 * before we validate the package abbreviation
	 */
	if(pt = strpbrk(pkg, NMBRK)) {
		if(presvr4flg && !strcmp(pt, WILD3))
			return(0); /* SVR3 packages have no validation */
		else if (!strcmp(pt, WILD1) || !strcmp(pt, WILD2)) {
			/* wildcard specification */
			if(!wild)
				return(1);
		} else {
			count = 0;
			while(*++pt) {
				count++;
				if(!isalpha(*pt) && !isdigit(*pt) && 
				  !strpbrk(pt, "-+"))
					return(-1);
			}
			if(!count || (count > 4))
				return(-1);
		}
	}

	/* check for valid package name */
	count = 0;
	if(!isalnum(*pkg) || (!presvr4flg && !isalpha(*pkg)))
		return(-1);
	while(*pkg && !strchr(NMBRK, *pkg)) {
		if(!isalnum(*pkg) && !strpbrk(pkg, "-+"))
			return(-1);
		count++, pkg++;
	}
	if(count > 9)
		return(-1);

	return(0); /* pkg is valid */
}

int
pkgnmchk(pkg, spec, presvr4flg)
register char *pkg, *spec;
int presvr4flg;	/* check for pre-svr4 package names also ? */
{
	/* pkg is assumed to be non-NULL upon entry */

	/* this routine reacts based on the value passed in spec:
	 * 	NULL	pkg must be valid and may be a wildcard spec
	 *	"all"	pkg must be valid and must be an instance
	 *	"x.*"	pkg must be valid and must be an instance of "x"
	 *	"x*"	pkg must be valid and must be an instance of "x"
	 */	

	if(valname(pkg, ((spec == NULL) ? 1 : 0), presvr4flg))
		return(1); /* invalid or reserved name */

	if((spec == NULL) || !strcmp(spec, "all"))
		return(0);

	while(*pkg == *spec) {
		if(!strcmp(spec, WILD1) || !strcmp(spec, WILD2) || 
		!strcmp(spec, WILD3))
			break; /* wildcard spec, so stop right here */
		else if(*pkg++ == '\0')
			return(0); /* identical match */
		spec++;
	}

	if(!strcmp(spec, WILD1) || !strcmp(spec, WILD2) || 
	!strcmp(spec, WILD3)) { 
		if((pkg[0] == '\0') || (pkg[0] == '.'))
			return(0);
	}
	if((spec[0] == '\0') && !strcmp(pkg, WILD3))
		return(0); /* compare pkg.name to pkg */
	return(1);
}
