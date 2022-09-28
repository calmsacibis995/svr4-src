/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkgproto/main.c	1.3.4.1"
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pkgstrct.h>

extern int	holdcinfo;
extern char	*optarg, errbuf[];
extern int	optind;

extern void	*calloc(), exit(),
		progerr(), logerr(),
		canonize();
extern int	getopt(), readlink(),
		averify(), cverify(),
		ppkgmap();

#define ERR_CLASSLONG	"classname argument too long"
#define ERR_CLASSCHAR	"bad character in classname"
#define ERR_STAT	"unable to stat <%s>"
#define ERR_WRITE	"write of entry failed"
#define ERR_POPEN	"unable to create pipe to <%s>"
#define ERR_PCLOSE	"unable to close pipe to <%s>"
#define ERR_RDLINK	"unable to read link for <%s>"

struct link {
	char	*path;
	ino_t	ino;
	dev_t	dev;
	struct link *next;
};

static struct link *firstlink = (struct link *)0;
static struct link *lastlink = (struct link *)0;

char	*class = "none";
char	*prog = NULL;

static int	errflg = 0;
static int	iflag = 0;
static int	xflag = 0;
static int	nflag = 0;
static char	mylocal[PATH_MAX];

static void	usage(), findlink(),
		output(), follow();

main(argc, argv)
int argc;
char *argv[];
{
	int c;
	char *pt, path[PATH_MAX];

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	while((c = getopt(argc, argv, "xnic:?")) != EOF) {
		switch(c) {
		  case 'x':
			xflag++;
			break;

		  case 'n':
			nflag++;
			break;

		  case 'c':
			class = optarg;
			/* validate that classname is acceptable */
			if(strlen(class) > (size_t)CLSSIZ) {
				progerr(ERR_CLASSLONG);
				exit(1);
			}
			for(pt=class; *pt; pt++) {
				if(!isalpha(*pt) && !isdigit(*pt)) {
					progerr(ERR_CLASSCHAR);
					exit(1);
				}
			}
			break;

		  case 'i':
			iflag++;
			break;
			
		   default:
			usage();
		}
	}

	holdcinfo = !xflag;
	if(optind == argc) {
		/* take path list from stdin */
		while(gets(path))
			output(path, 0, NULL);
	} else {
		while(optind < argc)
			follow(argv[optind++]);
	}

	exit(errflg ? 1 : 0);
	/*NOTREACHED*/
}

static void
output(path, n, local)
char *path, *local;
int n;
{
	struct cfent entry;
	char	mypath[PATH_MAX];

	entry.volno = 0;
	entry.ftype = '?';
	entry.path = mypath;
	(void) strcpy(entry.class, class);
	(void) strcpy(entry.path, path);
	entry.ainfo.local = NULL;
	entry.ainfo.mode = BADMODE;
	(void) strcpy(entry.ainfo.owner, BADOWNER);
	(void) strcpy(entry.ainfo.group, BADGROUP);

	if(xflag) {
		entry.ftype = '?';
		if(cverify(0, &entry.ftype, path, &entry.cinfo)) {
			errflg++;
			logerr("WARNING: %s", path);
			logerr(errbuf);
			return;
		}
	}
	if(averify(0, &entry.ftype, path, &entry.ainfo)) {
		errflg++;
		logerr("WARNING: %s", path);
		logerr(errbuf);
		return;
	}

	if(n) {
		/* replace first n characters with 'local' */
		if(strchr("fev", entry.ftype)) {
			entry.ainfo.local = mylocal;
			(void) strcpy(entry.ainfo.local, entry.path);
			canonize(entry.ainfo.local);
		}
		if(local[0]) {
			entry.ainfo.local = mylocal;
			(void) strcpy(entry.path, local);
			(void) strcat(entry.path, path+n);
		} else
			(void) strcpy(entry.path, 
				(path[n]=='/') ? path+n+1 : path+n);
	}
	canonize(entry.path);
	if(entry.path[0]) {
		findlink(&entry, path, entry.path);
		if(strchr("dcbp", entry.ftype) || 
		(nflag && !strchr("sl", entry.ftype)))
			entry.ainfo.local = NULL;
		if(ppkgmap(&entry, stdout)) {
			progerr(ERR_WRITE);
			exit(99);
		}
	}
}

static void
follow(path)
char *path;
{
	struct stat stbuf;
	FILE	*pp;
	char	*pt, 
		local[PATH_MAX], 
		newpath[PATH_MAX],
		cmd[PATH_MAX+32];
	int n;

	if(pt = strchr(path, '=')) {
		*pt++ = '\0';
		(void) strcpy(local, pt);
		n = strlen(path);
	} else {
		n = 0;
		local[0] = '\0';
	}

	if(stat(path, &stbuf)) {
		progerr(ERR_STAT, path);
		errflg++;
		return;
	}
	
	if(stbuf.st_mode & S_IFDIR) {
		(void) sprintf(cmd, "find %s -print", path);
		if((pp = popen(cmd, "r")) == NULL) {
			progerr(ERR_POPEN, cmd);
			exit(1);
		} 
		while(fscanf(pp, "%[^\n]\n", newpath) == 1)
			output(newpath, n, local);
		if(pclose(pp)) {
			progerr(ERR_PCLOSE, cmd);
			errflg++;
		}
	} else
		output(path, n, local);
}

static void
findlink(ept, path, svpath)
struct cfent *ept;
char *path, *svpath;
{
	struct stat	statbuf;
	struct link	*link, *new;
	char		buf[PATH_MAX];
	int		n;

	if(lstat(path, &statbuf)) {
		progerr(ERR_STAT, path);
		errflg++;
	}
	if((statbuf.st_mode & S_IFMT) == S_IFLNK) {
		if(!iflag) {
			ept->ainfo.local = mylocal;
			ept->ftype = 's';
			n = readlink(path, buf, PATH_MAX);
			if(n <= 0) {
				progerr(ERR_RDLINK, path);
				errflg++;
				(void) strcpy(ept->ainfo.local, "unknown");
			} else {
				(void) strncpy(ept->ainfo.local, buf, n);
				ept->ainfo.local[n] = '\0';
			}
		}
		return;
	}

	if(stat(path, &statbuf)) 
		return;
	if(statbuf.st_nlink <= 1)
		return;

	for(link=firstlink; link; link = link->next) {
		if((statbuf.st_ino == link->ino) &&
		(statbuf.st_dev == link->dev)) {
			ept->ftype = 'l';
			ept->ainfo.local = mylocal;
			(void) strcpy(ept->ainfo.local, link->path);
			return;
		}
	}
	new = (struct link *)calloc(1, sizeof(struct link));
	if(firstlink) {
		lastlink->next = new;
		lastlink = new;
	} else
		firstlink = lastlink = new;
		
	new->path = strdup(svpath);
	new->ino = statbuf.st_ino;
	new->dev = statbuf.st_dev;
	return;
}

static void
usage()
{
	(void) fprintf(stderr, "usage: %s [-i] [-c class] [path ...]\n", prog);
	exit(1);
	/*NOTREACHED*/
}
