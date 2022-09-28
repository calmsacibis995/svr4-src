/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:coff/coffm32.c	1.7"


#include "syn.h"
#include "filehdr.h"
#include "scnhdr.h"
#include "reloc.h"
#include "libelf.h"
#include "sys/elf_M32.h"
#include "decl.h"
#include "error.h"
#include "coff.h"
#include "cofftab.h"


/*	Each object file existing in the /boot directory contains an 
 *	optional header containing the following information.  This header
 *	is created by the mkboot(1M) program from the /etc/master data base.
 */

struct	master
{
	unsigned short		magic;
	unsigned short		flag;
	unsigned char		vec;
	unsigned char		nvec;
	char			prefix[4+1];
	unsigned char		soft;
	unsigned char		ndev;
	unsigned char		ipl;
	short			ndep;
	short			nparam;
	short			nrtn;
	short			nvar;
	unsigned short		o_depend;
	unsigned short		o_param;
	unsigned short		o_routine;
	unsigned short		o_variable;
};

#define	MMAGIC	0x3362	/* master.magic: "3b" */


#define BIT32	0x80000000L
#define LO31	0x7fffffffL


static void	do4	_((Byte *, Elf32_Addr));
static void	do4s	_((Byte *, Elf32_Addr));


/*ARGSUSED*/
void
_elf_coffm32_flg(elf, info, cflags)
	Elf		*elf;
	Info		*info;
	unsigned	cflags;
{

	if (cflags & F_BM32MAU)
		elf->ed_ehdr->e_flags |= EF_M32_MAU;
	return;
}


int
_elf_coffm32_opt(elf, info, hdr, sz)
	Elf		*elf;
	Info		*info;
	char		*hdr;
	size_t		sz;
{
	size_t		outsz;
	Elf_Scn		*scn;
	Elf32_Shdr	*sh;
	Elf_Data	src, *d;
	char		*buf;
	static
	const char	name[] = "AT&T UNIX mkboot";
	size_t		namesz;
	Elf32_Word	w[3];

	if (sz < sizeof(struct master))
		return (int)OK_YES;
	{
		unsigned short	magic;

		(void)memcpy(&magic, hdr, sizeof(magic));
		if (magic != MMAGIC)
			return (int)OK_YES;
	}
	if ((scn = elf_newscn(elf)) == 0
	|| (sh = elf32_getshdr(scn)) == 0
	|| (d = elf_newdata(scn)) == 0)
		return (int)OK_NO;
	outsz = 3 * ELF32_FSZ_WORD;
	if ((namesz = sizeof(name)) % ELF32_FSZ_WORD != 0)
		namesz += ELF32_FSZ_WORD - namesz % ELF32_FSZ_WORD;
	outsz += namesz + sz;
	if (outsz % ELF32_FSZ_WORD != 0)
		outsz += ELF32_FSZ_WORD - outsz % ELF32_FSZ_WORD;
	if ((d->d_buf = buf = malloc(outsz)) == 0)
	{
		_elf_err = EMEM_COPT;
		return (int)OK_NO;
	}
	((Dnode *)d)->db_buf = d->d_buf;
	w[0] = sizeof(name);
	w[1] = sz;
	w[2] = 1;
	src.d_buf = (Elf_Void *)w;
	src.d_size = sizeof(w);
	src.d_type = ELF_T_WORD;
	src.d_version = 1;
	d->d_size = outsz;
	(void)elf32_xlatetof(d, &src, ELFDATA2MSB);
	buf += d->d_size;
	(void)memcpy(buf, name, sizeof(name));
	buf += namesz;
	(void)memcpy(buf, hdr, sz);
	d->d_type = ELF_T_BYTE;
	d->d_size = outsz;
	d->d_off = 0;
	d->d_align = 4;
	d->d_version = _elf_work;

	sh->sh_name = _elf_coffname(info, NM_CONFIG);
	sh->sh_type = SHT_NOTE;
	sh->sh_offset = sizeof(FILHDR);
	sh->sh_size = outsz;
	sh->sh_addralign = 4;
	return (int)OK_YES;
}


/* Convert relocation
 *	This changes the relocation entries from coff to elf.
 *	On the 3b2, a relocation entry points at a place holding the
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
_elf_coffm32_rel(elf, info, rscn, dscn, crel)
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

		/*	Only error conditions may break
		 */

		switch (reloc.r_type)
		{
		case R_ABS:
			r->r_info = ELF32_R_INFO(ndx, R_M32_NONE);
			continue;

		case R_DIR32:
			r->r_info = ELF32_R_INFO(ndx, R_M32_32);
			if (r->r_offset > dvend)
			{
				rscn->s_err = ECOFF_RELOFF;
				break;
			}
			do4(bytes + r->r_offset, elfsym[ndx].st_value);
			continue;

		case R_DIR32S:
			r->r_info = ELF32_R_INFO(ndx, R_M32_32_S);
			if (r->r_offset > dvend)
			{
				rscn->s_err = ECOFF_RELOFF;
				break;
			}
			do4s(bytes + r->r_offset, elfsym[ndx].st_value);
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
_elf_coffm32_shdr(elf, info, esh, ch)
	Elf		*elf;
	Info		*info;
	Elf32_Shdr	*esh;
	SCNHDR		*ch;
{
	return;
}


/* fix relocated words
 *	Extract 2's complement, signed, big-endian, 4-byte value,
 *	subtract an address, and store the result.
 *	This code converts to native machine binary, if needed.
 */

static void
do4(p, subr)
	register Byte		*p;
	Elf32_Addr		subr;
{
	register Elf32_Sword	sw;
	register Elf32_Word	w;

	w = (((((((Elf32_Word)p[0]<<8)+p[1])<<8)+p[2])<<8)+p[3]);

	/*	check for 32-bit, 2's complement host
	 */

	/*CONSTANTCONDITION*/
	if (~(Elf32_Word)0 == -(Elf32_Sword)1	/* 32-bit 2s comp */
	&& ~(~(Elf32_Word)0 >> 1) == BIT32)
	{
		w -= subr;
	}
	else					/* other */
	{
		if ((sw = w) & BIT32)
		{
			w |= ~(Elf32_Word)LO31;
			w = ~w + 1;
			sw = -w;
		}
		if ((sw -= subr) < 0)
		{
			w = -sw;
			w = ~w + 1;
		}
		else
			w = sw;
	}
	p[3] = (unsigned char)w;
	p[2] = (unsigned char)(w >> 8);
	p[1] = (unsigned char)(w >> 16);
	p[0] = (unsigned char)(w >> 24);
	return;
}


/* fix relocated words
 *	Extract 2's complement, signed, little-endian, 4-byte value,
 *	subtract an address, and store the result.
 *	This code converts to native machine binary, if needed.
 */

static void
do4s(p, subr)
	register Byte		*p;
	Elf32_Addr		subr;
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
		w -= subr;
	}
	else					/* other */
	{
		if ((sw = w) & BIT32)
		{
			w |= ~(Elf32_Word)LO31;
			w = ~w + 1;
			sw = -w;
		}
		if ((sw -= subr) < 0)
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
