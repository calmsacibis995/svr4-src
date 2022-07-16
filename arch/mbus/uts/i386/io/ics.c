/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987, 1988, 1988, 1989 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */

#ident	"@(#)mbus:uts/i386/io/ics.c	1.3.1.1"

#ifndef lint
static char ics_copyright[] = "Copyright 1986, 1987, 1988, 1989 Intel Corporation 460945";
#endif /* lint */

/*
 *
 *  Multibus II Message Device Driver
 *
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/cmn_err.h"
#include "sys/cred.h"
#include "sys/ics.h"
#include "sys/ddi.h"

/* external variables which are configured in */

extern unsigned long ics_data_addr, ics_low_addr, ics_hi_addr;
extern char *ics_cpu_cfglist[];  /* Initialized in cpu driver */
extern ics_slot_t ics_slotmap[];
extern struct ics_bdev ics_bdev[];
extern long ics_max_numcpu;
extern dev_t 	rootdev; 
extern dev_t 	pipedev;
extern dev_t 	swapdev;
extern dev_t 	dumpdev;

static build_slotmap();

/*
 * globals 
 */

short ics_alive;
static unsigned char myslotid; /* PSB slot id set by icsinit */
static int	cpunum = 0;	/* Cpu number computed at boot time */

int
ics_cpunum()
{
	return(cpunum);
}

int icsdevflag = 0;	/* V4 DDI/DKI driver */

unsigned char
ics_myslotid()
{
	return(myslotid);
}

/*
 *	Interconnect Space initialization. icsinit reads the value
 *	of the cardslot ID and initializes the ICS registers specified
 *	in the configuration.
 */

icsinit()
{	struct ics_struct ics;
	unsigned short	i;

	if (ics_alive)		/* if init has been completed ok */
		return;
	ics_alive++;
	/*
	 * look for a PSB CONTROL RECORD to get the cardslot ID
	 */
	i = ics_find_rec(ICS_MY_SLOT_ID, ICS_PSB_CONTROL_TYPE);
	if (i != 0xffff) {
		/* get 5 bit slot id */
		myslotid = ics_read(ICS_MY_SLOT_ID, i+ICS_SLOT_ID_OFFSET) >> 3;
		if (myslotid > ICS_MAX_SLOT) 
			/*
			 * An illegal cardslot was returned. 
			 * How did this board pass the power up diagnostics.
			 */
			cmn_err (CE_PANIC, "Interconnect cardslot ID error");
		 
	} else 
		cmn_err(CE_PANIC, "No interconnect PSB CONTROL RECORD");

	build_slotmap();
	/* Enable local NMI sources */
	ics_write(myslotid,ICS_NMIEnable,ICS_ENABLE_NMI);
}


/* ARGSUSED */
icsopen (dev, flag, otyp, cred_p)
dev_t *dev;
int flag, otyp;
struct cred *cred_p;
{
	return (0);
}

/* ARGSUSED */
icsclose (dev, flag, otyp, cred_p)
dev_t *dev;
int flag, otyp;
struct cred *cred_p;
{
	return (0);
}

/*
 *	ics_rdwr reads or writes a specified number of Interconnect 
 *	Space registers from a given cardslot ID.
 */

ics_rdwr(cmd, addr)
int cmd;
struct ics_rw_struct *addr;
{	unsigned reg, ics_addr;
	unsigned short slot;
	int i;

	slot = (addr->slot_id == myslotid) ? ICS_MY_SLOT_ID : addr->slot_id;
	reg= addr->reg_id;
	for (i=0; i < (int)addr->count; i++) {
		if ( reg > ICS_MAX_REG) {
			return (EINVAL);
		}
		ics_addr = ( (slot & 0x1f) << 11) | ( reg++ << 2);
		outb( ics_hi_addr, ICS_HI(ics_addr));
		outb( ics_low_addr, ICS_LO(ics_addr));
		if ( cmd == ICS_READ_ICS )
			addr->buffer[i]= inb(ics_data_addr);
		else
			outb(ics_data_addr, addr->buffer[i]);
	}
	return (0);
}

/*
 *	ics_rw reads or writes a specified number of Interconnect 
 *	Space registers from a given cardslot ID.
 *
 *	Since the buffers used are only one byte long, this routine
 *	should be oboleted in favor of ics_rdwr() (vrs).
 */
ics_rw (cmd, addr)
int cmd;
struct ics_struct *addr;
{	
	unsigned reg, ics_addr;
	unsigned short slot;
	int i;

