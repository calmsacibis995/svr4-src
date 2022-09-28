/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/ar.c	1.6"


#include "syn.h"
#include <ar.h>
#include "libelf.h"
#include "decl.h"
#include "error.h"
#include "member.h"


#define MANGLE	'\177'


/* Archive processing
 *	When processing an archive member, two things can happen
 *	that are a little tricky.
 *
 * Sliding
 *	Archive members are only 2-byte aligned within the file.  To reuse
 *	the file's memory image, the library slides an archive member into
 *	its header to align the bytes.  This means the header must be
 *	disposable.
 *
 * Header reuse
 *	Because the library can trample the header, it must be preserved to
 *	avoid restrictions on archive member reuse.  That is, if the member
 *	header changes, the library may see garbage the next time it looks
 *	at the header.  After extracting the original header, the library
 *	writes the following into it.
 *
 *	[0]	'/'		/ names are special
 *	[1]	MANGLE		MANGLE shouldn't appear in a normal archive
 *	[2..]	pad		unused bytes up to memory alignment
 *	[t..]	elf		Elf32.p value for elf descriptor.  This gives a
 *				sanity check and is hard to forge.
 *	[m..]	member		This Elf32.p value points to the Member
 *				structure for this member.
 *	[u..]	unused		Unused bytes between the member pointer and
 *				the body of the file.
 *	[b..]	body
 *
 *	Because the member body slides back into the header, the header must
 *	be large enough to ensure alignment for all entities.  This is true
 *	iff sizeof(struct ar_hdr) >= 3 * sizeof(Elf32).  Because the ar_hdr
 *	is at least 60 bytes, this is pretty safe.
 *
 *	Unfortunately, this method has a small but non-zero chance of dumping
 *	core on corrupt files.  That is if the member's name really is
 *	"/<MANGLE>", and the following bytes happen to match the elf descriptor,
 *	and the member pointer's value is invalid, the program will die when
 *	it dereferences the member pointer.  This shouldn't happen in practice.
 */


/* Size check
 *	If the header is too small, the following generates a negative
 *	subscript for x.x and fails to compile.
 */

struct	x
{
	char	x[sizeof(struct ar_hdr) - 3 * sizeof(Elf32) - 1];
};


static Member		*ck_mangle	_((Elf *, struct ar_hdr *));
static void		do_mangle	_((Elf *, struct ar_hdr *, Member *));
static unsigned long	number		_((char *, char *, int));


static const char	fmag[] = ARFMAG;


static Member *
ck_mangle(elf, a)
	Elf			*elf;
	register struct ar_hdr	*a;
{
	register Elf32		*p;
	register size_t		sz;

	if (a->ar_name[0] != '/' || a->ar_name[1] != MANGLE)
		return 0;
	sz = (size_t)(a->ar_name - (char *)elf->ed_image) % sizeof(Elf32);
	p = (Elf32 *)(a->ar_name + sizeof(Elf32) - sz);
	if (p->p != (Elf_Void *)elf)
	{
		_elf_err = EFMT_ARBUG;
		return 0;
	}
	return (Member *)(p + 1)->p;
}


static void
do_mangle(elf, a, m)
	Elf			*elf;
	register struct ar_hdr	*a;
	register Member		*m;
{
	register Elf32		*p;
	register size_t		sz;

	a->ar_name[0] = '/';
	a->ar_name[1] = MANGLE;
	sz = (size_t)(a->ar_name - (char *)elf->ed_image) % sizeof(Elf32);
	p = (Elf32 *)(a->ar_name + sizeof(Elf32) - sz);
	p->p = (Elf_Void *)elf;
	(p + 1)->p = (Elf_Void *)m;
	return;
}


static unsigned long
number(p, end, base)
	register char	*p;
	register char	*end;
	int		base;
{
	register unsigned	c;
	register unsigned long	n = 0;

	while (p < end)
	{
		if ((c = *p - '0') >= base)
		{
			while (*p++ == ' ')
				if (p >= end)
					return n;
			return 0;
		}
		n *= base;
		n += c;
		++p;
	}
	return n;
}


/* Convert ar_hdr to Member
 *	Converts ascii file representation to the binary memory values.
 */

