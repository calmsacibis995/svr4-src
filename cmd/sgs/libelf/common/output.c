/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/output.c	1.3"


#include "syn.h"

#ifdef MMAP_IS_AVAIL
#	include <sys/mman.h>
#endif

#include "libelf.h"
#include "decl.h"
#include "error.h"


/* File output
 *	These functions write output files.
 *	On SVR4 and newer systems use mmap(2).  On older systems (or on
 *	file systems that don't support mmap), use write(2).
 */


/*ARGSUSED*/
char *
_elf_outmap(fd, sz, pflag)
	int		fd;
	size_t		sz;
	unsigned int	*pflag;
{
	char		*p;

	*pflag = 0;
#ifdef MMAP_IS_AVAIL
	/*
	 *	Must be running svr4 or later to use mmap.
	 *	Set file length to allow mapping
	 */

	if (_elf_svr4()
	&& ftruncate(fd, (off_t)sz) == 0
	&& (p = mmap((char *)0, sz, PROT_READ+PROT_WRITE,
			MAP_SHARED, fd, (off_t)0)) != (char *)-1)
	{
		*pflag = 1;
		return p;
	}

	/*	If mmap fails, try malloc.  Some file systems don't mmap
	 */
#endif
	if ((p = malloc(sz)) == 0)
		_elf_err = EMEM_OUT;
	return p;
}


/*ARGSUSED*/
size_t
_elf_outsync(fd, p, sz, flag)
	int		fd;
	char		*p;
	size_t		sz;
	unsigned	flag;
{

#ifdef MMAP_IS_AVAIL
	if (flag != 0)
	{
		fd = msync(p, sz, MS_ASYNC);
		(void)munmap(p, sz);
		if (fd == 0)
			return sz;
		_elf_err = EIO_SYNC;
		return 0;
	}
#endif
	if (lseek(fd, 0L, 0) != 0
	|| write(fd, p, sz) != sz)
	{
		_elf_err = EIO_WRITE;
		sz = 0;
	}
	free(p);
	return sz;
}
