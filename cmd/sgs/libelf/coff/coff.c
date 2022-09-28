/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:coff/coff.c	1.5"


#include "syn.h"
#include "filehdr.h"
#include "aouthdr.h"
#include "scnhdr.h"
#include "syms.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"
#include "foreign.h"
#include "coff.h"


/* COFF conversion
 *	This file converts a COFF (Common Object File Format)
 *	file to ELF.  The table allows the program to recognize
 *	one or more types of files.
 *
 *	This code makes some simplifying assumptions.  First, coff
 *	files can't be frozen.  Archives can't be frozen either, so
 *	all coff files can be cooked, regardless of source.
 *
 *	Second, the code makes several conservative guesses at size
 *	needs.  For example, the elf output will have no more symbol
 *	table entries than the input (plus 1), though it might have
 *	fewer.  The code allocates the maximum and uses what it needs.
 *	The output result is minimal, but internal storage use is not.
 *
 *	The justification for these assumptions is this.  People ought
 *	to convert a coff file once--permanently.  The library allows
 *	conversion on the fly, but that's obviously more expensive.
 *	If the policies used here make the conversion more expensive,
 *	that's ok.  Besides, these policies might even make the code
 *	run faster, not slower.
 */


static Okay	filehdr	_((Elf *, Info *));
static Okay	final	_((Elf *, Info *));
static Okay	opthdr	_((Elf *, Info *));


static const Info	info_init = { 0 };


int
_elf_coff(elf)
	register Elf	*elf;
{
	FILHDR		fh;
	const Coff	*coff;
	Info		info;
	int		kind;
	char		*hdr;

	if (elf->ed_fsz < sizeof(fh)
	|| _elf_vm(elf, (size_t)0, sizeof(fh)) != OK_YES)
		return (int)ELF_K_NONE;
	(void)memcpy(&fh, elf->ed_ident, sizeof(fh));
	for (coff = _elf_cofftab; ; ++coff)
	{
		if (coff->c_magic == 0)
			return (int)ELF_K_NONE;
		if (coff->c_magic == fh.f_magic)
			break;
	}
	info = info_init;
	info.i_coff = coff;
	info.i_filehdr = &fh;
	hdr = elf->ed_ident + sizeof(fh);

	/*	The order below is important.  coffstr() must be called
	 *	to initialize the string table.  Opthdr() sets the file
	 *	type for coffshdr(), ....
	 */

	elf->ed_status = ES_COOKED;
	if (filehdr(elf, &info) == OK_YES
	&& _elf_coffstr(elf, &info) == OK_YES
	&& opthdr(elf, &info) == OK_YES
	&& _elf_coffshdr(elf, &info) == OK_YES
	&& _elf_coffsym(elf, &info) == OK_YES
	&& _elf_coffscn(elf, &info) == OK_YES
	&& (*info.i_coff->c_opt)(elf, &info, hdr, fh.f_opthdr) == (int)OK_YES
	&& final(elf, &info) == OK_YES)
	{
		kind = ELF_K_COFF;
	}
	else
	{
		kind = -1;
		if (info.i_phdr)
			free(info.i_phdr);
		if (info.i_symtab)
			free(info.i_symtab);
		if (info.i_strtab)
			free(info.i_strtab);
	}
	if (info.i_fix)
		free(info.i_fix);
	if (info.i_coffscn)
		free(info.i_coffscn);
	return kind;
}


static Okay
filehdr(elf, info)
	register Elf		*elf;
	Info			*info;
{
	FILHDR			*fh = info->i_filehdr;
	register Elf32_Ehdr	*eh;

	if ((eh = (Elf32_Ehdr *)malloc(sizeof(*eh))) == 0)
	{
		_elf_err = EMEM_CEHDR;
		return OK_NO;
	}
	elf->ed_ehdr = eh;
	elf->ed_myflags |= EDF_EHALLOC;

	/*	fill ehdr.  Some values are set later
	 */

	*eh = _elf32_ehdr_init;
	eh->e_ident[EI_MAG0] = ELFMAG0;
	eh->e_ident[EI_MAG1] = ELFMAG1;
	eh->e_ident[EI_MAG2] = ELFMAG2;
	eh->e_ident[EI_MAG3] = ELFMAG3;
	eh->e_ident[EI_CLASS] = ELFCLASS32;
	eh->e_ident[EI_DATA] = info->i_coff->c_encode;
	elf->ed_encode = eh->e_ident[EI_DATA];
	eh->e_ident[EI_VERSION] = EV_CURRENT;
	eh->e_type = ET_REL;			/* can be overwritten */
	eh->e_machine = info->i_coff->c_machine;
	eh->e_version = EV_CURRENT;
	eh->e_shoff = sizeof(*fh) + fh->f_opthdr;
	eh->e_ehsize = sizeof(*fh);
	eh->e_shnum = fh->f_nscns;		/* overwritten */
	eh->e_shentsize = sizeof(SCNHDR);
	elf->ed_class = ELFCLASS32;
	elf->ed_version = EV_CURRENT;
	elf->ed_ehflags |= ELF_F_DIRTY;
	(*info->i_coff->c_flg)(elf, info, fh->f_flags);
	return OK_YES;
}


