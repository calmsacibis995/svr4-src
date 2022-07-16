/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:kdb/db_as.c	1.2"

#include "sys/types.h"
#include "sys/immu.h"
#include "sys/sysmacros.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/disp.h"
#include "sys/debug.h"
#include "vm/hat.h"
#include "vm/as.h"
#include "../db_as.h"

static proc_t	*db_uvirt_proc;
static pte_t	*pte;
static u_long	save_pfn;


#define BADADDR	((paddr_t)-1)

paddr_t
db_kvtop(vaddr)
	u_long	vaddr;
{
	register pte_t	*pte;

	pte = &kpd0[ptnum(vaddr)];
	if (!pte->pgm.pg_v)
		return BADADDR;
	pte = vatopte(vaddr, pte);
	if (!pte->pgm.pg_v)
		return BADADDR;
	return ptob(pte->pgm.pg_pfn) + PAGOFF(vaddr);
}


int
db_read(addr, buf, n)
	as_addr_t	addr;
	char		*buf;
	u_int		n;
{
	u_int		cnt;
	u_long		vaddr;
	paddr_t		paddr;

	do {
		vaddr = addr.a_addr;
		if (addr.a_as == AS_IO)
			cnt = n;
		else {
			cnt = PAGESIZE - PAGOFF(vaddr);
			if (cnt > n)
				cnt = n;
		}
		switch (addr.a_as) {
		case AS_KVIRT:
			if ((paddr = db_kvtop(vaddr)) == BADADDR)
				return -1;
			bcopy((caddr_t)phystokv(paddr), buf, cnt);
			break;
		case AS_UVIRT:
			if ((db_uvirt_proc = pid_entry(addr.a_mod)) == NULL)
				return -1;
			vaddr = db_uvtop(vaddr, db_uvirt_proc);
			if (vaddr == -1)
				return -1;
			/* Fall through ... */
		case AS_PHYS:
			pte = svtopte(KVBASE);
			save_pfn = pte->pgm.pg_pfn;
			pte->pgm.pg_pfn = PFNUM(vaddr);
			bcopy((caddr_t)(KVBASE + PAGOFF(vaddr)), buf, cnt);
			pte->pgm.pg_pfn = save_pfn;
			break;
		case AS_IO:
			while (cnt >= sizeof(u_long)) {
				*((u_long *)buf) = inl(vaddr);
				buf += sizeof(u_long);
				vaddr += sizeof(u_long);
				cnt -= sizeof(u_long);
				n -= sizeof(u_long);
			}
			if (cnt > 1) {
				*((u_short *)buf) = inw(vaddr);
				buf += sizeof(u_short);
				vaddr += sizeof(u_short);
				cnt -= sizeof(u_short);
				n -= sizeof(u_short);
			}
			if (cnt != 0)
				*((u_char *)buf) = inb(vaddr);
			break;
		}
		addr.a_addr += cnt;
	} while ((n -= cnt) > 0);

	return 0;
}

int
db_write(addr, buf, n)
	as_addr_t	addr;
	char		*buf;
	u_int		n;
{
	u_int		cnt;
	u_long		vaddr;
	paddr_t		paddr;

	do {
		vaddr = addr.a_addr;
		if (addr.a_as == AS_IO)
			cnt = n;
		else {
			cnt = PAGESIZE - PAGOFF(vaddr);
			if (cnt > n)
				cnt = n;
		}
		switch (addr.a_as) {
		case AS_KVIRT:
			if ((paddr = db_kvtop(vaddr)) == BADADDR)
				return -1;
			bcopy(buf, (caddr_t)phystokv(paddr), cnt);
			break;
		case AS_UVIRT:
			if ((db_uvirt_proc = pid_entry(addr.a_mod)) == NULL)
				return -1;
			vaddr = db_uvtop(vaddr, db_uvirt_proc);
			if (vaddr == -1)
				return -1;
			/* Fall through ... */
		case AS_PHYS:
			pte = svtopte(KVBASE);
			save_pfn = pte->pgm.pg_pfn;
			pte->pgm.pg_pfn = PFNUM(vaddr);
			bcopy(buf, (caddr_t)(KVBASE + PAGOFF(vaddr)), cnt);
			pte->pgm.pg_pfn = save_pfn;
			break;
		case AS_IO:
			while (cnt >= sizeof(u_long)) {
				outl(vaddr, *((u_long *)buf));
				buf += sizeof(u_long);
				vaddr += sizeof(u_long);
				cnt -= sizeof(u_long);
				n -= sizeof(u_long);
			}
			if (cnt > 1) {
				outw(vaddr, *((u_short *)buf));
				buf += sizeof(u_short);
				vaddr += sizeof(u_short);
				cnt -= sizeof(u_short);
				n -= sizeof(u_short);
			}
			if (cnt != 0)
				outb(vaddr, *((u_char *)buf));
			break;
		}
		addr.a_addr += cnt;
	} while ((n -= cnt) > 0);

	return 0;
}


paddr_t
db_uvtop(vaddr, proc)
	addr_t	vaddr;
	proc_t	*proc;
{
	register pte_t *ptp, *pdtep;
	struct	hat    *hatp;
	hatpt_t	*ptap, *eptap;
	int mcndx;
	int mcnum;
	hatpgt_t *pt;

	if ((UVUBLK <= (u_long)vaddr) &&
			((u_long)vaddr < (UVUBLK + ptob(proc->p_usize)))) {
		paddr_t	paddr;
		vaddr = (caddr_t)PTOU(proc) + ((u_long)vaddr - UVUBLK);
		if ((paddr = db_kvtop(vaddr)) == BADADDR)
			goto error;
		return paddr;
	} else if (KADDR((u_long)vaddr))
		return -1;

	/*
	 * Vaddr may be in the context of some process other than
	 * u.u_procp.  Scan its hatpts and the associated page tables.
	 */

	pdtep = vatopdte(vaddr);
	hatp = &(proc->p_as->a_hat);
	
	ptap = eptap = hatp->hat_ptlast;
	if (ptap == (hatpt_t *)NULL) {
		ASSERT(hatp->hat_pts == (hatpt_t *)NULL);
			goto error;
	} else {
		do {
			ASSERT(ptap && ptap->hatpt_aec);
			ASSERT(ptap->hatpt_pde.pg_pte && ptap->hatpt_pdtep);
			if (pdtep > ptap->hatpt_pdtep) {
				ptap = ptap->hatpt_forw;
				continue;
			}
			else
				break;
		} while (ptap != eptap);
		
		if (pdtep == ptap->hatpt_pdtep) { 
			hatp->hat_ptlast = ptap;
			pt = (hatpgt_t *) ptetokv(ptap->hatpt_pde.pg_pte);
			mcnum = HATMCNO(vaddr);
			mcndx = HATMCNDX(vaddr);
			ptp = pt->hat_pgtc[mcnum].hat_pte;
			ptp += mcndx;
		}
		else
			goto error;
	}
	if (!PG_ISVALID(ptp))
		goto error;
	return ptob(ptp->pgm.pg_pfn) + PAGOFF(vaddr);
error:
	return (paddr_t)-1;
}
