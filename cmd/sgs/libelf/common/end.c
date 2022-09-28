/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/end.c	1.11"


#ifdef __STDC__
	#pragma weak	elf_end = _elf_end
#endif


#include "syn.h"
#include <ar.h>
#include "libelf.h"
#include "decl.h"
#include "member.h"


int
elf_end(elf)
	register Elf		*elf;
{
	register Elf_Scn	*s;
	register Dnode		*d;
	Elf_Void		*trail = 0;

	if (elf == 0)
		return 0;
	if (--elf->ed_activ != 0)
		return elf->ed_activ;
loop:
	if (elf->ed_parent != 0)
		--elf->ed_parent->ed_activ;
	for (s = elf->ed_hdscn; s != 0; s = s->s_next)
	{
		for (d = s->s_hdnode; d != 0;)
		{
			register Dnode	*t;

			if (d->db_buf != 0)
				free(d->db_buf);
			if ((t = d->db_raw) != 0)
			{
				if (t->db_buf != 0)
					free(t->db_buf);
				if (t->db_myflags & DBF_ALLOC)
					free(t);
			}
			t = d->db_next;
			if (d->db_myflags & DBF_ALLOC)
				free(d);
			d = t;
		}
		if (s->s_myflags & SF_ALLOC)
		{
			if (trail != 0)
				free(trail);
			trail = (Elf_Void *)s;
		}
	}
	if (trail != 0)
	{
		free(trail);
		trail = 0;
	}
	{
		register Member	*m;

		for (m = elf->ed_memlist; m != 0; m = (Member *)trail)
		{
			trail = (Elf_Void *)m->m_next;
			free(m);
		}
	}
	if (elf->ed_myflags & EDF_EHALLOC)
		free(elf->ed_ehdr);
	if (elf->ed_myflags & EDF_PHALLOC)
		free(elf->ed_phdr);
	if (elf->ed_myflags & EDF_SHALLOC)
		free(elf->ed_shdr);
	if (elf->ed_myflags & EDF_RAWALLOC)
		free(elf->ed_raw);
	if (elf->ed_myflags & EDF_ASALLOC)
		free(elf->ed_arsym);

	/*	Don't release the image until the last reference dies.
	 */

	if (elf->ed_parent == 0)
	{
		if (elf->ed_vm != 0)
			free(elf->ed_vm);
		else
			_elf_unmap(elf->ed_image, elf->ed_imagesz);
	}
	trail = (Elf_Void *)elf;
	elf = elf->ed_parent;
	free(trail);
	trail = 0;
	if (elf != 0 && elf->ed_activ == 0)
		goto loop;
	return 0;
}
