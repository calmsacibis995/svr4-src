#ident	"@(#)ddi.c	1.2	92/03/01	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:ddi.c	1.3.2.4"

/*            UNIX Device Driver Interface functions          

 * This file contains functions that are to be added to the kernel 
 * to put the interface presented to drivers in conformance with
 * the DDI standard. Of the functions added to the kernel, 17 are
 * function equivalents of existing macros in sysmacros.h, map.h,
 * stream.h, and param.h

 * 18 additional functions --  drv_getparm(), drv_setparm(), setmapwant(),
 * physiock(), getrbuf(), freerbuf(), physmap(), physmap_free(),
 * getemajor(), geteminor(), etoimajor(), itoemajor(), drv_usectohz(),
 * drv_hztousec(), drv_usecwait(), drv_priv(), and pollwakeup()
 * -- are specified by DDI to exist
 * in the kernel and are implemented here.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/time.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/map.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/proc.h"
#include "sys/cmn_err.h"
#include "sys/stream.h"
#include "sys/uio.h"
#include "sys/kmem.h"
#include "sys/conf.h"
#include "sys/cred.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/poll.h"
#include "sys/session.h"
#include "sys/ddi.h"
#include "sys/mkdev.h"

extern struct user u; 

/* function: getmajor()     
 * purpose:  return internal major number corresponding to device
 *           number (new format) argument
 */

major_t
getmajor(dev)
register dev_t dev;
{
	return (major_t) ( (dev>>NBITSMINOR) & MAXMAJ );
}




/* function: getemajor()     
 * macro in: sysmacros.h 
 * purpose:  return external major number corresponding to device
 *           number (new format) argument
 */

major_t
getemajor(dev)
register dev_t dev;
{
	return (major_t) ( (dev>>NBITSMINOR) & MAXMAJ );
}



/* function: getminor()     
 * macro in: sysmacros.h 
 * purpose:  return internal minor number corresponding to device
 *           number (new format) argument
 */

minor_t
getminor(dev)
register dev_t dev;
{
	return (minor_t) (dev & MAXMIN);
}



/* function: geteminor()     
 * macro in: sysmacros.h 
 * purpose:  return external minor number corresponding to device
 *           number (new format) argument
 */

minor_t
geteminor(dev)
register dev_t dev;
{
	return (minor_t) (dev & MAXMIN);
}


/* function: etoimajor()     
 * purpose:  return internal major number corresponding to external
 *           major number argument or -1 if the external major number
 *	     exceeds the bdevsw and cdevsw count *
 *           For the 386 internal major = external major
 */

int
etoimajor(emajnum)
register major_t emajnum;
{
	if (emajnum > MAXMAJ)
		return (-1); /* invalid external major */

	return ( (int) emajnum);
}



/* function: itoemajor()     
 * purpose:  return external major number corresponding to internal
 *           major number argument or -1 if no external major number
 *	     can be found after lastemaj that maps to the internal
 *	     major number. Pass a lastemaj val of -1 to start
 *	     the search initially. (Typical use of this function is
 *	     of the form:
 *
 *	     lastemaj=-1;
 *	     while ( (lastemaj = itoemajor(imag,lastemaj)) != -1)
 *	        { process major number }
 *           For the 386 internal major = external major
 *           For the 386 internal minor = external minor
 */

int
itoemajor(imajnum, lastemaj)
register major_t imajnum;
int lastemaj;
{
	if (imajnum >= max(bdevcnt, cdevcnt))
		return (-1);

	/* if lastemaj == -1 then start from beginning of MAJOR table */
	if (lastemaj < -1) return (-1); 

	return ( (int) imajnum);
}



/* function: makedevice()
 * macro in: sysmacros.h 
 * purpose:  encode external major and minor number arguments into a
 *           new format device number
 */

dev_t
makedevice(maj, minor)
register major_t maj;
register minor_t minor;
{
	return (dev_t) ( (maj<<NBITSMINOR) | (minor&MAXMIN) ); 
}


