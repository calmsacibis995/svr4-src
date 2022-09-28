/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginfo/pkginfo.c	1.16.6.1"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <pkginfo.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include <pkglocs.h>

extern char	*optarg;
extern char	*pkgdir;
extern int	optind;
extern char	*errstr;

extern void	*calloc(), 
		exit(),
		logerr(),
		progerr();
extern int	getopt(),
		access(),
		srchcfile(),
		pkghead(),
		gpkgmap();
extern char	*pkgparam();

#define nblock(size)	((size + (BLKSIZ - 1)) / BLKSIZ)
#define	BLKSIZ		512
#define MAXCATG	64

char	*prog;

static char	*device = NULL;
static char	*paramlist[] = {
	"DESC", "PSTAMP", "INSTDATE", "VSTOCK", "SERIALNUM", "HOTLINE", 
	"EMAIL", NULL 
};

static char	contents[PATH_MAX];
static int	errflg = 0;
static int	qflag = 0;
static int	iflag = -1;
static int	pflag = -1;
static int	lflag = 0;
static int	Lflag = 0;
static int	Nflag = 0;
static int	xflag = 0;
static struct cfent	entry;
static char	**pkg = NULL;
static int	pkgcnt = 0;
static char	*ckcatg[MAXCATG] = {NULL};
static int	ncatg = 0;
static char	*ckvers = NULL;
static char	*ckarch = NULL;

static struct cfstat {
	char	pkginst[15];
	short	exec;
	short	dirs;
	short	link;
	short	partial;
	short	spooled;
	short	installed;
	short	info;
	short	shared;
	short	setuid;
	long	tblks;
	struct cfstat *next;
} *data;
static struct pkginfo info;

static struct cfstat	*fpkg();
static int	iscatg(),
		select(); 
static void	usage(), 
		look_for_installed(), 
		report(), 
		rdcontents(),
		getinfo(),
		pkgusage(),
		dumpinfo(); 

main(argc,argv)
int	argc;
char	**argv;
{
	int	c;

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	while ((c = getopt(argc,argv,"LNxv:a:d:qpilc:?")) != EOF) {
		switch(c) {
		  case 'v':
			ckvers = optarg;
			break;

		  case 'a':
			ckarch = optarg;
			break;

		  case 'd':
			/* -d could specify stream or mountable device */
			device = optarg;
			break;

		  case 'q':
			qflag++;
			break;

		  case 'i':
			iflag = 1;
			if(pflag > 0)
				usage();
			pflag = 0;
			break;

		  case 'p':
			pflag = 1;
			if(iflag > 0)
				usage();
			iflag = 0;
			break;

		  case 'N':
			Nflag++;
			break;

		  case 'L':
			if(xflag || lflag) {
				progerr("-L and -l/-x flags are incompatable");
				usage();
			}
			Lflag++;
			break;

		  case 'l':
			if(xflag) {
				progerr("-l and -x flags are incompatable");
				usage();
			}
			lflag++;
			break;

		  case 'x':
			if(lflag) {
				progerr("-l and -x flags are not compatable");
				usage();
			}
			xflag++;
			break;

		  case 'c':
			ckcatg[ncatg++] = strtok(optarg, " \t\n,");
			while(ckcatg[ncatg] = strtok(NULL, " \t\n,"))
				ncatg++;
			break;

		  default:
			usage();
		}
	}
	
	pkg = &argv[optind];
	pkgcnt = (argc - optind);
	
	if(pkg[0] && !strcmp(pkg[0], "all")) {
		pkgcnt = 0;
		pkg[0] = NULL;
	}

	if(pkgdir == NULL)
		pkgdir = PKGLOC;	/* we need this later */

	/* convert device appropriately */
	if(pkghead(device))
		exit(1);

	look_for_installed();

	if(lflag && !strcmp(pkgdir, PKGLOC)) {
		/* look at contents file */
		(void) sprintf(contents, "%s/contents", PKGADM);
		rdcontents();
	}

	report();

#ifdef i386	/* XENIX Support */
	if (pkgcnt == 0) {
		system ("/usr/bin/displaypkg XENIX");
	}
#endif /* i386 */

	(void) pkghead(NULL);
	exit(errflg ? 1 : 0);
}