	slot= ((addr->slot_id == myslotid) ? ICS_MY_SLOT_ID : addr->slot_id);
	reg= addr->reg_id;
	for (i=0; i < (int)addr->count; i++) {
		if ( reg > ICS_MAX_REG) {
			return (EINVAL);
		}
		ics_addr= ( (slot & 0x1f) << 11) | ( reg++ << 2);

		outb( ics_hi_addr, ICS_HI(ics_addr));
		outb( ics_low_addr, ICS_LO(ics_addr));
		if ( cmd == ICS_READ_ICS )
			addr->buffer[i]= inb(ics_data_addr);
		else
			outb(ics_data_addr, addr->buffer[i]);
	}
	return (0);
}

/*
 *  ics_hostid returns the host id field of the HOST ID record in 
 *  this boards Interconnect Space.
 */

ics_hostid()
{	
	unsigned id_reg;
	struct ics_struct *ics;
	unsigned buf[4];

	/*
	 * first find the HOST ID record
	 */
	id_reg= ics_find_rec(ICS_MY_SLOT_ID, ICS_HOST_ID_TYPE);
	if ( id_reg == 0xffff ) 
		return(0);			/* record not found */
	ics= (struct ics_struct *)buf;
	ics->slot_id= ICS_MY_SLOT_ID;
	ics->reg_id= id_reg + ICS_DATA_OFFSET;
	ics->count= 2;
	ics_rw(ICS_READ_ICS, ics);

	return((ics->buffer[1] & 0xff) | (ics->buffer[2] << 8));
}

/*
 *  ics_find_rec returns the register offset of the TYPE REGISTER of
 *  the FUNCTION RECORD being searched for.
 */
int
ics_find_rec(slot, record)
unsigned char slot;
unsigned char  record;
{	
	struct ics_struct *ics;
	unsigned reg;
	unsigned buf[4];

	/*
	 * Start scanning the registers at offset 32.
	 * Stop on a match or when an End of Template is read.
	 */

	reg= 0;
	ics= (struct ics_struct *)buf;
	ics->slot_id= slot;
	ics->reg_id= 32;
	ics->count= 2;
	do {
		ics_rw(ICS_READ_ICS, ics);
		if ( ics->buffer[0] == record ) 
			reg= ics->reg_id;
		else if ( ics->buffer[0] == (unsigned char)ICS_EOT_TYPE )
			reg= 0xffff;
		else {
			ics->reg_id += (((unsigned)ics->buffer[1]) & 0xff)+ICS_DATA_OFFSET;
			if(ics->reg_id > 0xff)	/* Don't run off the end */
				reg = 0xffff;
		}
	} while ( reg == 0 );

	return(reg);
}

/* ARGSUSED */
icsioctl(dev, cmd, addr, mode, cred_p, rval_p)
dev_t dev;
int cmd;
caddr_t addr;
int mode;
struct cred *cred_p;
int *rval_p;
{
	struct ics_struct ics;
	caddr_t uaddr;
	int count;
	int ret;

	if (ics_alive == 0) {
		return (ENODEV);
	}
	switch (cmd) {
		case ICS_READ_ICS:
		case ICS_WRITE_ICS:
			{	
			   struct ics_rdwr_args ioc;
				if(copyin(addr, &ioc, sizeof(struct ics_rdwr_args))) {
					return(EFAULT);
				}
				ics.slot_id = ioc.ics_slot;
				ics.reg_id = ioc.ics_reg;
				ics.count = 1;
				count = ioc.ics_count;
				uaddr =  ioc.ics_buf;
				do {
					if(cmd == ICS_READ_ICS) {
						if ((ret = ics_rw(ICS_READ_ICS,&ics)) != 0)
							return (ret);
						if(copyout(&ics.buffer[0], uaddr++,1)) 
							return (EFAULT);
					} else {
						if(copyin(uaddr++,&ics.buffer[0],1)) 
							return (EFAULT);
						if ((ret = ics_rw(ICS_WRITE_ICS,&ics)) != 0)
							return (ret);
					}
					ics.reg_id++;
				} while(--count);
				ioc.ics_count -= count;
				if(copyout(&ioc, addr, sizeof(struct ics_rdwr_args))) {
					return(EFAULT);
				}
			}
			break;
		case ICS_FIND_REC:
			{	
  			   struct ics_frec_args ioc;
				if(copyin(addr, &ioc, sizeof(struct ics_frec_args))) 
					return (EFAULT);
				ioc.ics_rval = ics_find_rec(ioc.ics_slot,ioc.ics_recid);
				if(copyout(&ioc,addr,sizeof(struct ics_frec_args))) 
					return (EFAULT);
			}
			break;
		case ICS_MY_SLOTID:
			if(copyout(&myslotid,addr,sizeof(unsigned char))) 
				return (EFAULT);
			break;
		case ICS_MY_CPUNUM:
			if(copyout(&cpunum,addr,sizeof(unsigned char))) 
				return (EFAULT);
			break;
		default:
			return (EINVAL);
			break;
	}
}


