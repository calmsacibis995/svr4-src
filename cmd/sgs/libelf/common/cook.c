/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/cook.c	1.11"


#include "syn.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"


/* Cook the input file.
 *	These functions take the input file buffer and extract
 *	the Ehdr, Phdr table, and the Shdr table.  They keep track
 *	of the buffer status as "fresh," "cooked," or "frozen."
 *
 *	fresh	The file buffer is in its original state and
 *		nothing has yet referenced it.
 *
 *	cooked	The application asked for translated data first
 *		and caused the library to return a pointer into
 *		the file buffer.  After this happens, all "raw"
 *		operations must go back to the disk.
 *
 *	frozen	The application first did a "raw" operation that
 *		prohibits reusing the file buffer.  This effectively
 *		freezes the buffer, and all "normal" operations must
 *		duplicate their data.
 *
 *	For archive handling, these functions conspire to align the
 *	file buffer to the host memory format.  Archive members
 *	are guaranteed only even byte alignment, but the file uses
 *	objects at least 4 bytes long.  If an archive member is about
 *	to be cooked and is not aligned in memory, these functions
 *	"slide" the buffer up into the archive member header.
 *	This sliding never occurs for frozen files.
 *
 *	Some processors might not need sliding at all, if they have
 *	no alignment constraints on memory references.  This code
 *	ignores that possibility for two reasons.  First, even machines
 *	that have no constraints usually handle aligned objects faster
 *	than unaligned.  Forcing alignment here probably leads to better
 *	performance.  Second, there's no way to test at run time whether
 *	alignment is required or not.  The safe thing is to align in
 *	all cases.
 *
 *	This sliding relies on the archive header being disposable.
 *	Only archive members that are object files ever slide.
 *	They're also the only ones that ever need to.  Archives never
 *	freeze to make headers disposable.  Any program peculiar enough
 *	to want a frozen archive pays the penalty.
 *
 *	The library itself inspects the Ehdr and the Shdr table
 *	from the file.  Consequently, it converts the file's data
 *	to EV_CURRENT version, not the working version.  This is
 *	transparent to the user.  The library never looks at the
 *	Phdr table; so that's kept in the working version.
 */


static int	ehdr	_((Elf *, int));
static int	phdr	_((Elf *, int));
static int	shdr	_((Elf *, int));
static int	slide	_((Elf *));


Okay
_elf_cook(elf)
	register Elf	*elf;
{
	register int	inplace = 1;

	if (elf->ed_status == ES_COOKED
	|| (elf->ed_myflags & EDF_READ) == 0
	|| elf->ed_kind != ELF_K_ELF)
		return OK_YES;

	/*	Here's where the unaligned archive member gets fixed.
	 */

	if (elf->ed_status == ES_FRESH && slide(elf) != 0)
		return OK_NO;
	if (elf->ed_status == ES_FROZEN)
		inplace = 0;
	if (ehdr(elf, inplace) != 0)
		return OK_NO;
	if (phdr(elf, inplace) != 0)
		goto xehdr;
	if (shdr(elf, inplace) != 0)
		goto xphdr;
	return OK_YES;

xphdr:
	if (elf->ed_myflags & EDF_PHALLOC)
	{
		elf->ed_myflags &= ~EDF_PHALLOC;
		free(elf->ed_phdr);
	}
	elf->ed_phdr = 0;
xehdr:
	if (elf->ed_myflags & EDF_EHALLOC)
	{
		elf->ed_myflags &= ~EDF_EHALLOC;
		free(elf->ed_ehdr);
	}
	elf->ed_ehdr = 0;
	return OK_NO;
}


Okay
_elf_cookscn(elf, cnt)
	register Elf		*elf;
	size_t			cnt;
{
	register Elf_Scn	*s;
	Elf_Scn			*end;
	register Elf32_Shdr	*sh;	

	if (cnt == 0)
		return OK_YES;
	if ((s = (Elf_Scn *)malloc(cnt * sizeof(*s))) == 0)
	{
		_elf_err = EMEM_SCN;
		return OK_NO;
	}
	end = s + cnt;
	elf->ed_hdscn = s;
	sh = elf->ed_shdr;
	do
	{
		register Dnode	*d = &s->s_dnode;
		size_t		fsz, msz;

		*s = _elf_snode_init.sb_scn;
		s->s_elf = elf;
		s->s_shdr = sh;
		s->s_index = s - elf->ed_hdscn;
		s->s_hdnode = s->s_tlnode = d;

		/*	Prepare d_data for inspection, but don't actually
		 *	translate data until needed.  Leave the READY
		 *	flag off.  NOBITS sections see zero size.
		 */

		d->db_scn = s;
		d->db_off = sh->sh_offset;
		d->db_data.d_align = sh->sh_addralign;
		d->db_data.d_version = elf->ed_version;
		d->db_data.d_type = _elf32_mtype(sh->sh_type, _elf_work);
		fsz = elf32_fsize(d->db_data.d_type, 1, elf->ed_version);
		msz = _elf32_msize(d->db_data.d_type, elf->ed_version);
		d->db_data.d_size = (sh->sh_size / fsz) * msz;
		d->db_shsz = sh->sh_size;
		if (sh->sh_type != SHT_NOBITS)
			d->db_fsz = sh->sh_size;
		++sh;
		s->s_next = s + 1;
	} while (++s < end);
	elf->ed_tlscn = --s;
	s->s_next = 0;

	/*	Section index SHN_UNDEF (0) does not and cannot
	 *	have a data buffer.  Fix it here.  Also mark the
	 *	initial section as being allocated for the block
	 */

	s = elf->ed_hdscn;
	s->s_myflags = SF_ALLOC;
	s->s_hdnode = 0;
	s->s_tlnode = 0;
	return OK_YES;
}


