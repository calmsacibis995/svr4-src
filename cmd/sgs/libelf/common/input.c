/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/input.c	1.6"


#include "syn.h"

#ifdef MMAP_IS_AVAIL
#	include <sys/mman.h>
#endif

#include "libelf.h"
#include "decl.h"
#include "error.h"
#include "tune.h"


/* File input
 *	These functions read input files.
 *	On SVR4 and newer systems use mmap(2).  On older systems (or on
 *	file systems that don't support mmap, this code simulates mmap.
 *	When reading a file, enough memory is allocated to hold the file's
 *	image, and reads are delayed.  When another part of the library
 *	wants to use a part of the file, it "fetches" the needed regions.
 *
 *	An elf descriptor has a bit array to manage this.  Each bit
 *	represents one "page" of the file.  Pages are grouped into regions.
 *	The page size is tunable.  Its value should be at least one disk
 *	block and small enough to avoid superfluous traffic.
 *
 *	NBITS	The number of bits in an unsigned.  Each unsigned object
 *		holds a "REGION."  A byte must have at least 8 bits;
 *		it may have more, though the extra bits at the top of
 *		the unsigned will be unused.  Thus, for 9-bit bytes and
 *		36-bit words, 4 bits at the top will stay empty.
 *
 *	This mechanism gives significant performance gains for library
 *	handling (among other things), because programs typically don't
 *	need to look at entire libraries.  The fastest I/O is no I/O.
 */


#define NBITS		(8 * sizeof(unsigned))
#define	REGSZ		(NBITS * PGSZ)
#define PGNUM(off)	((off % REGSZ) / PGSZ)
#define REGNUM(off)	(off / REGSZ)


Okay
_elf_vm(elf, base, sz)
	Elf			*elf;
	size_t			base;
	size_t			sz;
{
	register unsigned	*hdreg, hdbit;
	unsigned		*tlreg, tlbit;
	size_t			tail;
	off_t			off;
	Elf_Void		*iop;

	/*	always validate region
	 */

	if (elf->ed_fsz <= base || elf->ed_fsz - base < sz)
	{
		_elf_err = EFMT_VM;
		return OK_NO;
	}
	if (elf->ed_vm == 0 || sz == 0)
		return OK_YES;

	/*	This uses arithmetic instead of masking because sizeof(unsigned)
	 *	might not be a power of 2.
	 *
	 *	Tail gives one beyond the last offset that must be retrieved,
	 *	NOT the last in the region.
	 */

	if (elf->ed_parent && elf->ed_parent->ed_fd == -1)
		elf->ed_fd = -1;
	base += elf->ed_baseoff;
	tail = base + sz + PGSZ - 1;
	off = base - base % PGSZ;
	hdbit = 1 << PGNUM(base);
	tlbit = 1 << PGNUM(tail);
	hdreg = &elf->ed_vm[REGNUM(base)];
	tlreg = &elf->ed_vm[REGNUM(tail)];
	sz = 0;
	while (hdreg != tlreg || hdbit != tlbit)
	{
		if (*hdreg & hdbit)
		{
			if (sz != 0)
			{
				iop = (Elf_Void *)(elf->ed_image + off);
				if (elf->ed_imagesz - off < sz)
					sz = elf->ed_imagesz - off;
				if (lseek(elf->ed_fd, off, 0) != off
				|| read(elf->ed_fd, iop, sz) != sz)
				{
					_elf_err = EIO_VM;
					return OK_NO;
				}
				off += sz;
				sz = 0;
			}
			off += PGSZ;
		}
		else
		{
			if (elf->ed_fd < 0)
			{
				_elf_err = EREQ_NOFD;
				return OK_NO;
			}
			sz += PGSZ;
			*hdreg |= hdbit;
		}
		if (hdbit == ((unsigned)1 << (NBITS - 1)))
		{
			hdbit = 1;
			++hdreg;
		}
		else
			hdbit <<= 1;
	}
	if (sz != 0)
	{
		if (elf->ed_fd < 0)
		{
			_elf_err = EREQ_NOFD;
			return OK_NO;
		}
		iop = (Elf_Void *)(elf->ed_image + off);
		if (elf->ed_imagesz - off < sz)
			sz = elf->ed_imagesz - off;
		if (lseek(elf->ed_fd, off, 0) != off
		|| read(elf->ed_fd, iop, sz) != sz)
		{
			_elf_err = EIO_VM;
			return OK_NO;
		}
	}
	return OK_YES;
}


Okay
_elf_inmap(elf)
	register Elf	*elf;
{
	int		fd = elf->ed_fd;
	register size_t	sz;

	{
		register off_t	off = lseek(fd, (off_t)0, 2);

		if (off == 0)
			return OK_YES;
		if (off == -1)
		{
			_elf_err = EIO_FSZ;
			return OK_NO;
		}
		if ((sz = (size_t)off) != off)
		{
			_elf_err = EIO_FBIG;
			return OK_NO;
		}
	}
#ifdef MMAP_IS_AVAIL
	/*	If the file is mapped, elf->ed_vm will stay null
	 *	and elf->ed_image will need to be unmapped someday.
	 *	If the file is read, elf->ed_vm and the file image
	 *	are allocated together; free() elf->ed_vm.
	 *
	 *	If the file can be written, disallow mmap.
	 *	Otherwise, the input mapping and the output mapping
	 *	can collide.  Moreover, elf_update will truncate
	 *	the file, possibly invalidating the input mapping.
	 *	Disallowing input mmap forces the library to malloc
	 *	and read the space, which will make output mmap safe.
	 *	Using mmap for output reduces the swap space needed
	 *	for the process, so that is given preference.
	 */

	{
		register char	*p;

		if (_elf_svr4()
		&& (elf->ed_myflags & EDF_WRITE) == 0
		&& (p = mmap((char *)0, sz, PROT_READ+PROT_WRITE,
			MAP_PRIVATE, fd, (off_t)0)) != (char *)-1)
		{
			elf->ed_image = elf->ed_ident = p;
			elf->ed_imagesz = elf->ed_fsz = elf->ed_identsz = sz;
			return OK_YES;
		}
	}

	/*	If mmap fails, try read.  Some file systems don't mmap
	 */
#endif
	{
		register size_t	vmsz = sizeof(unsigned) * (REGNUM(sz) + 1);

		if (vmsz % sizeof(Elf32) != 0)
			vmsz += sizeof(Elf32) - vmsz % sizeof(Elf32);
		if ((elf->ed_vm = (unsigned *)malloc(vmsz + sz)) == 0)
		{
			_elf_err = EMEM_VM;
			return OK_NO;
		}
		(void)memset(elf->ed_vm, 0, vmsz);
		elf->ed_vmsz = vmsz / sizeof(unsigned);
		elf->ed_image = elf->ed_ident = (char *)elf->ed_vm + vmsz;
		elf->ed_imagesz = elf->ed_fsz = elf->ed_identsz = sz;
	}
	return _elf_vm(elf, (size_t)0, (size_t)1);
}


void
_elf_unmap(p, sz)
	char	*p;
	size_t	sz;
{
	if (p == 0 || sz == 0)
		return;
#ifdef MMAP_IS_AVAIL
	(void)munmap(p, sz);
#endif
	return;
}
