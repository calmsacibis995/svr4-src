/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkgmk/mkpkgmap.c	1.11.8.1"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <pkgstrct.h>
#include <pkginfo.h>
#include "install.h"

extern char	*errstr, *basedir,
		*root, *rootlist[],
		**environ;
extern int	errno, attrpreset;

extern void	cvtpath(), free(),
		progerr(), logerr(),
		basepath(), mappath(), 
		mapvar(), canonize(), 
		quit();
extern long	strtol();
extern int	chdir(), putenv(),
		cverify(), gpkgmap(),
		ppkgmap();
extern char	*getenv(), *getcwd(), 
		*qstrdup();

#define MAXPATHS	1024
#define PATHSIZE 1024
#define MAXPARAMS 256
#define NRECURS 20

#define MSG_BPARAMC	"parametric class specification for <%s> not allowed"
#define MSG_SRCHLOC	"no object for <%s> found in local path"
#define MSG_SRCHSRCH	"no object for <%s> found in search path"
#define MSG_SRCHROOT	"no object for <%s> found in root directory"
#define MSG_CONTENTS	"unable to process contents of object <%s>"
#define MSG_WRITE	"write of <%s> entry failed, errno=%d"
#define MSG_GARBDEFLT	"garbled default settings: %s"
#define MSG_BANG	"unknown directive: %s"
#define MSG_CHDIR	"unable to change directory to <%s>"
#define MSG_INCOMPLETE	"processing of <%s> may be incomplete"
#define MSG_NRECURS	"too many levels of include (limit is %d)"
#define MSG_RDINCLUDE	"unable to process include file <%s>, errno=%d"
#define MSG_IGNINCLUDE	"ignoring include file <%s>"
#define ERR_TEMP	"unable to obtain temporary file resources, errno=%d"
#define ERR_ENVBUILD	"unable to build parameter environment, errno=%d"
#define ERR_MAXPARAMS	"too many parameter definitions (limit is %d)"
#define ERR_GETCWD	"unable to get current directory"

static struct cfent entry;
static FILE	*fp,
		*sfp[20];
static char	*dname[NRECURS],
		*params[256],
		*proto[NRECURS],
		*rootp[NRECURS][16],
		*srchp[NRECURS][16],
		*d_own[NRECURS],
		*d_grp[NRECURS],
		*rdonly[256];
static int	d_mod[NRECURS];
static int	nfp = (-1);
static int	nrdonly = 0;
static int	errflg = 0;
static char	*separ = " \t\n,";

static char	*findfile(), *srchroot();
static void	doattrib(), dosearch(), dorsearch(), doinclude(),
		translate(), error(), pushenv(), lputenv(); 
static int	popenv();

