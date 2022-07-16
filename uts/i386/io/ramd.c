/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988, 1989 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	
#ident	"@(#)kern-io:ramd.c	1.3.1.1"

static char ramd_copyright[] = "Copyright 1987, 1988, 1989 Intel Corp. 462679";

/*
 * 	TITLE:	RAM Disk Driver
 *
 * 	This driver manages "memory". Memory is defined by a
 *	info structure which gives the size of the memory to be managed.
 *	Note that for a given system, there may be restrictions on the 
 *	configuration of the address and size of the memory.
 *
 * general structure of driver:
 *	ramdinit - initialize driver. verify (fix up if necessary) the
 *		device configuration and   report errors.
 *	ramdopen - 'open' the RAM disk, and allocate memory, optionally 
 *			load from secondary device if
 *              specified.
 *	ramdclose- frees memory and closes the disk
 *	ramdstrat- do the actual work. copy in/out of ram disk memory
 *	ramdioctl- handle all of the funnies such as building roving RAM disks
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/cmn_err.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/open.h" 
#include "sys/tape.h"
#include "sys/kmem.h"
#include "sys/uio.h"
#include "sys/cred.h"
#include "sys/systm.h"
#include "sys/ramd.h"
#include "sys/ddi.h"


extern int	ramd_tape_loc;
extern int	ramd_load_type;
extern dev_t	ramd_load_dev;
extern dev_t	rootdev;
extern dev_t	swapdev;
extern dev_t	pipedev;
extern dev_t	dumpdev;

int	ramddevflag = 0;

ramdinit()
{
	minor_t	unit ;
	cmn_err(CE_CONT, "Ram Disk Driver,Copyright (c) 1986, 1987, 1988, 1989 Intel Corp.\n");

	for( unit=0;unit<ramd_num;unit++) {		/*for each RAM disk */
		ramd_info[unit].ramd_state = RAMD_ALIVE;
		if( ramd_info[unit].ramd_flag & RAMD_RUNTIME ) {
			cmn_err(CE_CONT,"RAM Disk %d runtime definable\n",unit);
			continue;
		}
	}
	return(0);
}

ramdsize(dev)
dev_t dev;
{
	minor_t unit = getminor (dev);	

	if ( unit >= ramd_num ) {
		ramdprint( dev,"ramdsize - Invalid device %x");
		return(-1);
	}
	return(ramd_info[unit].ramd_size>>RAMD_DIV_BY_512);
}

ramdalloc(unit,size)
minor_t	unit;
ulong size;
{
	_VOID *addr;
	if(ramd_info[unit].ramd_state & RAMD_ALLOC) {
		return(EINVAL);
	}

	if (addr = kmem_zalloc( size, KM_NOSLEEP)) {
		ramd_info[unit].ramd_state |= RAMD_ALLOC;
		ramd_info[unit].ramd_addr = (caddr_t)addr;
		ramd_info[unit].ramd_size = size;
		cmn_err(CE_CONT,"RAM Disk %d Allocated: size= %dKb\n",
		unit,ramd_info[unit].ramd_size>>RAMD_DIV_BY_1024);
		return (0);
	}

	/* couldn't get the required memory for the RAM disk */
	return (ENOMEM);
}

ramdfree(unit)
minor_t	unit;
{
	register struct ramd_info *dp;	/* ptr to control struct of this unit */

	dp = &ramd_info[unit];

	if(!(dp->ramd_state & RAMD_ALLOC)) 
		return(EINVAL);

	if(dp->ramd_size && dp->ramd_addr)
		kmem_free( dp->ramd_addr, dp->ramd_size);

	dp->ramd_size = 0;
	dp->ramd_addr = NULL;
	dp->ramd_state &= ~(RAMD_ALLOC);
	return(0);
}

/*
* open only checks to see if the disk is alive
*
*/
ramdopen(devp,flag,otyp,cred_p)
dev_t *devp;		/* Device Number */
int flag;		/* flag with which the file was opened */
int otyp;		/* type of open (OTYP_BLK|OTYP_CHAR| .. ) */
struct cred *cred_p;	/* pointer to the user credential structure */
{
	minor_t unit = getminor (*devp);
	struct ramd_info *dp;			
	dev_t dev = *devp;	
	
	if ( unit >= ramd_num ) {
		return(ENXIO);
	}

	dp = &ramd_info[unit];
	if (dp->ramd_state & RAMD_OPEN)
		return(0);