/* cmpdev - compress new device format to old device format */

dev_t
cmpdev(dev)
register dev_t dev;
{
major_t major;
minor_t minor;

	major = (dev >> NBITSMINOR); 
	minor = (dev & MAXMIN);
	if (major > OMAXMAJ || minor > OMAXMIN)
		return(NODEV);
	return(((major << ONBITSMINOR) | minor)); 
}


dev_t
expdev(dev)
register dev_t dev;
{
major_t major;
minor_t minor;

	major = ((dev >> ONBITSMINOR) & OMAXMAJ);
	minor = (dev & OMAXMIN);
	return(((major << NBITSMINOR) | minor)); 
}


/* function: mapinit()
 * macro in: map.h 
 * purpose:  initialize the size of a map (the number of slots
 *           available is actually 1 less than the mapsiz arg
 *           because the first contains bookkeeping information.)
 *
 * alias:    rminit() -- DKI requirements; change of map-related
 *           function names for more uniformity in naming
 */

asm("	.text");
asm("	.align 4");
asm("	.globl rminit");
asm("rminit:	");

void
mapinit(mp, mapsiz)
register struct map *mp;
unsigned long mapsiz;
{
	mp[0].m_size = mapsiz-2; /* offset from start of map (mp[1]) */
				 /* of last slot in map */
}



/* function: mapwant()
 * macro in: map.h 
 * purpose:  return the number of processes waiting for free space in map
 *           (mp[0].m_addr)
 * alias:    rmwant -- DKI requirements call for name change in
 *           map-related functions
 */

asm("	.text");
asm("	.align 4");
asm("	.globl rmwant");
asm("rmwant:	");

unsigned long
mapwant(mp)
register struct map *mp;
{
	return(mp[0].m_addr);
}




/* function: kvtophys()
 * macro in: immu.h 
 * purpose:  return the physical address equivalent of the virtual address
 *           argument 
 */

paddr_t
kvtophys(vaddr)
register caddr_t vaddr;
{
	paddr_t svirtophys();

	return (svirtophys(vaddr));
}




/* function: datamsg()
 * macro in: stream.h 
 * purpose:  return true (1) if the message type input is a data
 *           message type, 0 otherwise
 */

int
datamsg(db_type)
register unsigned char db_type;
{
	return(db_type == M_DATA || db_type == M_PROTO || db_type == M_PCPROTO || db_type == M_DELAY);
}


/* function: OTHERQ()
 * macro in: stream.h 
 * purpose:  return a pointer to the other queue in the queue pair
 *           of qp
 */

queue_t *
OTHERQ(q)
register queue_t *q;
{
	return((q)->q_flag&QREADR? (q)+1: (q)-1);
}




/* function: putnxt()
 * macro in: stream.h 
 * purpose:  call the put routine of the queue linked to qp
 */

int
putnext(q, mp)
register queue_t *q;
register mblk_t *mp;
{
	return ((*q->q_next->q_qinfo->qi_putp)(q->q_next, mp));
}




/* function: RD()
 * macro in: stream.h 
 * purpose:  return a pointer to the read queue in the queue pair
 *           of qp; assumed that qp points to a write queue
 */

queue_t *
RD(q)
register queue_t *q;
{
	return(q-1);
}





/* function: splstr()
 * macro in: stream.h 
 * purpose:  set spl to protect critical regions of streams code;
 *           set to spltty(). This is spl5 on 3b2, spl7 on 386.
 */

int
splstr()
{
	return(spl6());
}



/* function: WR()
 * macro in: stream.h 
 * purpose:  return a pointer to the write queue in the queue pair
 *           of qp; assumed that qp points to a read queue
 */

queue_t *
WR(q)
register queue_t *q;
{
	return(q+1);
}



