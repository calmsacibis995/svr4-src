/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:coff/cofftab.c	1.5"


#include "syn.h"
#include "filehdr.h"
#include "scnhdr.h"
#include "libelf.h"
#include "decl.h"
#include "coff.h"
#include "cofftab.h"


/* COFF table
 *	The following table holds the information needed to build
 *	an elf file from coff.  Much of the translation is common, but
 *	some is machine-specific.
 *
 *	One can add or delete machines by changing the table.  Other
 *	coff conversion files either provide these machine-specific
 *	routines, or they provide common services that use this table
 *	to eliminate machine dependencies.
 */


Elf_Void	*memcpy		_((void *, const void *, size_t));


const Coff	_elf_cofftab[] =
{
	{ WE32MAGIC,	EM_M32,	ELFDATA2MSB,	0x2000L,
		_elf_coffm32_flg,	_elf_coffm32_opt,
		_elf_coffm32_rel,	_elf_coffm32_shdr },

	{ I386MAGIC,	EM_386,	ELFDATA2LSB,	0x1000L,
		_elf_coff386_flg,	_elf_coff386_opt,
		_elf_coff386_rel,	_elf_coff386_shdr },

	{ 0 },	/* null termination */
};


static const char	confignm[] = ".config_header";
static const char	relnm[] = ".rel";
static const char	relanm[] = ".rela";
static const char	strnm[] = ".strtab";
static const char	symnm[] = ".symtab";

static struct
{
	const char	*n_name;	/* special name */
	const size_t	n_len;		/* name length */
	size_t		n_index;	/* string table index */
} nmtab[] =
{
	{ 0 },						/* NM_Init */
	{ confignm,	sizeof(confignm),	0 },	/* NM_CONFIG */
	{ relnm,	sizeof(relnm),		0 },	/* NM_REL */
	{ relanm,	sizeof(relanm),		0 },	/* NM_RELA */
	{ strnm,	sizeof(strnm),		0 },	/* NM_STR */
	{ symnm,	sizeof(symnm),		0 },	/* NM_SYM */
	{ 0 },						/* NM_Num */
};


size_t
_elf_coffname(info, name)	/* init strtab or add special name */
	Info		*info;
	register Name	name;
{
	register size_t	sz;

	if ((size_t)name >= (size_t)NM_Num)
		return 0;
	if (name == NM_Init)	/* init table, return total space */
	{
		sz = 0;
		for (name = 1; nmtab[name].n_name; ++name)
		{
			nmtab[name].n_index = 0;
			sz += nmtab[name].n_len;
		}
		return sz;
	}
	if (nmtab[name].n_index != 0)
		return nmtab[name].n_index;
	return nmtab[name].n_index = _elf_coffnewstr(info, nmtab[name].n_name,
					nmtab[name].n_len - 1);
}


size_t
_elf_coffnewstr(info, str, len)		/* new strtab entry, add '\0' */
	register Info	*info;
	const char	*str;
	register size_t	len;
{
	size_t		r = info->i_struse;
	register char	*p = info->i_strtab + r;

	(void)memcpy(p, str, len);
	p[len] = '\0';
	while (*p != '\0')
		++p;
	info->i_struse = p - info->i_strtab + 1;
	return r;
}
