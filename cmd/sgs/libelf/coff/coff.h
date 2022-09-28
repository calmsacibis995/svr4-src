/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:coff/coff.h	1.4"


#ifndef _
#	ifdef __STDC__
#		define _(a)	a
#	else
#		define _(a)	()
#	endif
#endif


/* New string names
 *	Sections created during conversion have names.  All such names
 *	must have entries here, allowing allocation in the string table.
 *	If other kinds of strings also are "manufactured," they should
 *	be added to this list.
 */

typedef enum
{
	NM_Init = 0,	/* must be first, 0 */
	NM_CONFIG,	/* ".config_header" */
	NM_REL,		/* ".rel" */
	NM_RELA,	/* ".rela" */
	NM_STR,		/* ".strtab" */
	NM_SYM,		/* ".symtab" */
	NM_Num		/* must be last */
} Name;


/* Coff table
 *	The following structure holds the information needed to build
 *	an elf file from coff.  Much of the translation is common, but
 *	some is machine-specific.
 *
 *	c_magic		The coff magic number
 *	c_machine	The value for Ehdr.e_machine
 *	c_encode	The value for Ehdr.e_ident[EI_DATA]
 *	c_align		Segment alignment for a.out file
 *	c_flg		Function to convert FILHDR.f_flags
 *	c_opt		Function to convert optional header
 *	c_rel		Function to convert relocations
 *	c_shdr		Function to "fix" elf section header
 *
 *	c_opt() and c_rel() failures must return OK_NO and set the
 *	error number.  A delayed failure may appear through the
 *	section descriptor.  They return OK_YES on success.  Because
 *	of deficiencies in some compilers, the associated structure
 *	members are pointers to int functions, not Okay functions.
 *	This is a concession to compiler errors only.
 */


typedef struct Info	Info;

typedef struct
{
	int		c_magic;
	unsigned	c_machine;
	unsigned	c_encode;
	unsigned long	c_align;
	void		(*c_flg) _((Elf *, Info *, unsigned));
	int		(*c_opt) _((Elf *, Info *, char *, size_t));
	int		(*c_rel) _((Elf *, Info *, Elf_Scn *, Elf_Scn *, Elf_Data *));
	void		(*c_shdr) _((Elf *, Info *, Elf32_Shdr *, SCNHDR *));
} Coff;


struct	Info
{
	const Coff	*i_coff;	/* COFF indicia */
	FILHDR		*i_filehdr;	/* coff file header */
	SCNHDR		*i_coffscn;	/* coff section table */
	Elf32_Phdr	*i_phdr;	/* phdr table */
	size_t		i_phdruse;	/* count */
	Elf_Scn		*i_symscn;	/* symbol table section */
	Elf32_Sym	*i_symtab;	/* symbol table entries */
	size_t		i_symuse;	/* # entries in use */
	size_t		i_symlocals;	/* # local symbols */
	char		*i_strtab;	/* string table bytes */
	size_t		i_strsz;	/* string table size */
	size_t		i_struse;	/* string bytes in use */
	size_t		i_strcoff;	/* coff string size */
	size_t		*i_fix;		/* fix[coff-index] == elf index */
};


extern const Coff	_elf_cofftab[];

size_t	_elf_coffname	_((Info *, Name));
size_t	_elf_coffnewstr	_((Info *, const char *, size_t));
Okay	_elf_coffscn	_((Elf *, Info *));
Okay	_elf_coffshdr	_((Elf *, Info *));
Okay	_elf_coffstr	_((Elf *, Info *));
Okay	_elf_coffsym	_((Elf *, Info *));
