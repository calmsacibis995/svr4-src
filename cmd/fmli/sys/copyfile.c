/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)fmli:sys/copyfile.c	1.3"

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	"wish.h"
#include	<termio.h>
#define        _SYS_TERMIO_H
#include	"exception.h"

int	(*(ignoresigs()))();

/*
 * copy a file
 */
FILE *
cpfile(from, to)
char	*from;
char	*to;
{
	register int	c;
	register FILE	*src;
	register FILE	*dst;
	register int	(*fsave)();

	if ((src = fopen(from, "r")) == NULL)
		return NULL;
	if ((dst = fopen(to, "w+")) == NULL) {
		fclose(src);
		return NULL;
	}
	fsave = ignoresigs();
	while ((c = getc(src)) != EOF)
		putc(c, dst);
	if (ferror(src)) {
		fclose(src);
		fclose(dst);
		unlink(to);
		(void) restoresigs(fsave);
		return NULL;
	}
	fclose(src);
	(void) restoresigs(fsave);
	return dst;
}

copyfile(from, to)
char *from;
char *to;
{
	FILE *fp;

	if (fp = cpfile(from, to)) {
		fclose(fp);
		return(0);
	}
	return(-1);
}
/*
 * copy a file back to another file.  The destination file MUST exist
 */
bool
copyback(from, to, src)
char	*from;
char	*to;
FILE	*src;
{
	register int	c;
	register FILE	*dst;
	register int	(*fsave)();
	struct stat	s;

	if (stat(to, &s))
		return FALSE;
	fsave = ignoresigs();
	if (unlink(to) || (!from) || link(from, to)) {
		if ((dst = fopen(to, "w")) == NULL) {
			fclose(src);
			(void) restoresigs(fsave);
			return FALSE;
		}
		fseek(src, 0L, 0);
		while ((c = getc(src)) != EOF)
			putc(c, dst);
		fclose(dst);
		if (ferror(src)) {
			fclose(src);
			if (from)
				unlink(from);
			(void) restoresigs(fsave);
			return FALSE;
		}
	}
	fclose(src);
	if (from)
		unlink(from);
	chmod(to, s.st_mode);
	chown(to, s.st_uid, s.st_gid);
	restoresigs(fsave);
	return TRUE;
}

movefile(source, target)
register char *source, *target;
{
	char	*dirname();
	struct	stat s1;
	struct	utimbuf	{
		time_t	actime;
		time_t	modtime;
		};
	struct utimbuf times;

#ifdef _DEBUG
	_debug(stderr, "IN MOVEFILE(%s, %s)\n", source, target);
#endif
	if (link(source, target) < 0) {
		if (access(target, 00) != -1)
			return(-1);
		if (stat(source, &s1) < 0) 
			return(-1);
		if (copyfile(source, target) != 0) 
			return(-1);
		times.actime = s1.st_atime;
		times.modtime = s1.st_mtime;
		utime(target, &times);
		chmod(target, s1.st_mode);
		chown(target, geteuid(), getegid());
	}
	if (unlink(source) < 0) 
		return(-1);
	return(0);
}
