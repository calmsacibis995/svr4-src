/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libadm:pkginfo.c	1.2.2.1"

#include <stdio.h>
#include <limits.h>
#include <varargs.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pkginfo.h>
#include <pkgstrct.h>
#include <pkglocs.h>
#include <errno.h>

extern char	*pkgdir;
extern int	errno;
extern int	access(),
		pkgnmchk();
extern void	free();
extern char	*fpkgparam();

static void	initpkg();
static char	*svr4inst();
static int	rdconfig(), 
		svr4info(), 
		ckinfo(),
		ckinst(),
		verscmp(), 
		archcmp(), 
		compver();

char	*fpkginst();
int	fpkginfo();

/* 
 * Globals:
 *	pkgdir - specifies the directory where information about packages
 *	    resides, i.e. the pkginfo file is located in a subdirectory
 *
 * Caveats:
 *	The structure provided via "info" will contain malloc'd information; 
 *	    this will be free'd upon the next call to pkginfo with this
 *	    same structure.  Application calls must make sure this structure
 *	    is null on the first call, or else we'll free static memory areas
 *	If the "pkg" argument is a wildcard specification, the next found
 *	    instance available which matches the request will be returned
 *	If the "pkg" argument is a NULL pointer, the structure pointed to
 *	    via "info" will have its elements deallocated and all files
 *	    associated with this routine will be closed
 *
 * Return codes:
 *	A non-zero exit code indicates error with "errno" appropriately set:
 *	    EINVAL - invalid argument
 *	    ESRCH - there are no more instances of this package around
 *	    EACCESS - unable to access files which should have been there
 */

/*VARARGS*/
int
pkginfo(va_alist)
va_dcl
{
	struct pkginfo	*info;
	char	*pkginst, *ckarch, *ckvers;
	int	check;
	va_list ap;
	
	va_start(ap);
	info = va_arg(ap, struct pkginfo *);
	pkginst = va_arg(ap, char *);
	if(info == NULL) { 
		va_end(ap);
		errno = EINVAL;
		return(-1); 
	}
	if(pkginst == NULL) {
		va_end(ap);
		(void) fpkginfo(info, NULL);
		(void) fpkginst(NULL);
		return(0);
	}
	ckarch = va_arg(ap, char *);
	ckvers = va_arg(ap, char *);
	va_end(ap);

	check = 0;
	if(pkgnmchk(pkginst, "all", 1)) {
		/* wild card specification */
		pkginst = fpkginst(pkginst, ckarch, ckvers);
		if(pkginst == NULL)
			return(-1);
	} else {
		/* request to check indicated instance */
		if(ckarch || ckvers)
			check++;
	}
		
	if(fpkginfo(info, pkginst))
		return(-1);

	if(check) {
		/* 
		 * verify that the provided instance matches 
		 * any arch & vers specs that were provided
		 */
		if(ckinst(pkginst, info->arch, info->version, ckarch, ckvers)) {
			errno = ESRCH;
			return(-1);
		}
	}
	return(0);
}
/*ARGSUSED*/

int
fpkginfo(info, pkginst)
struct pkginfo *info;
char *pkginst;
{
	if(info == NULL) {
		errno = EINVAL;
		return(-1);
	}

	initpkg(info);

	if(pkginst == NULL)
		return(0);
	else if(pkgnmchk(pkginst, "all", 1)) {
		errno = EINVAL; /* not an instance identifier */
		return(-1);
	}
	if(pkgdir == NULL)
		pkgdir = PKGLOC;

	if(rdconfig(info, pkginst, NULL)) {
		initpkg(info);
		return(-1);
	}
	return(0);
}

static void
initpkg(info)
struct pkginfo *info;
{
	/* free previously allocated space */
	if(info->pkginst) {
		free(info->pkginst);
		if(info->arch)
			free(info->arch);
		if(info->version)
			free(info->version);
		if(info->basedir)
			free(info->basedir);
		if(info->name)
			free(info->name);
		if(info->vendor)
			free(info->vendor);
		if(info->catg)
			free(info->catg);
	}