	if(dp->ramd_state & RAMD_ALIVE) {
		if(ramdalloc(unit,ramd_info[unit].ramd_size) != 0)
			return(ENOMEM);

		if (ramd_info[unit].ramd_flag & RAMD_LOAD) {
			ramd_load_dev =
				makedevice(ramd_info[unit].ramd_maj,ramd_info[unit].ramd_min);
			if(ramd_load(unit,ramd_info[unit].ramd_size,ramd_load_dev)){
				if (dev == rootdev ) {
					if(ramd_load_type == RAMD_TAPE_LOAD)
						cmn_err(CE_NOTE,"System cannot load boot tape, contact your service representative.");
					else
						cmn_err(CE_NOTE,"System cannot load boot floppy, contact your service representative.");
					cmn_err(CE_PANIC,"System installation aborted.");
				}
				else
					return(ENOMEM);
			}
		}
		if (dev == rootdev ) {
			if ( getmajor(dev) == getmajor(swapdev) ) {
				nswap = (ramd_info[getminor(swapdev)].ramd_size
					>> RAMD_DIV_BY_512);
			}
		}
		dp->ramd_state |= RAMD_OPEN;
		return(0);
	}
	return(ENXIO);
}


ramdclose(dev,flag,otyp,cred_p)
dev_t dev;				
int flag;			
int otyp;		
struct cred *cred_p;
{
	/*
	 * never close these
	 */
	if ((dev == rootdev) || (dev == swapdev) || (dev == pipedev)) {
		return(0);
	}
	ramd_info[getminor(dev)].ramd_state &= ~RAMD_OPEN;
	return(0);
}

void
ramdstrategy(bp)
struct buf *bp;
{
	int	count;			/* count of bytes to transfer */
	off_t	offset;			/* offset of transfer */
	register struct ramd_info *dp;	/* ptr to control structure */
	
	dp = &ramd_info[getminor(bp->b_edev)];

	if((getminor(bp->b_edev) >= ramd_num ) ||
	   ((dp->ramd_state & RAMD_OPEN) != RAMD_OPEN)){
		bp->b_flags |= B_ERROR;
		biodone(bp);
		return;
	}
		
	/* 
	 * 	request within bounds of RAM disk? 
	 *	if it is set b_resid to 0 else set it to b_count
	 * 	and leave.
	 */
	offset = bp->b_blkno*NBPSCTR;
	if( offset >= dp->ramd_size ) {
		bp->b_resid = bp->b_bcount;
		biodone(bp);
		return;
	}else{
		bp->b_resid = 0;
	}

	/* 
	 * compute transfer size
	 * if larger than size of ramdisk just write 
	 * what we can, and set b_resid to the rest.
 	 */
	count = bp->b_bcount;
	if( offset+bp->b_bcount > dp->ramd_size ) {
		count = dp->ramd_size - (ulong)offset;
		bp->b_resid = bp->b_bcount - count;
	}

	/* perform the copy */

	if(bp->b_flags & B_READ)
		bcopy(dp->ramd_addr+offset, bp->b_un.b_addr, count);
	else
		bcopy(bp->b_un.b_addr, dp->ramd_addr+offset, count);

	biodone(bp);
	return;
}


/* 
 * raw read, use physio
 */
ramdread(dev, uio_p, cred_p)
dev_t	dev;
struct uio *uio_p;	
struct cred *cred_p;
{
	return(physiock(ramdstrategy, NULL, dev, B_READ, ramdsize(dev),uio_p));
}

/* 
 * raw write, use physio
 */
ramdwrite(dev, uio_p, cred_p)
dev_t	dev;
struct uio *uio_p;	
struct cred *cred_p;
{
	return(physiock(ramdstrategy, NULL, dev, B_WRITE, ramdsize(dev),uio_p));
}

/* 
 * Ioctl routine required for RAM disk 
 */
