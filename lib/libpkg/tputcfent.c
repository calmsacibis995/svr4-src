/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libpkg:tputcfent.c	1.7.3.1"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <pkgstrct.h>
	
extern char	*ctime();

void
tputcfent(ept, fp)
struct cfent *ept;
FILE *fp;
{
	int	count, status;
	char	*pt;
	struct pinfo *pinfo;

	if(ept->path == NULL)
		return;

	(void) fprintf(fp, "Pathname: %s\n", ept->path);
	(void) fprintf(fp, "Type: ");

	switch(ept->ftype) {
	  case 'f':
		(void) fputs("regular file\n", fp);
		break;

	  case 'd':
		(void) fputs("directory\n", fp);
		break;

	  case 'x':
		(void) fputs("exclusive directory\n", fp);
		break;

	  case 'v':
		(void) fputs("volatile file\n", fp);
		break;

	  case 'e':
		(void) fputs("editted file\n", fp);
		break;

	  case 'p':
		(void) fputs("named pipe\n", fp);
		break;

	  case 'i':
		(void) fputs("installation file\n", fp);
		break;

	  case 'c':
	  case 'b':
		(void) fprintf(fp, "%s special device\n", 
			(ept->ftype == 'b') ? "block" : "character");
		(void) fprintf(fp, "Major device number: %d\n", 
			ept->ainfo.major);
		(void) fprintf(fp, "Minor device number: %d\n", 
			ept->ainfo.minor);
		break;

	  case 'l':
		(void) fputs("linked file\n", fp);
		pt = (ept->ainfo.local ? ept->ainfo.local : "(unknown)");
		(void) fprintf(fp, "Source of link: %s\n", pt);
		break;

	  case 's':
		(void) fputs("symbolic link\n", fp);
		pt = (ept->ainfo.local ? ept->ainfo.local : "(unknown)");
		(void) fprintf(fp, "Source of link: %s\n", pt);
		break;

	  default:
		(void) fputs("unknown\n", fp);
		break;
	}

	if(!strchr("lsin", ept->ftype)) {
		if(ept->ainfo.mode < 0)
			(void) fprintf(fp, "Expected mode: ?\n");
		else
			(void) fprintf(fp, "Expected mode: %o\n", 
				ept->ainfo.mode);
		(void) fprintf(fp, "Expected owner: %s\n", ept->ainfo.owner);
		(void) fprintf(fp, "Expected group: %s\n", ept->ainfo.group);
	}
	if(strchr("?infv", ept->ftype)) {
		(void) fprintf(fp, "Expected file size (bytes): %ld\n", 
			ept->cinfo.size);
		(void) fprintf(fp, "Expected sum(1) of contents: %ld\n", 
			ept->cinfo.cksum);
		(void) fprintf(fp, "Expected last modification: %s", 
		  (ept->cinfo.modtime > 0) ?
		  ctime(&(ept->cinfo.modtime))+4 : "?\n");
	}
	if(ept->ftype == 'i') {
		(void) fputc('\n', fp);
		return;
	}

	status = count = 0;
	if(pinfo = ept->pinfo) {
		(void) fprintf(fp, "Referenced by the following packages:\n\t");
		while(pinfo) {
			if(pinfo->status)
				status++;
			(void) fprintf(fp, "%-15s", pinfo->pkg);
			if((++count % 5) == 0) {
				(void) fputc('\n', fp);
				(void) fputc('\t', fp);
				count = 0;
			}
			pinfo = pinfo->next;
		}
		(void) fputc('\n', fp);
	}
	(void) fprintf(fp, "Current status: %s\n", status ? 
		"partially installed" : "installed");
	(void) fputc('\n', fp);
}
