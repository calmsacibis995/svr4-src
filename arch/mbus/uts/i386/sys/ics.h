/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_ICS_H
#define _SYS_ICS_H

/*	Copyright (c) 1986, 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/ics.h	1.3.1.1"

#ifndef _SYS_CRED_H
#include "sys/cred.h"
#endif

struct ics_struct {
	unsigned short slot_id;	/* slot id of the target board */
	unsigned short reg_id;	/* register to be read */
	unsigned short count;	/* number of registers to read */
	unsigned char buffer[1];		/* data buffer */
};

struct ics_rw_struct {
	unsigned short slot_id;	/* slot id of the target board */
	unsigned short reg_id;	/* register to be read */
	unsigned short count;	/* number of registers to read */
	unsigned char *buffer;		/* data buffer */
};

#define ICS_MAX_SLOT 21
#define ICS_MAX_REG 511

#define ICS_PSB_CONTROL_TYPE	0x06
#define ICS_PARITY_CONTROL_TYPE	0x04
#define ICS_FW_COM_REG	0x0F
#define	ICS_BOARD_SPECIFIC_REC	0xFE
#define ICS_EOT_TYPE	0xff
#define ICS_HOST_ID_TYPE	0x10
#define ICS_LOCAL_PROC_REC_TYPE	19
#define ICS_MY_SLOT_ID	0x1f

/*
 *	Defines related to ICS_LOCAL_PROC_REC_TYPE
*/
#define ICS_RESET_OFFSET	(ICS_DATA_OFFSET+0)
#define ICS_RESET_MASK		0x01

#define ICS_SLOT_ID_OFFSET	2


#define ICS_DATA_OFFSET	2
#define ICS_PC16_STATUS_OFFSET 3
#define ICS_PC16_CONFIG_OFFSET 6

#define ICS_WRITE_ICS 	4
#define ICS_READ_ICS 	5
#define ICS_FIND_REC	10
#define ICS_MY_SLOTID	11
#define ICS_MY_CPUNUM	12

#define ICS_LO(x)	((x)&0xFF)
#define ICS_HI(x)	(((x)>>8)&0xFF)

#define ICS_PSB_SLOT_ID_REG 0x2D

#define ICS_ENABLE_NMI	0x80
#define ICS_DEBUG_NMI	0x02
#define ICS_DEBUG_MASK	0x02
#define ICS_SW_NMI		0x04
#define ICS_SWNMI_MASK	0x04
#define ICS_PARITY_MASK	0x80
#define ICS_PARITY_OFFSET	0x02

#define NAMESZ 11
typedef struct slot {
	unsigned short s_hostid;
	unsigned char  s_msgid;
	unsigned char  s_pcode[NAMESZ];
} ics_slot_t;


/* Interconnect space register offset for header record */
#define ICS_VendorIDL 0
#define ICS_VendorIDH 1
#define ICS_ProductCode 2		/* 10 char product "name" */
#define ICS_HardwareTestRev 16
#define ICS_ClassID 17
#define ICS_ProgramTableIndex 22
#define ICS_NMIEnable 23
#define ICS_GeneralStatus 24
#define ICS_GeneralControl 25
#define ICS_BistSupportLevel 26
#define ICS_BistDataIn 27
#define ICS_BistDataOut 28
#define ICS_BistSlaveStatus 29
#define ICS_BistMasterStatus 30
#define ICS_BistTestID 31

/*
 *	Here are some defines related to ICS_ProgramTableIndex:
*/
#define ICS_FW_INDEX	0x00

/* defintion of ioctl records passed form user */
struct ics_rdwr_args{
	unsigned short	ics_slot;	/* Slot number */
	unsigned short	ics_reg;	/* Register number */
	caddr_t			ics_buf;	/* Buffer to read from/write to */
	int				ics_count;	/* Number of bytes to read/write */
};

struct ics_frec_args{
	unsigned short	ics_rval;	/* Value returned */
	unsigned short	ics_slot;	/* Slot number */
	unsigned short	ics_recid;	/* Record ID to search for */
};

/* table to reconfigure UNIX for multiple cpu systems */
struct ics_bdev {
	major_t	rootdev_major;
	minor_t	rootdev_minor;
	major_t	swapdev_major;
	minor_t	swapdev_minor;
	major_t	pipedev_major;
	minor_t	pipedev_minor;
	major_t	dumpdev_major;
	minor_t	dumpdev_minor;
};
#ifdef _KERNEL
#if defined __STDC__
/*
 * Since the prototype definitions are only allowed word parameters, all
 * shorts and char's have been changed to int in the prototypes only
 */
extern int 	ics_cpunum();
extern unsigned char ics_myslotid();
extern int 	icsinit();
extern int 	icsopen (dev_t *, int, int, struct cred *);
extern int 	icsclose (dev_t *, int, int, struct cred *);
extern int 	ics_rdwr(int, struct ics_rw_struct *);
extern int 	ics_rw (int, struct ics_struct *);
extern int 	ics_hostid();
extern int 	icsioctl(dev_t, int, caddr_t, int, struct cred *, int *);
extern int 	ics_agent_cmp(char *[], unsigned int);
extern int 	ics_autoconfig();
extern int 	ics_find_rec(unsigned int, unsigned int);
extern int 	ics_read(unsigned int, unsigned int);
extern int 	ics_write(unsigned int, unsigned int, unsigned int);
#else
extern int 	ics_cpunum();
extern unsigned char ics_myslotid();
extern int 	icsinit();
extern int 	icsopen ();
extern int 	icsclose ();
extern int 	ics_rdwr();
extern int 	ics_rw ();
extern int 	ics_hostid();
extern int 	ics_find_rec();
extern int 	ics_read();
extern int	ics_write();
extern int 	icsioctl();
extern int 	ics_agent_cmp();
extern int 	ics_autoconfig();
#endif
#endif /* _KERNEL */

#endif	/* _SYS_ICS_H */