/* function: drv_getparm()
 * purpose:  store value of kernel parameter associated with parm in
 *           valuep and return 0 if parm is UPROCP, UCRED, PPGRP,
 *           LBOLT, PPID, PSID, TIME; return -1 otherwise
 */

int
drv_getparm(parm, valuep)
register unsigned long parm;
register unsigned long *valuep;

{
	switch (parm) {
	case UPROCP:
		*valuep= (unsigned long) u.u_procp;
		break;
	case UCRED:
		*valuep= (unsigned long) u.u_procp->p_cred;
		break;
	case PPGRP:
		*valuep= (unsigned long) u.u_procp->p_pgrp;
		break;
	case LBOLT:
		*valuep= (unsigned long) lbolt;
		break;
	case TIME:
		*valuep= (unsigned long) hrestime.tv_sec;
		break;
	case PPID:
		*valuep= (unsigned long) u.u_procp->p_pid;
		break;
	case PSID:
		*valuep= (unsigned long) u.u_procp->p_sessp->s_sid;
		break;
	default:
		return(-1);
	}

	return (0);
}



/* function: drv_setparm()
 * purpose:  set value of kernel parameter associated with parm to
 *           value and return 0 if parm is SYSRINT, SYSXINT,
 *           SYSMINT, SYSRAWC, SYSCANC, SYSOUTC; return -1 otherwise;
 *	     we splhi()/splx() to make this operation atomic
 */


int 
drv_setparm(parm, value)
register unsigned long parm;
register unsigned long value;
{
	int oldpri;

	oldpri = splhi();

	switch (parm) {
	case SYSRINT:
		sysinfo.rcvint+=value;
		break;
	case SYSXINT:
		sysinfo.xmtint+=value;
		break;
	case SYSMINT:
		sysinfo.mdmint+=value;
		break;
	case SYSRAWC:
		sysinfo.rawch+=value;
		break;
	case SYSCANC:
		sysinfo.canch+=value;
		break;
	case SYSOUTC:
		sysinfo.outch+=value;
		break;
	default:
		splx(oldpri);
		return(-1);
	}

	splx(oldpri);
	return(0);
}



/* function: physiock()
 * purpose:  perform raw device I/O on block devices
 *
 * The arguments are
 *	- the strategy routine for the device
 *	- a buffer, which is usually NULL, or else a special buffer
 *	  header owned exclusively by the device for this purpose
 *	- the device number
 *	- read/write flag
 *	- size of the device (in blocks)
 *	- uio structure containing the I/O parameters
 *
 * Returns 0 on success, or a non-zero errno on failure.
 */


int
physiock(strat, bp, dev, rw, devsize, uiop) 
void (*strat)();
register struct buf *bp;
dev_t dev;
int rw;
daddr_t devsize;
register struct uio *uiop;
{
	register struct iovec *iov;
	register unsigned cnt,iovlen;
	register off_t upper, limit;
	struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap; /* arg list for read/write system call */

	/* determine if offset is at or past end of device 
	 * if past end of device return ENXIO. Also, if at end 
	 * of device and writing, return ENXIO. If at end of device
	 * and reading, nothing more to read -- return 0
	 * If past end of device and reading -- return ENXIO
	 */

	limit = devsize << SCTRSHFT;
	if (uiop->uio_offset >= (off_t) limit)
	{
		if (uiop->uio_offset > limit || rw == B_WRITE)
			return(ENXIO);
		return(0);
	}

	/* adjust count of request so that it does not take I/O past
	 * end of device. Also update the count argument to the user's
	 * read(2) system call.
	 */

	iovlen = 0;
	for (cnt = 0,iov = uiop->uio_iov; cnt < uiop->uio_iovcnt; cnt++,iov++)
		iovlen += (off_t) iov->iov_len;

	upper = uiop->uio_offset + iovlen;

	if (upper > limit) {
		uap = (struct a *)u.u_ap;
		uap->count -= upper - limit;
		limit -= uiop->uio_offset;

		for (cnt = 0,iov = uiop->uio_iov; cnt < uiop->uio_iovcnt;
			 cnt++,iov++)
		{
			if (iov->iov_len <= limit)
				limit -= iov->iov_len;
			else
			{
				iov->iov_len = limit;
				limit = 0;
			}
		}
	}


	/* request in uio has been adjusted to fit device size. Now
	 * perform a uiophysio() and return the error number it
	 * returns 
	 */

	return(uiophysio(strat, bp, dev, rw, uiop));
}



