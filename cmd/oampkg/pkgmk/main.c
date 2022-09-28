/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkgmk/main.c	1.2.10.1"

#include <sys/param.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include <pkgdev.h>
#include <pkginfo.h>
#include <pkglocs.h>
#ifdef PRESVR4
#include <sys/statfs.h>
#else
#include <sys/statvfs.h>
#include <sys/utsname.h>
#endif /* PRESVR4 */

extern char	**environ, *optarg, 
		*pkgdir;
extern int	optind, errno;

extern struct cfent 
		**procmap();
extern void	*calloc(), *realloc(),
		progerr(), logerr(),
		putparam(), quit(),
		free(), exit();
extern char	*qstrdup(), *srcpath(), 
		*fpkgparam(), *pkgparam(), 
		*devattr(), *fpkginst(), 
		*getenv();
extern long	atol(), time();
extern int	getopt(), access(),
		cftime(), mkdir(),
		isdir(), symlink(),
		splpkgmap(), mkpkgmap(),
		copyf(), rrmdir(),
		setlist(), ppkgmap(), 
		devtype(), pkgmount(),
		pkgumount(), cverify();

#define MALSIZ	16
#define NROOT	8
#define SPOOLDEV	"spool"

#define MSG_PROTOTYPE	"## Building pkgmap from package prototype file.\n"
#define MSG_PKGINFO	"## Processing pkginfo file.\n"
#define MSG_VOLUMIZE	"## Attempting to volumize %d entries in pkgmap.\n"
#define ERR_MEMORY	"memory allocation failure, errno=%d"
#define ERR_NROOT	"too many paths listed with -r option, limit is %d"
#define ERR_PKGINST	"invalid package instance identifier <%s>"
#define ERR_PKGABRV	"invalid package abbreviation <%s>"
#define ERR_BADDEV	"unknown or invalid device specified <%s>"
#define ERR_TEMP	"unable to obtain temporary file resources, errno=%d"
#define ERR_DSTREAM	"invalid device specified (datastream) <%s>"
#define ERR_SPLIT	"unable to volumize package"
#define ERR_MKDIR	"unable to make directory <%s>"
#define ERR_SYMLINK	"unable to create symbolic link for <%s>"
#define ERR_OVERWRITE	"must use -o option to overwrite <%s>"
#define ERR_UMOUNT	"unable to unmount device <%s>"
#define ERR_NOPKGINFO	"unable to locate required pkginfo file"
#define ERR_RDPKGINFO	"unable to process pkginfo file <%s>"
#define ERR_PROTOTYPE	"unable to locate prototype file"
#define ERR_STATVFS	"unable to stat filesystem <%s>"
#define ERR_DEVICE	"unable to find info for device <%s>"
#define ERR_BUILD	"unable to build pkgmap from prototype file"
#define ERR_ONEVOL	"package does must fit on a single volume"
#define ERR_FREE	"package does not fit space currently available in <%s>"
#define ERR_NOPARAM	"parameter <%s> is not defined in <%s>"
#define WRN_MISSINGDIR	"WARNING: missing directory entry for <%s>"
#define WRN_SETPARAM	"WARNING: parameter <%s> set to \"%s\""
#define WRN_CLASSES	\
	 "WARNING: unreferenced class <%s> in prototype file"

struct pkgdev pkgdev;	/* holds info about the installation device */
int	started;
char	pkgloc[PATH_MAX];
char	*prog; 
char	*basedir; 
char	*root; 
char	*rootlist[NROOT];
char	*t_pkgmap; 
char	*t_pkginfo;
char	**class;
int	nclass = (-1);	/* forces procmap() to use all classes */

static struct cfent *svept;
static char	**allclass,
		*protofile,
		*device; 
static long	limit;
static int	overwrite,
		ilimit,
		nflag,
		sflag;
static void
	trap(), 
	addclass(), 
	outvol(),
	ckmissing(),
	usage();
static int
	slinkf();

