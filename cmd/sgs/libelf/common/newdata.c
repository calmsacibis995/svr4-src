/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/newdata.c	1.2"


#ifdef __STDC__
	#pragma weak	elf_newdata = _elf_newdata
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"


Elf_Data *
elf_newdata(s)
	register Elf_Scn	*s;
{
	register Dnode		*d;

	if (s == 0)
		return 0;
	if (s->s_index == SHN_UNDEF)
	{
		_elf_err = EREQ_SCNNULL;
		return 0;
	}

	/*	If this is the first new node, use the one allocated
	 *	in the scn itself.  Update data buffer in both cases.
	 */

	if (s->s_hdnode == 0)
	{
		s->s_dnode.db_uflags |= ELF_F_DIRTY;
		s->s_dnode.db_myflags |= DBF_READY;
		s->s_hdnode = &s->s_dnode;
		s->s_tlnode = &s->s_dnode;
		s->s_dnode.db_scn = s;
		s->s_dnode.db_data.d_version = _elf_work;
		return &s->s_dnode.db_data;
	}
	if ((d = _elf_dnode()) == 0)
		return 0;
	d->db_data.d_version = _elf_work;
	d->db_scn = s;
	d->db_uflags |= ELF_F_DIRTY;
	d->db_myflags |= DBF_READY;
	s->s_tlnode->db_next = d;
	s->s_tlnode = d;
	return &d->db_data;
}
