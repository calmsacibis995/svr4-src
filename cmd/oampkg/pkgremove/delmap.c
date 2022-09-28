/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkgremove/delmap.c	1.8.3.1"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <pkgstrct.h>

extern int	dbchg, 
		warnflag,
		otherstoo, 
		errno;
extern char	*errstr,
		*pkginst;
extern struct pinfo 
		*eptstat();
extern void	*calloc(), 
		*realloc(),
		free(),
		exit(),
		progerr(),
		quit();
extern char	*pathdup();
extern int	srchcfile(),
		putcfile(),
		ocfile(),
		swapcfile();

#define EPTMALLOC	128

struct cfent	**eptlist;
int	eptnum;

void
delmap(flag)
int	flag;
{
	struct cfent *ept;
	struct pinfo *pinfo;
	FILE	*fp, *fpo;
	int	n;

	if(ocfile(&fp, &fpo))
		quit(99);

	/* re-use any memory used to store pathnames */
	(void) pathdup(NULL);

	if(eptlist != NULL)
		free(eptlist);
	eptlist = (struct cfent **) calloc(EPTMALLOC, sizeof(struct cfent *));
	if(eptlist == NULL) {
		progerr("no memory, errno=%d", errno);
		quit(99);
	}

	ept = (struct cfent *) calloc(1, (unsigned) sizeof(struct cfent));
	if(!ept) {
		progerr("no memory, errno=%d", errno);
		quit(99);
	}

	eptnum = 0;
	while(n = srchcfile(ept, "*", fp, NULL)) {
		if(n < 0) {
			progerr("bad read of contents file");
			progerr("pathname=%s", ept->path);
			progerr("problem=%s", errstr);
			exit(99);
		}
		pinfo = eptstat(ept, pkginst, (flag ? '@' : '-'));
		if(ept->npkgs > 0) {
			if(putcfile(ept, fpo)) {
				progerr("write of entry failed, errno=%d", 
					errno);
				quit(99);
			}
		}

		if(flag || (pinfo == NULL))
			continue;

		dbchg++;

		/* 
		 * the path name is used by this package
		 * and this package only, or it is marked
		 * as an edittable file by this package
		 */
		if(!pinfo->editflag && otherstoo)
			ept->ftype = '\0';
		if(*pinfo->aclass)
			(void) strcpy(ept->class, pinfo->aclass);
		eptlist[eptnum] = ept;

		ept->path = pathdup(ept->path);
		if(ept->ainfo.local != NULL)
			ept->ainfo.local = pathdup(ept->ainfo.local);

		ept = (struct cfent *) calloc(1, sizeof(struct cfent));
		if((++eptnum % EPTMALLOC) == 0) {
			eptlist = (struct cfent **) realloc(eptlist, 
			(eptnum+EPTMALLOC)*sizeof(struct cfent *));
			if(eptlist == NULL) {
				progerr("no memory, errno=%d", errno);
				quit(99);
			}
		}
	}
	eptlist[eptnum] = (struct cfent *) NULL;

	(void) fclose(fp);
	if(swapcfile(fpo, (dbchg ? pkginst : NULL)))
		quit(99);
}
