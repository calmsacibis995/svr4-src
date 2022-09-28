/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:installf/main.c	1.13.5.1"

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/errno.h>
#include <pkginfo.h>
#include <limits.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include <pkglocs.h>
#include "install.h"

extern char	*optarg, *errstr;
extern char	dbst;
extern int	optind;
extern int	errno;

extern void	*calloc();
extern int	pkgnmchk(),
		pkgdbmerg(),
		ocfile(),
		swapcfile(),
		averify(),
		installf(),
		dofinal(),
		unlink(),
		getopt();
extern long	atol();
extern void	progerr(),
		logerr(),
		removef(),
		exit();
extern char	*mktemp(), 
		*getenv(), 
		*pkgparam(), 
		*fpkginst();

#define BASEDIR	"/BASEDIR/"

#define INSTALF	(*prog == 'i')
#define REMOVEF	(*prog == 'r')

#define ERR_CLASSLONG	"classname argument too long"
#define ERR_CLASSCHAR	"bad character in classname"
#define ERR_INVAL	"package instance <%s> is invalid"
#define ERR_NOTINST	"package instance <%s> is not installed"
#define ERR_MERG	"unable to merge contents file"

char	*prog;
char	*classname = NULL;

struct cfent
	**eptlist;
char	*pkginst;
int	eptnum,
	warnflag,
	nointeract,
	nosetuid, 
	nocnflct;

void	quit(),
	usage();

main(argc,argv)
int	argc;
char	**argv;
{
	FILE	*mapfp, *tmpfp;
	struct mergstat *mstat;
	char	*pt;
	int	c, n, dbchg;
	int	fflag = 0;

	(void) signal(SIGHUP, exit);
	(void) signal(SIGINT, exit);
	(void) signal(SIGQUIT, exit);

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	while ((c = getopt(argc,argv,"c:f?")) != EOF) {
		switch(c) {
		  case 'f':
			fflag++;
			break;

		  case 'c':
			classname = optarg;
			/* validate that classname is acceptable */
			if(strlen(classname) > (size_t)CLSSIZ) {
				progerr(ERR_CLASSLONG);
				exit(1);
			}
			for(pt=classname; *pt; pt++) {
				if(!isalpha(*pt) && !isdigit(*pt)) {
					progerr(ERR_CLASSCHAR);
					exit(1);
				}
			}
			break;

		  default:
			usage();
		}
	}

	pkginst = argv[optind++];
	if(fflag) {
		/* installf and removef must only have pkginst */
		if(optind != argc)
			usage();
	} else {
		/* installf and removef must have at minimum
		 * pkginst & pathname specified on command line 
		 */
		if(optind > argc)
			usage();
	}
	if(REMOVEF) {
		if(classname)
			usage();
	}
	if(pkgnmchk(pkginst, "all", 0)) {
		progerr(ERR_INVAL, pkginst);
		exit(1);
	}
	if(fpkginst(pkginst, NULL, NULL) == NULL) {
		progerr(ERR_NOTINST, pkginst);
		exit(1);
	}

	if(ocfile(&mapfp, &tmpfp))
		exit(1);

	if(fflag)
		dbchg = dofinal(mapfp, tmpfp, REMOVEF, classname);
	else {
		if(INSTALF) {
			dbst = '+';
			if(installf(argc-optind, &argv[optind]))
				quit(1);
		} else {
			dbst = '-';
			removef(argc-optind, &argv[optind]);
		}

		/*
		 * alloc an array to hold information about how each
		 * entry in memory matches with information already
		 * stored in the "contents" file
		 */
		mstat = (struct mergstat *) calloc((unsigned int)eptnum, 
			sizeof(struct mergstat));

		dbchg = pkgdbmerg(mapfp, tmpfp, eptlist, mstat, 0);
		if(dbchg < 0) {
			progerr(ERR_MERG);
			quit(99);
		}
	}
	(void) fclose(mapfp);
	if(dbchg) {
		if(swapcfile(tmpfp, pkginst))
			quit(99);
	}

	if(REMOVEF && !fflag) {
		for(n=0; eptlist[n]; n++) {
			if(!mstat[n].shared)
				(void) printf("%s\n", eptlist[n]->path);
		}
	} else if(INSTALF && !fflag) {
		for(n=0; eptlist[n]; n++) {
			if(strchr("dxcbp", eptlist[n]->ftype)) {
				(void) averify(1, &eptlist[n]->ftype, 
				   eptlist[n]->path, &eptlist[n]->ainfo);
			}
		}
	}
	return(warnflag ? 1 : 0);
}

void
quit(n)
int n;
{
	exit(n);
}

void
usage()
{
	(void) fprintf(stderr, "usage:\n");
	if(REMOVEF) {
		(void) fprintf(stderr, 
			"\t%s pkginst path [path ...]\n", prog);
		(void) fprintf(stderr, "\t%s -f pkginst\n", prog);
	} else {
		(void) fprintf(stderr, 
			"\t%s [-c class] <pkginst> <path>\n", prog);
		(void) fprintf(stderr, 
			"\t%s [-c class] <pkginst> <path> <specs>\n", prog);
		(void) fprintf(stderr, 
			"\t   where <specs> may be defined as:\n");
		(void) fprintf(stderr, "\t\tf <mode> <owner> <group>\n"); 
		(void) fprintf(stderr, "\t\tv <mode> <owner> <group>\n"); 
		(void) fprintf(stderr, "\t\te <mode> <owner> <group>\n"); 
		(void) fprintf(stderr, "\t\td <mode> <owner> <group>\n"); 
		(void) fprintf(stderr, "\t\tx <mode> <owner> <group>\n"); 
		(void) fprintf(stderr, "\t\tp <mode> <owner> <group>\n"); 
		(void) fprintf(stderr, 
			"\t\tc <major> <minor> <mode> <owner> <group>\n");
		(void) fprintf(stderr, 
			"\t\tb <major> <minor> <mode> <owner> <group>\n");
		(void) fprintf(stderr, "\t\ts <path>=<srcpath>\n");
		(void) fprintf(stderr, "\t\tl <path>=<srcpath>\n");
		(void) fprintf(stderr, "\t%s [-c class] -f pkginst\n", prog);
	}
	exit(1);
}
