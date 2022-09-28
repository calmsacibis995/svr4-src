/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/dofile.c	6.7"
# include	"../../hdr/defines.h"
# include	<dirent.h>


static int	nfiles;
char	had_dir;
char	had_standinp;

void
do_file(p,func)
register char *p;
int (*func)();
{
	extern char *Ffile;
	char str[FILESIZE];
	char ibuf[FILESIZE];
	DIR *iop;
	struct dirent *dirp;

	if (p[0] == '-') {
		had_standinp = 1;
		while (gets(ibuf) != NULL) {
			if (exists(ibuf) && (Statbuf.st_mode & S_IFMT) == S_IFDIR) {
				had_dir = 1;
				Ffile = ibuf;
				if((iop = opendir(ibuf)) == NULL)
					return;
				(void) readdir(iop);   /* skip "."  */
				(void) readdir(iop);   /* skip ".."  */
				while((dirp = readdir(iop)) != NULL) {
					sprintf(str,"%s/%s",ibuf,dirp->d_name);
					if(sccsfile(str)) {
						Ffile = str;
						(*func)(str);
						nfiles++;
					}
				}
				(void) closedir(iop);
			}
			else if (sccsfile(ibuf)) {
				Ffile = ibuf;
				(*func)(ibuf);
				nfiles++;
			}
		}
	}
	else if (exists(p) && (Statbuf.st_mode & S_IFMT) == S_IFDIR) {
		had_dir = 1;
		Ffile = p;
		if((iop = opendir(p)) == NULL)
			return;
		(void) readdir(iop);   /* skip "."  */
		(void) readdir(iop);   /* skip ".."  */
		while((dirp = readdir(iop)) != NULL) {
			sprintf(str,"%s/%s",p,dirp->d_name);
			if(sccsfile(str)) {
				Ffile = str;
				(*func)(str);
				nfiles++;
			}
		}
		(void) closedir(iop);
	}
	else {
		Ffile = p;
		(*func)(p);
		nfiles++;
	}
}
