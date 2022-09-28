/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginstall/pkgenv.c	1.1.3.1"

#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include <string.h>

extern char	*getenv(), *fpkgparam();
extern void	free(),
		progerr(),
		putparam();
extern int	access(),
		pkgnmchk();

#define PKGINFO	"pkginfo"
#define PKGMAP	"pkgmap"

#define ERR_PKGINFO	"unable to open pkginfo file <%s>"
#define ERR_PKGMAP	"unable to open pkgmap file <%s>"
#define ERR_NOPARAM	"%s parameter is not defined in <%s>"
#define ERR_PKGINFO	"unable to open pkginfo file <%s>"
#define ERR_PKGMAP	"unable to open pkgmap file <%s>"
#define ERR_PKGBAD	"PKG parameter is invalid <%s>"
#define ERR_PKGMTCH	"PKG parameter <%s> does not match instance <%s>"

char	*pkgarch;
char	*pkgvers;
char	*pkgabrv;
char	*pkgname;
char	pkgwild[PKGSIZ+1];

int
pkgenv(instdir, pkginst)
char	*instdir, *pkginst;
{
	FILE	*fp;
	char 	*value,
		path[PATH_MAX],
		param[64];
	int	errflg;

	errflg = 0;
	(void) sprintf(path, "%s/%s", instdir, PKGMAP);
	if(access(path, 0)) {
		progerr(ERR_PKGMAP);
		return(1);
	}
	(void) sprintf(path, "%s/%s", instdir, PKGINFO);
	if((fp = fopen(path, "r")) == NULL) {
		progerr(ERR_PKGINFO, path);
		return(1);
	}
	param[0] = '\0';
	while(value = fpkgparam(fp, param)) {
		if(strcmp("PATH", param))
			putparam(param, value);
		free(value);
		param[0] = '\0';
	}
	(void) fclose(fp);
	/* 
	 * verify that required parameters are now present in
	 * the environment
	 */
	if((pkgabrv = getenv("PKG")) == NULL) {
		progerr(ERR_NOPARAM, "PKG", path);
		errflg++;
	}
	if(pkgnmchk(pkgabrv, NULL, 0) || strchr(pkgabrv, '.')) {
		progerr(ERR_PKGBAD, pkgabrv);
		errflg++;
	}
	(void) sprintf(pkgwild, "%s.*", pkgabrv);
	if((pkgname = getenv("NAME")) == NULL) {
		progerr(ERR_NOPARAM, "NAME", path);
		errflg++;
	}
	if((pkgarch = getenv("ARCH")) == NULL) {
		progerr(ERR_NOPARAM, "ARCH", path);
		errflg++;
	}
	if((pkgvers = getenv("VERSION")) == NULL) {
		progerr(ERR_NOPARAM, "VERSION", path);
		errflg++;
	}
	if(getenv("CATEGORY") == NULL) {
		progerr(ERR_NOPARAM, "CATEGORY", path);
		errflg++;
	}
	/*
	 * verify consistency between PKG parameter and pkginst that
	 * was determined from the directory structure
	 */
	(void) sprintf(param, "%s.*", pkgabrv);
	if(pkgnmchk(pkginst, param, 0)) {
		progerr(ERR_PKGMTCH, pkgabrv, pkginst);
		errflg++;
	}
	return(errflg);
}