/*
 * This routine builds a map of the boards in the system
 * For each slot
 *	Try reading the Vendor ID
 *	If vendor ID == 0 
 *		no board exists in that slot.
 *	else
 *		read in Product code and store it in table.
 *		Try finding Host ID record from the interconnect space
 *		if host ID record is found
 *			read in Host ID and Message Id
 *	If there is no board in a slot, its product code is null.
 *	if there is no host ID record , host ID and Message ID
 *	is -1.
 */
static
build_slotmap ()
{
	register int i;
	register int j;
	struct ics_rw_struct ics;
	unsigned short	regno;
	unsigned short	x;
	unsigned char slot;
	unsigned short vendor;


	for(i=0;i<ICS_MAX_SLOT;i++) {
		slot = ( i == myslotid ? ICS_MY_SLOT_ID : i);
		vendor = ics_read(slot, 0);
		vendor += ics_read(slot, 1) << 8;
		if(vendor == 0) {
			ics_slotmap[i].s_pcode[0] = '\0';
			ics_slotmap[i].s_hostid = 0xffff;
			ics_slotmap[i].s_msgid = 0xff;
			goto bottom;
		}
		ics.slot_id = slot;
		ics.count = NAMESZ - 1;
		ics.reg_id = ICS_DATA_OFFSET;
		ics.buffer = ics_slotmap[i].s_pcode;
		ics_rdwr(ICS_READ_ICS, &ics);
		ics_slotmap[i].s_pcode[NAMESZ-1] = '\0';
		regno = ics_find_rec(slot,ICS_HOST_ID_TYPE) + ICS_DATA_OFFSET;
		if(regno == 0xffff) {
			ics_slotmap[i].s_hostid = 0xffff;
			ics_slotmap[i].s_msgid = 0xff;
			goto bottom;
		}
		x = ics_read(slot, regno);
		x += ics_read(slot, regno+1) << 8;
		ics_slotmap[i].s_hostid = x;
		ics_slotmap[i].s_msgid = ics_read(slot, regno+2);
bottom:		;
	}
}

/*   ics_read: read the contents of an interconnect register
 *      This exists to provide an easy kernel interface to IC
 */
int
ics_read(slot, reg)
unsigned char slot;
unsigned short reg;
{	struct ics_rw_struct istr;
	unsigned char value;

	istr.slot_id = slot;
	istr.reg_id = reg;
	istr.count = 1;
	istr.buffer = &value;
	ics_rdwr(ICS_READ_ICS, &istr);
	return(value);
}

/*   ics_write: write the contents of an interconnect register
 *      This exists to provide an easy kernel interface to IC
 */
ics_write(slot, reg, value)
unsigned char slot;
unsigned short reg;
unsigned char value;
{	struct ics_rw_struct istr;

	istr.slot_id = slot;
	istr.reg_id = reg;
	istr.count = 1;
	istr.buffer = &value;
	return(ics_rdwr(ICS_WRITE_ICS, &istr));
}

/*
 *  This routine allows each driver to detect functionally identical agents
 *  by matching driver-specific strings against the board ID in interconnect
 *  space.  This is invoked by the i224a, i258, ics, and 410 drivers, whose
 *  comparison strings are contained in their configuration files.
 */

int
ics_agent_cmp(name_list, slot)
char        **name_list;
unsigned char slot;
{
	while (*name_list) {
		if (strcmp(*name_list, ics_slotmap[slot].s_pcode) == 0)
			return 0;
		name_list++;
	}
	return 1;
}


/* this routine handles the auto configuration of UNIX kernel
 * For now, all it does is figures out which CPU you are
 * and assigns the rootdev and pipedev out of a table
 */
extern dev_t pipedev;   /* why is this the only one needed ?? */

ics_autoconfig()
{	
	unsigned char slot;
	struct ics_bdev *tdev;

	for(slot=0; slot < myslotid; slot++) {
		if (ics_agent_cmp(ics_cpu_cfglist, slot) == 0)
			cpunum++;
	}
	if(cpunum >= ics_max_numcpu)
		cmn_err(CE_PANIC,"kernel not configured for %x CPUs", cpunum);
	if(getminor(rootdev) != getminor(NODEV)) 
		return;
	/* cpunum contains the CPU # */
	tdev = &ics_bdev[cpunum];
	rootdev = makedevice (tdev->rootdev_major, tdev->rootdev_minor);
	pipedev = makedevice (tdev->pipedev_major, tdev->pipedev_minor);
	swapdev = makedevice (tdev->swapdev_major, tdev->swapdev_minor);
	dumpdev = makedevice (tdev->dumpdev_major, tdev->dumpdev_minor);
}
