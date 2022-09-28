/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginstall/cppath.c	1.7.4.2"
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <utime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pkglocs.h>
#include <errno.h>

extern int	errno;

extern void	progerr(),
		logerr(),
		echo();
extern int	mkdir(),
		unlink(),
		access();

#define MSG_IMPDIR	"%s <implied directory>"
#define WRN_RENAME	"WARNING: unable to rename <%s>"
#define MSG_RENAME	"- file left in indeterminate state"
#define ERR_STAT	"unable to stat <%s>"
#define MSG_PROCMV	"- executing process moved to <%s>"
#define ERR_READ	"unable to open <%s> for reading"
#define ERR_WRITE	"unable to open <%s> for writing"
#define ERR_OUTPUT	"error while writing file <%s>"
#define ERR_UNLINK	"unable to unlink <%s>"
#define ERR_LOG		"unable to open logfile <%s>"
#define ERR_UTIME	"unable to reset access/modification time of <%s>"
#define ERR_MKDIR	"unable to create directory <%s>"

static FILE	*logfp;
static char	*linknam;
static int	errflg = 0;

static FILE 	*writefile();
static int	fixpath();

cppath(dsp, f1, f2)
int	dsp;
char *f1, *f2;
{
	struct stat status;
	struct utimbuf times;
	FILE	*fp1, *fp2;
	int	c;
	char	busylog[PATH_MAX];

	if(stat(f1, &status)) {
		progerr(ERR_STAT, f1);
		errflg++;
		return(1);
	}
	times.actime = status.st_atime;
	times.modtime = status.st_mtime;

	if((fp1 = fopen(f1, "r")) == NULL) {
		progerr(ERR_READ, f1);
		errflg++;
		return(1);
	}
	if((fp2 = writefile(dsp, f2)) == NULL) {
		(void) fclose(fp1);
		return(1);
	}

	while((c=getc(fp1)) != EOF) {
		if(putc(c, fp2) == EOF) {
			progerr(ERR_OUTPUT, f2);
			(void) fclose(fp1);
			(void) fclose(fp2);
			return(1);
		}
	}

	(void) fclose(fp1);
	(void) fclose(fp2);

	if(linknam) {
		if(unlink(linknam)) {
			if(errno == ETXTBSY)
				logerr(MSG_PROCMV, linknam);
			else {
				progerr(ERR_UNLINK, linknam);
				errflg++;
			}
			if(!logfp) {
				(void) sprintf(busylog, "%s/textbusy", PKGADM);
				if((logfp = fopen(busylog, "a")) == NULL) {
					progerr(ERR_LOG, busylog);
					errflg++;
				} else
					(void) fprintf(logfp, "%s\n", linknam);
			} else
				(void) fprintf(logfp, "%s\n", linknam);
		}
	}
	if(utime(f2, &times)) {
		progerr(ERR_UTIME, f2);
		errflg++;
		return(1);
	}
	return(0);
}

static int
fixpath(dspmode, file)
int	dspmode;
char *file;
{
	char *pt;
	int found;

	found = 0;
	for(pt=file; *pt; pt++) {
		if((*pt == '/') && (pt != file)) {
			*pt = '\0';
			if(access(file, 0)) {
				if(mkdir(file, 0755)) {
					progerr(ERR_MKDIR, file);
					*pt = '/';
					return(1);
				}
				if(dspmode)
					echo(MSG_IMPDIR, file);
				found++;
			}
			*pt = '/';
		}
	}
	return(!found);
}

static FILE *
writefile(dsp, file)
int	dsp;
char *file;
{
	FILE *fp;
	char *pt, dirname[512];

	if(access(file, 0) == 0) {
		/* link the file to be copied to a temporary
		 * name in case it is executing or it is
		 * being written/used (e.g., a shell script
		 * currently being executed 
		 */
		(void) strcpy(dirname, file);
		if(pt = strrchr(dirname, '/'))
			*pt++ = '\0';
		else {
			(void) strcpy(dirname, ".");
			pt = file;
		}

		/* we won't be able to link files we can't read */
		linknam = tempnam(dirname, pt);
		if(!linknam || rename(file, linknam)) {
			logerr(WRN_RENAME, file);
			linknam = NULL;
		}
		if((fp = fopen(file, "w")) == NULL) {
			progerr(ERR_WRITE, file);
			/* try to relink file */
			if(!linknam || rename(linknam, file))
				logerr(MSG_RENAME);
			errflg++;
		}
	} else {
		linknam = NULL;
		if((fp = fopen(file, "w")) == NULL) {
			if(fixpath(dsp, file) == 0) {
				if((fp = fopen(file, "w")) == NULL) {
					progerr(ERR_WRITE, file);
					errflg++;
				}
			}
		}
	}
	return(fp);
}
