/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/update.c	1.20"


#ifdef __STDC__
	#pragma weak	elf_update = _elf_update
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"


/* Output file update
 *	These functions walk an Elf structure, update its information,
 *	and optionally write the output file.  Because the application
 *	may control of the output file layout, two upd_... routines
 *	exist.  They're similar but too different to merge cleanly.
 *
 *	The library defines a "dirty" bit to force parts of the file
 *	to be written on update.  These routines ignore the dirty bit
 *	and do everything.  A minimal update routine might be useful
 *	someday.
 */


static size_t	upd_lib	_((Elf *));
static size_t	upd_usr	_((Elf *));
static size_t	wrt	_((Elf *, size_t, unsigned));


off_t
elf_update(elf, cmd)
	Elf		*elf;
	Elf_Cmd		cmd;
{
	size_t		sz;
	unsigned	u;

	if (elf == 0)
		return -1;
	switch (cmd)
	{
	default:
		_elf_err = EREQ_UPDATE;
		return -1;

	case ELF_C_WRITE:
		if (elf->ed_myflags & EDF_COFFAOUT)
		{
			_elf_err = EREQ_COFFAOUT;
			return -1;
		}
		if ((elf->ed_myflags & EDF_WRITE) == 0)
		{
			_elf_err = EREQ_UPDWRT;
			return -1;
		}
		/*FALLTHRU*/
	case ELF_C_NULL:
		break;
	}
	if (elf->ed_ehdr == 0)
	{
		_elf_err = ESEQ_EHDR;
		return -1;
	}
	if ((u = elf->ed_ehdr->e_version) > EV_CURRENT)
	{
		_elf_err = EREQ_VER;
		return -1;
	}
	if (u == EV_NONE)
		elf->ed_ehdr->e_version = EV_CURRENT;
	if ((u = elf->ed_ehdr->e_ident[EI_DATA]) == ELFDATANONE)
	{
		if (_elf_encode == ELFDATANONE)
		{
			_elf_err = EREQ_ENCODE;
			return -1;
		}
		elf->ed_ehdr->e_ident[EI_DATA] = (Byte)_elf_encode;
	}
	u = 1;
	if (elf->ed_uflags & ELF_F_LAYOUT)
	{
		sz = upd_usr(elf);
		u = 0;
	}
	else
		sz = upd_lib(elf);
	if (sz != 0 && cmd == ELF_C_WRITE)
		sz = wrt(elf, sz, u);
	if (sz == 0)
		return -1;
	return sz;
}


