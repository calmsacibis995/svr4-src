/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkgremove/main.c	1.13.6.1"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include <pkginfo.h>
#include <pkglocs.h>
#include "install.h"

extern struct cfent
		**eptlist;
extern int	eptnum;
extern int	optind, errno;
extern char	*optarg, *pkgdir;
extern char	**environ;

extern void	free(), exit(), trap(),
		rckdepend(), rckpriv(),
		rckrunlevel(), predepend(),
		setadmin(), delmap(),
		putparam(), lockinst(),
		progerr(), echo(),
		quit();
extern int	access(), chdir(),
		rmdir(), getopt(),
		creat(), unlink(),
		pkgexecl(), setlist(),
		rrmdir();
extern char	*pkgparam(), *getenv(),
		*fpkgparam(), *tempnam(),
		*qstrdup();

#define DEFPATH		"/sbin:/usr/sbin:/usr/bin"
#define ERR_LOCKFILE	"unable to create lockfile <%s>"
#define ERR_CHDIR	"unable to change directory to <%s>"
#define ERR_PKGINFO	"unable to process pkginfo file <%s>"
#define ERR_CLASSES	"CLASSES parameter undefined in <%s>"
#define ERR_TMPFILE	"unable to establish temporary file"
#define ERR_WTMPFILE	"unable to write temporary file <%s>"
#define ERR_RMDIR	"unable to remove directory <%s>"
#define ERR_RMPATH	"unable to remove <%s>"
#define ERR_PREREMOVE	"preremove script did not complete successfully"
#define ERR_POSTREMOVE	"postremove script did not complete successfully"
#define ERR_CASFAIL	"class action script did not complete successfully"
#define MSG_NOTEMPTY	"%s <non-empty directory not removed>"
#define MSG_DIRBUSY	"%s <mount point not removed>"
#define MSG_SHARED	"%s <shared pathname not removed>"

struct admin
	adm;		/* holds info about installation admin */
int	reboot;		/* non-zero if reboot required after installation */
int	ireboot;	/* non-zero if immediate reboot required */
int	failflag;	/* non-zero if fatal error has occurred */
int	warnflag;	/* non-zero if non-fatal error has occurred */
int	nointeract;	/* non-zero if no interaction with user should occur */
int	started;

char	*pkginst;	/* current package (source) instance to process */
char	*prog;		/* the basename of argv[0] */

int	dbchg;
char	*msgtext;
char	pkgloc[PATH_MAX];

static char	pkgbin[PATH_MAX],
		rlockfile[PATH_MAX],
		*admnfile,	/* file to use for installation admin */
		*tmpdir,	/* location to place temporary files */
		**classlist;

static void	rmclass(),
		ckreturn(),
		usage();

main(argc, argv)
int	argc;
char	*argv[];
{
	FILE	*fp;
	char	*value, *pt;
	char	script[PATH_MAX],
		path[PATH_MAX],
		param[64];
	int	i, c;
	void	(*func)();

	(void)umask(0022);
	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];
	lockinst();

	while ((c = getopt(argc,argv,"a:n?")) != EOF) {
		switch(c) {
		  case 'a':
			admnfile = optarg;
			break;

		  case 'n':
			nointeract++;
			break;

		  default:
			usage();
		}
	}

	func = signal(SIGINT, trap);
	if(func != SIG_DFL)
		(void) signal(SIGINT, func);
	(void) signal(SIGHUP, trap);

	pkginst = argv[optind++];
	if(optind != argc)
		usage();

	tmpdir = getenv("TMPDIR");
	if(tmpdir == NULL)
		tmpdir = P_tmpdir;

	/*
	 * initialize installation admin parameters 
	 */
	setadmin(admnfile);

	(void) sprintf(pkgloc, "%s/%s", PKGLOC, pkginst);
	(void) sprintf(pkgbin, "%s/install", pkgloc);
	(void) sprintf(rlockfile, "%s/!R-Lock!", pkgloc);

	if(chdir(pkgbin)) {
		progerr(ERR_CHDIR, pkgbin);
		quit(99);
	}

	echo("\n## Removing installed package instance <%s>", pkginst);
	if(access(rlockfile, 0) == 0)
		echo("(A previous attempt may have been unsuccessful.)");

	/*
	 * process all parameters from the pkginfo file
	 * and place them in the execution environment
	 */
	(void) sprintf(path, "%s/pkginfo", pkgloc);
	if((fp = fopen(path, "r")) == NULL) {
		progerr(ERR_PKGINFO, path);
		quit(99);
	}

	environ = NULL;
	param[0] = '\0';
	while(value = fpkgparam(fp, param)) {
		if(strcmp(param, "PATH"))
			putparam(param, value);
		free(value);
		param[0] = '\0';
	}
	(void) fclose(fp);

	if(value = getenv("CLASSES"))
		(void) setlist(&classlist, qstrdup(value));
	else {
		progerr(ERR_CLASSES, path);
		quit(99);
	}

	(void) sprintf(path, "%s:%s", DEFPATH, PKGBIN);
	putparam("PATH", path);
	putparam("TMPDIR", tmpdir);

	/*
	 *  make sure current runlevel is appropriate
	 */
	rckrunlevel();

	/*
	 * determine if any packaging scripts provided with
	 * this package will execute as a priviledged user
	 */
	rckpriv();

	/*
	 *  verify package dependencies
	 */
	rckdepend();

	/*
	 *  create lockfile to indicate start of installation
	 */
	started++;
	if(creat(rlockfile, 0644) < 0) {
		progerr(ERR_LOCKFILE, rlockfile);
		quit(99);
	}

	echo("## Processing package information.");
	delmap(0);

	/*
	 *  execute preremove script, if any
	 */
	(void) sprintf(script, "%s/preremove", pkgbin);
	if(access(script, 0) == 0) {
		echo("## Executing preremove script.");
		ckreturn(pkgexecl(NULL, NULL, SHELL, script, NULL), 
			ERR_PREREMOVE);
	}

	/* reverse order of classes */
	for(i=0; classlist[i]; i++)
		;
	while(--i >= 0)
		rmclass(classlist[i]);
	rmclass(NULL);

	/*
	 *  execute postremove script, if any
	 */
	(void) sprintf(script, "%s/postremove", pkgbin);
	if(access(script, 0) == 0) {
		echo("## Executing postremove script.");
		ckreturn(pkgexecl(NULL, NULL, SHELL, script, NULL),
			ERR_POSTREMOVE);
	}

	echo("## Updating system information.");
	delmap(1);

	if(!warnflag && !failflag) {
		if(pt = getenv("PREDEPEND"))
			predepend(pt);
		(void) chdir("/");
		if(rrmdir(pkgloc))
			warnflag++;
	}
	quit(0);
	/*NOTREACHED*/
}

