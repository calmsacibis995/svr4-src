/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/newphdr.c	1.3"


#ifdef __STDC__
	#pragma weak	elf32_newphdr = _elf32_newphdr
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"


Elf32_Phdr *
elf32_newphdr(elf, count)
	register Elf	*elf;
	size_t		count;
{
	Elf_Void	*ph;
	size_t		sz;

	if (elf == 0)
		return 0;
	if (elf->ed_class != ELFCLASS32)
	{
		_elf_err = EREQ_CLASS;
		return 0;
	}
	if (elf32_getehdr(elf) == 0)		/* this cooks if necessary */
	{
		_elf_err = ESEQ_EHDR;
		return 0;
	}

	/*	Free the existing header if appropriate.  This could reuse
	 *	existing space if big enough, but that's unlikely, benefit
	 *	would be negligible, and code would be more complicated.
	 */

	if (elf->ed_myflags & EDF_PHALLOC)
	{
		elf->ed_myflags &= ~EDF_PHALLOC;
		free(elf->ed_phdr);
	}

	/*	Delete the header if count is zero.
	 */

	if ((sz = count * _elf32_msize(ELF_T_PHDR, _elf_work)) == 0)
	{
	delete:
		elf->ed_phflags &= ~ELF_F_DIRTY;
		elf->ed_phdr = 0;
		elf->ed_ehdr->e_phnum = 0;
		elf->ed_ehdr->e_phentsize = 0;
		elf->ed_phdrsz = 0;
		return 0;
	}
	if ((ph = (Elf_Void *)malloc(sz)) == 0)
	{
		_elf_err = EMEM_PHDR;
		goto delete;
	}
	elf->ed_myflags |= EDF_PHALLOC;
	(void)memset(ph, 0, sz);
	elf->ed_phflags |= ELF_F_DIRTY;
	elf->ed_ehdr->e_phnum = (Elf32_Half)count;
	elf->ed_ehdr->e_phentsize = elf32_fsize(ELF_T_PHDR, 1, _elf_work);
	elf->ed_phdrsz = sz;
	return (Elf32_Phdr *)(elf->ed_phdr = ph);
}
