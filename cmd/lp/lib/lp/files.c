/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/files.c	1.14.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "fcntl.h"
#include "string.h"
#include "errno.h"
#include "pwd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "stdlib.h"
#include "pwd.h"

#include "lp.h"

/**
 ** open_lpfile() - OPEN AND LOCK A FILE; REUSE STATIC BUFFER
 ** close_lpfile() - CLOSE FILE; RELEASE STATIC BUFFER
 **/

#define	MAX_NOFILES	100	/* current max. setting of NOFILES */

static struct buffers {
	FILE			*fp;
	char			*buffer;
}			buffers[MAX_NOFILES];

/*VARARGS2*/
FILE *
#if	defined(__STDC__)
open_lpfile (
	char *			path,
	char *			type,
	mode_t			mode
)
#else
open_lpfile (path, type, mode)
	char			*path,
				*type;
	mode_t			mode;
#endif
{
	struct flock		l;

	FILE			*fp;

	int			fd,
				oflag,
				create;

	register struct buffers	*bp;


	if (!path || !type) {
		errno = EINVAL;
		return (0);
	}

#define plus (type[1] == '+')
	switch (type[0]) {
	case 'w':
		oflag = (plus? O_RDWR : O_WRONLY) | O_TRUNC;
		create = 1;
		break;
	case 'a':
		oflag = (plus? O_RDWR : O_WRONLY) | O_APPEND;
		create = 1;
		break;
	case 'r':
		oflag = plus? O_RDWR : O_RDONLY;
		create = 0;
		break;
	default:
		return (0);
	}
	if ((fd = Open(path, oflag, mode)) == -1)
		if (errno == ENOENT && create) {
			int			old_umask = umask(0);
			int			save_errno;

			if ((fd = Open(path, oflag|O_CREAT, mode)) != -1)
				chown_lppath (path);
			save_errno = errno;
			if (old_umask)
				umask (old_umask);
			errno = save_errno;
		}

	if (fd == -1) switch (errno) {
	case ENOTDIR:
		errno = EACCES;
		/*FALLTHROUGH*/
	default:
		return (0);
	}

	l.l_type = (oflag & (O_WRONLY|O_RDWR)? F_WRLCK : F_RDLCK);
	l.l_whence = 1;
	l.l_start = 0;
	l.l_len = 0;
	if (Fcntl(fd, F_SETLK, &l) == -1) {
		/*
		 * Early UNIX op. sys. have wrong errno.
		 */
		if (errno == EACCES)
			errno = EAGAIN;
		Close (fd);
		return (0);
	}

	if (!(fp = fdopen(fd, type)))
		Close (fd);
	else {
		/*
		 * If MAX_NOFILES happens to be too low, no problem:
		 * the file will be buffered normally.
		 */
		for (bp = &buffers[0]; bp < &buffers[MAX_NOFILES]; bp++)
			if (!bp->fp) {
				if (!bp->buffer)
					bp->buffer = Malloc(BUFSIZ + 8);
				bp->fp = fp;
				setvbuf (fp, bp->buffer, _IOFBF, BUFSIZ + 8);
				break;
			}
	}

	return (fp);
}

int
#if	defined(__STDC__)
close_lpfile (
	FILE *			fp
)
#else
close_lpfile (fp)
	FILE			*fp;
#endif
{
	register struct buffers	*bp;


	for (bp = &buffers[0]; bp < &buffers[MAX_NOFILES]; bp++)
		if (bp->fp == fp) {
			bp->fp = 0;
			break;
		}

	/*
	 * Closing the file removes all locks.
	 */
	return (fclose(fp));
}

/**
 ** chown_lppath()
 **/

int
#if	defined(__STDC__)
chown_lppath (
	char *			path
)
#else
chown_lppath (path)
	char			*path;
#endif
{
	static uid_t		lp_uid;

	static gid_t		lp_gid;

	static int		gotids	= 0;

	struct passwd		*ppw;


	if (!gotids) {
		if (!(ppw = getpwnam(LPUSER)))
			ppw = getpwnam(ROOTUSER);
		endpwent ();
		if (!ppw)
			return (-1);
		lp_uid = ppw->pw_uid;
		lp_gid = ppw->pw_gid;
		gotids = 1;
	}
	return (Chown(path, lp_uid, lp_gid));
}

/**
 ** rmfile() - UNLINK FILE BUT NO COMPLAINT IF NOT THERE
 **/

int
#if	defined(__STDC__)
rmfile (
	char *			path
)
#else
rmfile (path)
	char			*path;
#endif
{
	return (Unlink(path) == 0 || errno == ENOENT);
}

/**
 ** loadline() - LOAD A ONE-LINE CHARACTER STRING FROM FILE
 **/

char *
#if	defined(__STDC__)
loadline (
	char *			path
)
#else
loadline (path)
	char			*path;
#endif
{
	register FILE		*fp;

	register char		*ret;

	register int		len;

	char			buf[BUFSIZ];


	if (!(fp = open_lpfile(path, "r", MODE_READ)))
		return (0);

	if (fgets(buf, BUFSIZ, fp)) {
		if ((len = strlen(buf)) && buf[len - 1] == '\n')
			buf[--len] = 0;
		if ((ret = Malloc(len + 1)))
			strcpy (ret, buf);
	} else {
		if (feof(fp))
			errno = 0;
		ret = 0;
	}

	close_lpfile (fp);
	return (ret);
}

/**
 ** loadstring() - LOAD A CHARACTER STRING FROM FILE
 **/

char *
#if	defined(__STDC__)
loadstring (
	char *			path
)
#else
loadstring (path)
	char			*path;
#endif
{
	register FILE		*fp;

	register char		*ret;

	register int		len;


	if (!(fp = open_lpfile(path, "r", MODE_READ)))
		return (0);

	if ((ret = sop_up_rest(fp, (char *)0))) {
		if ((len = strlen(ret)) && ret[len - 1] == '\n')
			ret[len - 1] = 0;
	} else
		if (feof(fp))
			errno = 0;

	close_lpfile (fp);
	return (ret);
}

/**
 ** dumpstring() - DUMP CHARACTER STRING TO FILE
 **/

int
#if	defined(__STDC__)
dumpstring (
	char *			path,
	char *			str
)
#else
dumpstring (path, str)
	char			*path,
				*str;
#endif
{
	register FILE		*fp;

	if (!str)
		return (rmfile(path));

	if (!(fp = open_lpfile(path, "w", MODE_READ)))
		return (-1);
	fprintf (fp, "%s\n", str);
	close_lpfile (fp);
	return (0);
}
