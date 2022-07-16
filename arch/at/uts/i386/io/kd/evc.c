/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/kd/evc.c	1.1"

/* evc.c - EVC-1 kd driver support */

/* 
 * Copyright 1989 Ing. C. Olivetti & C. S.p.A.
 * All rights reserved.
 */

/* Created:  5-Jun-89 Mike Slifcak */
/* Revision History:
 *  7-Jun-89 MJS First pass.
 *  8-Jun-89 MJS debugged color map write and read routines.
 *		 removed most macro dependencies.
 * 17-Jun-89 MJS changed evc_init to return qualified mode.
 * 29-Aug-89 MJS minor changes for prototyping.
 */

#ifdef	EVC
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/inline.h"
#include "sys/cmn_err.h"
#include "sys/vt.h"
#include "sys/at_ansi.h"
#include "sys/kd.h"
#include "sys/stream.h"
#include "sys/termios.h"
#include "sys/strtty.h"
#include "sys/stropts.h"
#include "sys/xque.h"
#include "sys/ws/ws.h"
#include "sys/vid.h"
#include "sys/vdc.h"

extern struct vdc_info	Vdc;

#include "sys/evc.h"

struct evc_state {
	int state;
	int montype;
	unsigned int enable;
	unsigned int control;
	unsigned int config;
	unsigned long mem;
};

struct evc_state Evc = { 0 };


/* evc_check ***********************************
 * Searches EISA slots 1 through 15 for EVC-1 board.
 * if found, sets up evc info structure.
 * Returns 1 if successful, else returns 0.
**********************************************/

int evc_check()
{
register unsigned int base;
	for (base = 0x1000; base <= 0xF000; base += 0x1000)
	{
		if (
		(inb(base+OfsEVC1BoardIdReg0) == EVC1Id0) &&
		(inb(base+OfsEVC1BoardIdReg1) == EVC1Id1) &&
		(inb(base+OfsEVC1BoardIdReg2) == EVC1Id2) &&
		(inb(base+OfsEVC1BoardIdReg3) == EVC1Id3)
		) {
			Evc.state = 2;
			Evc.enable	= base + OfsEVC1BoardEnableReg;
			Evc.control	= base + OfsEVC1BoardControlReg;
			Evc.config	= base + OfsEVC1BoardConfigReg;
			Evc.mem		= EVC_BASE;
			return (1);
		}
	}
	Evc.state = 1;
	return (0);
}

/* evc_reset **********************************
 * resets the EVC-1 board, reads monitor type. 
 * Returns 1 if successful, else returns 0.
**********************************************/

static int evc_reset()
{
register int j,k;
	if (inb(Evc.enable) & EISA_IOCHKERR) {	/* NOT IMPLEMENTED. I/O error recovery */
		return (0);
	}
	outb(Evc.enable,EISA_STARTRS);
	for (j=0; j< 20;j++ )		/* delay AT LEAST 500 nanoseconds */
	{
		k = Evc.enable; Evc.enable++ ; Evc.enable++; Evc.enable = k;
	}
	outb(Evc.enable,EISA_STOPRS);
	outb(Evc.enable,EISA_ENABLE);
	Evc.state = 3;
	outb(Evc.config, EVC1Bus8);
	Evc.montype = (inb(Evc.control) & EVC1MonitorMask) >> 4;

	return (1);
}

/* evc_init ***********************************
 * finds EVC-1, initializes it, and determines the monitor type.
 * presets mode qualified by the monitor type.
 * return input mode if no EVC-1 OR EVC-1 and mode is appropriate.
 * return DM_VGA_C80x25 if EVC-1 AND hi-res mode attempted on low-res monitor.
**********************************************/

