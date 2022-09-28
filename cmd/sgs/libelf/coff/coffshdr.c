/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:coff/coffshdr.c	1.7"


#include "syn.h"
#include "filehdr.h"
#include "scnhdr.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"
#include "coff.h"


/* COFF section header conversion
 *	This file converts COFF (Common Object File Format) sections
 *	to their elf equivalents.  The header conversion is done fairly
 *	early to let later functions deal with sections through the
 *	normal elf library.  That simplifies relocations conversion,
 *	for example.
 *
 *	Existing coff sections keep their indexes in elf.  Thus, coff
 *	section 1 will be elf section 1.  Coff doesn't have section 0,
 *	while elf places 0 in the file but reserves it as empty.
 *
 *	Section name conversion is delayed until the symbol table
 *	and string table are started.
 *
 *	Even though it looks odd, elf sections preserve the coff
 *	section virtual addresses.  Relocation conversion requires
 *	this, and it actually makes sense for a.out files.  Reloc
 *	files could set the elf addresses back to zero after the
 *	relocations are converted.
 */


Okay
_elf_coffshdr(elf, info)
	Elf			*elf;
	Info			*info;
{
	FILHDR			*fh = info->i_filehdr;
	register SCNHDR		*ch;
	SCNHDR			*ech;
	register Elf32_Shdr	*sh;
	size_t			sz;
	size_t			j;

	if (fh->f_nscns == 0)
		return OK_YES;

	/*	Get section headers and ensure alignment.
	 *	Extra [0] entry makes indexing easier.
	 */

	j = sizeof(*fh) + fh->f_opthdr;
	sz = fh->f_nscns * sizeof(SCNHDR);
	if (_elf_vm(elf, j, sz) != OK_YES)
		return OK_NO;
	if ((info->i_coffscn = (SCNHDR *)malloc(sz + sizeof(SCNHDR))) == 0
	|| (sh = (Elf32_Shdr *)malloc((fh->f_nscns + 1) * sizeof(*sh))) == 0)
	{
		_elf_err = EMEM_CSHDR;
		return OK_NO;
	}
	elf->ed_shdr = sh;
	elf->ed_myflags |= EDF_SHALLOC;
	*sh = _elf_snode_init.sb_shdr;
	ch = info->i_coffscn;
	(void)memset(ch, 0, sizeof(*ch));
	++ch;
	(void)memcpy(ch, elf->ed_ident + j, sz);
	ech = ch + fh->f_nscns;
	for (++sh; ch < ech; (*info->i_coff->c_shdr)(elf, info, sh, ch),
		++ch, ++sh)
	{
		register long	flags;
		Elf32_Phdr	*ph;

		sh->sh_name = _elf_coffnewstr(info, ch->s_name, sizeof(ch->s_name));
		sh->sh_type = SHT_PROGBITS;
		sh->sh_flags = 0;
		sh->sh_addr = ch->s_vaddr;
		sh->sh_offset = ch->s_scnptr;
		sh->sh_size = ch->s_size;
		sh->sh_link = 0;
		sh->sh_info = 0;
		sh->sh_addralign = 4;
		if (ch->s_vaddr % 4 != 0)
			sh->sh_addralign = 1;
		sh->sh_entsize = 0;

		/*	Figure out section type.  Some coff bits are
		 *	mutually exclusive, others should be but might
		 *	not be.  Don't change the following order unless
		 *	it is proven wrong.
		 *
		 *	A coff scn offset of zero means no data.  Check
		 *	for this below, preventing a null coff section
		 *	with non-zero size from being anything except
		 *	nobits.  Do NOT change zero sized sections to
		 *	nobits, because that makes .text, ... appear as
		 *	the wrong type.
		 */

		flags = ch->s_flags;
		if (flags & STYP_LIB)
		{
			sh->sh_type = SHT_SHLIB;
			if (sh->sh_offset == 0 && sh->sh_size != 0)
				sh->sh_type = SHT_NOBITS;
			if ((ph = info->i_phdr) == 0)
				continue;
			ph += info->i_phdruse++;
			ph->p_type = PT_SHLIB;
			ph->p_offset = ch->s_scnptr;
			ph->p_vaddr = ph->p_paddr = 0;
			ph->p_filesz = ch->s_size;
			ph->p_memsz = 0;
			ph->p_flags = 0;
			ph->p_align = 0;
			continue;
		}
		if (flags & STYP_INFO)
		{
			if (sh->sh_offset == 0 && sh->sh_size != 0)
				sh->sh_type = SHT_NOBITS;
			continue;
		}
		if (flags & STYP_BSS)
		{
			sh->sh_type = SHT_NOBITS;
			sh->sh_flags |= SHF_ALLOC | SHF_WRITE;
		}
		if (flags & STYP_TEXT)
		{
			sh->sh_type = SHT_PROGBITS;
			sh->sh_flags |= SHF_ALLOC | SHF_EXECINSTR;
		}
		if (flags & STYP_DATA)
		{
			sh->sh_type = SHT_PROGBITS;
			sh->sh_flags |= SHF_ALLOC | SHF_WRITE;
		}
		if (flags & (STYP_COPY | STYP_OVER))
		{
			sh->sh_type = SHT_PROGBITS;
			sh->sh_flags &= ~SHF_ALLOC;
		}
		if (flags & (STYP_DSECT | STYP_NOLOAD))
		{
			sh->sh_type = SHT_NOBITS;
			sh->sh_flags &= ~SHF_ALLOC;
		}
		if (sh->sh_offset == 0 && sh->sh_size != 0)
			sh->sh_type = SHT_NOBITS;
		if ((ph = info->i_phdr) == 0)
			continue;
		if (((flags = sh->sh_flags) & SHF_ALLOC) == 0)
			continue;
		ph += info->i_phdruse++;
		ph->p_type = PT_LOAD;
		ph->p_offset = ch->s_scnptr;
		ph->p_vaddr = ch->s_vaddr;
		ph->p_paddr = ch->s_paddr;
		ph->p_memsz = ph->p_filesz = ch->s_size;
		if (sh->sh_type == SHT_NOBITS)
			ph->p_filesz = 0;
		ph->p_flags = PF_R;
		if (flags & SHF_EXECINSTR)
			ph->p_flags |= PF_X;
		if (flags & SHF_WRITE)
			ph->p_flags |= PF_W;
		ph->p_align = info->i_coff->c_align;
	}
	return _elf_cookscn(elf, fh->f_nscns + 1);
}
