/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkgmk/splpkgmap.c	1.15.5.1"

#include	<sys/param.h>
#include	<stdio.h> 
#include	<string.h> 
#include	<limits.h>
#include	<sys/types.h>
#include	<pkgstrct.h>

extern int	optind, errno;
extern char	*optarg, *errstr;

extern void	*calloc(), *realloc();
extern char	*qstrdup();
extern void	progerr(),
		quit(),
		free();
extern long	atol();
extern int	ppkgmap();

#define MALSIZ	500
#define DIRSIZE	2
#define EFACTOR	128L	/* typical size of a single entry in a pkgmap file */

/* the following defines are specific to S5 file systems */
#define DIRECT	10	/* Number of direct blocks */
	/* Number of pointers in an indirect block */
#define INDIR	128
	/* Number of right shifts to divide by INDIR */
#define INSHFT	7
/* end s5 specific defines */

#define ERR_MEMORY	"memory allocation failure, errno=%d"
#define ERR_TOOBIG	"%s (%ld blocks) does not fit on a volume"
#define ERR_INFOFIRST	"information file <%s> must appear on first part"
#define ERR_INFOSPACE	"all install files must appear on first part"

struct data {
	long	blks;
	struct cfent *ept;
};

struct class {
	char *name;
	int first;
	int last;
};

static long	btotal, 	/* blocks stored on current part */
		ftotal, 	/* files stored on current part */
		bmax, 		/* maximum number of blocks on any part */
		fmax, 		/* maximum number of files on any part */
		bpkginfo;	/* blocks used by pkginfo file */
static char	**dirlist;
static int	volno = 0;	/* current part */
static int	nclass;
static struct class
		*cl;

static void	allocnode(),
		addclass(),
		newvolume(),
		sortsize();
static int	store(), nodecount();
static long	nblk();

int
splpkgmap(eptlist, eptnum, order, bsize, plimit, pilimit)
struct cfent	**eptlist;
int	eptnum;
char	*order[];
ulong	bsize;
long	*plimit;
int	*pilimit;
{
	struct data	*f, **sf;
	struct cfent	*ept;
	register int	i, j;
	int	flag, errflg;
	long	btemp;

	f = (struct data *) calloc((unsigned) eptnum, sizeof(struct data));
	if(f == NULL) {
		progerr(ERR_MEMORY, errno);
		quit(99);
	}

	sf = (struct data **) calloc((unsigned) eptnum, sizeof(struct data *));
	if(sf == NULL) {
		progerr(ERR_MEMORY, errno);
		quit(99);
	}

	nclass = 0;
	cl = (struct class *) calloc(MALSIZ, sizeof(struct class));
	if(cl == NULL) {
		progerr(ERR_MEMORY, errno);
		quit(99);
	}

	errflg = 0;

	/* calculate number of physical blocks used by each object */
	for(i=0; i < eptnum; i++) {
		f[i].ept = ept = eptlist[i];
		addclass(ept->class, 0);
		if(strchr("dxlcbp", ept->ftype))
			f[i].blks = 0; /* virtual object (no contents) */
		else
			f[i].blks = nblk(ept->cinfo.size);
	}

	/* establish an array sorted by decreasing file size */
	sortsize(f, sf, eptnum);

	/* initialize first volume */
	newvolume(sf, eptnum);
	
	/* reserve room on first volume for pkgmap */
	btotal += nblk(eptnum * EFACTOR);
	ftotal++;

	/* initialize directory info */
	allocnode(NULL);

	/* make sure all files will fit on any volume */
	for(j=0; j < eptnum; ++j) {
		btemp = nodecount(f[j].ept->path) * DIRSIZE;
		if((f[j].blks + btemp) > *plimit) {
			errflg++;
			progerr(ERR_TOOBIG, f[j].ept->path, f[j].blks);
		}
	}
	if(errflg)
		quit(0);

	/* place installation files on first volume! */
	flag = 0;
	for(j=0; j < eptnum; ++j) {
		if(f[j].ept->ftype != 'i')
			continue;
		if(!strcmp(f[j].ept->path, "pkginfo"))
			bpkginfo = f[j].blks;
		else if(!flag++) {
			/* save room for install directory */
			ftotal++;
			btotal += 2;
		}
		if(!f[j].ept->volno) {
			f[j].ept->volno = 1;
			ftotal++;
			btotal += f[j].blks;
		} else if(f[j].ept->volno != 1) {
			progerr(ERR_INFOFIRST, f[j].ept->path);
			errflg++;
		}
	}
	if(errflg)
		quit(0);
	if(btotal > *plimit) {
		progerr(ERR_INFOSPACE);
		quit(0);
	}

	/* place classes listed on command line */
	if(order) {
		for(i=0; order[i]; ++i)  {
			while(store(sf, eptnum, order[i], *plimit, *pilimit))
				; /* stay in loop until store is complete */
		}
	}

	while(store(sf, eptnum, (char *)0, *plimit, *pilimit))
		; /* stay in loop until store is complete */

	/* place all virtual objects, e.g. links and spec devices */
	for(i=0; i < nclass; ++i) {
		/* if no objects were associated, attempt to 
		 * distribute in order of class list
		 */
		if(cl[i].first == 0)
			cl[i].last = cl[i].first = (i ? cl[i-1].last : 1);
		for(j=0; j < eptnum; j++) {
			if((f[j].ept->volno == 0) && 
			 !strcmp(f[j].ept->class, cl[i].name)) {
				if(strchr("sl", f[j].ept->ftype))
					f[j].ept->volno = cl[i].last;
				else
					f[j].ept->volno = cl[i].first;
			}
		}
	}

	if(btotal)
		newvolume(sf, eptnum);

	*plimit = bmax;
	*pilimit = fmax;

	/* free up dynamic space used by this module */
	free(f);
	free(sf);
	for(i=0; i < nclass; ++i)
		free(cl[i].name);
	free(cl);
	for(i=0; dirlist[i]; i++)
		free(dirlist[i]);
	free(dirlist);

	return(errflg ? -1 : volno-1);
}

