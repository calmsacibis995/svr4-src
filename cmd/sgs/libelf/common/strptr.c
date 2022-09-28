/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/strptr.c	1.6"


#ifdef __STDC__
	#pragma weak	elf_strptr = _elf_strptr
#endif


#include "syn.h"
#include "libelf.h"
#include "decl.h"
#include "error.h"


char *
elf_strptr(elf, ndx, off)
	Elf			*elf;
	size_t			ndx;
	size_t			off;
{
	register Elf_Scn	*s;
	register Elf_Data	*d;

	if (elf == 0)
		return 0;
	if ((s = elf_getscn(elf, ndx)) == 0
	|| s->s_shdr == 0
	|| s->s_shdr->sh_type != SHT_STRTAB)
	{
		_elf_err = EREQ_STRSCN;
		return 0;
	}

	/*	If the layout bit is set, use the offsets and
	 *	sizes in the data buffers.  Otherwise, take
	 *	data buffers in order.
	 */

	d = 0;
	if (elf->ed_uflags & ELF_F_LAYOUT)
	{
		while ((d = elf_getdata(s, d)) != 0)
		{
			if (d->d_buf == 0)
				continue;
			if (off >= d->d_off
			&& off < d->d_off + d->d_size)
				return (char *)d->d_buf + off - d->d_off;
		}
	}
	else
	{
		size_t	sz = 0, j;

		while ((d = elf_getdata(s, d)) != 0)
		{
			if ((j = d->d_align) > 1 && sz % j != 0)
			{
				j -= sz % j;
				sz += j;
				if (off < j)
					break;
				off -= j;
			}
			if (d->d_buf != 0)
			{
				if (off < d->d_size)
					return (char *)d->d_buf + off;
			}
			sz += d->d_size;
			if (off < d->d_size)
				break;
			off -= d->d_size;
		}
	}
	_elf_err = EREQ_STROFF;
	return 0;
}
