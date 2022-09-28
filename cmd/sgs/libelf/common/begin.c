/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/begin.c	1.15"


#ifdef __STDC__
	#pragma weak	elf_begin = _elf_begin
#endif


#include "syn.h"
#include <ar.h>
#include "libelf.h"
#include "decl.h"
#include "member.h"
#include "error.h"
#include "foreign.h"


static Elf		*member _((int, Elf *, unsigned));
static Elf		*regular _((int, unsigned));


static const Elf	elf_init = { 0 };
static const char	armag[] = ARMAG;


Elf *
elf_begin(fd, cmd, ref)
	int		fd;
	Elf_Cmd		cmd;
	Elf		*ref;
{
	register Elf	*elf;
	register char	*base;
	int		(*const *f) _((Elf *));
	unsigned	flags = 0;

	if (_elf_work == EV_NONE)	/* version() not called yet */
	{
		_elf_err = ESEQ_VER;
		return 0;
	}
	switch (cmd)
	{
	default:
		_elf_err = EREQ_BEGIN;
		return 0;

	case ELF_C_NULL:
		return 0;

	case ELF_C_WRITE:
		if ((elf = (Elf *)malloc(sizeof(Elf))) == 0)
		{
			_elf_err = EMEM_ELF;
			return 0;
		}
		*elf = elf_init;
		elf->ed_fd = fd;
		elf->ed_activ = 1;
		elf->ed_myflags |= EDF_WRITE;
		return elf;

	case ELF_C_RDWR:
		flags = EDF_WRITE | EDF_READ;
		break;

	case ELF_C_READ:
		flags = EDF_READ;
		break;
	}

	/*	A null ref asks for a new file
	 *	Non-null ref bumps the activation count
	 *		or gets next archive member
	 */

	if (ref == 0)
	{
		if ((elf = regular(fd, flags)) == 0)
			return 0;
	}
	else
	{
		if ((ref->ed_myflags & flags) != flags)
		{
			_elf_err = EREQ_RDWR;
			return 0;
		}
		if (ref->ed_kind != ELF_K_AR)	/* new activation */
		{
			++ref->ed_activ;
			return ref;
		}
		if ((elf = member(fd, ref, flags)) == 0)
			return 0;
	}
	elf->ed_activ = 1;

	/*	ELF?
	 */

	base = elf->ed_ident;
	if (elf->ed_fsz >= EI_NIDENT
	&& _elf_vm(elf, (size_t)0, (size_t)EI_NIDENT) == OK_YES
	&& base[EI_MAG0] == ELFMAG0
	&& base[EI_MAG1] == ELFMAG1
	&& base[EI_MAG2] == ELFMAG2
	&& base[EI_MAG3] == ELFMAG3)
	{
		elf->ed_kind = ELF_K_ELF;
		elf->ed_class = base[EI_CLASS];
		elf->ed_encode = base[EI_DATA];
		if ((elf->ed_version = base[EI_VERSION]) == 0)
			elf->ed_version = 1;
		elf->ed_identsz = EI_NIDENT;
		return elf;
	}

	/*	Archive?
	 */

	if (elf->ed_fsz >= SARMAG
	&& _elf_vm(elf, (size_t)0, (size_t)SARMAG) == OK_YES
	&& memcmp(base, armag, SARMAG) == 0)
	{
		_elf_arinit(elf);
		elf->ed_kind = ELF_K_AR;
		elf->ed_identsz = SARMAG;
		return elf;
	}

	/*	Foreign file conversion?
	 */

	for (f = _elf_foreign; *f; ++f)
	{
		int	j = (**f)(elf);

		if (j > (int)ELF_K_NONE)
		{
			elf->ed_kind = (Elf_Kind)j;
			return elf;
		}
		if (j == (int)ELF_K_NONE)
			continue;
		(void)elf_end(elf);
		return 0;
	}

	/*	Return a few ident bytes, but not so many that
	 *	getident() must read a large file.  512 is arbitrary.
	 */

	elf->ed_kind = ELF_K_NONE;
	if ((elf->ed_identsz = elf->ed_fsz) > 512)
		elf->ed_identsz = 512;
	return elf;
}


static Elf *
member(fd, ref, flags)			/* initialize archive member */
	int		fd;
	register Elf	*ref;
	unsigned	flags;
{
	register Elf	*elf;
	Member		*mh;
	size_t		base;

	if (ref->ed_nextoff >= ref->ed_fsz)
		return 0;
	if (ref->ed_fd == -1)		/* disabled */
		fd = -1;
	if (flags & EDF_WRITE)
	{
		_elf_err = EREQ_ARRDWR;
		return 0;
	}
	if (ref->ed_fd != fd)
	{
		_elf_err = EREQ_ARMEMFD;
		return 0;
	}
	if (_elf_vm(ref, ref->ed_nextoff, sizeof(struct ar_hdr)) != OK_YES
	|| (mh = _elf_armem(ref, ref->ed_ident + ref->ed_nextoff, ref->ed_fsz)) == 0)
		return 0;
	base = ref->ed_nextoff + sizeof(struct ar_hdr);
	if (ref->ed_fsz - base < mh->m_hdr.ar_size)
	{
		_elf_err = EFMT_ARMEMSZ;
		return 0;
	}
	if ((elf = (Elf *)malloc(sizeof(Elf))) == 0)
	{
		_elf_err = EMEM_ELF;
		return 0;
	}
	*elf = elf_init;
	++ref->ed_activ;
	elf->ed_parent = ref;
	elf->ed_fd = fd;
	elf->ed_myflags |= flags;
	elf->ed_armem = mh;
	elf->ed_fsz = mh->m_hdr.ar_size;
	elf->ed_baseoff = ref->ed_baseoff + base;
	elf->ed_memoff = base;
	elf->ed_siboff = base + elf->ed_fsz + (elf->ed_fsz & 1);
	ref->ed_nextoff = elf->ed_siboff;
	elf->ed_image = ref->ed_image;
	elf->ed_imagesz = ref->ed_imagesz;
	elf->ed_vm = ref->ed_vm;
	elf->ed_vmsz = ref->ed_vmsz;
	elf->ed_ident = ref->ed_ident + base;

	/*	If this member is the archive string table,
	 *	we've already altered the bytes.
	 */

	if (ref->ed_arstroff == ref->ed_nextoff)
		elf->ed_status = ES_COOKED;
	return elf;
}


static Elf *
regular(fd, flags)			/* initialize regular file */
	int		fd;
	unsigned	flags;
{
	Elf		*elf;

	if ((elf = (Elf *)malloc(sizeof(Elf))) == 0)
	{
		_elf_err = EMEM_ELF;
		return 0;
	}
	*elf = elf_init;
	elf->ed_fd = fd;
	elf->ed_myflags |= flags;
	if (_elf_inmap(elf) != OK_YES)
	{
		free(elf);
		return 0;
	}
	return elf;
}
