/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/Syscalls.c	1.14.3.1"
/* LINTLIBRARY */

#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "errno.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"

/**
 ** Auto-restarting system calls:
 **/

int
#if	defined(__STDC__)
_Access (
	char *			s,
	int			i
)
#else
_Access (s, i)
	char *			s;
	int			i;
#endif
{
	register int		n;

	while ((n = access(s, i)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Chdir (
	char *			s
)
#else
_Chdir (s)
	char *			s;
#endif
{
	register int		n;

	while ((n = chdir(s)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Chmod (
	char *			s,
	int			i
)
#else
_Chmod (s, i)
	char *			s;
	int			i;
#endif
{
	register int		n;

	while ((n = chmod(s, i)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Chown (
	char *			s,
	int			i,
	int			j
)
#else
_Chown (s, i, j)
	char *			s;
	int			i;
	int			j;
#endif
{
	register int		n;

	while ((n = chown(s, i, j)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Close (
	int			i
)
#else
_Close (i)
	int			i;
#endif
{
	register int		n;

	while ((n = close(i)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Creat (
	char *			s,
	int			i
)
#else
_Creat (s, i)
	char *			s;
	int			i;
#endif
{
	register int		n;

	while ((n = creat(s, i)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Fcntl (
	int			i,
	int			j,
	struct flock *		k
)
#else
_Fcntl (i, j, k)
	int			i;
	int			j;
	struct flock *		k;
#endif
{
	register int		n;

	while ((n = fcntl(i, j, k)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Fstat (
	int			i,
	struct stat *		st
)
#else
_Fstat (i, st)
	int			i;
	struct stat *		st;
#endif
{
	register int		n;

	while ((n = fstat(i, st)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Link (
	char *			s1,
	char *			s2
)
#else
_Link (s1, s2)
	char *			s1;
	char *			s2;
#endif
{
	register int		n;

	while ((n = link(s1, s2)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Lstat (
	char *			s,
	struct stat *		st
)
#else
_Lstat (s, st)
	char *			s;
	struct stat *		st;
#endif
{
	register int		n;

	while ((n = lstat(s, st)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Mknod (
	char *			s,
	int			i,
	int			j
)
#else
_Mknod (s, i, j)
	char *			s;
	int			i;
	int			j;
#endif
{
	register int		n;

	while ((n = mknod(s, i, j)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Open (
	char *			s,
	int			i,
	int			j
)
#else
_Open (s, i, j)
	char *			s;
	int			i;
	int			j;
#endif
{
	register int		n;

	while ((n = open(s, i, j)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Read (
	int			i,
	char *			s,
	unsigned int		j
)
#else
_Read (i, s, j)
	int			i;
	char *			s;
	unsigned int		j;
#endif
{
	register int		n;

	while ((n = read(i, s, j)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Readlink (
	char *			s1,
	char *			s2,
	unsigned int		j
)
#else
_Readlink (s1, s2, j)
	char *			s1;
	char *			s2;
	unsigned int		j;
#endif
{
	register int		n;

	while ((n = readlink(s1, s2, j)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Rename (
	char *			s1,
	char *			s2
)
#else
_Rename (s1, s2)
	char *			s1;
	char *			s2;
#endif
{
	register int		n;

	while  ((n = rename(s1, s2)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Stat (
	char *			s,
	struct stat *		st
)
#else
_Stat (s, st)
	char *			s;
	struct stat *		st;
#endif
{
	register int		n;

	while ((n = stat(s, st)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Symlink (
	char *			s1,
	char *			s2
)
#else
_Symlink (s1, s2)
	char *			s1;
	char *			s2;
#endif
{
	register int		n;

	while ((n = symlink(s1, s2)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Unlink (
	char *			s
)
#else
_Unlink (s)
	char *			s;
#endif
{
	register int		n;

	while ((n = unlink(s)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Wait (
	int *			i
)
#else
_Wait (i)
	int *			i;
#endif
{
	register int		n;

	while ((n = wait(i)) == -1 && errno == EINTR)
		;
	return (n);
}

int
#if	defined(__STDC__)
_Write (
	int			i,
	char *			s,
	unsigned int		j
)
#else
_Write (i, s, j)
	int			i;
	char *			s;
	unsigned int		j;
#endif
{
	register int		n;

	while ((n = write(i, s, j)) == -1 && errno == EINTR)
		;
	return (n);
}

/**
 ** _Malloc()
 ** _Realloc()
 ** _Calloc()
 ** _Strdup()
 ** _Free()
 **/

#if	!defined(TRACE_MALLOC)

#if	defined(__STDC__)
void			(*lp_alloc_fail_handler)( void ) = 0;
#else
void			(*lp_alloc_fail_handler)() = 0;
#endif

#if	defined(__STDC__)
typedef void *alloc_type;
#else
typedef char *alloc_type;
#endif

alloc_type
#if	defined(__STDC__)
_Malloc (
	size_t			size,
	const char *		file,
	int			line
)
#else
_Malloc (size, file, line)
	size_t			size;
	char *			file;
	int			line;
#endif
{
	alloc_type		ret	= malloc(size);

	if (!ret) {
		if (lp_alloc_fail_handler)
			(*lp_alloc_fail_handler)();
		errno = ENOMEM;
	}
	return (ret);
}

alloc_type
#if	defined(__STDC__)
_Realloc (
	void *			ptr,
	size_t			size,
	const char *		file,
	int			line
)
#else
_Realloc (ptr, size, file, line)
	char *			ptr;
	size_t			size;
	char *			file;
	int			line;
#endif
{
	alloc_type		ret	= realloc(ptr, size);

	if (!ret) {
		if (lp_alloc_fail_handler)
			(*lp_alloc_fail_handler)();
		errno = ENOMEM;
	}
	return (ret);
}

alloc_type
#if	defined(__STDC__)
_Calloc (
	size_t			nelem,
	size_t			elsize,
	const char *		file,
	int			line
)
#else
_Calloc (nelem, elsize, file, line)
	size_t			nelem;
	size_t			elsize;
	char *			file;
	int			line;
#endif
{
	alloc_type		ret	= calloc(nelem, elsize);

	if (!ret) {
		if (lp_alloc_fail_handler)
			(*lp_alloc_fail_handler)();
		errno = ENOMEM;
	}
	return (ret);
}

char *
#if	defined(__STDC__)
_Strdup (
	const char *		s,
	const char *		file,
	int			line
)
#else
_Strdup (s, file, line)
	char *			s;
	char *			file;
	int			line;
#endif
{
	char *			ret	= strdup(s);

	if (!ret) {
		if (lp_alloc_fail_handler)
			(*lp_alloc_fail_handler)();
		errno = ENOMEM;
	}
	return (ret);
}

void
#if	defined(__STDC__)
_Free (
	void *			ptr,
	const char *		file,
	int			line
)
#else
_Free (ptr, file, line)
	char *			ptr;
	char *			file;
	int			line;
#endif
{
	free (ptr);
	return;
}

#else
# include "mdl.c"
#endif