char *fmt = "%10s:  %s\n";

static void
report()
{
	struct cfstat *dp, *choice;
	int	i, output;

	output = 0;
	for(;;) {
		choice = (struct cfstat *)0;
		for(dp=data; dp; dp=dp->next) {
			/* get information about this package */
			if(dp->installed < 0)
				continue; /* already used */
			if(Lflag && pkgcnt) {
				choice = dp;
				break;
			} else if(!choice ||
			  (strcmp(choice->pkginst, dp->pkginst) > 0))
				choice = dp;
		}
		if(!choice)
			break; /* no more packages */

		if(pkginfo(&info, choice->pkginst, ckarch, ckvers)) {
			choice->installed = (-1);
			continue;
		}

		/* is it in an appropriate catgory? */
		if(iscatg(info.catg)) {
			choice->installed = (-1);
			continue;
		}

		if(!pflag && 
			/* don't include partially installed packages */
			(choice->partial || (info.status == PI_PARTIAL) || 
				(info.status == PI_UNKNOWN))) {
			errflg++;
			choice->installed = (-1);
			continue;
		}

		if(Nflag && (info.status == PI_PRESVR4)) {
			/* don't include preSVR4 packages */
			choice->installed = (-1);
			continue;
		}

		if(!iflag && ((info.status == PI_INSTALLED) ||
		  (info.status == PI_PRESVR4))) {
			/* don't include completely installed packages */
			choice->installed = (-1);
			continue;
		}

		output++;
		dumpinfo(choice);
		choice->installed = (-1);
		if(pkgcnt && !qflag) {
			i = select(choice->pkginst);
			if(i >= 0)
				pkg[i] = NULL;
		}
	}
	if(!output)
		errflg++;

	if(qflag)
		return;

	/* verify that each package listed on command line was output */
	for(i=0; i < pkgcnt; ++i) {
		if(pkg[i]) {
			logerr("ERROR: information for \"%s\" was not found", 
				pkg[i]);
			errflg++;
		}
	}
	(void) pkginfo(&info, NULL); /* free up all memory and open fds */
}

static void
dumpinfo(dp)
struct cfstat *dp;
{
	register int i;
	char *pt, category[128];

	if(qflag)
		return; /* print nothing */

	if(Lflag) {
		(void) puts(info.pkginst);
		return;
	} else if(xflag) {
		(void) printf("%-14.14s  %s\n", info.pkginst, info.name);
		if(info.arch || info.version) 
			(void) printf("%14.14s  ", "");
		if(info.arch)
			(void) printf("(%s) ", info.arch);
		if(info.version)
			(void) printf("%s", info.version);
		if(info.arch || info.version)
			(void) printf("\n");
		return;
	} else if(!lflag) {
		if(info.catg)
			(void) sscanf(info.catg, "%[^, \t\n]", category);
		else if(info.status == PI_PRESVR4)
			(void) strcpy(category, "preSVR4");
		else
			(void) strcpy(category, "(unknown)");
		(void) fprintf(stdout, "%-11.11s %-14.14s %s\n", category, 
			info.pkginst, info.name);
		return;
	}
	if(info.pkginst)
		(void) printf(fmt, "PKGINST", info.pkginst);
	if(info.name)
		(void) printf(fmt, "NAME", info.name);
	if(lflag && info.catg)
		(void) printf(fmt, "CATEGORY", info.catg);
	if(lflag && info.arch)
		(void) printf(fmt, "ARCH", info.arch);
	if(info.version)
		(void) printf(fmt, "VERSION", info.version);
	if(info.basedir)
		(void) printf(fmt, "BASEDIR", info.basedir);
	if(info.vendor)
		(void) printf(fmt, "VENDOR", info.vendor);

	if(info.status == PI_PRESVR4)
		(void) printf(fmt, "STATUS", "preSVR4");
	else {
		for(i=0; paramlist[i]; ++i) {
			if((pt = pkgparam(info.pkginst, paramlist[i])) && *pt)
				(void) printf(fmt, paramlist[i], pt);
		}
		if(info.status == PI_SPOOLED)
			(void) printf(fmt, "STATUS", "spooled");
		else if(info.status == PI_PARTIAL)
			(void) printf(fmt, "STATUS", "partially installed");
		else if(info.status == PI_INSTALLED)
			(void) printf(fmt, "STATUS", "completely installed");
		else
			(void) printf(fmt, "STATUS", "(unknown)");
	}
	(void) pkgparam(NULL, NULL);

