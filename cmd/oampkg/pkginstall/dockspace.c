/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginstall/dockspace.c	1.7.8.3"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/param.h>
#include <limits.h>
#include <pkgstrct.h>

extern struct cfent
		**eptlist;
extern char	*basedir;
extern char	**class;

#define LOGBLK	512	/* logical block */
#define LSIZE	256
#define NFSYS 10
#define LIM_BFREE	150
#define LIM_FFREE	25
#define WRN_STATVFS	"WARNING: unable to stat filesystem mounted on <%s>"

/* the following defines are specific to S5 file systems */
#define DIRECT	10	/* Number of direct blocks */
	/* Number of pointers in an indirect block */
#define INDIR	128
	/* Number of right shifts to divide by INDIR */
#define INSHFT	7
/* end s5 specific defines */

extern char	*strrchr(),
		*malloc();
extern void	progerr(),
		logerr(),
		mappath(),
		basepath();

static int	nfsys = 0;
static int	rootfsys = 0;
static void	warn();
static int	fsys(), fsyscmp(), readspace(), readmap();
static long	nblk();

static struct tbl {
	char	name[PATH_MAX];
	u_long	bsize;	/* fundamental file system block size */
	u_long	bfree;	/* total # of free blocks */
	u_long	bused;	/* total # of free blocks */
	u_long	ffree;	/* total # of free file nodes */
	u_long	fused;	/* total # of free file nodes */
} *table;

int
dockspace(spacefile)
char	*spacefile;
{
	struct statvfs statvfsbuf;
	FILE	*pp;
	char	*path, line[LSIZE];
	long	bfree, ffree;
	int	i, error, repeat;

	if((pp=popen("/sbin/mount", "r")) == NULL) {
		progerr("unable to create pipe to /sbin/mount");
		return(-1);
	}

	if((table = (struct tbl *)malloc(NFSYS * sizeof(struct tbl))) == (struct tbl *)NULL) {
		progerr("unable to malloc space\n");
		return(-1);
	}

	nfsys = error = 0;
	repeat = 1;
	while(fgets(line, LSIZE, pp)) {
		path = strtok(line, " \t\n");
		if(!strcmp(path, "/"))
			rootfsys = nfsys;
		if(statvfs(path, &statvfsbuf)) {
			logerr(WRN_STATVFS, path);
			error++;
			continue;
		}
		if(nfsys == (repeat * NFSYS)) {
			repeat++;
			if((table = (struct tbl *)realloc(table, NFSYS * repeat * sizeof(struct tbl))) == (struct tbl *)NULL) {
				progerr("unable to malloc space\n");
				return(-1);
			}
		}	
		(void) strcpy(table[nfsys].name, path);
		/* statvfs returns the number of blocks of size f_bsize
		 * convert these to 512 byte blocks since that's the way
		 * pkgmk stored them.
		 */
		if (statvfsbuf.f_bavail > statvfsbuf.f_blocks)
			table[nfsys].bfree = (u_long)(-1);
		else
			table[nfsys].bfree = statvfsbuf.f_bavail;
		table[nfsys].ffree = statvfsbuf.f_ffree;
		table[nfsys].bsize = statvfsbuf.f_bsize;
		table[nfsys].bused = (u_long) 0;
		table[nfsys].fused = (u_long) 0;
		if ( table[nfsys].bsize != NBPSCTR )
			table[nfsys].bfree *= ((table[nfsys].bsize+NBPSCTR-1)/NBPSCTR);
	}
	if(pclose(pp)) {
		progerr("unable to obtain mounted filesystems from /sbin/mount");
		return(-1);
	}

	(void) readmap();
	if ((spacefile) && (readspace(spacefile) < 0))
		return(-1);

	for(i=0; i < nfsys; ++i) {
		if((!table[i].fused) && (!table[i].bused))
			continue; /* not used by us */
		bfree = (long) table[i].bfree - (long) table[i].bused;
		ffree = (long) table[i].ffree - (long) table[i].fused;
		if(bfree <= 0) {
			logerr("WARNING:");
			logerr("%lu free blocks are needed in the %s filesystem,",
				table[i].bused, table[i].name);
			logerr("your only free space is in the filesystem reserve area.");
			error++;
		} else
		if(bfree < LIM_BFREE) {
			warn("blocks", table[i].name, 
				table[i].bused + LIM_BFREE, table[i].bfree);
			error++;
		}
		if(ffree < LIM_FFREE) {
			warn("file nodes", table[i].name, 
				table[i].fused + LIM_FFREE, table[i].ffree);
			error++;
		}
	}
	return(error);
}

static void
warn(type, name, need, avail)
char *type, *name;
ulong need, avail;
{
	logerr("WARNING:");
	logerr("%lu free %s are needed in the %s filesystem,",
		 need, type, name);
	logerr("but only %lu %s are currently available.", avail, type);
}

static int
fsys(path)
char *path;
{
	register int i;
	int n, level, found;

	found = rootfsys;
	level = 0;
	for(i=0; i < nfsys; i++) {
		if(i == rootfsys)
			continue;
		if(n = fsyscmp(table[i].name, path)) {
			if(n > level) {
				level = n;
				found = i;
			}
		}
	}
	return(found);
}

static int
fsyscmp(fsystem, path)
char *fsystem, *path;
{
	int level;

	level = 0;
	while(*fsystem) {
		if(*fsystem != *path)
			break;
		if(*fsystem++ == '/')
			level++;
		path++;
	}
	if((*fsystem != '\0') || (*path && (*path != '/')))
		return(0);
	return(level);
}

static int
readmap()
{
	struct cfent *ept;
	struct stat statbuf;
	long	blk;
	int	i, n;

	for(i=0; (ept = eptlist[i]) != NULL; i++) {
		if(ept->ftype == 'i')
			continue;
		n = fsys(ept->path);
		if(stat(ept->path, &statbuf)) {
			/* path cannot be accessed */
			table[n].fused++;
			if(strchr("dxlspcb", ept->ftype))
				blk = nblk((long)table[n].bsize);
			else if((ept->ftype != 'e') && 
			(ept->cinfo.size != BADCONT))
				blk = nblk(ept->cinfo.size);
			else
				blk = 0;
		} else {
			/* path already exists */
			if(strchr("dxlspcb", ept->ftype))
				blk = 0;
			else if((ept->ftype != 'e') && 
			(ept->cinfo.size != BADCONT)) {
				blk = nblk(ept->cinfo.size);
				blk -= nblk(statbuf.st_size);
				/* negative blocks show room freed, but since
				 * order of installation is uncertain show
				 * 0 blocks usage 
				 */
				if(blk < 0)
					blk = 0;
			} else
				blk = 0;
		}
		table[n].bused += blk;
	}
	return(0);
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
readspace(spacefile)
char	*spacefile;
{
	FILE	*fp;
	char	*pt, path[256], line[LSIZE];
	long	blocks, nodes;
	int	n;

	if(spacefile == NULL)
		return(0);

	if((fp=fopen(spacefile, "r")) == NULL) {
		progerr("unable to open spacefile %s", spacefile);
		return(-1);
	}

	while(fgets(line, LSIZE, fp)) {
		for(pt=line; isspace(*pt);)
			pt++;
		if((*line == '#') || !*line)
			continue;

		(void) sscanf(line, "%s %ld %ld", path, &blocks, &nodes);
		mappath(2, path);
		basepath(path, basedir);

		n = fsys(path);
		table[n].bused += blocks;
		table[n].fused += nodes;
	}
	(void) fclose(fp);
	return(0);
}