static void
rmclass(class)
char	*class;
{
	struct cfent	*ept;
	FILE	*fp;
	char	*tmpfile;
	char	script[PATH_MAX];
	int	i;

	if(class == NULL) {
		for(i=0; i < eptnum; i++) {
			if(eptlist[i] != NULL)
				rmclass(eptlist[i]->class);
		}
		return;
	}

	/* locate class action script to execute */
	(void) sprintf(script, "%s/r.%s", pkgbin, class);
	if(access(script, 0)) {
		(void) sprintf(script, "%s/r.%s", PKGSCR, class);
		if(access(script, 0))
			script[0] = '\0';
	}
	if(script[0]) {
		tmpfile = tempnam(tmpdir, "RMLIST");
		if(tmpfile == NULL) {
			progerr(ERR_TMPFILE);
			quit(99);
		}
		if((fp = fopen(tmpfile, "w")) == NULL) {
			progerr(ERR_WTMPFILE, tmpfile);
			quit(99);
		}
	}
	echo("## Removing pathnames in <%s> class", class);

	/* process paths in reverse order */
	i = eptnum;
	while(--i >= 0) {
		ept = eptlist[i];
		if((ept == NULL) || strcmp(class, ept->class))
			continue;

		if(!ept->ftype)
			echo(MSG_SHARED, ept->path);
		else if(script[0])
			(void) fprintf(fp, "%s\n", ept->path);
		else if(strchr("dx", ept->ftype)) {
			if(rmdir(ept->path)) {
				/* remove directory */
				if(errno == EBUSY)
					echo(MSG_DIRBUSY, ept->path);
				if(errno == EEXIST)
					echo(MSG_NOTEMPTY, ept->path);
				else if(errno != ENOENT) {
					progerr(ERR_RMDIR, ept->path);
					warnflag++;
				} /* else directory does not exist */
			} else
			   if (!(ADM(list_files,"nocheck")))
				echo("%s", ept->path);
		} else if(unlink(ept->path)) {
			if(errno != ENOENT) {
				progerr(ERR_RMPATH, ept->path);
				warnflag++;
			} /* else pathname does not exist */
		} else
		   if (!(ADM(list_files,"nocheck")))
			echo("%s", ept->path);

		/* free memory allocated for this entry;
		 * memory used for pathnames will be freed
		 * later by a call to pathdup() */
		free(ept);
		eptlist[i] = NULL;
	}
	if(script[0]) {
		(void) fclose(fp);
		ckreturn(pkgexecl(tmpfile, NULL, SHELL, script, NULL),
			ERR_CASFAIL);
	}
}

static void
ckreturn(retcode, msg)
int	retcode;
char	*msg;
{
	switch(retcode) {
	  case 2:
	  case 12:
	  case 22:
		warnflag++;
		/* fall through */
		if(msg)
			progerr(msg);
	  case 10:
	  case 20:
		if(retcode >= 10)
			reboot++;
		if(retcode >= 20)
			ireboot++;
		/* fall through */
	  case 0:
		break; /* okay */

	  case -1:
		retcode = 99;
	  case 99:
	  case 1:
	  case 11:
	  case 21:
	  case 4:
	  case 14:
	  case 24:
	  case 5:
	  case 15:
	  case 25:
		if(msg)
			progerr(msg);
		/* fall through */
	  case 3:
	  case 13:
	  case 23:
		quit(retcode);
		/* NOT REACHED */
	  default:
		if(msg)
			progerr(msg);
		quit(1);
	}
}

static void
usage()
{
	(void) fprintf(stderr, "usage: %s [-a admin] [-n] pkginst\n", prog);
	exit(1);
}
