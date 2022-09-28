/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginstall/sortmap.c	1.11.3.1"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include "install.h"

struct cfent	
		**eptlist;
struct mergstat
		*mstat;

extern char	**class;
extern int	dbchg;

extern void	*calloc(),
		free(),
		progerr(),
		quit(),
		echo();
extern int	pkgdbmerg();

extern struct cfent	**procmap();

int
sortmap(pkgmapfp, mapfp, tmpfp)
FILE	*pkgmapfp, *mapfp, *tmpfp;
{
	int	i, n, nparts;
	
	echo("## Processing package information.");

	/*
	 * read the pkgmap provided by this package into
	 * memory; map parameters specified in the pathname
	 * and sort in memory by pathname
	 */
	(void) fseek(pkgmapfp, 0L, 0); /* rewind input file */
	eptlist = procmap(pkgmapfp, 2);
	if(eptlist == NULL) {
		progerr("unable to process pkgmap");
		quit(99);
	}

	echo("## Processing system information.");

	/* 
	 * calculate the number of parts in this package
	 * by locating the entry with the largest "volno"
	 * associated with it
	 */
	nparts = 0;
	for(i=0; eptlist[i]; i++) {
		n = eptlist[i]->volno;
		if(n > nparts)
			nparts = n;
	}

	/*
	 * alloc an array to hold information about how each
	 * entry in memory matches with information already
	 * stored in the "contents" file
	 */
	if(mstat)
		free(mstat);
	mstat = (struct mergstat *)calloc((unsigned)i, sizeof(struct mergstat));
	dbchg = pkgdbmerg(mapfp, tmpfp, eptlist, mstat, 60);
	if(dbchg < 0) {
		progerr("unable to merge package and system information");
		quit(99);
	}
	return(nparts);
}