static long 
nblk(size)
long	size;
{
 	long blocks, tot;

        blocks = tot = (size + NBPSCTR - 1) >> SCTRSHFT;
	if(blocks > DIRECT)
		tot += ((blocks - DIRECT - 1) >> INSHFT) + 1;
	if(blocks > DIRECT + INDIR)
		tot += ((blocks - DIRECT - INDIR - 1) >> (INSHFT * 2)) + 1;
	if(blocks > DIRECT + INDIR + INDIR*INDIR)
		tot++;
	return(tot+1);
}

static int
store(sf, eptnum, class, limit, ilimit)
struct data **sf;
int	eptnum;
char	*class;
long	limit;
int	ilimit;
{
	int	i, svnodes, choice, select;
	long	btemp, ftemp;

	select = 0;
	choice = (-1);
	for(i=0; i < eptnum; ++i) {
		if(sf[i]->ept->volno || strchr("sldxcbp", sf[i]->ept->ftype))
			continue; /* defer storage until class is selected */
		if(class && strcmp(class, sf[i]->ept->class))
			continue;
		select++; /* we need to place at least one object */
		ftemp = nodecount(sf[i]->ept->path);
		btemp = sf[i]->blks + (ftemp * DIRSIZE);
		if(((limit <= 0) || ((btotal + btemp) <= limit)) &&
		  ((ilimit <= 0) || ((ftotal + ftemp) < ilimit))) {
			/* largest object which fits on this volume */
			choice = i;
			svnodes = ftemp;
			break;
		}
	}
	if(!select)
		return(0); /* no more to objects to place */

	if(choice < 0) {
		newvolume(sf, eptnum);
		return(store(sf, eptnum, class, limit, ilimit));
	}
	sf[choice]->ept->volno = (char) volno;
	ftotal += svnodes + 1;
	btotal += sf[choice]->blks + (svnodes*DIRSIZE);
	allocnode(sf[i]->ept->path);
	addclass(sf[choice]->ept->class, volno);
	return(++choice); /* return non-zero if more work to do */
}

