/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:coff/coffsym.c	1.7"


#include "syn.h"
#include "filehdr.h"
#include "scnhdr.h"
#include "syms.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"
#include "coff.h"


/* COFF symbol conversion
 *	COFF symbol table uses indexes 0..x, where symtab[0] is real.
 *	Elf reserves symtab[0].  The fix array is built to map
 *	coff input to elf output:  fix[coff_index] == elf_index.
 *	If the coff symbol appears in the output symbol table, its
 *	elf index will be nonzero.  Coff string table indexes are kept.
 *
 *	This conversion creates Elf32_Sym entries with its own internal
 *	structure.  It simply closes its eyes and hopes that matches
 *	the working version.  If a new version violates this contraint,
 *	the code will stop working.  MORAL: IF THE SYMBOL TABLE MUST
 *	CHANGE, MAKE A NEW SECTION TYPE.
 *
 *	Coff symbol values are "virtual addresses," even for relocatable
 *	files.  Moreover, reloc. files' sections have virtual addresses
 *	assigned.  Having the original symbol values makes relocation
 *	translation easier.  A second pass through the elf symbol table
 *	converts the values if necessary.  That also allows this code
 *	and the relocation code to avoid knowing whether the "output"
 *	file is ET_REL or ET_EXEC.
 *
 *	Coff common symbols look like undefined symbols with a non-zero
 *	"value," which gives the size (alignment assumed to be 4,
 *	unless size < 4).  This code passes on common symbol "values"
 *	to use during relocation conversion.  The location to change is
 *	the addend plus the value, even for common symbols.
 *
 *	_elf_coffstr assures the symbol table is in memory already.
 */