	info->pkginst = NULL;
	info->arch = info->version = NULL;
	info->basedir = info->name = NULL;
	info->vendor = info->catg = NULL;
	info->status = PI_UNKNOWN;
}

static int
rdconfig(info, pkginst, ckvers)
struct pkginfo *info;
char *pkginst, *ckvers;
{
	FILE	*fp;
	char	temp[256];
	char	*value, *pt, *copy, **memloc;
	int	count;

	(void) sprintf(temp, "%s/%s/pkginfo", pkgdir, pkginst);
	if((fp = fopen(temp, "r")) == NULL) {
		if((errno == ENOENT) && !strcmp(pkgdir, PKGLOC))
			return(svr4info(info, pkginst, ckvers));
		errno = EACCES;
		return(-1);
	}

	*temp = '\0';
	count = 0;
	while(value = fpkgparam(fp, temp)) {
		if(!strcmp(temp, "ARCH") || !strcmp(temp, "CATEGORY")) {
			/* remove all whitespace from value */
			pt = copy = value;
			while(*pt) {
				if(!isspace(*pt))
					*copy++ = *pt;
				pt++;
			}
			*copy = '\0';
		}
		count++;
		memloc = NULL;
		if(!strcmp(temp, "NAME"))
			memloc = &info->name;
		else if(!strcmp(temp, "VERSION"))
			memloc = &info->version;
		else if(!strcmp(temp, "ARCH"))
			memloc = &info->arch;
		else if(!strcmp(temp, "VENDOR"))
			memloc = &info->vendor;
		else if(!strcmp(temp, "BASEDIR"))
			memloc = &info->basedir;
		else if(!strcmp(temp, "CATEGORY"))
			memloc = &info->catg;

		temp[0] = '\0';
		if(memloc == NULL)
			continue; /* not a parameter we're looking for */

		*memloc = strdup(value);
		if(!*memloc) {
			fclose(fp);
			errno = ENOMEM;
			return(-1); /* malloc from strdup failed */
		}
	}
	fclose(fp);

	if(!count) {
		errno = ESRCH;
		return(-1);
	}

	info->status = (strcmp(pkgdir, PKGLOC) ? PI_SPOOLED : PI_INSTALLED);

	if(info->status == PI_INSTALLED) {
		(void) sprintf(temp, "%s/%s/!I-Lock!", pkgdir, pkginst);
		if(access(temp, 0) == 0)
			info->status = PI_PARTIAL;
		else {
			(void) sprintf(temp, "%s/%s/!R-Lock!", pkgdir, pkginst);
			if(access(temp, 0) == 0)
				info->status = PI_PARTIAL;
		}
	}
	info->pkginst = strdup(pkginst);
	return(0);
}

