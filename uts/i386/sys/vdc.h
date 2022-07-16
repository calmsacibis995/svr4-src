/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/vdc.h	1.1.1.1"

#ifndef	_SYS_VDC_H
#define	_SYS_VDC_H

struct vdc_info {
	int	v_type;
	unchar	v_switch,
		v_mode2sel;
	struct	kd_vdctype
		v_info;
};

#define VTYPE(x)	(Vdc.v_type & (x))	/* Type of VDC card (if any) */
#define VSWITCH(x)	(Vdc.v_switch & (x))	/* VDC switch settings */

#define V400	1
#define V750	2
#define V600	4
#define CAS2	8 	/* indicates a Cascade 2 w/ on-board VGA */
#ifdef	EVC
#define VEVC	0x10	/* Olivetti EVC-1 is installed */
#endif	/*EVC*/
#ifdef EVGA
#define VEVGA   0x20    /* Extended VGA type board */
#endif	/*EVGA*/

#define V750_IDADDR	0xc0009
#define V600_IDADDR	0xc0035
#define CAS2_IDADDR	0xe0010

#define ATTCGAMODE	0x01
#define ATTDISPLAY	0x02

#endif /* SYS_VDC_H */
