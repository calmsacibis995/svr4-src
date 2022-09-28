/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:maillock.c	1.8.3.1"

#include "maillock.h"
#include <sys/types.h>
#include <sys/dir.h>
#include <errno.h>
#include <fcntl.h>
#ifdef	F_SETLKW	/* Is file locking mechanism present on this system? */
#   include <unistd.h>
#endif
#include <stdio.h> 
#include <string.h> 
#include <signal.h> 
#ifdef SVR3
typedef int pid_t;
#endif

static	char	curlock[sizeof(MAILDIR) + 15];
static	int	locked = 0;
static	int	lffd = -1;	/* lock file file descriptor */

maillock (user, retrycnt)
char	*user;
int	retrycnt;
{
	char		tmplock[sizeof(SAVEDIR) + 15];
	char	 	buf[80];
	register int	i;
	pid_t 		lpid;
	int		len;
	extern	int	errno;
	FILE	 	*fp;
	pid_t		pid = getpid();

	if (locked) {
		return (L_SUCCESS);
	}

	/*
		Cannot create a lockfile with a basename of more than
		13 characters, as we couldn't discern between the
		lockfile and the file itself.
	*/
	if (strlen(user) > 13) {
		return (L_NAMELEN);
	}

	sprintf(tmplock,"%sLCK..%d", SAVEDIR, pid);
	unlink (tmplock); /* In case it's left over from some disaster */
	if ((lffd = open(tmplock, O_WRONLY | O_CREAT | O_TRUNC, 02644)) == -1) {
		sprintf(buf,"Cannot open '%s'", tmplock);
		perror(buf);
		return (L_TMPLOCK);
        }
	sprintf(buf,"%d",pid);
	len = strlen(buf) + 1;
	if (write(lffd,buf,len) != len) {
		sprintf(buf,"Error writing pid to '%s'", tmplock);
		perror(buf);
		(void) close(lffd);
		(void) unlink(tmplock);
		return (L_TMPWRITE);
	}
#ifdef F_SETLKW
	if (lockf(lffd, F_TLOCK, (long)0) == -1) {
		sprintf(buf,"Error setting mandatory lock on '%s'", tmplock);
		perror(buf);
		(void) close(lffd);
		(void) unlink(tmplock);
		return (L_MANLOCK);
	}
#endif
	/* Don't close lock file here to keep mandatory file lock in place. */
	/* It gets closed below in mailunlock() */

	/*
	 * Attempt to link temp lockfile to real lockfile.
	 * If link fails because real lockfile already exists,
	 * check that the pid it contains refers to a 'live'
	 * process. If not, remove it and try again...
	 */
	sprintf(curlock,"%s%s.lock", MAILDIR, user);
	for (i = 0; i < retrycnt; i++) {
		if (link(tmplock, curlock) == -1) {
			if (errno == EEXIST) {
				if ((fp = fopen(curlock,"r+")) != NULL) {
					if (fscanf(fp,"%ld",&lpid) == 1) {
					    if ((kill(lpid,0) == (pid_t)-1) &&
						(errno == ESRCH)) {
						    rewind(fp);
						    len =
						     sprintf(buf,"%ld",pid);
						    fwrite(buf, len+1, 1, fp);
						    (void)unlink(curlock);
					    }
					}
					(void)fclose(fp);
				}
				sleep(5*(i+1));
				continue;
			} else {
				sprintf(buf,"Link of mailfile lock failed");
				perror(curlock);
				(void) close(lffd);
				(void) unlink(tmplock);
				return (L_ERROR);
			}
		}
		(void) unlink(tmplock);
		locked++;
		return (L_SUCCESS);
	}
	(void) close(lffd);
	(void) unlink(tmplock);
	return (L_MAXTRYS);
}

mailunlock()
{
	if (locked) {
		(void) close(lffd); lffd = -1;
		(void) unlink(curlock);
		locked = 0;
	}
	return (L_SUCCESS);
}