Member *
_elf_armem(elf, file, fsz)
	Elf			*elf;
	register char		*file;	/* file version */
	size_t			fsz;
{
	register struct ar_hdr	*f = (struct ar_hdr *)file;
	register Member		*m;

	if (fsz < sizeof(*f))
	{
		_elf_err = EFMT_ARHDRSZ;
		return 0;
	}
	if ((m = ck_mangle(elf, f)) != 0)
		return m;
	if (f->ar_fmag[0] != fmag[0] || f->ar_fmag[1] != fmag[1])
	{
		_elf_err = EFMT_ARFMAG;
		return 0;
	}
	if ((m = (Member *)malloc(sizeof(*m))) == 0)
	{
		_elf_err = EMEM_ARMEM;
		return 0;
	}
	m->m_next = elf->ed_memlist;
	elf->ed_memlist = m;
	m->m_err = 0;
	(void)memcpy(m->m_name, f->ar_name, ARSZ(ar_name));
	m->m_name[ARSZ(ar_name)] = '\0';
	m->m_hdr.ar_name = m->m_name;
	(void)memcpy(m->m_raw, f->ar_name, ARSZ(ar_name));
	m->m_raw[ARSZ(ar_name)] = '\0';
	m->m_hdr.ar_rawname = m->m_raw;

	/*	Classify file name.
	 *	If a name error occurs, delay until getarhdr().
	 */

	if (f->ar_name[0] != '/')	/* regular name */
	{
		register char	*p;

		p = &m->m_name[sizeof(m->m_name)];
		while (*--p != '/')
			if (p <= m->m_name)
				break;
		*p = '\0';
	}
	else if (f->ar_name[1] >= '0' && f->ar_name[1] <= '9')	/* strtab */
	{
		register unsigned long	j;

		j = number(&f->ar_name[1], &f->ar_name[ARSZ(ar_name)], 10);
		if (j < elf->ed_arstrsz)
			m->m_hdr.ar_name = elf->ed_arstr + j;
		else
		{
			m->m_hdr.ar_name = 0;
			m->m_err = EFMT_ARSTRNM;
		}
	}
	else if (f->ar_name[1] == ' ')				/* "/" */
		m->m_name[1] = '\0';
	else if (f->ar_name[1] == '/' && f->ar_name[2] == ' ')	/* "//" */
		m->m_name[2] = '\0';
	else							/* "/?" */
	{
		m->m_hdr.ar_name = 0;
		m->m_err = EFMT_ARUNKNM;
	}

	m->m_hdr.ar_date = number(f->ar_date, &f->ar_date[ARSZ(ar_date)], 10);
	m->m_hdr.ar_uid = number(f->ar_uid, &f->ar_uid[ARSZ(ar_uid)], 10);
	m->m_hdr.ar_gid = number(f->ar_gid, &f->ar_gid[ARSZ(ar_gid)], 10);
	m->m_hdr.ar_mode = number(f->ar_mode, &f->ar_mode[ARSZ(ar_mode)], 8);
	m->m_hdr.ar_size = number(f->ar_size, &f->ar_size[ARSZ(ar_size)], 10);
	do_mangle(elf, f, m);
	return m;
}


/* Initial archive processing
 *	An archive may have two special members.
 *	A symbol table, named /, must be first if it is present.
 *	A string table, named //, must precede all "normal" members.
 *
 *	This code "peeks" at headers but doesn't change them.
 *	Later processing wants original headers.
 *
 *	String table is converted, changing '/' name terminators
 *	to nulls.  The last byte in the string table, which should
 *	be '\n', is set to nil, guaranteeing null termination.  That
 *	byte should be '\n', but this code doesn't check.
 *
 *	The symbol table conversion is delayed until needed.
 */

void
_elf_arinit(elf)
	Elf			*elf;
{
	char			*base = elf->ed_ident;
	register char		*end = base + elf->ed_fsz;
	register struct ar_hdr	*a;
	register char		*hdr = base + SARMAG;
	register char		*mem;
	int			j;
	size_t			sz = SARMAG;

	elf->ed_status = ES_COOKED;
	elf->ed_nextoff = SARMAG;
	for (j = 0; j < 2; ++j)		/* 2 special members */
	{
		unsigned long	n;

		if (end - hdr < sizeof(*a)
		|| _elf_vm(elf, (size_t)(hdr - elf->ed_ident), sizeof(*a)) != OK_YES)
			return;
		a = (struct ar_hdr *)hdr;
		mem = (char *)a + sizeof(*a);
		n = number(a->ar_size, &a->ar_size[ARSZ(ar_size)], 10);
		if (end - mem < n || a->ar_name[0] != '/' || (sz = n) != n)
			return;
		hdr = mem + sz;
		if (a->ar_name[1] == ' ')
		{
			elf->ed_arsym = mem;
			elf->ed_arsymsz = sz;
			elf->ed_arsymoff = (char *)a - base;
		}
		else if (a->ar_name[1] == '/' && a->ar_name[2] == ' ')
		{
			if (_elf_vm(elf, (size_t)(mem - elf->ed_ident), sz) != OK_YES)
				return;
			elf->ed_arstr = mem;
			elf->ed_arstrsz = sz;
			elf->ed_arstroff = (char *)a - base;
			while (mem < hdr)
			{
				if (*mem == '/')
					*mem = '\0';
				++mem;
			}
			*(mem - 1) = '\0';
		}
		else
			return;
		hdr += sz & 1;
	}
}