main(argc,argv)
int	argc;
char	*argv[];
{
#ifdef PRESVR4
	struct statfs statbuf;
#else
	struct utsname utsbuf;
	struct statvfs statvfsbuf;
#endif /* PRESVR4 */
	struct cfent	**eptlist;
	FILE	*fp;
	int	i, j, c, n, eptnum, found,
		part, nparts, npkgs, objects;
	char	buf[64], temp[64], param[64],
		*pt, *value, *pkginst, *tmpdir,
		**envparam, **order;
	void	(*func)();
	long	clock;
	ulong	bsize;

        limit = 0L;
	ilimit = 0;
	bsize = (ulong) 0;

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	func = sigset(SIGINT, trap);
	if(func != SIG_DFL)
		func = sigset(SIGINT, func);
	func = sigset(SIGHUP, trap);
	if(func != SIG_DFL)
		func = sigset(SIGHUP, func);

	environ = NULL;
	while((c = getopt(argc, argv, "osnp:l:r:b:d:f:a:v:?")) != EOF) {
		switch(c) {
		  case 'n':
			nflag++;
			break;

		  case 's':
			sflag++;
			break;

		  case 'o':
			overwrite++;
			break;

		  case 'p':
			putparam("PSTAMP", optarg);
			break;

		  case 'l':
			limit = atol(optarg);
			break;

		  case 'r':
			pt = strtok(optarg, " \t\n,");
			n = 0;
			do {
				rootlist[n++] = pt;
				if(n >= NROOT) {
					progerr(ERR_NROOT, NROOT);
					quit(1);
				}
			} while(pt = strtok(NULL, " \t\n,"));
			rootlist[n] = NULL;
			break;

		  case 'b':
			basedir = optarg;
			break;

		  case 'f':
			protofile = optarg;
			break;

		  case 'd':
			device = optarg;
			break;

		  case 'a':
			putparam("ARCH", optarg);
			break;

		  case 'v':
			putparam("VERSION", optarg);
			break;

		  default:
			usage();
		}
	}

	/* 
	 * add command line variable assignments to environment
	 */
	envparam = &argv[optind];
	while(argv[optind] && strchr(argv[optind], '='))
		optind++;
	if(pkginst = argv[optind]) {
		if(pkgnmchk(pkginst, "all", 0)) {
			progerr(ERR_PKGINST, pkginst);
			quit(1);
		}
		argv[optind++] = NULL;
	}
	if(optind != argc)
		usage();

	tmpdir = getenv("TMPDIR");
	if(tmpdir == NULL) 
		tmpdir = P_tmpdir;

	if(device == NULL) {
		device = devattr(SPOOLDEV, "pathname");
		if(device == NULL) {
			progerr(ERR_DEVICE, SPOOLDEV);
			exit(99);
		}
	}

	if(protofile == NULL) {
		if(access("prototype", 0) == 0)
			protofile = "prototype";
		else if(access("Prototype", 0) == 0)
			protofile = "Prototype";
		else {
			progerr(ERR_PROTOTYPE);
			quit(1);
		}
	}

	if(devtype(device, &pkgdev)) {
		progerr(ERR_BADDEV, device);
		quit(1);
	}
	if(pkgdev.norewind) {
		/* initialize datastream */
		progerr(ERR_DSTREAM, device);
		quit(1);
	}
	if(pkgdev.mount) {
		if(pkgmount(&pkgdev, NULL, 0, 0, 1))
			quit(n);
	}

	/*
	 * convert prototype file to a pkgmap, while locating
	 * package objects in the current environment
	 */
	t_pkgmap = tempnam(tmpdir, "tmpmap");
	if(t_pkgmap == NULL) {
		progerr(ERR_TEMP, errno);
		exit(99);
	}

	(void) fprintf(stderr, MSG_PROTOTYPE);
	if(n = mkpkgmap(t_pkgmap, protofile, envparam)) {
		progerr(ERR_BUILD);
		quit(1);
	}
	if((fp = fopen(t_pkgmap, "r")) == NULL) {
		progerr(ERR_TEMP, errno);
		quit(99);
	}
	eptlist = procmap(fp, 0);
	if(eptlist == NULL)
		quit(1);
	(void) fclose(fp);

	(void) fprintf(stderr, MSG_PKGINFO); 
	pt = NULL;
	for(i=0; eptlist[i]; i++) {
		ckmissing(eptlist[i]->path, eptlist[i]->ftype);
		if(eptlist[i]->ftype != 'i')
			continue;
		if(!strcmp(eptlist[i]->path, "pkginfo"))
			svept = eptlist[i];
	}
	if(svept == NULL) {
		progerr(ERR_NOPKGINFO);
		quit(99);
	}
	eptnum = i;

	/*
	 * process all parameters from the pkginfo file
	 * and place them in the execution environment
	 */
	if((fp = fopen(svept->ainfo.local, "r")) == NULL) {
		progerr(ERR_RDPKGINFO, svept->ainfo.local);
		quit(99);
	}
	param[0] = '\0';
	while(value = fpkgparam(fp, param)) {
		if(getenv(param) == NULL)
			putparam(param, value);
		free((void *)value);
		param[0] = '\0';
	}
	(void) fclose(fp);

	/* make sure parameters are valid */
	(void) time(&clock);
	if(pt = getenv("PKG")) {
		if(pkgnmchk(pt, NULL, 0) || strchr(pt, '.')) {
			progerr(ERR_PKGABRV, pt);
			quit(1);
		}
		if(pkginst == NULL)
			pkginst = pt;
	} else {
		progerr(ERR_NOPARAM, "PKG", svept->path);
		quit(1);
	}
	if(getenv("NAME") == NULL) {
		progerr(ERR_NOPARAM, "NAME", svept->path);
		quit(1);
	}
	if(getenv("VERSION") == NULL) {
		(void) cftime(buf, "%m/%d/%y", &clock);
		(void) sprintf(temp, "Dev Release %s", buf);
		putparam("VERSION", temp);
		logerr(WRN_SETPARAM, "VERSION", temp);
	}
	if(getenv("ARCH") == NULL) {
#ifdef PRESVR4
		putparam("ARCH", "unknown");
		logerr(WRN_SETPARAM, "ARCH", "unknown");
#else
		(void) uname(&utsbuf);
		putparam("ARCH", utsbuf.machine);
		logerr(WRN_SETPARAM, "ARCH", utsbuf.machine);
#endif
	}
	if(getenv("PSTAMP") == NULL) {
		/* use octal value of '%' to fight sccs expansion */
		(void) cftime(buf, "%y%m%d\045H\045M\045S", &clock);
#ifdef PRESVR4
		(void) strcpy(temp, "unk");
#else
		(void) uname(&utsbuf);
		(void) sprintf(temp, "%s%s", utsbuf.nodename, buf);
#endif
		putparam("PSTAMP", temp);
		logerr(WRN_SETPARAM, "PSTAMP", temp);
	}
	if(getenv("CATEGORY") == NULL) {
		putparam("CATEGORY", "application");
		logerr(WRN_SETPARAM, "CATEGORY", "application");
	}

	/*
	 * warn user of classes listed in package which do
	 * not appear in CLASSES variable in pkginfo file
	 */
	objects = 0;
	allclass = (char **) calloc(MALSIZ, sizeof(char *));
	for(i=0; eptlist[i]; i++) {
		if(eptlist[i]->ftype != 'i') {
			objects++;
			addclass(eptlist[i]->class);
		}
	}

	pt = getenv("CLASSES");
	if(pt == NULL) {
		class = allclass;
		if(class[0]) {
			j = 1; /* room for ending null */
			for(i=0; class[i]; i++)
				j += strlen(class[i]) + 1;
			pt = (char *) calloc((unsigned)j, sizeof(char));
			(void) strcpy(pt, class[0]);
			for(i=1; class[i]; i++) {
				(void) strcat(pt, " ");
				(void) strcat(pt, class[i]);
			}
			logerr(WRN_SETPARAM, "CLASSES", pt);
			putparam("CLASSES", pt);
			free((void *)pt);
		}
	} else {
		(void) setlist(&class, qstrdup(pt));
		for(i=0; allclass[i]; i++) {
			found = 0;
			for(j=0; class[j]; j++) {
				if(!strcmp(class[j], allclass[i])) {
					found++;
					break;
				}
			}
			if(!found)
				logerr(WRN_CLASSES, allclass[i]);
		}
	}

	(void) fprintf(stderr, MSG_VOLUMIZE, objects);
	order = (char **)0;
	if(pt = getenv("ORDER")) {
		pt = qstrdup(pt);
		(void) setlist(&order, pt);
	}

#ifdef PRESVR4
	if (statfs(pkgdev.dirname, &statbuf, sizeof (struct statfs), 0) != 0)
	   if (statfs(pkgdev.bdevice, &statbuf, sizeof (struct statfs), 1) != 0) {
		progerr (ERR_STATVFS, pkgdev.dirname);
		quit(99);
	}
	bsize = (ulong) statbuf.f_bsize;
	if ( limit == 0 )
		limit = (statbuf.f_bfree - 10);
	if ( ilimit == 0 )
		ilimit = (statbuf.f_ffree - 5);
#else
	/* stat the intended output filesystem to get blocking information */
	if(statvfs(pkgdev.dirname, &statvfsbuf)) {
		progerr(ERR_STATVFS, pkgdev.dirname);
		quit(99);
	}
        bsize = statvfsbuf.f_bsize;
	/* if the "-l" option is used, assume the argument is in 512 byte blocks already. */
	if(limit == 0) {
		limit = statvfsbuf.f_bavail;
		if ( bsize != NBPSCTR )
			limit *= ((bsize+NBPSCTR-1)/NBPSCTR);
	}
	if(ilimit == 0)
		ilimit = statvfsbuf.f_ffree;
#endif /* PRESVR4 */

	nparts = splpkgmap(eptlist, eptnum, order, bsize, &limit, &ilimit);
	if(nparts <= 0) {
		progerr(ERR_SPLIT);
		quit(1);
	}

	if(nflag) {
		for(i=0; eptlist[i]; i++)
			(void) ppkgmap(eptlist[i], stdout);
		exit(0);
		/*NOTREACHED*/
	} 

	(void) sprintf(pkgloc, "%s/%s", pkgdev.dirname, pkginst);
	if(!isdir(pkgloc) && !overwrite) {
		progerr(ERR_OVERWRITE, pkgloc);
		quit(1);
	}

	/* output all environment parameters */
	t_pkginfo = tempnam(tmpdir, "pkginfo");
	if((fp = fopen(t_pkginfo, "w")) == NULL) {
		progerr(ERR_TEMP, errno);
		exit(99);
	}
	for(i=0; environ[i]; i++) {
		(void) fputs(environ[i], fp);
		(void) fputc('\n', fp);
	}
	(void) fclose(fp);

	started++;
	(void) rrmdir(pkgloc);
	if(mkdir(pkgloc, 0755)) {
		progerr(ERR_MKDIR, pkgloc);
		quit(1);
	}

	/* determine how many packages already reside on the medium */
	pkgdir = pkgdev.dirname;
	npkgs = 0;
	while(pt = fpkginst("all", NULL, NULL))
		npkgs++;
	(void) fpkginst(NULL); /* free resource usage */

	if(nparts > 1) {
		if(!limit && !pkgdev.mount) {
			/* if no limit was specified and 
			 * the output device is a directory,
			 * we exceed the free space available
			 */
			progerr(ERR_FREE, pkgloc);
			quit(1);
		}
		if(pkgdev.mount && npkgs) {
			progerr(ERR_ONEVOL);
			quit(1);
		}
	}

	/*
	 *  update pkgmap entry for pkginfo file, since it may
	 *  have changed due to command line or failure to
	 *  specify all neccessary parameters
	 */
	for(i=0; eptlist[i]; i++) {
		if(eptlist[i]->ftype != 'i')
			continue;
		if(!strcmp(eptlist[i]->path, "pkginfo")) {
			svept = eptlist[i];
			svept->ftype = '?';
			svept->ainfo.local = t_pkginfo;
			(void) cverify(0, &svept->ftype, t_pkginfo, 
				&svept->cinfo);
			svept->ftype = 'i';
			break;
		}
	}

	for(part=1; part <= nparts; part++) {
		if((part > 1) && pkgdev.mount) {
			if(pkgumount(&pkgdev)) {
				progerr(ERR_UMOUNT, pkgdev.mount);
				quit(99);
			}
			if(n = pkgmount(&pkgdev, NULL, part, nparts, 1))
				quit(n);
			(void)rrmdir(pkgloc);
			if(mkdir(pkgloc, 0555)) {
				progerr(ERR_MKDIR, pkgloc);
				quit(99);
			}
		}
		outvol(eptlist, eptnum, part, nparts);
	}
	quit(0);
	/*NOTREACHED*/
}

