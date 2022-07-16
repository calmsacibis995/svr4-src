/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libpkg:putcfile.c	1.5.3.1"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <pkgstrct.h>
	
int
putcfile(ept, fp)
struct cfent *ept;
FILE *fp;
{
	struct pinfo *pinfo;

	if(ept->ftype == 'i')
		return(0); /* no ifiles stored in contents DB */

	if(ept->path == NULL)
		return(-1);

	if(fprintf(fp, "%s", ept->path) < 0)
		return(-1);

	if(ept->ainfo.local) {
		if(fprintf(fp, "=%s", ept->ainfo.local) < 0)
			return(-1);
	}

	if(ept->volno) {
		if(fprintf(fp, " %d", ept->volno) < 0)
			return(-1);
	}

	if(fprintf(fp, " %c %s", ept->ftype, ept->class) < 0)
		return(-1);

	if(strchr("cb", ept->ftype)) {
		if(ept->ainfo.major == BADMAJOR) {
			if(fprintf(fp, " ?") < 0)
				return(-1);
		} else {
			if(fprintf(fp, " %d", ept->ainfo.major) < 0)
				return(-1);
		}
		if(ept->ainfo.minor == BADMINOR) {
			if(fprintf(fp, " ?") < 0)
				return(-1);
		} else {
			if(fprintf(fp, " %d", ept->ainfo.minor) < 0)
				return(-1);
		}
	}

	if(strchr("dxcbpfve", ept->ftype)) {
		if(fprintf(fp, ((ept->ainfo.mode == BADMODE) ? " ?" : " %04o"), 
		   ept->ainfo.mode) < 0)
			return(-1);
		if(fprintf(fp, " %s %s", ept->ainfo.owner, ept->ainfo.group) < 0)
			return(-1);
	}

	if(strchr("ifve", ept->ftype)) {
		if(fprintf(fp, ((ept->cinfo.size == BADCONT) ? " ?" : " %ld"), 
		   ept->cinfo.size) < 0)
			return(-1);
		if(fprintf(fp, ((ept->cinfo.cksum == BADCONT) ? " ?" : " %ld"), 
		   ept->cinfo.cksum) < 0)
			return(-1);
		if(fprintf(fp, ((ept->cinfo.modtime == BADCONT) ? " ?" : " %ld"), 
		   ept->cinfo.modtime) < 0)
			return(-1);
	}

	if(ept->ftype == 'i') {
		if(fputc('\n', fp) == EOF)
			return(-1);
		return(0);
	}

	pinfo = ept->pinfo;
	while(pinfo) {
		if(fputc(' ', fp) == EOF)
			return(-1);
		if(pinfo->status) {
			if(fputc(pinfo->status, fp) == EOF)
				return(-1);
		}
		if(fprintf(fp, "%s", pinfo->pkg) < 0)
			return(-1);
		if(pinfo->editflag) {
			if(fprintf(fp, "\\") < 0)
				return(-1);
		}
		if(pinfo->aclass[0]) {
			if(fprintf(fp, ":%s", pinfo->aclass) < 0) 
				return(-1);
		}
		pinfo = pinfo->next;
	}
	if(fprintf(fp, "\n") < 0) 
		return(-1);
	return(0);
}