Dnode *
_elf_dnode()
{
	register Dnode	*d;

	if ((d = (Dnode *)malloc(sizeof(*d))) == 0)
	{
		_elf_err = EMEM_DNODE;
		return 0;
	}
	*d = _elf_snode_init.sb_scn.s_dnode;
	d->db_myflags = DBF_ALLOC;
	return d;
}


Snode *
_elf_snode()
{
	register Snode	*s;

	if ((s = (Snode *)malloc(sizeof(*s))) == 0)
	{
		_elf_err = EMEM_SNODE;
		return 0;
	}
	*s = _elf_snode_init;
	s->sb_scn.s_myflags = SF_ALLOC;
	s->sb_scn.s_shdr = &s->sb_shdr;
	return s;
}


static int
ehdr(elf, inplace)
	register Elf	*elf;
	register int	inplace;
{
	register size_t	fsz;
	Elf_Data	dst, src;

	fsz = elf32_fsize(ELF_T_EHDR, 1, elf->ed_version);
	if (fsz > elf->ed_fsz)
	{
		_elf_err = EFMT_EHDRSZ;
		return -1;
	}
	if (inplace && fsz >= sizeof(Elf32_Ehdr))
	{
		elf->ed_ehdr = (Elf32_Ehdr *)elf->ed_ident;
		elf->ed_status = ES_COOKED;
	}
	else
	{
		elf->ed_ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
		if (elf->ed_ehdr == 0)
		{
			_elf_err = EMEM_EHDR;
			return -1;
		}
		elf->ed_myflags |= EDF_EHALLOC;
	}

	/*	Memory size >= fsz, because otherwise the memory version
	 *	loses information and cannot accurately implement the
	 *	file.
	 */

	src.d_buf = (Elf_Void *)elf->ed_ident;
	src.d_type = ELF_T_EHDR;
	src.d_size = fsz;
	src.d_version = elf->ed_version;
	dst.d_buf = (Elf_Void *)elf->ed_ehdr;
	dst.d_size = sizeof(Elf32_Ehdr);
	dst.d_version = EV_CURRENT;
	if (_elf_vm(elf, (size_t)0, fsz) != OK_YES
	|| elf32_xlatetom(&dst, &src, elf->ed_encode) == 0)
	{
	bad:
		if (elf->ed_myflags & EDF_EHALLOC)
		{
			elf->ed_myflags &= ~EDF_EHALLOC;
			free(elf->ed_ehdr);
		}
		elf->ed_ehdr = 0;
		return -1;
	}
	if (elf->ed_ehdr->e_version != elf->ed_version)
	{
		_elf_err = EFMT_VER2;
		goto bad;
	}
	return 0;
}


static int
phdr(elf, inplace)
	register Elf		*elf;
	register int		inplace;
{
	register size_t		fsz, msz;
	Elf_Data		dst, src;
	register Elf32_Ehdr	*eh = elf->ed_ehdr;	/* must be present */

	if (eh->e_phnum == 0)
		return 0;
	fsz = elf32_fsize(ELF_T_PHDR, 1, elf->ed_version);
	if (eh->e_phentsize != fsz)
	{
		_elf_err = EFMT_PHDRSZ;
		return -1;
	}
	fsz *= eh->e_phnum;
	msz = _elf32_msize(ELF_T_PHDR, _elf_work) * eh->e_phnum;
	if (eh->e_phoff <= 0
	|| elf->ed_fsz <= eh->e_phoff
	|| elf->ed_fsz - eh->e_phoff < fsz)
	{
		_elf_err = EFMT_PHTAB;
		return -1;
	}
	if (inplace && fsz >= msz && eh->e_phoff % sizeof(Elf32) == 0)
	{
		elf->ed_phdr = (Elf_Void *)(elf->ed_ident + eh->e_phoff);
		elf->ed_status = ES_COOKED;
	}
	else
	{
		if ((elf->ed_phdr = (Elf_Void *)malloc(msz)) == 0)
		{
			_elf_err = EMEM_PHDR;
			return -1;
		}
		elf->ed_myflags |= EDF_PHALLOC;
	}
	src.d_buf = (Elf_Void *)(elf->ed_ident + eh->e_phoff);
	src.d_type = ELF_T_PHDR;
	src.d_size = fsz;
	src.d_version = elf->ed_version;
	dst.d_buf = elf->ed_phdr;
	dst.d_size = msz;
	dst.d_version = _elf_work;
	if (_elf_vm(elf, (size_t)eh->e_phoff, fsz) != OK_YES
	|| elf32_xlatetom(&dst, &src, elf->ed_encode) == 0)
	{
		if (elf->ed_myflags & EDF_PHALLOC)
		{
			elf->ed_myflags &= ~EDF_PHALLOC;
			free(elf->ed_phdr);
		}
		elf->ed_phdr = 0;
		return -1;
	}
	elf->ed_phdrsz = msz;
	return 0;
}