static void
trap(n)
int n;
{
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGHUP, SIG_IGN);

	if(n == SIGINT)
		quit(3);
	else {
		(void) fprintf(stderr, "%s terminated (signal %d).\n", prog, n);
		quit(99);
	}
}

static void
outvol(eptlist, eptnum, part, nparts)
struct cfent	**eptlist;
int	eptnum;
int	part, nparts;
{
	FILE	*fp;
	char	*svpt, *path, temp[PATH_MAX];
	int	i;

	if(part == 1) {
		/* re-write pkgmap, but exclude local pathnames */
		(void) sprintf(temp, "%s/pkgmap", pkgloc);
		if((fp = fopen(temp, "w")) == NULL) {
			progerr(ERR_TEMP, errno);
			quit(99);
		}
		(void) fprintf(fp, ": %d %ld\n", nparts, limit);
		for(i=0; eptlist[i]; i++) {
			svpt = eptlist[i]->ainfo.local;
			if(!strchr("sl", eptlist[i]->ftype))
				eptlist[i]->ainfo.local = NULL;
			if(ppkgmap(eptlist[i], fp)) {
				progerr(ERR_TEMP, errno);
				quit(99);
			}
			eptlist[i]->ainfo.local = svpt;
		}
		(void) fclose(fp);
		(void) fprintf(stderr, "%s\n", temp);
	}

	(void) sprintf(temp, "%s/pkginfo", pkgloc);
	if(copyf(svept->ainfo.local, temp, svept->cinfo.modtime))
		quit(1);
	(void) fprintf(stderr, "%s\n", temp);

	for(i=0; i < eptnum; i++) {
		if(eptlist[i]->volno != part)
			continue;
		if(strchr("dxslcbp", eptlist[i]->ftype))
			continue;
		if(eptlist[i]->ftype == 'i') {
			if(eptlist[i] == svept)
				continue; /* don't copy pkginfo file */
			(void) sprintf(temp, "%s/install/%s", pkgloc, 
				eptlist[i]->path);
			path = temp;
		} else
			path = srcpath(pkgloc, eptlist[i]->path, part, nparts);
		if(sflag) {
			if(slinkf(eptlist[i]->ainfo.local, path))
				quit(1);
		} else if(copyf(eptlist[i]->ainfo.local, path, 
		   eptlist[i]->cinfo.modtime))
			quit(1);
		(void) fprintf(stderr, "%s\n", path);
	}
}