	if(!lflag) {
		(void) putchar('\n');
		return;
	}

	if(info.status != PI_PRESVR4) {
		if(strcmp(pkgdir, PKGLOC))
			getinfo(dp);

		if(dp->spooled)
			(void) printf("%10s:  %5d spooled pathnames\n", 
				"FILES", dp->spooled);
		if(dp->installed)
			(void) printf("%10s:  %5d installed pathnames\n", 
				"FILES", dp->installed);
		if(dp->partial)
			(void) printf("%18d partially installed pathnames\n", 
				dp->partial);
		if(dp->shared)
			(void) printf("%18d shared pathnames\n", 
				dp->shared);
		if(dp->link)
			(void) printf("%18d linked files\n", dp->link);
		if(dp->dirs)
			(void) printf("%18d directories\n", dp->dirs);
		if(dp->exec)
			(void) printf("%18d executables\n", dp->exec);
		if(dp->setuid)
			(void) printf("%18d setuid/setgid executables\n", 
				dp->setuid);
		if(dp->info)
			(void) printf("%18d package information files\n", 
				dp->info+1); /* pkgmap counts! */
		
		if(dp->tblks)
			(void) printf("%18ld blocks used (approx)\n", 
				dp->tblks);
	}
	(void) putchar('\n');
}

static struct cfstat *
fpkg(pkginst)
char *pkginst;
{
	struct cfstat *dp, *last;

	dp = data;
	last = (struct cfstat *)0;
	while(dp) {
		if(!strcmp(dp->pkginst, pkginst))
			return(dp);
		last = dp;
		dp = dp->next;
	}
	dp = (struct cfstat *)calloc(1, sizeof(struct cfstat));
	if(!dp) {
		progerr("no memory, malloc() failed");
		exit(1);
	}
	if(!last)
		data = dp;
	else
		last->next = dp; /* link list */
	(void) strncpy(dp->pkginst, pkginst, 14);
	return(dp);
}
	
#define SEPAR	','

static int
iscatg(list)
char *list;
{
	register int i;
	register char *pt;
	int	match;

	if(!ckcatg[0])
		return(0); /* no specification implies all packages */
	if(info.status == PI_PRESVR4) {
		for(i=0; ckcatg[i]; ) {
			if(!strcmp(ckcatg[i++], "preSVR4"))
				return(0);
		}
		return(1);
	}
	if(!list)
		return(1); /* no category specified in pkginfo is a bug */

	match = 0;
	do {
		if(pt = strchr(list, ','))
			*pt = '\0';

		for(i=0; ckcatg[i]; ) {
			if(!strcmp(list, ckcatg[i++])) {
				match++;	
				break;
			}
		}

		if(pt)
			*pt++ = ',';
		if(match)
			return(0);
		list = pt; /* points to next one */
	} while(pt);
	return(1);
}

static void
look_for_installed()
{
	struct dirent *drp;
	struct stat	status;
	DIR	*dirfp;
	char	path[PATH_MAX];
	int	n;

	if(!strcmp(pkgdir, PKGLOC) && (dirfp = opendir(PKGOLD))) {
		while(drp = readdir(dirfp)) {
			if(drp->d_name[0] == '.')
				continue;
			n = strlen(drp->d_name);
			if((n > 5) && !strcmp(&drp->d_name[n-5], ".name")) {
				(void) sprintf(path, "%s/%s", PKGOLD, 
					drp->d_name);
				if(lstat(path, &status))
					continue;
				if((status.st_mode & S_IFMT) != S_IFREG)
					continue;
				drp->d_name[n-5] = '\0';
				if(!pkgcnt || (select(drp->d_name) >= 0))
					(void) fpkg(drp->d_name);
			}
		}
		(void) closedir(dirfp);
	}

	if((dirfp = opendir(pkgdir)) == NULL)
		return;

	while(drp = readdir(dirfp)) {
		if(drp->d_name[0] == '.')
			continue;

		if(pkgcnt && (select(drp->d_name) < 0))
			continue;

		(void) sprintf(path, "%s/%s/pkginfo", pkgdir, drp->d_name);
		if(access(path, 0))
			continue; /* doesn't appear to be a package */
		(void) fpkg(drp->d_name);
	}
	(void) closedir(dirfp);
}

