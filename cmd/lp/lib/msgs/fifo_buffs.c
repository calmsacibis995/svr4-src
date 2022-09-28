/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:lib/msgs/fifo_buffs.c	1.1.1.1"
/* LINTLIBRARY */


#include	<errno.h>
#include	<fcntl.h>
#include	<stdio.h>
#include	"lp.h"
#include	"msgs.h"
#include	"debug.h"

static	fifobuffer_t	**FifoBufferTable	= NULL;
static	int		FifoBufferTableSize	= 0;

/*
**	Local functions
*/
#ifdef	__STDC__
static	int		InitFifoBufferTable (void);
static	int		GrowFifoBufferTable (int);
static	fifobuffer_t	*NewFifoBuffer (int);
#else
static	int		InitFifoBufferTable (void);
static	int		GrowFifoBufferTable ();
static	fifobuffer_t	*NewFifoBuffer ();
#endif

#ifdef	__STDC__
int
ResetFifoBuffer (int fd)
#else
int
ResetFifoBuffer (fd)

int	fd;
#endif
{
	DEFINE_FNNAME (ResetFifoBuffer)
	ENTRYP
	TRACEd (fd)
	if (!FifoBufferTableSize)
	{
		if (InitFifoBufferTable () < 0)
		{
			TRACEP ("InitFifoBufferTable () failed.")
			EXITP
			return	-1;
		}
	}
	if (fd >= FifoBufferTableSize)
	{
		EXITP
		return	0;
	}
	if (FifoBufferTable [fd])
	{
		FifoBufferTable [fd]->full = 0;
		FifoBufferTable [fd]->psave =
		FifoBufferTable [fd]->psave_end = 
			FifoBufferTable [fd]->save;
	}
	EXITP
	return	0;
}
#ifdef	__STDC__
fifobuffer_t *
GetFifoBuffer (int fd)
#else
fifobuffer_t *
GetFifoBuffer (fd)

int	fd
#endif
{
	DEFINE_FNNAME (GetFifoBuffer)
	ENTRYP
	TRACEd (fd)
	if (fd < 0)
	{
		errno = EINVAL;
		TRACEd (errno)
		EXITP
		return	NULL;
	}
	if (fd >= FifoBufferTableSize)
	{
		if (GrowFifoBufferTable (fd) < 0)
		{
			/* errno is set by GrowFifoBufferTable */
			TRACEP ("GrowFifoBufferTable (fd) failed.")
			EXITP
			return	NULL;
		}
	}
	if (!FifoBufferTable [fd])
	{
		if (!NewFifoBuffer (fd))
		{
			/* errno is set by NewFifoBuffer */
			TRACEP ("NewFifoBuffer (fd) failed.")
			EXITP
			return	NULL;
		}
		FifoBufferTable [fd]->full = 0;
		FifoBufferTable [fd]->psave =
		FifoBufferTable [fd]->psave_end = 
			FifoBufferTable [fd]->save;
	}
	EXITP
	return	FifoBufferTable [fd];
}
#ifdef	__STDC__
static	int
InitFifoBufferTable (void)
#else
static	int
InitFifoBufferTable ()
#endif
{
	DEFINE_FNNAME (InitFifoBuffer)
	ENTRYP
	if (FifoBufferTableSize)
	{
		TRACEd (FifoBufferTableSize)
		EXITP
		return	0;
	}

	FifoBufferTable = (fifobuffer_t **)
		Calloc (100, sizeof (fifobuffer_t *));
	if (!FifoBufferTable)
	{
		TRACE (FifoBufferTable)
		EXITP
		return	-1;	/* ENOMEM is already set. */
	}

	FifoBufferTableSize = 100;

	EXITP
	return	0;
}

#ifdef	__STDC__
static int
GrowFifoBufferTable (int fd)
#else
static int
GrowFifoBufferTable (fd)

int	fd;
#endif
{
	fifobuffer_t	**newpp;

	DEFINE_FNNAME (GrowFifoBufferTable)
	ENTRYP
	TRACEd (fd)
	newpp = (fifobuffer_t **)
		Realloc ((void*)FifoBufferTable,
		(fd+10)*sizeof (fifobuffer_t *));
	if (!newpp)
	{
		TRACE (newpp)
		EXITP
		return	-1;	/* ENOMEM is already set. */
	}

	FifoBufferTableSize = fd+10;

	EXITP
	return	0;
}
#ifdef	__STDC__
static fifobuffer_t *
NewFifoBuffer (int fd)
#else
static fifobuffer_t *
NewFifoBuffer (fd)

int	fd;
#endif
{
	int	i;

	DEFINE_FNNAME (NewFifoBuffer)
	ENTRYP
	TRACEd (fd)
	for (i=0; i < FifoBufferTableSize; i++)
	{
		if (FifoBufferTable [i] &&
		    Fcntl (i, F_GETFL) < 0 &&
                    errno == EBADF)
		{
			FifoBufferTable [fd] = FifoBufferTable [i];
			FifoBufferTable [i] = NULL;
			EXITP
			return	FifoBufferTable [fd];
		}
	}
	FifoBufferTable [fd] = (fifobuffer_t *)
		Calloc (1, sizeof (fifobuffer_t));

	if (!FifoBufferTable [fd])
	{
		TRACE (FifoBufferTable [fd])
		EXITP
		return	NULL;	/* ENOMEM is already set. */
	}

	EXITP
	return	FifoBufferTable [fd];
}
