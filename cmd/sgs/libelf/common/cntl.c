/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/cntl.c	1.5"


#ifdef __STDC__
	#pragma weak	elf_cntl = _elf_cntl
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"


int
elf_cntl(elf, cmd)
	Elf	*elf;
	Elf_Cmd	cmd;
{

	if (elf == 0)
		return 0;
	switch (cmd)
	{
	case ELF_C_FDREAD:
	{
		int	j = 0;

		if ((elf->ed_myflags & EDF_READ) == 0)
		{
			_elf_err = EREQ_CNTLWRT;
			return -1;
		}
		if (elf->ed_status != ES_FROZEN
		&& (_elf_cook(elf) != OK_YES
		|| _elf_vm(elf, (size_t)0, elf->ed_fsz) != OK_YES))
			j = -1;
		elf->ed_fd = -1;
		return j;
	}

	case ELF_C_FDDONE:
		if ((elf->ed_myflags & EDF_READ) == 0)
		{
			_elf_err = EREQ_CNTLWRT;
			return -1;
		}
		elf->ed_fd = -1;
		return 0;

	default:
		_elf_err = EREQ_CNTLCMD;
		break;
	}
	return -1;
}
