/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_RAMD_H 
#define _SYS_RAMD_H

/*	Copyright (c) 1983, 1984, 1986, 1987, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)head.sys:sys/ramd.h	1.1.2.1"

/*
 * RAM disk device driver include file.
 */

struct	ramd_info {
	ulong	ramd_size;	/* Size of disk in bytes */
	ulong	ramd_flag;	/* see defs below */
	dev_t	ramd_maj;	/* major device to load from */
	dev_t	ramd_min;	/* minor device to load from */
	ulong	ramd_state;	/* runtime state  see defs below */
	caddr_t	ramd_addr;	/* Kernel virtual addr */
};

dev_t ramd_load_dev;

extern struct ramd_info ramd_info[];
extern struct buf     ramd_buf[];
extern minor_t	      ramd_num  ;
/*
 * General defines 
 */
#define RAMD_DIV_BY_512		9	/* shift for 512 divide */
#define RAMD_DIV_BY_1024	10	/* shift for 1024 divide */
#define RAMD_GRAN		0x1000	/* ramd disk transfer size*/
#define RAMD_TAPE_LOAD		0x0001	/* LOAD is from TAPE */
#define RAMD_FLOPPY_LOAD	0x0002	/* LOAD is from FLOPPY */
#define RAMD_NO_LOAD		0x0004	/* Don't LOAD */

/*
 * Flag definitions for ramd_flags 
 */
#define RAMD_STATIC	0x01		/* Ramd disk statically allocated */
#define RAMD_RUNTIME	0x02		/* Runtime definable RAM Disk */
#define RAMD_LOAD	0x04		/* Auto fill the RAM Disk at init */

/*
 * State definitions for ramd_state 
 */
#define RAMD_ALIVE	0x01		/* Disk is present */
#define RAMD_OPEN	0x02		/* Disk is open */
#define RAMD_ALLOC	0x04		/* Memory has been allocated */
#define RAMD_FAIL	0x08		/* Disk configuration is not usable */

/*
 * Ram Disk ioctl's
 */
#define RAMD_IOC_GET_INFO	0x00	/* Return ramd_info structure */
#define RAMD_IOC_R_ALLOC	0x01	/* Allocate ramd memory space */
#define RAMD_IOC_R_FREE		0x02	/* De-allocate ramd memory space*/
#define RAMD_IOC_LOAD		0x04	/* Load a ramd partition */

/*
 * Function Prototypes
 */
#ifdef  __STDC__ 
extern	int ramdAutoLoad (minor_t,daddr_t,dev_t);
extern	int ramdsize(dev_t);
extern	int ramdalloc(minor_t,daddr_t);
extern	int ramdfree(minor_t);
extern	int ramdopen(dev_t *,int,int,struct cred *);
extern	int ramdclose(dev_t,int,int,struct cred *);
extern	void ramdstrategy(struct buf *);
extern	int ramdread(dev_t, struct uio *, struct cred *);
extern	int ramdwrite(dev_t, struct uio *, struct cred *);
extern	int ramdioctl(dev_t, int, caddr_t, int, struct cred *, int *);
extern	int ramdint(dev_t);
extern	int ramdprint (dev_t,char *);
#endif /* __STDC__ */
#endif	/* _SYS_RAMD_H */