static int
shdr(elf, inplace)
	register Elf		*elf;
	int			inplace;
{
	register size_t		fsz, msz;
	Elf_Data		dst, src;
	register Elf32_Ehdr	*eh = elf->ed_ehdr;	/* must be present */

	if (eh->e_shnum == 0)
		return 0;
	fsz = elf32_fsize(ELF_T_SHDR, 1, elf->ed_version);
	if (eh->e_shentsize != fsz)
	{
		_elf_err = EFMT_SHDRSZ;
		return -1;
	}
	fsz *= eh->e_shnum;
	msz = eh->e_shnum * sizeof(Elf32_Shdr);
	if (eh->e_shoff <= 0
	|| elf->ed_fsz <= eh->e_shoff
	|| elf->ed_fsz - eh->e_shoff < fsz)
	{
		_elf_err = EFMT_SHTAB;
		return -1;
	}
	if (inplace && fsz >= msz && eh->e_shoff % sizeof(Elf32) == 0)
	{
		elf->ed_shdr = (Elf32_Shdr *)(elf->ed_ident + eh->e_shoff);
		elf->ed_status = ES_COOKED;
	}
	else
	{
		if ((elf->ed_shdr = (Elf32_Shdr *)malloc(msz)) == 0)
		{
			_elf_err = EMEM_SHDR;
			return -1;
		}
		elf->ed_myflags |= EDF_SHALLOC;
	}
	src.d_buf = (Elf_Void *)(elf->ed_ident + eh->e_shoff);
	src.d_type = ELF_T_SHDR;
	src.d_size = fsz;
	src.d_version = elf->ed_version;
	dst.d_buf = (Elf_Void *)elf->ed_shdr;
	dst.d_size = msz;
	dst.d_version = EV_CURRENT;
	if (_elf_vm(elf, (size_t)eh->e_shoff, fsz) != OK_YES
	|| elf32_xlatetom(&dst, &src, elf->ed_encode) == 0
	|| _elf_cookscn(elf, eh->e_shnum) != OK_YES)
	{
		if (elf->ed_myflags & EDF_SHALLOC)
		{
			elf->ed_myflags &= ~EDF_SHALLOC;
			free(elf->ed_shdr);
		}
		elf->ed_shdr = 0;
		return -1;
	}
	return 0;
}


static int
slide(elf)
	Elf	*elf;
{
	Elf		*par = elf->ed_parent;
	size_t		sz;
	register char	*dst;
	register char	*src = elf->ed_ident;
	register char	*end = src + elf->ed_fsz;

	if (par == 0 || par->ed_kind != ELF_K_AR)
		return 0;

	/*	This code relies on other code to ensure
	 *	the ar_hdr is big enough to move into.
	 */

	if ((sz = (size_t)(src - (char *)elf->ed_image) % sizeof(Elf32)) == 0)
		return 0;
	dst = src - sz;
	elf->ed_ident = dst;
	elf->ed_memoff -= sz;
	if (_elf_vm(par, elf->ed_memoff, sz + elf->ed_fsz) != OK_YES)
		return -1;

	/*	The useless "loop:" stops compiler warning.
	 *	This code requires ed_fsz to be nonzero, which
	 *	is true for elf files.
	 */

	switch (elf->ed_fsz & 0x7)
	{
loop:		do
		{
	case 0:		*dst++ = *src++;	/*FALLTHRU*/
	case 7:		*dst++ = *src++;	/*FALLTHRU*/
	case 6:		*dst++ = *src++;	/*FALLTHRU*/
	case 5:		*dst++ = *src++;	/*FALLTHRU*/
	case 4:		*dst++ = *src++;	/*FALLTHRU*/
	case 3:		*dst++ = *src++;	/*FALLTHRU*/
	case 2:		*dst++ = *src++;	/*FALLTHRU*/
	case 1:		*dst++ = *src++;
		} while (src < end);
	}
	return 0;
}