static int
svr4info(info, pkginst, ckvers)
struct pkginfo *info;
char *pkginst, *ckvers;
{
	static DIR *pdirfp;
	struct stat status;
	FILE *fp;
	char *pt, path[128], line[128];
	char	temp[PKGSIZ+1];

	if(strcmp(pkginst, "all")) {
		if(pdirfp) {
			closedir(pdirfp);
			pdirfp = NULL;
		}
		/* determine pkginst - remove '.*' extension, if any */
		(void) strncpy(temp, pkginst, PKGSIZ);
		if((pt = strchr(temp, '.')) && !strcmp(pt, ".*"))
			*pt = '\0';
	}

	/* look in /usr/options direcotry for 'name' file */
	(void) sprintf(path, "%s/%s.name", PKGOLD, temp);
	if(lstat(path, &status)) {
		errno = (errno == ENOENT) ? ESRCH : EACCES;
		return(-1);
	}
	if((status.st_mode & S_IFMT) != S_IFREG) {
		errno = ESRCH;
		return(-1);
	}
	if((fp = fopen(path, "r")) == NULL) {
		errno = (errno == ENOENT) ? ESRCH : EACCES;
		return(-1);
	}

	/* /usr/options/xxx.name exists */
	(void) fgets(line, 128, fp);
	(void) fclose(fp);
	if(pt = strchr(line, '\n'))
		*pt = '\0'; /* remove trailing newline */
	if(pt = strchr(line, ':'))
		*pt++ = '\0'; /* assumed version specification */

	if(info) {
		info->name = strdup(line);
		info->pkginst = strdup(temp);
		if(!info->name || !info->pkginst) {
			errno = ENOMEM;
			return(-1);
		}
		info->status = PI_PRESVR4;
		info->version = NULL;
	}

	if(pt) {
		/* eat leading space off of version spec */
		while(isspace(*pt))
			pt++;
	}
	if(ckvers && verscmp(ckvers, pt)) {
		errno = ESRCH;
		return(-1);
	}
	if(info && *pt)
		info->version = strdup(pt);
	return(0);
}

static int
ckinst(pkginst, pkgarch, pkgvers, ckarch, ckvers)
char *pkginst, *pkgarch, *pkgvers, *ckarch, *ckvers;
{
	if(ckarch && archcmp(ckarch, pkgarch))
		return(-1);
	if(ckvers) {
		if(ckvers[0] == '~') {
			ckvers++;
			/* check for compatable version */
			if(verscmp(ckvers, pkgvers) && 
			   compver(pkginst, ckvers))
				return(-1);
		} else if(verscmp(ckvers, pkgvers))
			return(-1);
	}
	return(0);
}

/*VARARGS*/
char *
fpkginst(va_alist)
va_dcl
{
	static char pkginst[PKGSIZ+1];
	static DIR *pdirfp;
	struct dirent *dp;
	char	*pt, *pkg, *ckarch, *ckvers;
	va_list	ap;

	va_start(ap);
	pkg = va_arg(ap, char *);	
	if(pkg == NULL) {
		/* request to close or rewind the file */
		if(pdirfp) {
			(void) closedir(pdirfp);
			pdirfp = NULL;
		}
		svr4inst(NULL); /* close any files used here */
		return(NULL);
	}

	ckarch = va_arg(ap, char *);
	ckvers = va_arg(ap, char *);
	va_end(ap);

	if(!pkgdir)
		pkgdir = PKGLOC;

	if(!pdirfp && ((pdirfp = opendir(pkgdir)) == NULL)) {
		errno = EACCES;
		return(NULL);
	}

	while((dp = readdir(pdirfp)) != NULL) {
		if(dp->d_name[0] == '.')
			continue;

		if(pkgnmchk(dp->d_name, pkg, 0))
			continue; /* ignore invalid SVR4 package names */

		if(ckinfo(dp->d_name, ckarch, ckvers))
			continue;
			
		/* leave directory open in case user requests another instance
		 */
		strcpy(pkginst, dp->d_name);
		return(pkginst);
	}

	/* if we are searching the directory which contains info
	 * about installed packages, check the pre-svr4 directory
	 * for an instance and be sure it matches any version
	 * specification provided to us
	 */
	if(!strcmp(pkgdir, PKGLOC) && (ckarch == NULL)) {
		/* search for pre-SVR4 instance */
		if(pt = svr4inst(pkg))
			return(pt);
	}
	errno = ESRCH;
	/* close any file we might have open */
	(void) closedir(pdirfp);
	pdirfp = NULL;
	return(NULL);
}
/*ARGSUSED*/

