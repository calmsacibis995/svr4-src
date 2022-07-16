/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:mem.c	1.3.1.2"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/uio.h"
#include "sys/cred.h"
#include "sys/proc.h"
#include "sys/disp.h"
#include "sys/debug.h"

#include "sys/mman.h"
#include "sys/kmem.h"

#include "sys/vm.h"
#include "vm/seg.h"
#include "vm/seg_vn.h"

#include "sys/ddi.h"

#ifdef __STDC__
STATIC int mmrw(dev_t, struct uio *, struct cred *, enum uio_rw);
#else
STATIC int mmrw();
#endif


#define	M_MEM		0	/* /dev/mem - physical main memory */
#define	M_KMEM		1	/* /dev/kmem - virtual kernel memory & I/O */
#define	M_NULL		2	/* /dev/null - EOF & Rathole */
#define	M_PMEM		3	/* /dev/pmem - any physical memory */
#define	M_ZERO		4	/* /dev/zero - source of private memory */

int mmdevflag = 0;

/*
 * Avoid addressing invalid kernel page.  This can happen, for example,
 * if a server process issues a read or write after seeking to a bad address.
 */
extern int memprobe();

/* ARGSUSED */
int
mmopen(devp, flag, type, cr)
        dev_t *devp;
        int flag;
	int type;
        struct cred *cr;
{
        return 0;
}

/* ARGSUSED */
int
mmclose(dev, flag, cr)
        dev_t dev;
        int flag;
        struct cred *cr;
{
        return 0;
}

/* ARGSUSED */
int
mmioctl(dev, cmd, arg, flag, cr, rvalp)
	dev_t dev;
	int cmd;
	int arg;
	int flag;
	struct cred *cr;
	int *rvalp;
{
	return ENODEV;
}

/* ARGSUSED */
int
mmread(dev, uiop, cr)
        dev_t dev;
        struct uio *uiop;
        struct cred *cr;
{
	return (mmrw(dev, uiop, cr, UIO_READ));
}

/* ARGSUSED */
int
mmwrite(dev, uiop, cr)
	dev_t dev;
        struct uio *uiop;
        struct cred *cr;
{
	return (mmrw(dev, uiop, cr, UIO_WRITE));
}

/*
 * When reading the M_ZERO device, we simply copyout the zeroes
 * array in NZEROES sized chunks to the user's address.
 *
 * XXX - this is not very elegant and should be redone.
 */
#define NZEROES		0x100
static char zeroes[NZEROES];

/* ARGSUSED */
STATIC int
mmrw(dev, uiop, cr, rw)
dev_t dev;
register struct uio *uiop;
struct cred *cr;
enum uio_rw rw;
{
	register off_t off, n;
	register unsigned long po;
	int error = 0;
	caddr_t	 addr;
	u_int	m;

        while (error == 0 && uiop->uio_resid != 0) {
		/* It may take long here, so we put in a preemption point */
		PREEMPT(); 
		/*
		 * Don't cross page boundaries.  uio_offset could be
		 * negative, so don't just take a simpler MIN.
		 */
		po = (unsigned long) (off = uiop->uio_offset) % ctob(1);
                n = MIN(MIN(uiop->uio_resid, ctob(1)), ctob(1) - po);

		switch (m = getminor(dev)) {
			/*
		 	 * Get appropriate addres for /dev/mem (physical),
		 	 * /dev/kmem (virtual), or /dev/pmem (any physical).
		 	 */
			case M_MEM:
			case M_PMEM:
			case M_KMEM:
				if (m == M_KMEM)
					addr = (caddr_t)off;
				else if (m == M_PMEM && btop(off) != 0)
					addr = physmap((paddr_t)off,
							ptob(1),
							KM_SLEEP) + po;
				else
					addr = (caddr_t)xphystokv(off);
				if (memprobe(addr) ||
				    uiomove(addr, n, rw, uiop)) {
	                       		error = ENXIO;
				}
				break;

			case M_ZERO:
				if (rw == UIO_READ) {
					n = MIN(n, sizeof (zeroes));
					if (uiomove(zeroes, n, rw, uiop))
						error = ENXIO;
					break;
				}
				/* FALLTHROUGH */

			case M_NULL:
				if (rw == UIO_WRITE) {
					uiop->uio_offset += uiop->uio_resid;
					uiop->uio_resid = 0;
				}
				return 0;

			default:
				return(ENXIO);
		}
		if (m == M_PMEM && btop(off) != 0)
			physmap_free(addr - po, ptob(1), 0);
	}
	return(error);
}


/*ARGSUSED*/
mmmmap(dev, off, prot)
dev_t dev;
register off_t off;
{
	u_int	m;

	switch (m = getminor(dev)) {

	case M_MEM:
	case M_PMEM:
		if (m == M_PMEM && btop(off) != 0) {
			caddr_t addr = physmap((paddr_t)off, 1, KM_SLEEP);
			if (memprobe(addr)) {
				physmap_free(addr, 1, 0);
				break;
			}
			physmap_free(addr, 1, 0);
		}
		else if (memprobe((caddr_t)xphystokv(off)))
			break;
		return hat_getppfnum((paddr_t)off, PSPACE_MAINSTORE);

	case M_KMEM:
		if (memprobe((caddr_t)(off)))
			break;
		return hat_getkpfnum((caddr_t)off);

	case M_ZERO:
		/*
		 * We shouldn't be mmap'ing to /dev/zero here as this
		 * mmsegmap should have already converted the mapping
		 * request for this device to a mapping using seg_vn.
		 */
	default:
		break;
	}
	return NOPAGE;
}

/*
 * This function is called when a memory device is mmap'ed.
 * Set up the mapping to the correct device driver.
 */
int
mmsegmap(dev, off, as, addrp, len, prot, maxprot, flags, cred)
dev_t dev;
u_int off;
struct as *as;
addr_t *addrp;
u_int len;
u_int prot, maxprot;
u_int flags;
struct cred *cred;
{
	struct segvn_crargs vn_a;

	/*
	 * If we are not mapping /dev/zero, then use spec_segmap()
	 * to set up the mapping which resolves to using mmmap().
	 */
	if (getminor(dev) != M_ZERO) {
		return (spec_segmap(dev, off, as, addrp, len, prot, maxprot,
		    flags, cred));
	}

	if ((flags & MAP_FIXED) == 0) {
		/*
		 * No need to worry about vac alignment since this
		 * is a "clone" object that doesn't yet exist.
		 */
		map_addr(addrp, len, (off_t)off, 0);
		if (*addrp == NULL)
			return (ENOMEM);
	} else {
		/*
		 * User specified address -
		 * Blow away any previous mappings.
		 */
		(void) as_unmap(as, *addrp, len);
	}

	/*
	 * Use seg_vn segment driver for /dev/zero mapping.
	 * Passing in a NULL amp gives us the "cloning" effect.
	 */
	vn_a.vp = NULL;
	vn_a.offset = 0;
	vn_a.type = (flags & MAP_TYPE);
	vn_a.prot = (u_char)prot;
	vn_a.maxprot = (u_char)maxprot;
	vn_a.cred = cred;
	vn_a.amp = NULL;

	return (as_map(as, *addrp, len, segvn_create, (caddr_t)&vn_a));
}
