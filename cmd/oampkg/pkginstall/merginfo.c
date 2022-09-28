/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginstall/merginfo.c	1.8.4.1"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <install.h>

extern char	instdir[],
		pkgbin[],
		pkgloc[],
		savlog[],
		tmpdir[],
		*respfile,
		**environ,
		**class,
		**pclass;

extern void	progerr(),
		quit();
extern int	unlink(),
		cppath();

void
merginfo()
{
	struct dirent 
		*dp;
	FILE	*fp;
	DIR	*pdirfp;
	char	path[PATH_MAX], temp[PATH_MAX];
	int	i, j, found;

	/* remove savelog from previous attempts */
	(void) unlink(savlog);

	/* 
	 * output packaging environment to create a pkginfo file in pkgloc[]
	 */
	(void) sprintf(path, "%s/%s", pkgloc, PKGINFO);
	if((fp = fopen(path, "w")) == NULL) {
		progerr("unable to open <%s> for writing", path);
		quit(99);
	}
	(void) fputs("CLASSES=", fp);
	if(pclass) {
		(void) fprintf(fp, "%s", pclass[0]);
		for(i=1; pclass[i]; i++)
			(void) fprintf(fp, " %s", pclass[i]);
	}
	for(i=0; class[i]; i++) {
		found = 0;
		if(pclass) {
			for(j=0; pclass[j]; ++j) {
				if(!strcmp(pclass[j], class[i])) {
					found++;
					break;
				}
			}
		}
		if(!found)
			(void) fprintf(fp, " %s", class[i]);
	}
	(void) fputc('\n', fp);
		
	/* output all other environment parameters except CLASSES */
	for(i=0; environ[i]; i++) {
		if(!strncmp(environ[i], "CLASSES=", 8))
			continue;
		(void) fputs(environ[i], fp);
		(void) fputc('\n', fp);
	}
	(void) fclose(fp);

	/* 
	 * copy all packaging scripts to appropriate directory
	 */
	(void) sprintf(path, "%s/install", instdir);
	if((pdirfp = opendir(path)) == NULL) 
		return;
	while((dp = readdir(pdirfp)) != NULL) {
		if(dp->d_name[0] == '.')
			continue;
		
		(void) sprintf(path, "%s/install/%s", instdir, dp->d_name);
		(void) sprintf(temp, "%s/%s", pkgbin, dp->d_name);
		if(cppath(0, path, temp)) {
			progerr("unable to save copy of <%s> in %s", dp->d_name,
				pkgbin);
			quit(99);
		}
	}
	(void) closedir(pdirfp);
}
