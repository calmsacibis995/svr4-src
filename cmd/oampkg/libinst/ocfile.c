/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:libinst/ocfile.c	1.4.4.1"

#include <stdio.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <pkglocs.h>

static char
	contents[PATH_MAX],
	t_contents[PATH_MAX];

extern int	errno;
extern char	*prog;
extern int	warnflag;

extern int	ctime(),
		creat(),
		close(),
		lockf(),
		unlink(),
		fsync();
extern time_t	time();
extern void	logerr(),
		progerr(),
		quit(),
		echo();
		
int
ocfile(mapfp, tmpfp)
FILE	**mapfp, **tmpfp;
{
	int	fd;

	*mapfp = *tmpfp = NULL;

	/* we open the file for update because we don't
	 * want any other process either reading or
	 * writing the contents file while we're
	 * mucking with it
	 */
	(void) sprintf(contents, "%s/contents", PKGADM);
	if((*mapfp = fopen(contents, "r")) == NULL) {
		if(errno == ENOENT) {
			fd = creat(contents, 0644);
			if(fd < 0) {
				progerr("unable to create contents file <%s>", 
					contents);
				logerr("(errno %d)", errno);
				return(-1);
			}
			echo("## Software contents file initialized");
			(void) close(fd);
			*mapfp = fopen(contents, "r");
		}
		if(*mapfp == NULL) {
			progerr("unable to open contents file <%s>", contents);
			logerr("(errno %d)", errno);
			return(-1);
		}
	}

	(void) sprintf(t_contents, "%s/t.contents", PKGADM);
	if((*tmpfp = fopen(t_contents, "w")) == NULL) {
		progerr("unable to open <%s> for writing", t_contents);
		logerr("(errno %d)", errno);
		(void) fclose(*tmpfp);
		*mapfp = NULL;
		return(-1);
	}

	if(lockf(fileno(*tmpfp), F_TLOCK, 0)) {
		progerr("unable to lock <%s> for modification", t_contents);
		logerr("(errno %d)", errno);
		(void) fclose(*mapfp);
		(void) fclose(*tmpfp);
		*tmpfp = NULL;
		*mapfp = NULL;
		return(-1);
	}
	return(0);
}

int
swapcfile(tmpfp, pkginst)
FILE	*tmpfp;
char	*pkginst;
{
	char	s_contents[PATH_MAX];
	time_t	clock;

	if(pkginst == NULL) {
		if(fclose(tmpfp)) {
			logerr("WARNING: unable to close <%s>", t_contents);
			logerr("(errno %d)", errno);
			warnflag++;
		}
		if(unlink(t_contents)) {
			logerr("WARNING: unable to close <%s>", t_contents);
			logerr("(errno %d)", errno);
			warnflag++;
		}
		return(0);
	}
		
	/* need to modify file */
	(void) time(&clock);
	(void) fprintf(tmpfp, "# Last modified by %s for %s package\n# %s",
		prog, pkginst, ctime(&clock));
	if(fflush(tmpfp)) {
		progerr("unable to update contents file");
		logerr("fflush failed (errno %d)", errno);
		return(-1);
	}
	if(fsync(fileno(tmpfp))) {
		progerr("unable to update contents file");
		logerr("fsync failed (errno %d)", errno);
		return(-1);
	}
		
	if(fclose(tmpfp)) {
		progerr("unable to update contents file");
		logerr("fclose failed (errno %d)", errno);
		return(-1);
	}
	(void) sprintf(s_contents, "%s/s.contents", PKGADM);
	if(rename(contents, s_contents)) {
		progerr("unable to update contents file");
		logerr("rename(%s, %s) failed (errno %d)",
			contents, s_contents, errno);
		return(-1);
	}
	if(rename(t_contents, contents)) {
		progerr("unable to establish contents file");
		logerr("rename(%s, %s) failed (errno %d)",
			t_contents, contents, errno);
		if(rename(s_contents, contents)) {
			progerr("attempt to restore <%s> failed", 
				contents);
			logerr("rename(%s, %s) failed (errno %d)",
				s_contents, contents, errno);
		}
		return(-1);
	}
	if(unlink(s_contents)) {
		logerr("WARNING: unable to unlink <%s>", s_contents);
		logerr("(errno %d)", errno);
		warnflag++;
	}
	return(0);
}
