/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)oampkg:libinst/procmap.c	1.9.6.1"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include "install.h"

#define ERR_MEMORY	"memory allocation failure, errno=%d"
#define ERR_DUPPATH	"duplicate pathname <%s>"

extern char	*basedir;
extern char	*errstr, **class;
extern int	errno, nclass;

extern char	*getenv(), 
		*pathdup(),
		*pkgparam();
extern void	*calloc(), 
		*realloc();
extern void	progerr(),
		logerr(),
		quit(),
		free(),
		canonize(),
		mappath(),
		basepath(),
		mapvar();
extern int	gpkgmap();

#define DIRMALLOC	128
#define EPTMALLOC	512

static struct cfent *space;
static struct cfent **eptlist;
static int	eptnum;

static int	errflg = 0;
static int	ckdup(),
		sortentry();

struct cfent **
procmap(fp, mapflag)
FILE	*fp;
int	mapflag;
{
	struct cfent *ept;
	int	i, n, nparts;
	char	source[PATH_MAX+1];

	errflg = nparts = eptnum = 0;

	if(space)
		free(space);
	if(eptlist)
		free(eptlist);

	/* initialize dynamic memory used to store
	 * path information which is read in
	 */
	(void) pathdup((char *)0);

	space = (struct cfent *) calloc(EPTMALLOC, 
		(unsigned) sizeof(struct cfent));
	if(space == NULL) {
		progerr(ERR_MEMORY, errno);
		quit(99);
	}

	for(;;) {
		ept = &space[eptnum];
		n = gpkgmap(ept, fp);
		if(n == 0)
			break; /* no more entries in pkgmap */
		else if(n < 0) {
			progerr("bad entry read in pkgmap");
			logerr("pathname=%s", ept->path);
			logerr("problem=%s", errstr);
			return(NULL);
		}
		if((nclass >= 0) && (ept->ftype != 'i')) {
			for(n=0; n < nclass; n++) {
				if(!strcmp(class[n], ept->class))
					break;
			}
			if(n >= nclass)
				continue;
		}

		if(ept->volno > nparts)
			nparts++;

		/* generate local paths for files which need them */
		if((mapflag > 1) && strchr("fve", ept->ftype)) {
			if(ept->ainfo.local == NULL) {
				source[0] = '~';
				(void) strcpy(&source[1], ept->path);	
				ept->ainfo.local = pathdup(source);
			}
			canonize(ept->ainfo.local);
		}		

		if(mapflag && (ept->ftype != 'i')) {
			mappath(2, ept->path);
			basepath(ept->path, basedir);
		}
		canonize(ept->path);

		if(strchr("sl", ept->ftype)) {
			if(mapflag) {
				mappath(2, ept->ainfo.local);
				if(strchr("l", ept->ftype) || 
				   (!isdot(ept->ainfo.local) &&
				    !isdotdot(ept->ainfo.local)))
					basepath(ept->ainfo.local, basedir);
			}
			if(strchr("l", ept->ftype) ||
			   (!isdot(ept->ainfo.local) &&
			    !isdotdot(ept->ainfo.local)))
				canonize(ept->ainfo.local);

		} else if(mapflag && !strchr("isl", ept->ftype)) {
			/* map owner and group info */
			mapvar(2, ept->ainfo.owner);
			mapvar(2, ept->ainfo.group);
		}

		ept->path = pathdup(ept->path);
		if(ept->ainfo.local != NULL)
			ept->ainfo.local = pathdup(ept->ainfo.local);

		if((++eptnum % EPTMALLOC) == 0) {
			space = (struct cfent *) realloc(space, 
			   (unsigned) (sizeof(struct cfent) *
				(eptnum+EPTMALLOC)));
			if(space == NULL) {
				progerr(ERR_MEMORY, errno);
				return(NULL);
			}
		}
	}

	/* setup a pointer array to point to malloc'd entries space */
	eptlist = (struct cfent **) calloc(eptnum+1, sizeof(struct cfent *));
	if(eptlist == NULL) {
		progerr(ERR_MEMORY, errno);
		quit(99);
	}
	for(i=0; i < eptnum; i++)
		eptlist[i] = &space[i];

	(void) sortentry(-1);
	for(i=0; i < eptnum; ) {
		if(!sortentry(i))
			i++;
	}
	return(errflg ? NULL : eptlist);
}

static int
sortentry(index)
int index;
{
	struct cfent *ept;
	static int last = 0;
	int	i, n, j;
	int	upper, lower;

	if(index == 0)
		return(0);
	else if(index < 0) {
		last = 0;
		return(0);
	}

	ept = eptlist[index];
	/* quick comparison optimization for pre-sorted arrays */
	if(strcmp(ept->path, eptlist[index-1]->path) > 0) {
		/* do nothing */
		last = index-1;
		return(0);
	}

	lower = 0;
	upper = index-1;
	i = last;
	do {
		n = strcmp(ept->path, eptlist[i]->path);
		if(n == 0) {
			if(ckdup(ept, eptlist[i])) {
				progerr(ERR_DUPPATH, ept->path);
				errflg++;
			}
			/* remove the entry at index */
			while(index < eptnum) {
				eptlist[index] = eptlist[index+1];
				index++;
			}
			eptnum--;
			return(1);
		} else if(n < 0) {
			/* move down array */
			upper = i;
			i = lower + (upper-lower)/2; 
		} else {
			/* move up array */
			lower = i+1;
			i = upper - (upper-lower)/2; 
		}
	} while(upper != lower);
	last = i = upper;

	/* expand to insert at i */
	for(j=index; j > i; j--)
		eptlist[j] = eptlist[j-1];

	eptlist[i] = ept;
	return(0);
}

static int
ckdup(ept1, ept2)
struct cfent *ept1, *ept2;
{
	/* ept2 will be modified to contain "merged" entries */

	if(!strchr("?dx", ept1->ftype)) 
		return(1);

	if(!strchr("?dx", ept2->ftype)) 
		return(1);

	if(ept2->ainfo.mode < 0)
		ept2->ainfo.mode = ept1->ainfo.mode;
	if((ept1->ainfo.mode != ept2->ainfo.mode) &&
	   (ept1->ainfo.mode >= 0))
		return(1);

	if(!strcmp(ept2->ainfo.owner, "?"))
		(void) strcpy(ept2->ainfo.owner, ept1->ainfo.owner);
	if(strcmp(ept1->ainfo.owner, ept2->ainfo.owner) &&
	   strcmp(ept1->ainfo.owner, "?"))
		return(1);

	if(!strcmp(ept2->ainfo.group, "?"))
		(void) strcpy(ept2->ainfo.group, ept1->ainfo.group);
	if(strcmp(ept1->ainfo.group, ept2->ainfo.group) &&
	   strcmp(ept1->ainfo.group, "?"))
		return(1);

	return(0);
}