static Okay
final(elf, info)
	Elf			*elf;
	Info			*info;
{
	Elf_Data		*d;
	register Elf32_Shdr	*sh;
	Elf_Scn			*strscn;

	/*	finish the string and symbol tables
	 */

	if ((strscn = elf_newscn(elf)) == 0)
		return OK_NO;
	elf->ed_ehdr->e_shstrndx = elf_ndxscn(strscn);
	if (elf->ed_tlscn != 0)
		elf->ed_ehdr->e_shnum = elf->ed_tlscn->s_index + 1;
	if (info->i_symscn != 0)
	{
		if ((sh = elf32_getshdr(info->i_symscn)) == 0)
			return OK_NO;
		sh->sh_name = _elf_coffname(info, NM_SYM);
		sh->sh_type = SHT_SYMTAB;
		sh->sh_offset = info->i_filehdr->f_symptr;
		sh->sh_link = elf->ed_ehdr->e_shstrndx;
		sh->sh_info = info->i_symlocals;
		sh->sh_addralign = ELF32_FSZ_WORD;
		sh->sh_entsize = SYMESZ;
		sh->sh_size = info->i_filehdr->f_nsyms * SYMESZ;
	}

	/*	Add name to string table before setting final size
	 */

	if ((sh = elf32_getshdr(strscn)) == 0
	|| (d = elf_newdata(strscn)) == 0)
		return OK_NO;
	sh->sh_name = _elf_coffname(info, NM_STR);
	sh->sh_type = SHT_STRTAB;
	if (info->i_strcoff != 0)
	{
		sh->sh_offset = info->i_filehdr->f_symptr
				+ info->i_filehdr->f_nsyms * SYMESZ;
		sh->sh_size = info->i_strcoff;
	}
	((Dnode *)d)->db_buf = info->i_strtab;
	d->d_buf = info->i_strtab;
	d->d_size = info->i_struse;

	/*	Update the symbol table values for relocatable files
	 */

	if (elf->ed_ehdr->e_type == ET_REL && info->i_symtab)
	{
		register Elf32_Sym	*s, *e;
		register SCNHDR		*ch;
		size_t			nscn = info->i_filehdr->f_nscns;

		ch = info->i_coffscn;
		s = info->i_symtab;
		for (e = s + info->i_symuse; s < e; ++s)
		{
			if (s->st_shndx == SHN_COMMON)
			{
				if ((s->st_size = s->st_value) < 4)
					s->st_value = 2;
				else
					s->st_value = 4;
				continue;
			}
			if (s->st_shndx > nscn)
				continue;
			s->st_value -= (Elf32_Addr)ch[s->st_shndx].s_vaddr;
		}
	}

	/*	Update phdr table
	 *	If working version could be different from EV_CURRENT,
	 *	this would have to convert.
	 */

	if (info->i_phdr != 0)
	{
		elf->ed_phdr = (Elf_Void *)info->i_phdr;
		elf->ed_ehdr->e_phoff = sizeof(FILHDR);
		elf->ed_ehdr->e_phnum = info->i_phdruse;
		elf->ed_ehdr->e_phentsize = info->i_filehdr->f_opthdr;
		elf->ed_phdrsz = info->i_phdruse * sizeof(Elf32_Phdr);
		elf->ed_myflags |= EDF_PHALLOC;
	}
	if (elf->ed_ehdr->e_type == ET_EXEC)
		elf->ed_myflags |= EDF_COFFAOUT;
	return OK_YES;
}


static Okay
opthdr(elf, info)
	Elf	*elf;
	Info	*info;
{
	FILHDR		*fh = info->i_filehdr;
	struct aouthdr	ah;
	size_t		sz;
	char		*hdr;

	if ((sz = fh->f_opthdr) == 0)
		return OK_YES;
	if (_elf_vm(elf, sizeof(*fh), sz) != OK_YES)
		return OK_NO;
	hdr = (char *)elf->ed_ident + sizeof(*fh);
	if (sz < sizeof(ah))
		return OK_YES;
	(void)memcpy(&ah, hdr, sizeof(ah));
	switch (ah.magic)
	{
	case 0407:		/* a.out */
	case 0410:		/* normal a.out */
	case 0413:		/* paging a.out */
	case 0443:		/* static shared library */
		elf->ed_ehdr->e_entry = ah.entry;
		if ((fh->f_flags & F_EXEC) == 0)
			return OK_YES;
		elf->ed_ehdr->e_type = ET_EXEC;
		sz = fh->f_nscns * sizeof(Elf32_Phdr);
		if ((info->i_phdr = (Elf32_Phdr *)malloc(sz)) == 0)
		{
			_elf_err = EMEM_CPHDR;
			return OK_NO;
		}
		break;

	default:
		break;
	}
	return OK_YES;
}