static char *
svr4inst(pkg)
char *pkg;
{
	static char pkginst[PKGSIZ];
	static DIR *pdirfp;
	struct dirent *dp;
	struct stat	status;	/* file status buffer */
	char	*pt;
	char	path[PATH_MAX];

	if(pkg == NULL) {
		if(pdirfp) {
			closedir(pdirfp);
			pdirfp = NULL;
		}
		return(NULL);
	}

	if(!pdirfp && ((pdirfp = opendir(PKGOLD)) == NULL))
		return(NULL);

	while((dp = readdir(pdirfp)) != NULL) {
		if(dp->d_name[0] == '.')
			continue;
		pt = strchr(dp->d_name, '.');
		if(pt && !strcmp(pt, ".name")) {
			/* the pkgnmchk function works on .name extensions */
			if(pkgnmchk(dp->d_name, pkg, 1))
				continue;
			(void) sprintf(path, "%s/%s", PKGOLD, dp->d_name);
			if(lstat(path, &status))
				continue;
			if((status.st_mode & S_IFMT) != S_IFREG)
				continue;
			*pt = '\0';
			(void) strcpy(pkginst, dp->d_name);
			return(pkginst);
		}
	}
	closedir(pdirfp);
	pdirfp = NULL;
	return(NULL);
}

static int
verscmp(request, actual)
char *request, *actual;
{
	/* eat leading white space */
	while(isspace(*actual))
		actual++;
	while(isspace(*request))
		request++;

	while(*request) {
		if(isspace(*request)) {
			if(*actual && !isspace(*actual))
				break;
			while(isspace(*request))
				request++;
			while(isspace(*actual))
				actual++;
			continue;
		}
		if(*request++ != *actual)
			break;
		actual++;
	}
	while(isspace(*actual))
		actual++;
	return(*actual ? -1 : 0);
}

static int
compver(pkginst, version)
char *pkginst;
char *version;
{
	FILE *fp;
	char temp[256];

	(void) sprintf(temp, "%s/%s/install/compver", PKGLOC, pkginst);
	if((fp = fopen(temp, "r")) == NULL) 
		return(-1);
	
	while(fgets(temp, 256, fp)) {
		if(*temp == '#')
			continue;
		if(verscmp(temp, version) == 0) {
			(void) fclose(fp);
			return(0);
		}
	}
	(void) fclose(fp);
	return(-1);
}

static int
archcmp(arch, archlist)
char *arch, *archlist;
{
	register char *pt;

	if(arch == NULL)
		return(0);

	/* arch and archlist must not contain whitespace! */

	while(*archlist) {
		for(pt=arch; *pt && (*pt == *archlist);)
			pt++, archlist++;
		if(!*pt && (!*archlist || (*archlist == ',')))
			return(0);
		while(*archlist) {
			if(*archlist++ == ',')
				break;
		}
	}
	return(-1);
}

static int
ckinfo(inst, arch, vers)
char	*inst, *arch, *vers;
{
	FILE	*fp;
	char	temp[128];
	char	file[PATH_MAX];
	char	*pt, *copy, *value, *myarch, *myvers;
	int	errflg;

	(void) sprintf(file, "%s/%s/pkginfo", pkgdir, inst);
	if((fp = fopen(file, "r")) == NULL)
		return(1);

	if((arch == NULL) && (vers == NULL)) {
		fclose(fp);
		return(0);
	}
	temp[0] = '\0';
	myarch = myvers = NULL;
	while(value = fpkgparam(fp, temp)) {
		if(!strcmp(temp, "ARCH")) {
			/* remove all whitespace from value */
			pt = copy = value;
			while(*pt) {
				if(!isspace(*pt))
					*copy++ = *pt;
				pt++;
			}
			*copy = '\0';
			myarch = value;
			if(myvers)
				break;
		} else if(!strcmp(temp, "VERSION")) {
			myvers = value;
			if(myarch)
				break;
		} else
			(void) free(value);
		temp[0] = '\0';
	}
	fclose(fp);
	errflg = 0;

	if(ckinst(inst, myarch, myvers, arch, vers))
		errflg++;

	if(myarch)
		(void) free(myarch);
	if(myvers)
		(void) free(myvers);

	return(errflg);
}
