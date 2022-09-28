/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:installf/dofinal.c	1.10.3.1"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include "install.h"

extern struct cfent 
		**eptlist;
extern char	errbuf[],
		*errstr,
		*prog,
		*pkginst;
extern int	errno,
		warnflag;

extern struct pinfo
		*eptstat();
extern void	progerr(),
		logerr(),
		quit();
extern int	finalck(),
		srchcfile(),
		putcfile();

#define ERR_WRITE \
"write of intermediate contents file failed"


dofinal(fp, fpo, rmflag, myclass)
FILE	*fp;
FILE	*fpo;
int	rmflag;
char	*myclass;
{
	struct cfent entry;
	struct pinfo *pinfo;
	int	n, indx, dbchg;

	entry.pinfo = NULL;
	indx = 0;
	while(eptlist[indx] && (eptlist[indx]->ftype == 'i'))
		indx++;

	dbchg = 0;
	while(n = srchcfile(&entry, "*", fp, fpo)) {
		if(n < 0) {
			progerr("bad entry read in contents file");
			logerr("pathname=%s", entry.path);
			logerr("problem=%s", errstr);
			quit(99);
		}
		if(myclass && strcmp(myclass, entry.class)) {
			if(putcfile(&entry, fpo)) {
				progerr(ERR_WRITE);
				quit(99);
			}
			continue;
		}

		pinfo = entry.pinfo;
		while(pinfo) {
			if(!strcmp(pkginst, pinfo->pkg))
				break;
			pinfo = pinfo->next;
		}
		if(pinfo) {
			if(rmflag && (pinfo->status == '-')) {
				dbchg++;
				(void) eptstat(&entry, pkginst, '@');
				if(entry.npkgs)
					if(putcfile(&entry, fpo)) {
						progerr(ERR_WRITE);
						quit(99);
					}
				continue;
			} else if(!rmflag && (pinfo->status == '+')) {
				dbchg++;
				pinfo->status = (finalck(&entry, 1, 1) ? 
					'!' : '\0');
			}
		}
		if(putcfile(&entry, fpo)) {
			progerr(ERR_WRITE);
			quit(99);
		}
	}
	return(dbchg);
}