/*ARGSUSED*/
ramdioctl(dev, cmd, cmdarg, mode, cred_p, rval_p)
dev_t	dev;		/* major, minor numbers */
int	cmd;		/* command code */
caddr_t cmdarg;		/* user structure with parameters */
int	mode;		/* value set when device was openned (not used) */
struct cred *cred_p;	/* pointer to the user credential structure */
int *rval_p;		/* pointer to return value for calling process */
{
	minor_t unit = getminor(dev);	
	struct ramd_info ramd_stats; 
	ulong ramd_size;		
	int ramd_error = 0;	

	if ( unit >= ramd_num ) {
		return( ENXIO );
	}

	switch (cmd) {
		case RAMD_IOC_GET_INFO:
			if ( copyout( (caddr_t)&ramd_info[unit], cmdarg, sizeof(struct ramd_info))) {
				ramd_error = EFAULT;
				break;
			}
			break;

		case RAMD_IOC_R_ALLOC:
			if (copyin(cmdarg, (caddr_t)&ramd_size, 
				(unsigned)sizeof(unsigned long))) {
				ramd_error = EFAULT;	/* can't access */
				break;
			}
			ramd_error = ramdalloc(unit,ramd_size);
			break;

		case RAMD_IOC_R_FREE:
			ramd_error = ramdfree(unit);
			break;

		case RAMD_IOC_LOAD:
			if (copyin(cmdarg, (caddr_t)&ramd_size, 
				(unsigned)sizeof(unsigned long))) {
				ramd_error = EFAULT;	/* can't access */
				break;
			}
			ramd_load_dev = 
				makedevice(ramd_info[unit].ramd_maj,ramd_info[unit].ramd_min);
			(void) ramd_load (unit,ramd_size,ramd_info[unit].ramd_min);
			break;

		default:
			ramd_error = EINVAL;	/* bad command */
			break;
	}
	return(ramd_error);
}


/*
 * Print error message; called from Unix kernel via bdevsw
 */
ramdprint (dev,str)
dev_t	dev;
char	*str;
{
	cmn_err(CE_NOTE, "%s on  Ram Disk partition %d\n",str,getminor(dev));
}
int
ramd_load (unit,size,dev)
minor_t	unit;
ulong size;
dev_t dev;
{
	int error, i, sect, remcount;
	ulong remainder;
	struct cred cred;
	major_t maj = getmajor(dev);

	/* open the auto load driver */


	error = (*bdevsw[maj].d_open)(&dev, 0, OTYP_CHR , &cred);
	if (error) {
		(void)(*bdevsw[maj].d_close)(dev, 0, OTYP_CHR,&cred);
		return(error);
	}

	if(ramd_load_type == RAMD_TAPE_LOAD){
		error = (*cdevsw[maj].d_ioctl)(dev, T_RWD, 0, 0, 0, 0);
		error = (*cdevsw[maj].d_ioctl)(dev, T_SFF, ramd_tape_loc,0,0,0);
		if (error){
			ramdprint(dev,"Failed seek to file mark");
			return (ENXIO);
		}
	}

	if(ramd_load_type == RAMD_TAPE_LOAD){
		error = ramd_issue_read(size, ramd_info[unit].ramd_addr,
			0L, dev, unit,maj);
		error = (*cdevsw[maj].d_ioctl)(dev, T_SFF, 0, 0, 0, 0);

	} else { 	/* Floppy */
           if ((remainder = size % RAMD_GRAN) != 0)
		size = (size / RAMD_GRAN) * RAMD_GRAN;
	   for (i=0,sect=0; (((i*RAMD_GRAN+1) < size) && !error); i++,sect+=8) {
		error = ramd_issue_read(RAMD_GRAN, ramd_info[unit].ramd_addr + 
			(i * RAMD_GRAN), sect, dev, unit, maj);
		remcount = i;
	   }
           if ((remainder > 0) && !error)
		error = ramd_issue_read(remainder, ramd_info[unit].ramd_addr +
			(++remcount * RAMD_GRAN), sect, dev, unit, maj);
	}

	/* close the driver */

	(void)(*bdevsw[maj].d_close)(dev, 0, OTYP_CHR,&cred);
	return(error);
}

int
ramd_issue_read(rdsize, addr, sect, dev, unit, maj)
ulong rdsize;
caddr_t addr;
int sect;
dev_t dev;
minor_t unit;
major_t maj;
{
	extern struct buf ramd_buf[];
	struct buf *bp = &ramd_buf[unit];
	int error;

	/* Setup the request buffer.  */
	bp->b_flags = B_READ|B_BUSY;
	bp->b_error = 0;
	bp->b_blkno = sect;
	bp->b_sector = sect;
	bp->b_edev = dev;
	bp->b_proc = 0;
	bp->b_bcount = rdsize;
	bp->b_resid = 0;
	bp->b_un.b_addr = addr;

	/* perform the read request */
	(*bdevsw[maj].d_strategy)(bp);

	biowait(bp);
	return(geterror(bp));
}