/* function: setmapwant()
 * purpose:  increment the count on the wait flag (mp[0].m_addr) of
 *           map pointed to by mp; we splhi()/splx() to protect the
 *           critical region where the flag is incremented
 * alias:    rmsetwant; DKI requirments call for name changes in
 *           map-related functions
 */

asm("	.text");
asm("	.align 4");
asm("	.globl rmsetwant");
asm("rmsetwant:	");

void
setmapwant(mp)
register struct map *mp;
{
	int oldpri;

	oldpri = splhi();
	mp[0].m_addr++;
	splx(oldpri);
}

/*
 * function: getrbuf
 * purpose:  allocate space for buffer header and return pointer to it.
 *           preferred means of obtaining space for a local buf header.
 *           returns pointer to buf upon success, NULL for failure
 */
struct buf *
getrbuf(sleep)
	register long sleep;
{
	register struct buf *bp;

	bp = (struct buf *)kmem_zalloc(sizeof(struct buf), sleep);
	if (bp == NULL)
		return (NULL);
	bp->b_flags = B_KERNBUF | B_BUSY;
	bp->b_dev = (o_dev_t)NODEV;
	bp->b_edev = NODEV;
	return (bp);
}


/* function: freerbuf
 * purpose:  free up space allocated by getrbuf()
 */

void
freerbuf(bp)
struct buf *bp;
{
	kmem_free( (void *)bp, sizeof(struct buf));
}

/* function: btop
 * macro in: param.h
 * purpose:  convert byte count input to logical page units
 *           (byte counts that are not a page-size multiple
 *           are rounded down)
 */

unsigned long
btop (numbytes)
register unsigned long numbytes;
{
	return ( numbytes >> PAGESHIFT );
}



/* function: btopr
 * macro in: param.h
 * purpose:  convert byte count input to logical page units
 *           (byte counts that are not a page-size multiple
 *           are rounded up)
 */

unsigned long
btopr (numbytes)
register unsigned long numbytes;
{
	return ( (numbytes+PAGEOFFSET) >> PAGESHIFT );
}



/* function: ptob
 * macro in: param.h
 * purpose:  convert size in pages to to bytes.
 */

unsigned long
ptob (numpages)
register unsigned long numpages;
{
	return ( numpages << PAGESHIFT );
}



#define MAXCLOCK_T 0x7FFFFFFF

/* function: drv_hztousec
 * purpose:  convert from system time units (given by parameter HZ)
 *           to microseconds. This code makes no assumptions about the
 *           relative values of HZ and ticks and is intended to be
 *           portable. 
 *
 *           A zero or lower input returns 0, otherwise we use the formula
 *           microseconds = (hz/HZ) * 1,000,000. To minimize overflow
 *           we divide first and then multiply. Note that we want
 *           upward rounding, so if there is any fractional part,
 *           we increment the return value by one. If an overflow is
 *           detected (i.e.  resulting value exceeds the
 *           maximum possible clock_t, then truncate
 *           the return value to MAXCLOCK_T.
 *
 *           No error value is returned.
 *
 *           This function's intended use is to remove driver object
 *           file dependencies on the kernel parameter HZ.
 *           many drivers may include special diagnostics for
 *           measuring device performance, etc., in their ioctl()
 *           interface or in real-time operation. This function
 *           can express time deltas (i.e. lbolt - savelbolt)
 *           in microsecond units.
 *
 */

