/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/mpwlib/lockit.c	6.12"
/*
	Process semaphore.
	Try repeatedly (`count' times) to create `lockfile' mode 444.
	Sleep 10 seconds between tries.
	If `tempfile' is successfully created, write the process ID
	`pid' in `tempfile' (in binary), link `tempfile' to `lockfile',
	and return 0.
	If `lockfile' exists and it hasn't been modified within the last
	minute, and either the file is empty or the process ID contained
	in the file is not the process ID of any existing process,
	`lockfile' is removed and it tries again to make `lockfile'.
	After `count' tries, or if the reason for the create failing
	is something other than EACCES, return xmsg().
 
	Unlockit will return 0 if the named lock exists, contains
	the given pid, and is successfully removed; -1 otherwise.
*/

# include	"../../hdr/defines.h"
# include	"errno.h"
# include       "sys/utsname.h"
# include       <ccstypes.h>
# define        nodenamelength 9

static int	onelock();

int
lockit(lockfile,count,pid,uuname)
register char *lockfile;
register unsigned count;
pid_t pid;
char *uuname;
{
	register int fd;
	int ret;
	pid_t opid;
	char ouuname[nodenamelength];
	
	long ltime;
	long omtime;
	extern int errno;
	static char tempfile[1024];
	char	dir_name[1024];
	struct stat sbuf;
	unsigned short min_mode;
	int	strcmp(), close(), read(), open(), stat(), unlink(), kill();
	unsigned int	sleep();

	copy(lockfile,dir_name);
/*
	sprintf(tempfile,"%s/%u.%ld",dname(dir_name),pid,uuname,time((long *)0));
*/
	sprintf(tempfile,"%s/%u.%s%ld",dname(dir_name),pid,uuname,time((long *)0));

/*
** Determine if we have a chance to create the lockfile.
** (Is directory writeable?)
*/	

	(void) stat(dir_name,&sbuf);
	min_mode = 0003;
	if(sbuf.st_gid == getegid())
		min_mode = 0030;
	if(sbuf.st_uid == geteuid())
		min_mode = 0300;
	if(!((sbuf.st_mode & min_mode) == min_mode))
		return(-1);

	for (++count; --count; (void) sleep(10)) {
		if (onelock(pid,uuname,tempfile,lockfile) == 0)
			return(0);
		if (!exists(lockfile))
			continue;
		omtime = Statbuf.st_mtime;
		if ((fd = open(lockfile,0)) < 0)
			continue;
		ret = read(fd,(char *)&opid,sizeof(opid));
		(void) read(fd,ouuname,nodenamelength);
		(void) close(fd);
		if (ret != sizeof(pid) || ret != Statbuf.st_size) {
			(void) unlink(lockfile);
			continue;
		}
		/* check for pid */
		if (equal(ouuname, uuname))
		  if (kill((int) opid,0) == -1 && errno == ESRCH) {
			if (exists(lockfile) &&
				omtime == Statbuf.st_mtime) {
					(void) unlink(lockfile);
					continue;
			}
		  }
		if ((ltime = time((long *)0) - Statbuf.st_mtime) < 60L) {
			if (ltime >= 0 && ltime < 60)
				(void) sleep((unsigned) (60 - ltime));
			else
				(void) sleep(60);
		}
		continue;
	}
	return(-1);
}

int
unlockit(lockfile,pid,uuname)
register char *lockfile;
pid_t pid;
char *uuname;

{
	register int fd, n;
	pid_t opid;
	char ouuname[nodenamelength];
	int unlink(), open();

	if ((fd = open(lockfile,0)) < 0)
		return(-1);
	n = read(fd,(char *)&opid,sizeof(opid));
	(void) read(fd,ouuname,nodenamelength);
	(void) close(fd);
	if (n == sizeof(opid) && opid == pid && (equal(ouuname,uuname)))
		return(unlink(lockfile));
	else
		return(-1);
}


static int
onelock(pid,uuname,tempfile,lockfile)
pid_t pid;
char *uuname;
char *tempfile;
char *lockfile;
{
	int	fd;
	extern int errno;
	int	xmsg(), write(), unlink(), creat(), link();

	if ((fd = creat(tempfile,(mode_t)0444)) >= 0) {
		(void) write(fd,(char *)&pid,sizeof(pid));
		(void) write(fd,uuname,nodenamelength);
		(void) close(fd);
		if (link(tempfile,lockfile) < 0) {
			(void) unlink(tempfile);
			return(-1);
		}
		(void) unlink(tempfile);
		return(0);
	}
	if (errno == ENFILE) {
		(void) unlink(tempfile);
		return(-1);
	}
	if (errno != EACCES)
		return(xmsg(tempfile,"lockit"));
	return(-1);
}


mylock(lockfile,pid,uuname)
register char *lockfile;
pid_t pid;
char *uuname;

{
	register int fd, n;
	pid_t opid;
	char ouuname[nodenamelength];
	int	open();

	if ((fd = open(lockfile,0)) < 0)
		return(0);
	n = read(fd,(char *)&opid,sizeof(opid));
	(void) read(fd,ouuname,nodenamelength);
	(void) close(fd);
	if (n == sizeof(opid) && opid == pid && (equal(ouuname, uuname)))
		return(1);
	else
		return(0);
}