int
mkpkgmap(outfile, protofile, envparam)
char	*outfile, *protofile, **envparam;
{
	FILE	*tmpfp;
	char	*pt, *path, mybuff[PATHSIZE], temp[PATHSIZE];
	char	**envsave;
	int	c;
	int	i, n;

	if((tmpfp = fopen(outfile, "w")) == NULL) {
		progerr(ERR_TEMP, errno);
		quit(99);
	}
	envsave = environ;
	environ = params; /* use only local environ */
	for(i=0; pt = strchr(envparam[i], '='); i++) {
		*pt = '\0';
		rdonly[nrdonly++] = qstrdup(envparam[i]);
		*pt = '=';
		if(putenv(qstrdup(envparam[i++]))) {
			progerr(ERR_ENVBUILD, errno);
			quit(99);
		}
		if(nrdonly >= MAXPARAMS) {
			progerr(ERR_MAXPARAMS, MAXPARAMS);
			quit(1);
		}
	}

	pushenv(protofile);
	attrpreset = 1;
	errflg = 0;
again:
	while(!feof(fp)) {
		c = getc(fp);
		while(isspace(c))
			c = getc(fp);

		if(c == '#') {
			do c = getc(fp); while((c != EOF) && (c != '\n'));
			continue;
		}
		if(c == EOF)
			break;

		if(c == '!') {
			(void) fscanf(fp, "%s", temp);
			if(!strcmp(temp, "include"))
				doinclude();
			else if(!strcmp(temp, "rsearch"))
				dorsearch();
			else if(!strcmp(temp, "search"))
				dosearch();
			else if(!strcmp(temp, "default"))
				doattrib();
			else if(strchr(temp, '=')) {
				translate(temp, mybuff);
				lputenv(mybuff);
				(void) fscanf(fp, "%*[^\n]"); /* rest of line */
				(void) fscanf(fp, "\n"); /* rest of line */
			} else {
				error(1);
				logerr(MSG_BANG, temp);
				(void) fscanf(fp, "%*[^\n]"); /* read of line */
				(void) fscanf(fp, "\n"); /* read of line */
			}
			continue;
		}
		(void) ungetc(c, fp);

		entry.ainfo.mode = d_mod[nfp];
		(void) strcpy(entry.ainfo.owner, d_own[nfp] ? d_own[nfp] : "?");
		(void) strcpy(entry.ainfo.group, d_grp[nfp] ? d_grp[nfp] : "?");

		if((n = gpkgmap(&entry, fp)) < 0) {
			error(1);
			logerr("garbled entry");
			logerr("- pathname: %s", entry.path);
			logerr("- problem: %s", errstr);
			break;
		}
		if(n == 0)
			break; /* done with file */

		/* don't allow classname to be parametric */
		if(entry.ftype != 'i') {
			if(entry.class[0] == '$') {
				error(1);
				logerr(MSG_BPARAMC, entry.path);
			}
		}

		/** COMMENT OUT (allow owner & group to have parametric values
		if(!strchr("ilsn", entry.ftype)) {
			if((entry.ainfo.owner[0] == '$') &&
			  !isupper(entry.ainfo.owner[1])) {
				error(1);
				logerr(MSG_BPARAMO, entry.path);
			}
			if((entry.ainfo.group[0] == '$') &&
			  !isupper(entry.ainfo.group[1])) {
				error(1);
				logerr(MSG_BPARAMG, entry.path);
			}
		}
		** END COMMENT OUT */

		if(strchr("dxlscbp", entry.ftype))
			path = NULL; /* don't need to search for things without
					any contents in them */
		else {
			path = findfile(entry.path, entry.ainfo.local);
			if(!path)
				continue;

			entry.ainfo.local = path;
			if(strchr("fevin?" , entry.ftype)) {
				if(cverify(0, &entry.ftype, path, 
				&entry.cinfo)) {
					error(1);
					logerr(MSG_CONTENTS, path);
				}
			}
		}

		/* substitute parameters in the path which 
		 * begin with a lowercase character with their
		 * appropriate values
		 */
		if(strchr("fevdxcbp", entry.ftype)) {
			mapvar(1, entry.ainfo.owner);
			mapvar(1, entry.ainfo.group);
		} else if(strchr("ls", entry.ftype)) {
			if(strchr("l", entry.ftype) ||
			   (!isdot(entry.ainfo.local) &&
			    !isdotdot(entry.ainfo.local))) {
				mappath(1, entry.ainfo.local);
				canonize(entry.ainfo.local);
			}
		}
		mappath(1, entry.path);
		canonize(entry.path);
		if(ppkgmap(&entry, tmpfp)) {
			error(1);
			logerr(MSG_WRITE, entry.path, errno);
			break;
		}
	}
	if(popenv())
		goto again;

	(void) fclose(tmpfp);
	environ = envsave; /* restore environment */

	return(errflg ? 1 : 0);
}

static char*
findfile(path, local)
char *path;
char *local;
{
	struct stat statbuf;
	static char host[PATHSIZE];
	register char *pt;
	char	temp[PATHSIZE], *basename;
	int	i;

	/* map any parameters specified in path to their 
	 * corresponding values and make sure the path is
	 * in its canonical form; any parmeters for which
	 * a value is not defined will be left unexpanded
	 */
	(void) strcpy(temp, (local && local[0] ? local : path));
	mappath(0, temp);
	canonize(temp);

	*host = '\0';
	if(rootlist[0] || (basedir && (*temp != '/'))) {
		/* search for path in the pseduo-root/basedir directory;
		 * note that package information files should NOT be 
		 * included in this list */
		if(entry.ftype != 'i')
			return(srchroot(temp, host));
	}

	/* looking for local object file  */
	if(*local) {
		basepath(temp, dname[nfp]);
		if(stat(temp, &statbuf) || !((statbuf.st_mode & S_IFMT) == S_IFREG)) {
			error(1);
			logerr(MSG_SRCHLOC, path);
			return(NULL);
		}
		(void) strcpy(host, temp);
		return(host);
	}

	for(i=0; rootp[nfp][i]; i++) {
		(void) sprintf(host, "%s/%s", rootp[nfp][i], 
			temp + (*temp == '/'));
		if((stat(host, &statbuf) == 0) && ((statbuf.st_mode & S_IFMT) == S_IFREG)) {
			return(host);
		}
	}

	pt = strrchr(temp, '/');
	if(!pt++)
		pt = temp;

	basename = pt;

	for(i=0; srchp[nfp][i]; i++) {
		(void) sprintf(host, "%s/%s", srchp[nfp][i], basename);
		if((stat(host, &statbuf) == 0) && ((statbuf.st_mode & S_IFMT) == S_IFREG)) {
			return(host);
		}
	}

	/* check current directory as a last resort */
	(void) sprintf(host, "%s/%s", dname[nfp], basename);
	if((stat(host, &statbuf) == 0) && ((statbuf.st_mode & S_IFMT) == S_IFREG))
		return(host);

	error(1);
	logerr(MSG_SRCHSRCH, path);
	return(NULL);
}