static size_t
upd_lib(elf)
	Elf		*elf;
{
	size_t		hi;
	Elf_Scn		*s;
	register size_t	sz;
	Elf32_Ehdr	*eh = elf->ed_ehdr;
	unsigned	ver = eh->e_version;

	/*	Ehdr and Phdr table go first
	 */

	{
		register char	*p = (char *)eh->e_ident;

		p[EI_MAG0] = ELFMAG0;
		p[EI_MAG1] = ELFMAG1;
		p[EI_MAG2] = ELFMAG2;
		p[EI_MAG3] = ELFMAG3;
		p[EI_CLASS] = ELFCLASS32;
		p[EI_VERSION] = (Byte)ver;
		hi = elf32_fsize(ELF_T_EHDR, 1, ver);
		eh->e_ehsize = (Elf32_Half)hi;
		if (eh->e_phnum != 0)
		{
			eh->e_phentsize = elf32_fsize(ELF_T_PHDR, 1, ver);
			eh->e_phoff = hi;
			hi += eh->e_phentsize * eh->e_phnum;
		}
		else
		{
			eh->e_phoff = 0;
			eh->e_phentsize = 0;
		}
	}

	/*	Loop through sections, skipping index zero.
	 *	Compute section size before changing hi.
	 *	Allow null buffers for NOBITS.
	 */

	if ((s = elf->ed_hdscn) == 0)
		eh->e_shnum = 0;
	else
	{
		eh->e_shnum = 1;
		*s->s_shdr = _elf_snode_init.sb_shdr;
		s = s->s_next;
	}
	for (; s != 0; s = s->s_next)
	{
		register Dnode	*d;
		register size_t	fsz, j;

		++eh->e_shnum;
		if (s->s_shdr->sh_type == SHT_NULL)
		{
			*s->s_shdr = _elf_snode_init.sb_shdr;
			continue;
		}
		s->s_shdr->sh_addralign = 1;
		if ((sz = _elf32_entsz(s->s_shdr->sh_type, ver)) != 0)
			s->s_shdr->sh_entsize = sz;
		sz = 0;
		for (d = s->s_hdnode; d != 0; d = d->db_next)
		{
			if ((fsz = elf32_fsize(d->db_data.d_type, 1, ver)) == 0)
				return 0;
			j = _elf32_msize(d->db_data.d_type, ver);
			fsz *= d->db_data.d_size / j;
			d->db_osz = fsz;
			if ((j = d->db_data.d_align) > 1)
			{
				if (j > s->s_shdr->sh_addralign)
					s->s_shdr->sh_addralign = j;
				if (sz % j != 0)
					sz += j - sz % j;
			}
			d->db_data.d_off = sz;
			sz += fsz;
		}
		s->s_shdr->sh_size = sz;
		j = s->s_shdr->sh_addralign;
		if ((fsz = hi % j) != 0)
			hi += j - fsz;
		s->s_shdr->sh_offset = hi;
		if (s->s_shdr->sh_type != SHT_NOBITS)
			hi += sz;
	}

	/*	Shdr table last
	 */

	if (eh->e_shnum != 0)
	{
		if (hi % ELF32_FSZ_WORD != 0)
			hi += ELF32_FSZ_WORD - hi % ELF32_FSZ_WORD;
		eh->e_shoff = hi;
		eh->e_shentsize = elf32_fsize(ELF_T_SHDR, 1, ver);
		hi += eh->e_shentsize * eh->e_shnum;
	}
	else
	{
		eh->e_shoff = 0;
		eh->e_shentsize = 0;
	}
	return hi;
}


static size_t
upd_usr(elf)
	Elf		*elf;
{
	size_t		hi;
	Elf_Scn		*s;
	register size_t	sz;
	Elf32_Ehdr	*eh = elf->ed_ehdr;
	unsigned	ver = eh->e_version;

	/*	Ehdr and Phdr table go first
	 */

	{
		register char	*p = (char *)eh->e_ident;

		p[EI_MAG0] = ELFMAG0;
		p[EI_MAG1] = ELFMAG1;
		p[EI_MAG2] = ELFMAG2;
		p[EI_MAG3] = ELFMAG3;
		p[EI_CLASS] = ELFCLASS32;
		p[EI_VERSION] = (Byte)ver;
		hi = elf32_fsize(ELF_T_EHDR, 1, ver);
		eh->e_ehsize = (Elf32_Half)hi;

		/*	If phnum is zero, phoff "should" be zero too,
		 *	but the application is responsible for it.
		 *	Allow a non-zero value here and update the
		 *	hi water mark accordingly.
		 */

		if (eh->e_phnum != 0)
			eh->e_phentsize = elf32_fsize(ELF_T_PHDR, 1, ver);
		else
			eh->e_phentsize = 0;
		if ((sz = eh->e_phoff + eh->e_phentsize * eh->e_phnum) > hi)
			hi = sz;
	}

	/*	Loop through sections, skipping index zero.
	 *	Compute section size before changing hi.
	 *	Allow null buffers for NOBITS.
	 */

	if ((s = elf->ed_hdscn) == 0)
		eh->e_shnum = 0;
	else
	{
		eh->e_shnum = 1;
		*s->s_shdr = _elf_snode_init.sb_shdr;
		s = s->s_next;
	}
	for (; s != 0; s = s->s_next)
	{
		register Dnode	*d;
		register size_t	fsz, j;

		++eh->e_shnum;
		sz = 0;
		for (d = s->s_hdnode; d != 0; d = d->db_next)
		{
			if ((fsz = elf32_fsize(d->db_data.d_type, 1, ver)) == 0)
				return 0;
			j = _elf32_msize(d->db_data.d_type, ver);
			fsz *= d->db_data.d_size / j;
			d->db_osz = fsz;
			if ((s->s_shdr->sh_type != SHT_NOBITS) &&
			((j = d->db_data.d_off + d->db_osz) > sz))
				sz = j;
		}
		if (s->s_shdr->sh_size < sz)
		{
			_elf_err = EFMT_SCNSZ;
			return 0;
		}
		if (s->s_shdr->sh_type != SHT_NOBITS
		&& hi < s->s_shdr->sh_offset + s->s_shdr->sh_size)
			hi = s->s_shdr->sh_offset + s->s_shdr->sh_size;
	}

	/*	Shdr table last.  Comment above for phnum/phoff applies here.
	 */

	if (eh->e_shnum != 0)
		eh->e_shentsize = elf32_fsize(ELF_T_SHDR, 1, ver);
	else
		eh->e_shentsize = 0;
	if ((sz = eh->e_shoff + eh->e_shentsize * eh->e_shnum) > hi)
		hi = sz;
	return hi;
}