clock_t
drv_hztousec(ticks)
register clock_t ticks;
{
	clock_t quo, rem;
	register clock_t remusec, quousec;

	if (ticks <= 0) return (0);

	quo = ticks / HZ; /* number of seconds */
	rem = ticks % HZ; /* fraction of a second */
	quousec = 1000000 * quo; /* quo in microseconds */
	remusec = 1000000 * rem; /* remainder in millionths of HZ units */

	/* check for overflow */
	if (quo != quousec / 1000000) return (MAXCLOCK_T);
	if (rem != remusec / 1000000) remusec = MAXCLOCK_T;
	
	/* adjust remusec since it was in millionths of HZ units */
	remusec = (remusec % HZ) ? remusec/HZ + 1 : remusec/HZ;

	/* check for overflow again. If sum of quousec and remusec
	 * would exceed MAXCLOCK_T then return MAXCLOCK_T 
	 */

	if ( (MAXCLOCK_T - quousec) < remusec) return (MAXCLOCK_T);

	return (quousec + remusec);
}


/* function: drv_usectohz
 * purpose:  convert from microsecond time units to system time units
 *           (given by parameter HZ) This code also makes no assumptions
 *           about the relative values of HZ and ticks and is intended to
 *           be portable. 
 *
 *           A zero or lower input returns 0, otherwise we use the formula
 *           hz = (microsec/1,000,000) * HZ. Note that we want
 *           upward rounding, so if there is any fractional part,
 *           we increment by one. If an overflow is detected, then
 *           the maximum clock_t value is returned. No error value
 *           is returned.
 *
 *           The purpose of this function is to allow driver objects to
 *           become independent of system parameters such as HZ, which
 *           may change in a future release or vary from one machine
 *           family member to another.
 */

clock_t
drv_usectohz(microsecs)
register clock_t microsecs;
{
	clock_t quo, rem;
	register clock_t remhz, quohz;

	if (microsecs <= 0) return (0);

	quo = microsecs / 1000000; /* number of seconds */
	rem = microsecs % 1000000; /* fraction of a second */
	quohz = HZ * quo; /* quo in HZ units */
	remhz = HZ * rem; /* remainder in millionths of HZ units */

	/* check for overflow */
	if (quo != quohz / HZ) return (MAXCLOCK_T);
	if (rem != remhz / HZ) remhz = MAXCLOCK_T;
	
	/* adjust remhz since it was in millionths of HZ units */
	remhz = (remhz % 1000000) ? remhz/1000000 + 1 : remhz/1000000;


	if ( (MAXCLOCK_T - quohz) < remhz ) return (MAXCLOCK_T);
	return (quohz + remhz);
}



/* function: drv_usecwait
 * purpose:  when a driver is in its init() routine or at interrupt
 *           level, it cannot call a function that might put it
 *           to sleep. delay() is such a function. In the past,
 *           drivers set up FOR loops of their own such as tenmicrosec()
 *           (see i386 kernel code) or used macros to busy-wait a
 *           period of time (i.e. when writing device registers).
 *           These solutions are not very portable and introduce
 *           binary compatibility problems. This function will
 *           offer the same basic functionality without detracting
 *           from the portability and binary compatibility of
 *           drivers.
 */

void
drv_usecwait(count)
clock_t count;
{
	int tens=0;

	if (count > 10)
		tens=count/10;
	tens++;			/* roundup; wait at least 10 microseconds */
	while(tens > 0){
		tenmicrosec();
		tens--;
	}
}


/*
 * Function to notify system of an event so processes sleeping
 * in poll() can be awakened.
 */