int evc_init(mode)
int mode;
{
int retcode = mode;
void evc_setup();
	if (!Evc.state) {
		if (!evc_check()) {
			return retcode; /* No EVC present */
		}
	}
	if (!evc_reset())
		return retcode;

	switch (mode) {
		case DM_EVC1024x768E:
		case DM_EVC1024x768D:
			switch (Evc.montype) {
				case MONHiRMono:
				case MONHiRColor:
					evc_setup(mode);
					break;
				default:
					evc_setup(DM_VGA_C80x25);
					retcode = DM_VGA_C80x25;
					break;
			}
			break;
		default:
			evc_setup(mode);
			break;
	}

	return retcode;
}

/************************************************************
  The extended registers are reset to their initial values
  when the board is reset (above).  Set the necessary values
  for all modes first, then the extras for the indicated modes.
************************************************************/

#define VGAExtendedIndex	0x3D6
#define	VGAExtendedData		0x3D7
#define	VGAFeatureControl	0x3DA

static
unsigned short ExtRegsInit[] = {
	0x0604,
	0x6E06,
	0x010B,
	0x0028,
	0x007F,
	0x0000
};

static
unsigned short ExtRegs640x480V[] = {
	0x050B,
	0x0000
};

static
unsigned short ExtRegs1024x768E[] = {
	0x0228,
	0x0000
};

static
unsigned short ExtRegs1024x768D[] = {
	0x0228,
	0x027F,
	0x0000
};


/* evc_setup *****************************
 * local routine that sets up EVC-1.
 * mode must be qualified by the monitor type.
**********************************************/

static void evc_setup(mode)
int mode;
{
	outb(Evc.config, EVC1Bus8);
	outb(Evc.control,EVC1StartSetup);
	outb(0x102,1);
	outb(0x103,0x80);
	outb(Evc.control,EVC1StopSetup);
	outb(VGAExtendedIndex, 2); outb(VGAExtendedData, 3);
/*XXX*/	outb(Evc.config, EVC1Bus16);	/**/
}

void evc_finish(mode)
int mode;
{
register unsigned short * pp;

	pp = ExtRegsInit;
	while (*pp) { outw( VGAExtendedIndex, *pp ); pp++; }
	switch (mode) {
		case DM_EVC640x480V:
			pp = ExtRegs640x480V;
			while (*pp) { outw( VGAExtendedIndex, *pp ); pp++; }
			outb(VGAFeatureControl,1);
			break;
		case DM_EVC1024x768E:
			pp = ExtRegs1024x768E;
			while (*pp) { outw( VGAExtendedIndex, *pp ); pp++; }
			outb(VGAFeatureControl,2);
			break;
		case DM_EVC1024x768D:
			pp = ExtRegs1024x768D;
			while (*pp) { outw( VGAExtendedIndex, *pp ); pp++; }
			outb(VGAFeatureControl,2);
			outb(Evc.control,EVC1DirectHIGH);
			break;
		default:
			outb(VGAFeatureControl,0);
			break;
	}
}

/*
 * fill in Vdc info structure.
 * returns 0 if no EVC, or if EVC reset failed.
 */

evc_info(vp)
vidstate_t	*vp;
{
	static restart;

	if (!Evc.state) {
		if (!evc_check()) {
			return 0; /* No EVC present */
		}
	}
	if (!restart) {
	  if (!evc_reset())
		return 0;
	}
	restart = 1;

	switch (Evc.montype) {
		case MONVGAMono:
			Vdc.v_info.dsply = KD_STAND_M;
			break;
		case MONVGAColor:
			Vdc.v_info.dsply = KD_STAND_C;
			break;
		case MONHiRMono:
			Vdc.v_info.dsply = KD_MULTI_M;
			break;
		case MONHiRColor:
			Vdc.v_info.dsply = KD_MULTI_C;
			break;
		default:
			Vdc.v_info.dsply = KD_UNKNOWN;
			break;
	}
	Vdc.v_type = VEVC;
	Vdc.v_info.cntlr = KD_VGA;
	return 1;
}

#endif	/*EVC*/