static size_t
wrt(elf, outsz, fill)
	Elf		*elf;
	size_t		outsz;
	unsigned	fill;
{
	Elf_Data	dst, src;
	unsigned	flag;
	size_t		hi, sz;
	char		*image;
	Elf_Scn		*s;
	Elf32_Ehdr	*eh = elf->ed_ehdr;
	unsigned	ver = eh->e_version;
	unsigned	encode = eh->e_ident[EI_DATA];

	/*	Two issues can cause trouble for the output file.
	 *	First, begin() with ELF_C_RDWR opens a file for both
	 *	read and write.  On the write update(), the library
	 *	has to read everything it needs before truncating
	 *	the file.  Second, using mmap for both read and write
	 *	is too tricky.  Consequently, the library disables mmap
	 *	on the read side.  Using mmap for the output saves swap
	 *	space, because that mapping is SHARED, not PRIVATE.
	 *
	 *	If the file is write-only, there can be nothing of
	 *	interest to bother with.
	 *
	 *	The following reads the entire file, which might be
	 *	more than necessary.  Better safe than sorry.
	 */

	if (elf->ed_myflags & EDF_READ
	&& _elf_vm(elf, (size_t)0, elf->ed_fsz) != OK_YES)
		return 0;

	if ((image = _elf_outmap(elf->ed_fd, outsz, &flag)) == 0)
		return 0;

	/*	If an error occurs below, a "dirty" bit may be cleared
	 *	improperly.  To save a second pass through the file,
	 *	this code sets the dirty bit on the elf descriptor
	 *	when an error happens, assuming that will "cover" any
	 *	accidents.
	 */

	/*	Hi is needed only when 'fill' is non-zero.
	 *	Fill is non-zero only when the library
	 *	calculates file/section/data buffer offsets.
	 *	The lib guarantees they increase monotonically.
	 *	That guarantees proper filling below.
	 */


	/*	Ehdr first
	 */

	src.d_buf = (Elf_Void *)eh;
	src.d_type = ELF_T_EHDR;
	src.d_size = sizeof(Elf32_Ehdr);
	src.d_version = EV_CURRENT;
	dst.d_buf = (Elf_Void *)image;
	dst.d_size = eh->e_ehsize;
	dst.d_version = ver;
	if (elf32_xlatetof(&dst, &src, encode) == 0)
		return 0;
	elf->ed_ehflags &= ~ELF_F_DIRTY;
	hi = eh->e_ehsize;

	/*	Phdr table if one exists
	 */

	if (eh->e_phnum != 0)
	{
		/*	Unlike other library data, phdr table is 
		 *	in the user version.  Change src buffer
		 *	version here, fix it after translation.
		 */

		src.d_buf = (Elf_Void *)elf->ed_phdr;
		src.d_type = ELF_T_PHDR;
		src.d_size = elf->ed_phdrsz;
		src.d_version = _elf_work;
		dst.d_buf = (Elf_Void *)(image + eh->e_phoff);
		dst.d_size = eh->e_phnum * eh->e_phentsize;
		hi = eh->e_phoff + dst.d_size;
		if (elf32_xlatetof(&dst, &src, encode) == 0)
			goto bad;
		elf->ed_phflags &= ~ELF_F_DIRTY;
		src.d_version = EV_CURRENT;
	}

	/*	Loop through sections
	 */

	for (s = elf->ed_hdscn; s != 0; s = s->s_next)
	{
		register Dnode	*d, *prevd;
		size_t		off = 0;
		char		*start = image + s->s_shdr->sh_offset;
		char		*here;

		/*	Just "clean" DIRTY flag for "empty" sections.  Even if
		 *	NOBITS needs padding, the next thing in the
		 *	file will provide it.  (And if this NOBITS is
		 *	the last thing in the file, no padding needed.)
		 */

		if (s->s_shdr->sh_type == SHT_NOBITS
		|| s->s_shdr->sh_type == SHT_NULL) {
			d = s->s_hdnode, prevd = 0;
			for (; d != 0; prevd = d, d = d->db_next)
				d->db_uflags &= ~ELF_F_DIRTY;
			continue;
		}
		if (fill && s->s_shdr->sh_offset > hi)
		{
			sz = s->s_shdr->sh_offset - hi;
			(void)memset(start - sz, _elf_byte, sz);
		}
		for (d = s->s_hdnode, prevd = 0; d != 0; prevd = d, d = d->db_next)
		{
			d->db_uflags &= ~ELF_F_DIRTY;
			here = start + d->db_data.d_off;
			if (fill && d->db_data.d_off > off)
			{
				sz = d->db_data.d_off - off;
				(void)memset(here - sz, _elf_byte, sz);
			}
			if ((d->db_myflags & DBF_READY) == 0
			&& elf_getdata(s, &prevd->db_data) != &d->db_data)
				goto bad;
			dst.d_buf = (Elf_Void *)here;
			dst.d_size = d->db_osz;
			if (elf32_xlatetof(&dst, &d->db_data, encode) == 0)
				goto bad;
			off = d->db_data.d_off + dst.d_size;
		}
		hi = s->s_shdr->sh_offset + s->s_shdr->sh_size;
	}

	/*	Shdr table last
	 */

	if (fill && eh->e_shoff > hi)
	{
		sz = eh->e_shoff - hi;
		(void)memset(image + hi, _elf_byte, sz);
	}

	src.d_type = ELF_T_SHDR;
	src.d_size = sizeof(Elf32_Shdr);
	dst.d_buf = (Elf_Void *)(image + eh->e_shoff);
	dst.d_size = eh->e_shentsize;
	for (s = elf->ed_hdscn; s != 0; s = s->s_next)
	{
		s->s_shflags &= ~ELF_F_DIRTY;
		s->s_uflags &= ~ELF_F_DIRTY;
		src.d_buf = (Elf_Void *)s->s_shdr;
		if (elf32_xlatetof(&dst, &src, encode) == 0)
			goto bad;
		dst.d_buf = (char *)dst.d_buf + eh->e_shentsize;
	}

	if ((hi = _elf_outsync(elf->ed_fd, image, outsz, flag)) != 0)
	{
		elf->ed_uflags &= ~ELF_F_DIRTY;
		return hi;
	}
bad:
	elf->ed_uflags |= ELF_F_DIRTY;
	return 0;
}