void
pollwakeup(php, event)
	register struct pollhead *php;
	register short event;
{
	register int s;
	register struct polldat *pdp;
	extern void polldel();

	s = splhi();
	if ((event == POLLHUP) || (event == POLLERR)) {
		while ((pdp = php->ph_list) != NULL) {
			(void) (*pdp->pd_fn)(pdp->pd_arg);
			polldel(php, pdp);
		}
	} else {
		while (event & php->ph_events) {
			for (pdp = php->ph_list; pdp; pdp = pdp->pd_next) {
				if (pdp->pd_events & event) {
					(void) (*pdp->pd_fn)(pdp->pd_arg);
					polldel(php, pdp);
					break;
				}
			}
		}
	}
	splx(s);
}



/* function: drv_priv()
 * purpose:  determine if the supplied credentials identify a privileged
 *           process.  To be used only when file access modes and
 *           special minor device numbers are insufficient to provide
 *           protection for the requested driver function.  Returns 0
 *           if the privilege is granted, otherwise EPERM.
 */ 

int
drv_priv(cr)
cred_t *cr;
{
	return((cr->cr_uid == 0) ? 0 : EPERM);
}


/* function: physmap()
 * purpose:  allocate a virtual address mapping for a given range of
 *           physical addresses.  Generally used from a driver's init or
 *           start routine to get a pointer to device memory (a.k.a.
 *           memory-mapped I/O).  The flags argument may be KM_SLEEP or
 *           KM_NOSLEEP.  Returns a virtual address, or NULL if the
 *           mapping cannot be allocated.
 */

addr_t
physmap(physaddr, nbytes, flags)
	paddr_t		physaddr;
	ulong_t		nbytes;
	uint_t		flags;
{
	ulong_t		base, size;
	addr_t		addr;

	flags &= KM_NOSLEEP;

	base = btop((ulong_t)physaddr);
	size = btopr((ulong_t)physaddr + nbytes) - base;

	/* sptalloc treats (base == 0) specially, so we have to handle
	   it as a special case.  We assume the request is not bigger
	   than the size of base memory. */
	if (base == 0)
		return phystokv(physaddr);

	while ((addr = (addr_t)sptalloc((int)size, PG_V|PG_PCD, (caddr_t)base,
			(int)flags)) == NULL && !(flags & KM_NOSLEEP)) {
		setmapwant(mapwant(sptmap) + 1);
		sleep((caddr_t)sptmap, PMEM);
	}
	if (addr == (addr_t)NULL)
		return (addr_t)NULL;
	else
		return addr + PAGOFF(physaddr);
}


/* function: physmap_free()
 * purpose:  release a mapping allocated by physmap().  The nbytes argument
 *           must be identical to that given to physmap().  Generally,
 *           physmap_free() will never be called, since drivers keep the
 *           mapping forever, but it is provided in case a driver wants to
 *           dynamically allocate mappings.  Currently, no flags are
 *           supported and the flags argument should be zero.
 */

void
physmap_free(vaddr, nbytes, flags)
	addr_t		vaddr;
	ulong_t		nbytes;
	uint_t		flags;
{
	uint_t		offset = PAGOFF(vaddr);

	if ((vaddr -= offset) == phystokv(0))
		return;
	sptfree(vaddr, (int)btopr(nbytes + offset), 1);
}

/* function: btoc()
 * macro in: sysmacros.h
 * purpose: convert size in bytes to size in clicks (pages)
 * FOR SCSI ONLY!! Not a documented DDI interface
 */

unsigned long
btoc(bytes)
register unsigned long bytes;
{

#ifdef BPCSHIFT
	return(((unsigned)(bytes)+(NBPC-1))>>BPCSHIFT);
#else
	return(((unsigned)(bytes)+(NBPC-1))/NBPC);
#endif

}



/* function: ctob()
 * macro in: sysmacros.h
 * purpose: convert size in clicks (pages) to bytes
 * FOR SCSI ONLY!! Not a documented DDI interface
 */

unsigned long
ctob(clicks)
register unsigned long clicks;
{
#ifdef BPCSHIFT
	return(((clicks)<<BPCSHIFT));
#else
	return(((clicks)*NBPC));
#endif
}