static void
allocnode(path)
char *path;
{
	register int i;
	int	found;
	char	*pt;

	if(path == NULL) {
		if(dirlist) {
			/* free everything */
			for(i=0; dirlist[i]; i++)
				free(dirlist[i]);
			free(dirlist);
		}
		dirlist = (char **) calloc(MALSIZ, sizeof(char *));
		if(dirlist == NULL) {
			progerr(ERR_MEMORY, errno);
			quit(99);
		}
		return;
	}

	pt = path;
	if(*pt == '/')
		pt++;
	/* since the pathname supplied is never just 
	 * a directory, we store only the dirname of
	 * of the path
	 */
	while(pt = strchr(pt, '/')) {
		*pt = '\0';
		found = 0;
		for(i=0; dirlist[i] != NULL; i++) {
			if(!strcmp(path, dirlist[i])) {
				found++;
				break;
			}
		}
		if(!found) {
			/* insert this path in node list */
			dirlist[i] = qstrdup(path);
			if((++i % MALSIZ) == 0) {
				dirlist = (char **) realloc(dirlist, 
					(i+MALSIZ)*sizeof(char *));
				if(dirlist == NULL) {
					progerr(ERR_MEMORY, errno);
					quit(99);
				}
			}
			dirlist[i] = (char *) NULL;
		}
		*pt++ = '/';
	}
}

static int
nodecount(path)
char *path;
{
	char	*pt;
	int	i, found, count;

	pt = path;
	if(*pt == '/')
		pt++;

	/* we want to count the number of path
	 * segments that need to be created, not
	 * including the basename of the path;
	 * this works only since we are never
	 * passed a pathname which itself is a
	 * directory
	 */
	count = 0;
	while(pt = strchr(pt, '/')) {
		*pt = '\0';
		found = 0;
		for(i=0; dirlist[i]; i++) {
			if(!strcmp(path, dirlist[i])) {
				found++;
				break;
			}
		}
		if(!found)
			count++;
		*pt++ = '/';
	}
	return(count);
}

static void
newvolume(sf, eptnum)
struct data **sf;
{
	register int i;
	int	newnodes;

	if(volno) {
		(void) fprintf(stderr, "part %2d -- %ld blocks, %ld entries\n",
			volno, btotal, ftotal);
		if(btotal > bmax)
			bmax = btotal;
		if(ftotal > fmax)
			fmax = ftotal;
		btotal = bpkginfo + 2L;
		ftotal = 3;
	} else {
		btotal = 2L;
		ftotal = 2;
	}
	volno++;

	/* zero out directory storage */
	allocnode((char *)0);

	/* force storage of files whose volume 
         * number has already been assigned
	 */
	for(i=0; i < eptnum; i++) {
		if(sf[i]->ept->volno == volno) {
			newnodes = nodecount(sf[i]->ept->path);
			ftotal += newnodes + 1;
			btotal += sf[i]->blks + (newnodes*DIRSIZE);
		}
	}
}

static void
addclass(class, vol)
char *class;
int vol;
{
	int i;

	for(i=0; i < nclass; ++i) {
		if(!strcmp(cl[i].name, class)) {
			if(vol <= 0)
				return;
			if(!cl[i].first || (vol < cl[i].first))
				cl[i].first = vol;
			if(vol > cl[i].last)
				cl[i].last = vol;
			return;
		}
	}
	cl[nclass].name = qstrdup(class);
	cl[nclass].first = vol;
	cl[nclass].last = vol;
	if((++nclass % MALSIZ) == 0) {
		cl = (struct class *) realloc((char *)cl, 
			sizeof(struct class)*(nclass+MALSIZ));
		if(!cl) {
			progerr(ERR_MEMORY, errno);
			quit(99);
		}
	}
	return;
}

static void
sortsize(f, sf, eptnum)
struct data *f, **sf;
int	eptnum;
{
	int	nsf;
	int	i, j, k;

	nsf = 0;
	for(i=0; i < eptnum; i++) {
		for(j=0; j < nsf; ++j) {
			if(f[i].blks > sf[j]->blks) {
				for(k=nsf; k > j; k--) {
					sf[k] = sf[k-1];
				}
				break;
			}
		}
		sf[j] = &f[i];
		nsf++;
	}
}
