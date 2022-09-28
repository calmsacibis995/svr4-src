/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:libinst/copyf.c	1.5.4.1"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <utime.h>
#include <sys/stat.h>

extern int	errno;
extern int	isdir(),
		mkdir();
extern void	progerr();

#define ERR_NODIR	"unable to create directory <%s>, errno=%d"
#define ERR_STAT	"unable to stat pathname <%s>, errno=%d"
#define ERR_READ	"unable to open <%s> for reading, errno=%d"
#define ERR_WRITE	"unable to open <%s> for writing, errno=%d"
#define ERR_MODTIM	\
	"unable to reset access/modification time of <%s>, errno=%d"

copyf(from, to, mytime)
char	*from, *to;
long	mytime;
{
	struct stat status;
	struct utimbuf times;
	FILE	*fp1, *fp2;
	int	c;
	char	*pt;

	if(mytime == 0) {
		if(stat(from, &status)) {
			progerr(ERR_STAT, from, errno);
			return(-1);
		}
		times.actime = status.st_atime;
		times.modtime = status.st_mtime;
	} else {
		times.actime = mytime;
		times.modtime = mytime;
	}

	if((fp1 = fopen(from, "r")) == NULL) {
		progerr(ERR_READ, from, errno);
		return(-1);
	}
	if((fp2 = fopen(to, "w")) == NULL) {
		pt = to;
		while(pt = strchr(pt+1, '/')) {
			*pt = '\0';
			if(isdir(to)) {
				if( mkdir(to, 0755)) {
					progerr(ERR_NODIR, to, errno);
					*pt = '/';
					return(-1);
				}
			}
			*pt = '/';
		}
		if((fp2 = fopen(to, "w")) == NULL) {
			progerr(ERR_WRITE, to, errno);
			(void) fclose(fp1);
			return(-1);
		}
	}
	while((c = getc(fp1)) != EOF)
		putc((char) c, fp2);
	(void) fclose(fp1);
	(void) fclose(fp2);

	if(utime(to, &times)) {
		progerr(ERR_MODTIM, to, errno);
		return(-1);
	}

	return(0);
}
