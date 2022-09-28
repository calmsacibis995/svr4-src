/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lpNet/debug.c	1.1.2.1"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ulimit.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#ifndef	DEBUG
#define	DEBUG
#endif

#include	"debug.h"

int	_nestCount	= 0;
char	*_Unknownp	= "?";
char	*_Nullp		= "Null";
char	*_FnNamep	= "?";
char	*_FnNames [_MAX_NEST_COUNT];
FILE	*_DebugFilep	= NULL;

static	char	*_DebugPathp	= NULL;

static	int	_CheckDebugFile (void);

int
_SetDebugPath (char *pathp)
{
	if (!pathp)
	{
		errno = EINVAL;
		return	0;
	}
	if (_DebugPathp)
		free (_DebugPathp);

	_DebugPathp = (char *) malloc (strlen(pathp)+1);

	if (!_DebugPathp)
		return	0;

	(void)	strcpy (_DebugPathp, pathp);

	return	1;
}

int
_OpenDebugFile (char *pathp)
{
	int		i, fd, fd2, fdlim;
	char		path [32];

	_CheckDebugFile ();

	if (pathp)
		goto	OpenDebugFile;

	if (_DebugFilep)
		return	1;

	if (_DebugPathp)
		pathp = _DebugPathp;
	else
	{
		(void)	sprintf (path, "/tmp/%d.debug", getpid ());
		_SetDebugPath (path);
		pathp = _DebugPathp;
	}

OpenDebugFile:
	if ((fd = open (pathp, O_WRONLY|O_APPEND|O_CREAT)) < 0)
		/*
		**  'errno' is set.
		*/
		return	0;

	fdlim = ulimit (UL_GDESLIM);

	if (fd < (fdlim-1))
	{
		fd2 = -1;
		for (i=(fdlim-1); i > (fdlim/2); i--)
			if ((fd2 = fcntl (fd, F_DUPFD, i)) < 0)
				continue;
			else
			{
				(void)	close (fd);
				fd = fd2;
				break;
			}
		if (fd2 < 0)
			return	0;
	}
	if (_DebugFilep)
	{
		(void)	fclose (_DebugFilep);
		_DebugFilep = (FILE *) 0;
	}
	_DebugFilep = fdopen (fd, "a+");

	return	1;
}

static int
_CheckDebugFile (void)
{
	int		fd;
	struct stat	statbuf;

	if (!_DebugFilep)
		return	0;

	fd = fileno (_DebugFilep);
	if (fstat (fd, &statbuf) < 0)
	{
		(void)	fclose (_DebugFilep);
		_DebugFilep = (FILE *) 0;
		return	0;
	}
	return	1;
}