Okay
_elf_coffsym(elf, info)
	Elf			*elf;
	Info			*info;
{
	Dnode			*dsym;
	FILHDR			*fh = info->i_filehdr;
	size_t			*fix;
	register Elf32_Sym	*elfsym;
	char			*sp;	/* current coff symbol ptr */
	char			*ep;	/* end coff symbol ptr */
	size_t			top;	/* active elf entry at top */
	size_t			bottom;	/* empty elf entry at bottom */
	size_t			sz;

	if (fh->f_symptr <= 0 || fh->f_nsyms <= 0)
		return OK_YES;
	if ((info->i_symscn = elf_newscn(elf)) == 0
	|| (dsym = (Dnode *)elf_newdata(info->i_symscn)) == 0)
		return OK_NO;

	sp = (char *)elf->ed_ident + fh->f_symptr;
	ep = sp + fh->f_nsyms * SYMESZ;
	top = fh->f_nsyms + 1;

	/*	This allocates an extra entry in the fix array, but
	 *	that's ok.
	 */

	if ((info->i_fix = (size_t *)malloc(top * sizeof(*fix))) == 0
	|| (info->i_symtab = (Elf32_Sym *)malloc(top * sizeof(*elfsym))) == 0)
	{
		_elf_err = EMEM_CFIX;
		return OK_NO;
	}
	fix = info->i_fix;
	(void)memset(fix, 0, top * sizeof(*fix));

	/*	A coff symbol table is unordered.  Elf requires
	 *	locals to precede non-locals.  This single loop
	 *	through the coff table puts locals in the bottom
	 *	of the elf table and globals at the top.  After
	 *	coff processing is done, the top moves down and
	 *	final indexes are assigned.
	 */

	info->i_symuse = top;
	bottom = 1;
	elfsym = info->i_symtab;
	elfsym->st_name = 0;
	elfsym->st_value = 0;
	elfsym->st_size = 0;
	elfsym->st_info = ELF32_ST_INFO(STB_LOCAL, STT_NOTYPE);
	elfsym->st_other = 0;
	elfsym->st_shndx = SHN_UNDEF;
	while (sp < ep)
	{
		register Elf32_Sym	*new;
		SYMENT			old;
		int			bind;
		int			type = STT_NOTYPE;

		(void)memcpy(&old, sp, SYMESZ);
		switch (old.n_sclass)
		{
		case C_FILE:
			new = &elfsym[*fix++ = bottom++];
			new->st_value = 0;
			new->st_size = 0;
			new->st_info = ELF32_ST_INFO(STB_LOCAL, STT_FILE);
			new->st_other = 0;
			new->st_shndx = SHN_ABS;
			if (old.n_numaux != 1 || ep - sp < SYMESZ + AUXESZ)
				break;
			new->st_name = _elf_coffnewstr(info,
				((AUXENT *)(sp + SYMESZ))->x_file.x_fname,
				(size_t)FILNMLEN);
			break;

		case C_STAT:
		case C_USTATIC:
		case C_LABEL:
		case C_ULABEL:
			new = &elfsym[*fix++ = bottom++];
			bind = STB_LOCAL;
			goto common;

		case C_EXT:		/* globals */
		case C_EXTDEF:
			new = &elfsym[*fix++ = --top];
			bind = STB_GLOBAL;
		common:
			new->st_value = old.n_value;
			new->st_size = 0;
			new->st_other = 0;
			new->st_shndx = SHN_ABS;

			/*	SHN_UNDEF and N_UNDEF are both zero.
			 *	The following looks strange, but it would
			 *	work, even if SHN_UNDEF were != N_UNDEF
			 */

			if ((unsigned)old.n_scnum <= fh->f_nscns)
				new->st_shndx = old.n_scnum;
			if (old.n_scnum == N_UNDEF &&
				old.n_sclass != C_STAT)
			{
				new->st_shndx = SHN_UNDEF;
				if (old.n_value != 0)		/* common */
				{
					type = STT_OBJECT;
					new->st_shndx = SHN_COMMON;
				}
			}
			if (ISFCN(old.n_type))
			{
				AUXENT	a;

				type = STT_FUNC;
				if (old.n_numaux == 1
				&& ep - sp >= SYMESZ + AUXESZ)
				{
					(void)memcpy(&a, sp + SYMESZ, AUXESZ);
					new->st_size = a.x_sym.x_misc.x_fsize;
				}
			}
			new->st_info = ELF32_ST_INFO(bind, type);
			new->st_name = 0;
			if (old.n_zeroes != 0)
				new->st_name = _elf_coffnewstr(info,
							old.n_name, SYMNMLEN);
			else if ((size_t)old.n_offset < info->i_strcoff)
				new->st_name = old.n_offset;
			else
			{
				info->i_symscn->s_err = ECOFF_SYMSTR;
				dsym->db_myflags &= ~DBF_READY;
				free(info->i_symtab);
				info->i_symtab = 0;
				return OK_YES;
			}
			break;

		default:		/* drop */
			++fix;
			break;
		}
		sp += SYMESZ;
		if (old.n_numaux <= 0)
			continue;
		if ((sz = old.n_numaux * AUXESZ) < ep - sp)
		{
			sp += sz;
			fix += sz / AUXESZ;
		}
		else
			sp = ep;
	}

	/*	Eliminate dead space, correct fix[].
	 *	Bottom gives first available index after locals.
	 *	Top gives first active index for globals.
	 */

	sz = top - bottom;
	info->i_symuse -= sz;
	info->i_symlocals = bottom;
	if (sz != 0)
	{
		Elf32_Sym		*end = elfsym + fh->f_nsyms + 1;
		register Elf32_Sym	*src, *dst;

		fix = info->i_fix + fh->f_nsyms;
		while (fix-- > info->i_fix)
		{
			if (*fix >= bottom)
				*fix -= sz;
		}

		/*	This doesn't use memcpy because the source and
		 *	destination may overlap, giving undefined behavior.
		 */

		src = &elfsym[top];
		dst = &elfsym[bottom];
		while (src < end)
			*dst++ = *src++;
	}
	dsym->db_buf = (Elf_Void *)info->i_symtab;
	dsym->db_data.d_buf = (Elf_Void *)info->i_symtab;
	dsym->db_data.d_type = ELF_T_SYM;
	dsym->db_data.d_size = info->i_symuse * sizeof(Elf32_Sym);
	dsym->db_data.d_align = ELF32_FSZ_WORD;
	return OK_YES;
}
