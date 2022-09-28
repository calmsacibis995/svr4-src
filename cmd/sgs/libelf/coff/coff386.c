/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:coff/coff386.c	1.7"


#include "syn.h"
#include "filehdr.h"
#include "scnhdr.h"
#include "reloc.h"
#include "libelf.h"
#include "sys/elf_386.h"
#include "decl.h"
#include "error.h"
#include "coff.h"
#include "cofftab.h"


#define BIT32	0x80000000L
#define LO31	0x7fffffffL


static void	do4le	_((Byte *, Elf32_Addr, Elf32_Addr));


/*ARGSUSED*/
void
_elf_coff386_flg(elf, info, cflags)
	Elf		*elf;
	Info		*info;
	unsigned	cflags;
{
	return;
}


/*ARGSUSED*/
int
_elf_coff386_opt(elf, info, hdr, sz)
	Elf		*elf;
	Info		*info;
	char		*hdr;
	size_t		sz;
{

	return (int)OK_YES;
}


/* Convert relocation
 *	This changes the relocation entries from coff to elf.
 *	On the 386, a relocation entry points at a place holding the
 *	the sum of the symbol's value and the addend.  ELF puts only
 *	the addend there.  Consequently, this code subtracts out the
 *	symbol value.
 *
 *	This code relies on the elf symbol table values still holding
 *	virtual addresses, rather than section offsets.  It also needs
 *	the elf section header to hold the original coff section vaddr.
 */

/*ARGSUSED*/
int
_elf_coff386_rel(elf, info, rscn, dscn, crel)
	Elf			*elf;
	Info			*info;
	Elf_Scn			*rscn;		/* new relocation scn */
	Elf_Scn			*dscn;		/* relocated data scn */
	Elf_Data		*crel;		/* coff relocations */
{
	Dnode			*dst;
	Elf32_Word		dvaddr;
	Elf32_Word		dvend;
	Byte			*bytes;
	Elf32_Sym		*elfsym = info->i_symtab;
	size_t			*fix;
	size_t			nfix;
	register Elf32_Rel	*r;
	char			*p, *ep;
	size_t			sz;

	{
		Elf_Data	*d;
		Elf32_Shdr	*sh;

		if ((d = elf_getdata(dscn, (Elf_Data *)0)) == 0
		|| (sh = elf32_getshdr(dscn)) == 0
		|| (dst = (Dnode *)elf_newdata(rscn)) == 0)
			return (int)OK_NO;
		bytes = (Byte *)d->d_buf;
		dvaddr = sh->sh_addr;
		dvend = dvaddr + d->d_size - 4;
	}
	{
		sz = crel->d_size / RELSZ;
		dst->db_data.d_size = sz * sizeof(Elf32_Rel);

		/*	Although coff relocations are bigger than elf,
		 *	reusing the file space is tricky because of
		 *	alignment issues.  Best to allocate new space.
		 */

		if ((dst->db_buf = malloc(dst->db_data.d_size)) == 0)
		{
			_elf_err = EMEM_CREL;
			return (int)OK_NO;
		}
	}
	fix = info->i_fix;
	nfix = info->i_filehdr->f_nsyms;
	p = (char *)crel->d_buf;
	r = (Elf32_Rel *)dst->db_buf;
	for (ep = p + crel->d_size; p < ep; p += RELSZ, ++r)
	{
		RELOC		reloc;
		Elf32_Word	ndx;

		(void)memcpy(&reloc, p, RELSZ);
		ndx = 0;
		if ((unsigned long)reloc.r_symndx < nfix)
			ndx = fix[reloc.r_symndx];
		r->r_offset = reloc.r_vaddr - dvaddr;

		/*	Only errors break
		 */

		switch (reloc.r_type)
		{
		case R_ABS:
			r->r_info = ELF32_R_INFO(ndx, R_386_NONE);
			continue;

		case R_DIR32:
			r->r_info = ELF32_R_INFO(ndx, R_386_32);
			if (r->r_offset > dvend)
			{
				rscn->s_err = ECOFF_RELOFF;
				break;
			}
			do4le(bytes + r->r_offset, elfsym[ndx].st_value,
				(Elf32_Addr)0);
			continue;

		case R_PCRLONG:
			r->r_info = ELF32_R_INFO(ndx, R_386_PC32);
			if (r->r_offset > dvend)
			{
				rscn->s_err = ECOFF_RELOFF;
				break;
			}
			do4le(bytes + r->r_offset, elfsym[ndx].st_value,
				(Elf32_Addr)reloc.r_vaddr);
			continue;

		default:
			rscn->s_err = ECOFF_RELTYP;
			break;
		}
		free(dst->db_buf);
		dst->db_buf = 0;
		dst->db_data.d_size = 0;
		dst->db_myflags &= ~DBF_READY;
		break;
	}
	dst->db_data.d_buf = dst->db_buf;
	dst->db_data.d_type = ELF_T_REL;
	dst->db_data.d_align = ELF32_FSZ_WORD;
	dst->db_fsz = 0;
	dst->db_off = 0;
	{
		register Elf32_Shdr	*sh;

		if ((sh = elf32_getshdr(rscn)) == 0)
			return (int)OK_NO;
		sh->sh_name = _elf_coffname(info, NM_REL);
		sh->sh_type = SHT_REL;
		sh->sh_entsize = RELSZ;
		sh->sh_size = crel->d_size;
		sh->sh_link = elf_ndxscn(info->i_symscn);
		sh->sh_info = elf_ndxscn(dscn);
		sh->sh_offset = info->i_coffscn[sh->sh_info].s_relptr;
		sh->sh_addralign = ELF32_FSZ_WORD;
	}
	return (int)OK_YES;
}


/*ARGSUSED*/
void
_elf_coff386_shdr(elf, info, esh, ch)
	Elf		*elf;
	Info		*info;
	Elf32_Shdr	*esh;
	SCNHDR		*ch;
{
	return;
}


/* fix relocated words
 *	Extract 2's complement, signed, little-endian, 4-byte value,
 *	subtract one address, add another, and store the result.
 *	This code converts to native machine binary, if needed.
 */

static void
do4le(p, subr, addr)
	register unsigned char	*p;
	Elf32_Addr		subr;
	Elf32_Addr		addr;
{
	register Elf32_Sword	sw;
	register Elf32_Word	w;

	w = (((((((Elf32_Word)p[3]<<8)+p[2])<<8)+p[1])<<8)+p[0]);

	/*	check for 32-bit, 2's complement host
	 */

	/*CONSTANTCONDITION*/
	if (~(Elf32_Word)0 == -(Elf32_Sword)1	/* 32-bit 2s comp */
	&& ~(~(Elf32_Word)0 >> 1) == BIT32)
	{
		w = w - subr + addr;
	}
	else					/* other */
	{
		if ((sw = w) & BIT32)
		{
			w |= ~(Elf32_Word)LO31;
			w = ~w + 1;
			sw = -w;
		}
		if ((sw = sw - subr + addr) < 0)
		{
			w = -sw;
			w = ~w + 1;
		}
		else
			w = sw;
	}
	p[0] = (unsigned char)w;
	p[1] = (unsigned char)(w >> 8);
	p[2] = (unsigned char)(w >> 16);
	p[3] = (unsigned char)(w >> 24);
	return;
}
