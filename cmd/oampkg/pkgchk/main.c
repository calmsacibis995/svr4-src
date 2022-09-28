/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkgchk/main.c	1.5.5.1"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pkginfo.h>
#include <pkglocs.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include <pkgtrans.h>

extern int	optind;
extern char	*optarg;

extern char	*qstrdup(), *devattr(),
		**gpkglist();
extern void	progerr(), free(), exit();
extern int	getopt(), isdir(), mkdir(),
		pkgtrans(), rrmdir(),
		checkmap();

#define MAXPATHS	1024
#define MALLSIZ		50

#define ERR_POPTION	"no pathname included with -p option"
#define ERR_MAXPATHS	"too many pathnames in option list (limit is %d)"
#define ERR_NOTADIR	"spool specification <%s> must be a directory"

char	*prog = NULL;
char	**pkg = NULL;
int	pkgcnt = 0;
char	*basedir;
char	*pathlist[MAXPATHS];
short	used[MAXPATHS];
short	npaths;

int	aflag = (-1);
int	cflag = (-1);
int	vflag = 0;
int	nflag = 1;
int	lflag = 0;
int	Lflag = 0;
int	fflag = 0;
int	xflag = 0;
int	qflag = 0;	
char 	*device,
 	pkgspool[PATH_MAX];

static char	*allpkg[] = {"all", NULL};
static char	*mapfile,
		*spooldir,
		*envfile;
static int	errflg = 0;

static void	setpathlist(),
		usage();

main(argc, argv)
int argc;
char *argv[];
{
	char	*tmpdir,
		file[PATH_MAX+1];
	int	c, n;

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	while ((c = getopt(argc,argv,"e:p:d:nNLli:vam:cqxf?")) != EOF) {
		switch(c) {
		  case 'p':
			pathlist[npaths] = strtok(optarg, " ,");
			if(pathlist[npaths++] == NULL) {
				progerr(ERR_POPTION);
				exit(1);
			}
			while(pathlist[npaths] = strtok(NULL, " ,")) {
				if(npaths++ >= MAXPATHS) {
					progerr(ERR_MAXPATHS, MAXPATHS);
					exit(1);
				}
			}
			break;

		  case 'd':
			device = optarg;
			break;

		  case 'N':
			nflag = 0;
			break;

		  case 'n':
			nflag++;
			break;

		  case 'f':
			fflag++;
			break;

		  case 'i':
			setpathlist(optarg);
			break;

		  case 'v':
			vflag++;
			break;

		  case 'l':
			lflag++;
			break;

		  case 'L':
			Lflag++;
			break;

		  case 'x':
			if(aflag < 0) aflag = 0;
			if(cflag < 0) cflag = 0;
			xflag++;
			break;

		  case 'q':
			qflag++;
			break;

		  case 'a':
			if(cflag < 0)
				cflag = 0;
			aflag = 1;
			break;

		  case 'c':
			if(aflag < 0)
				aflag = 0;
			cflag = 1;
			break;

		  case 'e':
			envfile = optarg;
			break;

		  case 'm':
			mapfile = optarg;
			break;

		  default:
			usage();
		}
	}

	if(lflag || Lflag) {
		/* we're only suppose to list information */
		if((cflag >=0) || (aflag >=0) || 
		qflag || xflag || fflag || nflag || vflag)
			usage();
	}

	pkg = &argv[optind];
	pkgcnt = (argc - optind);
	
	errflg = 0;
	if(mapfile) {
		/* check for incompatable options */
		if(device || pkgcnt)
			usage();
		if(checkmap(0, mapfile, envfile, NULL))
			errflg++;
	} else if(device) {
		/* check for incompatable options */
		if((cflag >= 0) || (aflag >= 0))
			usage();
		if(qflag || xflag || fflag || nflag || envfile)
			usage();

		tmpdir = NULL;
		if((spooldir = devattr(device, "pathname")) == NULL)
			spooldir = device;
		if(isdir(spooldir)) {
			tmpdir = spooldir = qstrdup(tmpnam(NULL));
			if(mkdir(spooldir, 0755)) {
				progerr("unable to make directory <%s>", 
					spooldir);
				exit(99);
			}
			if(n = pkgtrans(device, spooldir, pkg, PT_SILENT))
				exit(n);
			pkg = gpkglist(spooldir, allpkg);
		} else
			pkg = gpkglist(spooldir, pkgcnt ? pkg : allpkg);

		if(pkg == NULL) {
			progerr("no packages selected for verification");
			exit(1);
		}

		aflag = 0;
		for(n=0; pkg[n]; n++) {
			(void) printf("## checking spooled package <%s>\n",
				 pkg[n]);
			(void) sprintf(pkgspool, "%s/%s", spooldir, pkg[n]);
			(void) sprintf(file, "%s/pkgmap", pkgspool);
			if(checkmap(0, file, NULL, pkg[n]))
				errflg++;
		}
		if(tmpdir)
			(void) rrmdir(tmpdir);
	} else {
		if(envfile)
			usage();

		(void) sprintf(file, "%s/contents", PKGADM);
		if(checkmap(1, file, NULL, NULL))
			errflg++;
	} 
	exit(errflg ? 1 : 0);
	/*NOTREACHED*/
}

static void
setpathlist(file)
char *file;
{
	FILE *fplist;
	char pathname[PATH_MAX+1];

	if((fplist = fopen(file, "r")) == NULL) {
		progerr("unable to open input file <%s>", file);
		exit(1);
	}
	while(fscanf(fplist, "%s", pathname) == 1) {
		pathlist[npaths] = qstrdup(pathname);
		if(npaths++ > MAXPATHS) {
			progerr("too many pathnames in list, limit is %d",
				MAXPATHS);
			exit(1);
		}
	}
	(void) fclose(fplist);
}

void
quit(n)
int n;
{
	exit(n);
	/*NOTREACHED*/
}

#define USAGE1	"\t%s [-l|vqacnNxf] [-p path[,...]] [-i file] [options]\n"
#define USAGE2	"\t%s -d device [-l|v] [-p path[,...]] [-i file] [pkginst [...]]\n"
#define USEOPTS	"\t-m pkgmap [-e envfile]\n\tpkginst [...]\n"

static void
usage()
{
	(void) fprintf(stderr, "usage:\n");
	(void) fprintf(stderr, USAGE1, prog);
	(void) fprintf(stderr, USAGE2, prog);
	(void) fprintf(stderr, 
		"   where options may include ONE of the following:\n");
	(void) fprintf(stderr, USEOPTS);
	exit(1);
	/*NOTREACHED*/
}