static void
dosearch()
{
	char temp[PATHSIZE], lookpath[PATHSIZE], *pt;
	int n;

	(void) fgets(temp, PATHSIZE, fp);
	translate(temp, lookpath);

	for(n=0; srchp[nfp][n]; n++)
		free(srchp[nfp][n]);

	n = 0;
	pt = strtok(lookpath, separ);
	do {
		if(*pt != '/') {
			/* make relative path an absolute directory */
			(void) sprintf(temp, "%s/%s", dname[nfp], pt);
			pt = temp;
		}
		canonize(pt);
		srchp[nfp][n++] = qstrdup(pt);
	} while(pt = strtok(NULL, separ));
	srchp[nfp][n] = NULL;
}

static void
dorsearch()
{
	char temp[PATHSIZE], lookpath[PATHSIZE], *pt;
	int n;

	(void) fgets(temp, PATHSIZE, fp);
	translate(temp, lookpath);

	for(n=0; rootp[nfp][n]; n++)
		free(srchp[nfp][n]);

	n = 0;
	pt = strtok(lookpath, separ);
	do {
		if(*pt != '/') {
			/* make relative path an absolute directory */
			(void) sprintf(temp, "%s/%s", dname[nfp], pt);
			pt = temp;
		}
		canonize(pt);
		rootp[nfp][n++] = qstrdup(pt);
	} while(pt = strtok(NULL, separ));
	rootp[nfp][n] = NULL;
}

static void
doattrib()
{
	char *pt, attrib[PATHSIZE];
	int mode;
	char owner[ATRSIZ+1], group[ATRSIZ+1];

	(void) fgets(attrib, PATHSIZE, fp);
	mapvar(1, attrib);
	/*translate(temp, attrib); replace attrib above with temp*/

	/* don't use scanf, since we want to force an octal
	 * interpretation and need to limit the length of
	 * the owner and group specifications
	 */
	mode = strtol(attrib, &pt, 8);
	if(pt == attrib) {
		error(1);
		logerr(MSG_GARBDEFLT, attrib);
		return;
	}
	if((pt = strtok(pt, " \t\n")) == NULL) {
		error(1);
		logerr(MSG_GARBDEFLT, attrib);
		return;
	}
	(void) strncpy(owner, pt, ATRSIZ);
	owner[ATRSIZ] = '\0';
	if((pt = strtok(NULL, " \t\n")) == NULL) {
		error(1);
		logerr(MSG_GARBDEFLT, attrib);
		return;
	}
	(void) strncpy(group, pt, ATRSIZ);
	group[ATRSIZ] = '\0';
	if(strtok(NULL, " \t\n")) {
		/* extra tokens on the line */
		error(1);
		logerr(MSG_GARBDEFLT, attrib);
		return;
	}

	/* free any previous memory from qstrdup */
	if(d_own[nfp]) 
		free(d_own[nfp]);
	if(d_grp[nfp])
		free(d_grp[nfp]);

	d_mod[nfp] = mode;
	d_own[nfp] = qstrdup(owner);
	d_grp[nfp] = qstrdup(group);
}

static void
doinclude()
{
	char file[PATHSIZE], temp[PATHSIZE];

	(void) fgets(temp, PATHSIZE, fp);
	(void) sscanf(temp, "%s", file);
	translate(file, temp);
	canonize(temp);

	if(*temp != '/')
		(void) sprintf(file, "%s/%s", dname[nfp], temp);
	else
		(void) strcpy(file, temp);

	canonize(file);
	pushenv(file);
}

static void
translate(pt, copy)
register char *pt, *copy;
{
	char *pt2, varname[64];

token:
	/* eat white space */
	while(isspace(*pt))
		pt++;
	while(*pt && !isspace(*pt)) {
		if(*pt == '$') {
			pt2 = varname;
			while(*++pt && !strchr("/= \t\n\r", *pt))
				*pt2++ = *pt;
			*pt2 = '\0';
			if(pt2 = getenv(varname)) {
				while(*pt2)
					*copy++ = *pt2++;
			}
		} else
			*copy++ = *pt++;
	}
	if(*pt) {
		*copy++ = ' ';
		goto token;
	}
	*copy = '\0';
}

