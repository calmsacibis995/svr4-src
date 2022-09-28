/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:coff/coffstr.c	1.5"


#include "syn.h"
#include "filehdr.h"
#include "scnhdr.h"
#include "syms.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"
#include "coff.h"


/* COFF string table conversion
 *	The elf string table preserves existing coff string table indexes.
 *	Coff symbols with short names are assigned string indexes for the
 *	new string table.
 *
 *	Newly allocated arrays may have too much room, but they'll have
 *	enough.  This is true for the string table because of a subtle fact.
 *	Coff file symbols have one entry to introduce the file name and an
 *	auxent to hold it.  The first entry's name is ".file" and is dropped.
 *	The auxent needs FILNMLEN+1 (15) bytes.  The allocated string table
 *	has SYMNMLEN+1 (9) bytes for each table entry, giving 18 bytes in the
 *	new table to hold 15.
 *
 *	This allocates string table space for input sections names.
 *
 *	The elf library guarantees sections from an input file will have
 *	one data buffer, not many.  That forces the following to copy
 *	the coff string table into the new table.
 *
 *	This doesn't actually create a section here, because that would
 *	make coff and elf have different section numbers.
 */


Okay
_elf_coffstr(elf, info)
	Elf			*elf;
	Info			*info;
{
	FILHDR			*fh = info->i_filehdr;
	char			*sp;
	size_t			sz;

	info->i_strsz = 2 + _elf_coffname(info, NM_Init);
	info->i_strsz += fh->f_nscns * (sizeof(((SCNHDR *)0)->s_name) + 1);

	/*	Bring symbol table and string table into memory
	 */

	if (fh->f_symptr > 0 && fh->f_nsyms > 0)
	{
		if (elf->ed_fsz < (size_t)fh->f_symptr)
		{
			_elf_err = ECOFF_SYMPTR;
			return OK_NO;
		}
		sz = elf->ed_fsz - (size_t)fh->f_symptr;
		if (sz / SYMESZ < fh->f_nsyms)
		{
			_elf_err = ECOFF_SYMSZ;
			return OK_NO;
		}
		if (_elf_vm(elf, (size_t)fh->f_symptr, sz) != OK_YES)
			return OK_NO;

		/*	Check if string table exists.  If so, it follows the
		 *	symbol table immediately, without alignment.
		 */

		sp = (char *)elf->ed_ident + fh->f_symptr
			+ fh->f_nsyms * SYMESZ;
		sz -= fh->f_nsyms * SYMESZ;
		if (sz >= sizeof(long))
		{
			long		nstr;

			(void)memcpy(&nstr, sp, sizeof(nstr));
			if (nstr < 0 || nstr > sz)
			{
				_elf_err = ECOFF_STRSZ;
				return OK_NO;
			}
			info->i_strcoff = nstr;
		}
		info->i_strsz += info->i_strcoff
				+ fh->f_nsyms * (SYMNMLEN + 1);
	}
	if ((info->i_strtab = malloc(info->i_strsz)) == 0)
	{
		_elf_err = EMEM_CSTR;
		return OK_NO;
	}

	if (info->i_strcoff != 0)		/* copy in old string table */
	{
		(void)memcpy(info->i_strtab, sp, info->i_strcoff);
		info->i_struse = info->i_strcoff;
	}
	else
		info->i_struse = 1;
	*info->i_strtab = '\0';
	return OK_YES;
}
