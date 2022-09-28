/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:installf/removef.c	1.8.3.1"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include "install.h"

#define MALSIZ	64
#define ERR_MEMORY	"memory allocation failure, errno=%d"
#define ERR_RELPATH	"ERROR: relative pathname <%s> ignored"

extern struct cfent 
		**eptlist;
extern int	errno,
		eptnum,
		warnflag;
extern void	*calloc(),
		*realloc();
extern void	progerr(),
		logerr(),
		quit(),
		canonize(),
		free(),
		qsort();
extern int	cfentcmp();
extern char	*pathdup();

void
removef(argc, argv)
int argc;
char *argv[];
{
	struct cfent *new;
	char	path[PATH_MAX];
	int	flag;

	flag = !strcmp(argv[0], "-");

	/* read stdin to obtain entries, which need to be sorted */
	eptlist = (struct cfent **) calloc(MALSIZ, 
		sizeof(struct cfent *));

	eptnum = 0;
	new = NULL;
	for(;;) {
		if(flag) {
			if(fgets(path, PATH_MAX, stdin) == NULL)
				break;
		} else {
			if(argc-- <= 0)
				break;
			(void) strcpy(path, argv[argc]);
		}
		canonize(path);
		if(path[0] != '/') {
			logerr(ERR_RELPATH, path);
			if(new)
				free(new);
			warnflag++;
			continue;
		}
		new = (struct cfent *) calloc(1, sizeof(struct cfent));
		if(new == NULL) {
			progerr(ERR_MEMORY, errno);
			quit(99);
		}
		new->ftype = '-';
		new->path = pathdup(path);
		eptlist[eptnum] = new;
		if((++eptnum % MALSIZ) == 0) { 
			eptlist = (struct cfent **) realloc((void *)eptlist, 
			   (unsigned) (sizeof(struct cfent)*(eptnum+MALSIZ)));
			if(!eptlist) {
				progerr(ERR_MEMORY, errno);
				quit(99);
			}
		}
	}
	eptlist[eptnum] = (struct cfent *)NULL;

	qsort((char *)eptlist, 
		(unsigned)eptnum, sizeof(struct cfent *), cfentcmp);
}
