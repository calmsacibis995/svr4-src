/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*      Portions Copyright (c) 1988, Sun Microsystems, Inc.     */ 
/*      All Rights Reserved.                                    */ 
 
#ident	"@(#)ucbprt:lib/dofile.c	1.1.1.1"

#include	"../hdr/defines.h"
#include	<sys/dir.h>


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
	char	dbuf[BUFSIZ];
	FILE *iop;
	struct direct dir[2];
	int	imatch(), stat();

	if (p[0] == '-') {
		had_standinp = 1;
		while (gets(ibuf) != NULL) {
			if (exists(ibuf) && (Statbuf.st_mode & S_IFMT) == S_IFDIR) {
				had_dir = 1;
				Ffile = ibuf;
				if((iop = fopen(ibuf,"r")) == NULL)
					return;
				setbuf(iop,dbuf);
				dir[1].d_ino = 0;
				(void) fread(dir,sizeof(dir[0]),1,iop);   /* skip "."  */
				(void) fread(dir,sizeof(dir[0]),1,iop);   /* skip ".."  */
				while(fread(dir,sizeof(dir[0]),1,iop) == 1) {
					if(dir[0].d_ino == 0) continue;
					sprintf(str,"%s/%s",ibuf,dir[0].d_name);
					if(sccsfile(str)) {
						Ffile = str;
						(*func)(str);
						nfiles++;
					}
				}
				(void) fclose(iop);
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
		if((iop = fopen(p,"r")) == NULL)
			return;
		setbuf(iop,dbuf);
		dir[1].d_ino = 0;
		(void) fread(dir,sizeof(dir[0]),1,iop);   /* skip "."  */
		(void) fread(dir,sizeof(dir[0]),1,iop);   /* skip ".."  */
		while(fread(dir,sizeof(dir[0]),1,iop) == 1) {
			if(dir[0].d_ino == 0) continue;
			sprintf(str,"%s/%s",p,dir[0].d_name);
			if(sccsfile(str)) {
				Ffile = str;
				(*func)(str);
				nfiles++;
			}
		}
		(void) fclose(iop);
	}
	else {
		Ffile = p;
		(*func)(p);
		nfiles++;
	}
}