static int
select(p)
char *p;
{
	register int i;

	for(i=0; i < pkgcnt; ++i) {
		if(pkg[i] && (pkgnmchk(p, pkg[i], 1) == 0))
			return(i);
	}
	return(-1);
}

static void
rdcontents()
{
	FILE *fp1;
	struct cfstat *dp;
	struct pinfo *pinfo;
	int n;

	if((fp1 = fopen(contents, "r")) == NULL) {
		progerr("unable open \"%s\" for reading", contents);
		exit(1);
	}

	/* check the contents file to look for referenced packages */
	while((n = srchcfile(&entry, "*", fp1, NULL)) > 0) {
		for(pinfo=entry.pinfo; pinfo; pinfo=pinfo->next) {
			/* see if entry is used by indicated packaged */
			if(pkgcnt && (select(pinfo->pkg) < 0))
				continue;

			dp = fpkg(pinfo->pkg);
			pkgusage(dp, &entry);

			if(entry.npkgs > 1)
				dp->shared++;

			if(pinfo->status)
				dp->partial++;
			else
				dp->installed++;
		}
	}
	if(n < 0) {
		progerr("bad entry read in contents file");
		logerr("pathname: %s", entry.path);
		logerr("problem: %s", errstr);
		exit(1);
	}

	(void) fclose(fp1);
}

static void
getinfo(dp)
struct cfstat	*dp;
{
	FILE *fp;
	int n;
	char pkgmap[256];

	(void) sprintf(pkgmap, "%s/%s/pkgmap", pkgdir, dp->pkginst);
	if((fp = fopen(pkgmap, "r")) == NULL) {
		progerr("unable open \"%s\" for reading", pkgmap);
		exit(1);
	}
	dp->spooled = 1; /* pkgmap counts! */
	while((n = gpkgmap(&entry, fp)) > 0) {
		dp->spooled++;
		pkgusage(dp, &entry);
	}
	if(n < 0) {
		progerr("bad entry read in pkgmap file");
		logerr("pathname: %s", entry.path);
		logerr("problem: %s", errstr);
		exit(1);
	}
	(void) fclose(fp);
}
	
static void
pkgusage(dp, pentry)
struct cfstat	*dp;
struct cfent	*pentry;
{
	if(pentry->ftype == 'i') {
		dp->info++;
		return;
	} else if(pentry->ftype == 'l') {
		dp->link++;
	} else {
		if((pentry->ftype == 'd') || (pentry->ftype == 'x'))
			dp->dirs++;
		if(pentry->ainfo.mode & 06000)
			dp->setuid++;
		if(!strchr("dxcbp", pentry->ftype) && 
		(pentry->ainfo.mode & 0111))
			dp->exec++;
	}

	if(strchr("ifve", pentry->ftype)) 
		dp->tblks += nblock(pentry->cinfo.size);
}

static void
usage()
{
	(void) fprintf(stderr, "usage:\n");
	(void) fprintf(stderr,
	"  %s [-q] [-p|-i] [-x|-l] [options] [pkg ...]\n", prog);
	(void) fprintf(stderr,
	"  %s -d device [-q] [-x|-l] [options] [pkg ...]\n", prog);
	(void) fprintf(stderr, "where\n");
	(void) fprintf(stderr, "  -q #quiet mode\n");
	(void) fprintf(stderr, "  -p #select partially installed packages\n");
	(void) fprintf(stderr, "  -i #select completely installed packages\n");
	(void) fprintf(stderr, "  -x #extracted listing\n"); 
	(void) fprintf(stderr, "  -l #long listing\n"); 
	(void) fprintf(stderr, "and options may include:\n");
	(void) fprintf(stderr, "  -c category,[category...]\n");
	(void) fprintf(stderr, "  -a architecture\n");
	(void) fprintf(stderr, "  -v version\n");
	exit(1);
}