static void
addclass(cl)
char	*cl;
{
	int	i;

	for(i=0; allclass[i]; i++)
		if(!strcmp(cl, allclass[i]))
			return;

	allclass[i] = qstrdup(cl);
	if((++i % MALSIZ) == 0) {
		allclass = (char **) realloc((void *)allclass, 
			(i+MALSIZ) * sizeof(char *));
		if(allclass == NULL) {
			progerr(ERR_MEMORY, errno);
			quit(99);
		}
	}
	allclass[i] = (char *)NULL;
}

static void
ckmissing(path, type)
char *path, type;
{
	static char	**dir;
	static int	ndir;
	char	*pt;
	int	i, found;

	if(dir == NULL) {
		dir = (char **) calloc(MALSIZ, sizeof(char *));
		if(dir == NULL) {
			progerr(ERR_MEMORY, errno);
			quit(99);
		}
	}

	if(strchr("dx", type)) {
		dir[ndir] = path;
		if((++ndir % MALSIZ) == 0) {
			dir = (char **) realloc((void *)dir, 
				(ndir+MALSIZ)*sizeof(char *));
			if(dir == NULL) {
				progerr(ERR_MEMORY, errno);
				quit(99);
			}
		}
		dir[ndir] = (char *)NULL;
	}

	pt = path;
	if(*pt == '/')
		pt++;
	while(pt = strchr(pt, '/')) {
		*pt = '\0';
		found = 0;
		for(i=0; i < ndir; i++) {
			if(!strcmp(path, dir[i])) {
				found++;
				break;
			}
		}
		if(!found) {
			logerr(WRN_MISSINGDIR, path);
			ckmissing(qstrdup(path), 'd');
		}
		*pt++ = '/';
	}
}