static void
error(flag)
int flag;
{
	static char *lasterr = NULL;

	if(lasterr != proto[nfp]) {
		lasterr = proto[nfp];
		(void) fprintf(stderr, "ERROR in %s:\n", lasterr);
	}
	if(flag)
		errflg++;
}

static void
pushenv(file)
char *file;
{
	register char *pt;
	static char	topdir[PATHSIZE];

	if((nfp+1) >= NRECURS) {
		error(1);
		logerr(MSG_NRECURS, NRECURS);
		logerr(MSG_IGNINCLUDE, file);
		return;
	}

	if(!strcmp(file, "-"))
		fp = stdin;
	else if((fp = fopen(file, "r")) == NULL) {
		error(1);
		logerr(MSG_RDINCLUDE, file, errno);
		if(nfp >= 0) {
			logerr(MSG_IGNINCLUDE, file);
			fp = sfp[nfp];
			return;
		} else
			quit(1);
	}
	sfp[++nfp] = fp;
	srchp[nfp][0] = NULL;
	rootp[nfp][0] = NULL;
	d_mod[nfp] = (-1);
	d_own[nfp] = NULL;
	d_grp[nfp] = NULL;

	if(!nfp) {
		/* upper level proto file */
		proto[nfp] = file;
		if(file[0] == '/')
			pt = strcpy(topdir, file);
		else {
			/* path is relative to the prototype file specified */
			pt = getcwd(NULL, PATHSIZE);
			if(pt == NULL) {
				progerr(ERR_GETCWD, errno);
				quit(99);
			}
			(void) sprintf(topdir, "%s/%s", pt, file);
		}
		if(pt = strrchr(topdir, '/'))
			*pt = '\0'; /* should always happen */
		if(topdir[0] == '\0')
			(void) strcpy(topdir, "/");
		dname[nfp] = topdir;
	} else {
		proto[nfp] = qstrdup(file);
		dname[nfp] = qstrdup(file);
		if(pt = strrchr(dname[nfp], '/'))
			*pt = '\0';
		else {
			/* same directory as the last prototype */
			free(dname[nfp]);
			dname[nfp] = qstrdup(dname[nfp-1]);
			return; /* no need to canonize() or chdir() */
		}
	}

	canonize(dname[nfp]);

	if(chdir(dname[nfp])) {
		error(1);
		logerr(MSG_CHDIR, dname[nfp]);
		if(!nfp)
			quit(1); /* must be able to cd to upper level */
		logerr(MSG_IGNINCLUDE, proto[nfp]);
		(void) popenv();
	}
}

static int
popenv()
{
	int i;

	(void) fclose(fp);
	if(nfp) {
		if(proto[nfp])
			free(proto[nfp]);
		if(dname[nfp])
			free(dname[nfp]);
		for(i=0; srchp[nfp][i]; i++)
			free(srchp[nfp][i]);
		for(i=0; rootp[nfp][i]; i++)
			free(rootp[nfp][i]);
		if(d_own[nfp])
			free(d_own[nfp]);
		if(d_grp[nfp])
			free(d_grp[nfp]);
		
		fp = sfp[--nfp];

		if(chdir(dname[nfp])) {
			error(1);
			logerr(MSG_CHDIR, dname[nfp]);
			logerr(MSG_INCOMPLETE, proto[nfp]);
			return(popenv());
		}
		return(1);
	}
	return(0);
}

static void
lputenv(s)
char *s;
{
	char *pt;
	int i;

	pt = strchr(s, '=');
	if(!pt)
		return;

	*pt = '\0';
	for(i=0; i < nrdonly; i++) {
		if(!strcmp(rdonly[i], s)) {
			*pt = '=';
			return;
		}
	}
	*pt = '=';

	if(putenv(qstrdup(s))) {
		progerr(ERR_ENVBUILD);
		quit(99);
	}
}

static char *
srchroot(path, copy)
char *path, *copy;
{
	struct stat statbuf;
	int i;

	i = 0;
	root = rootlist[i++];
	do {
		/* convert with root & basedir info */
		cvtpath(path, copy);
		/* make it pretty again */
		canonize(copy);

		if(stat(copy, &statbuf) || !((statbuf.st_mode & S_IFMT) == S_IFREG)) {
			root = rootlist[i++];
			continue; /* host source must be a regular file */
		}
		return(copy);
	} while(root != NULL);
	error(1);
	logerr(MSG_SRCHROOT, path);
	return(NULL);
}
