/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/flag.c	1.6"


#ifdef __STDC__
	#pragma weak	elf_flagdata = _elf_flagdata
	#pragma weak	elf_flagehdr = _elf_flagehdr
	#pragma weak	elf_flagelf = _elf_flagelf
	#pragma weak	elf_flagphdr = _elf_flagphdr
	#pragma weak	elf_flagscn = _elf_flagscn
	#pragma weak	elf_flagshdr = _elf_flagshdr
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"


unsigned
elf_flagdata(data, cmd, flags)
	Elf_Data	*data;
	Elf_Cmd		cmd;
	unsigned	flags;
{
	if (data == 0)
		return 0;
	if (cmd == ELF_C_SET)
		return ((Dnode *)data)->db_uflags |= flags;
	if (cmd == ELF_C_CLR)
		return ((Dnode *)data)->db_uflags &= ~flags;
	_elf_err = EREQ_FLAG;
	return 0;
}


unsigned int
elf_flagehdr(elf, cmd, flags)
	Elf		*elf;
	Elf_Cmd		cmd;
	unsigned	flags;
{
	if (elf == 0)
		return 0;
	if (cmd == ELF_C_SET)
		return elf->ed_ehflags |= flags;
	if (cmd == ELF_C_CLR)
		return elf->ed_ehflags &= ~flags;
	_elf_err = EREQ_FLAG;
	return 0;
}


unsigned
elf_flagelf(elf, cmd, flags)
	Elf		*elf;
	Elf_Cmd		cmd;
	unsigned	flags;
{
	if (elf == 0)
		return 0;
	if (cmd == ELF_C_SET)
		return elf->ed_uflags |= flags;
	if (cmd == ELF_C_CLR)
		return elf->ed_uflags &= ~flags;
	_elf_err = EREQ_FLAG;
	return 0;
}


unsigned
elf_flagphdr(elf, cmd, flags)
	Elf		*elf;
	Elf_Cmd		cmd;
	unsigned	flags;
{
	if (elf == 0)
		return 0;
	if (cmd == ELF_C_SET)
		return elf->ed_phflags |= flags;
	if (cmd == ELF_C_CLR)
		return elf->ed_phflags &= ~flags;
	_elf_err = EREQ_FLAG;
	return 0;
}


unsigned
elf_flagscn(scn, cmd, flags)
	Elf_Scn		*scn;
	Elf_Cmd		cmd;
	unsigned	flags;
{
	if (scn == 0)
		return 0;
	if (cmd == ELF_C_SET)
		return scn->s_uflags |= flags;
	if (cmd == ELF_C_CLR)
		return scn->s_uflags &= ~flags;
	_elf_err = EREQ_FLAG;
	return 0;
}


unsigned
elf_flagshdr(scn, cmd, flags)
	Elf_Scn		*scn;
	Elf_Cmd		cmd;
	unsigned	flags;
{
	if (scn == 0)
		return 0;
	if (cmd == ELF_C_SET)
		return scn->s_shflags |= flags;
	if (cmd == ELF_C_CLR)
		return scn->s_shflags &= ~flags;
	_elf_err = EREQ_FLAG;
	return 0;
}