static int
slinkf(from, to)
char	*from, *to;
{
	char	*pt;

	pt = to;
	while(pt = strchr(pt+1, '/')) {
		*pt = '\0';
		if(isdir(to) && mkdir(to, 0755)) {
			progerr(ERR_MKDIR, to);
			*pt = '/';
			return(-1);
		}
		*pt = '/';
	}
	if(symlink(from, to)) {
		progerr(ERR_SYMLINK, to);
		return(-1);
	}
	return(0);
}

static void
usage()
{
	(void) fprintf(stderr, 
		"usage: %s [options] [VAR=value [VAR=value]] [pkginst]\n",
		prog);
	(void) fprintf(stderr, "   where options may include:\n");
	(void) fprintf(stderr, "\t-o\n");
	(void) fprintf(stderr, "\t-a arch\n");
	(void) fprintf(stderr, "\t-v version\n");
	(void) fprintf(stderr, "\t-p pstamp\n");
	(void) fprintf(stderr, "\t-l limit\n");
	(void) fprintf(stderr, "\t-r rootpath\n");
	(void) fprintf(stderr, "\t-b basedir\n");
	(void) fprintf(stderr, "\t-d device\n");
	(void) fprintf(stderr, "\t-f protofile\n");
	exit(1);
	/*NOTREACHED*/
}
