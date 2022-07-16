/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)kern-os:oem.c	1.3.2.2"

#include "sys/types.h"
#if defined (MB1) || defined (MB2)
#include "sys/param.h"
#include "sys/sysmacros.h"
#endif /* MB1 */
#include "sys/immu.h"
#if defined (MB1) || defined (MB2)
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/seg.h"
#include "sys/mount.h"
#include "sys/inode.h"
#include "sys/proc.h"
#include "sys/var.h"
#include "sys/debug.h"
#include "sys/utsname.h"
#include "sys/conf.h"
#include "sys/inline.h"
#ifdef WEITEK
#include "sys/weitek.h"
#endif
#include "sys/fp.h"
#endif /* MB1 */
#include "sys/cram.h"
#include "sys/cmn_err.h"
#include "sys/pit.h"

#define	IO_ROM_INIT	0xfff0
#define	IO_ROM_SEG	0xf000
#define	RESET_FLAG	0x1234
#define	SHUT_DOWN	0x8f
#define	SHUT5		5
#define	STAT_8042	0x64

#ifdef AT386
extern int eisa_bus, sanity_clk;
#endif

/*
 * general hook for things which need to run prior to turning
 * on interrupts.
 */
oem_pre_clk_init()
{
#if defined (MB2) || defined(MB2AT)
		mpsinit();
		ics_autoconfig();
#endif
#if defined (AT386)
		oem_fclex();	/* Assure NDP busy latch is clear */
#endif

}

/*
 * oeminit() - oem specific initialization.
 *
 * This routine is called after the system initialization
 * functions have been called (io_init[]) and after interrupts
 * have been enabled.  This routine is called from init_tbl[].
 */
oeminit()
{
#if defined (MB1) || defined (MB2)
#define FLOPPY_MINOR 76
#define FLOPPY_MAJOR 0
		/*
		 * If rootdev is on a floppy (major == 0 and minor == 84),
		 * ask user to insert the rootfs floppy.
		 */
		if ((major(rootdev) == FLOPPY_MAJOR)  && 
			(minor(rootdev) == FLOPPY_MINOR)) {
			int c;
			cmn_err(CE_CONT,"\nPlease insert the Root FS Floppy");
			cmn_err(CE_CONT,"\nand strike <RETURN> ");
			intr_disable();
 			while ((c=conssw.ci()) == -1) {
 				c = 100000;
 				while (--c); /* delay a bit */
 			}
			intr_restore();
			cmn_err(CE_CONT,"\n");
		}
#endif
}

/*
 * oem_doarg(s) - oem specific boot argument parsing
 *
 * This routine is called after first checking an argument
 * against the standard set of supported arguments.  Unknown
 * (non-standard) arguments will be passed to this routine.
 *
 * Return Values:	 0 if argument handled
 *			-1 if argument unknown
 */	 
oem_doarg(s)
char *s;
{
	return(-1);		/* unknown argument */
}

/*
 * Intended use for flag values:
 * 	flag == 0	halt the system and wait for user interaction
 * 	flag >= 1	automatic reboot, no user interaction required
 */
oemreboot(flag)
int flag;
{
#ifdef AT386
        /* if sanity timer in use, turn off to allow clean soft reboot */
	if (eisa_bus && sanity_clk)
		outb(SANITY_CHECK, RESET_SANITY);
#endif
	if (flag)	
		cmn_err(CE_CONT, "\nAutomatic Boot Procedure\n");
	else{
		dhalt();
		rmc_pwroff();	/* call to the rmc driver */
		cmn_err(CE_CONT, "Reboot the system now.\n");
		rtnfirm();
	}
	softreset();
	outb( STAT_8042, 0xfe );	/* trigger reboot */
forever:
	asm("	cli	");
	asm("	hlt	");
	goto forever;
}

softreset()
{
	int io_rom;

	/* do soft reboot; only do memory check after power-on */
	io_rom = phystokv( 0x467 );
	*(long *)io_rom = (((unsigned long)IO_ROM_SEG) << 16) |
			  (unsigned long)IO_ROM_INIT;
	io_rom = phystokv( 0x472 );
	*(short *)io_rom = RESET_FLAG;

	/* set shutdown flag to reset using int 19 */
	outb(CMOS_ADDR, SHUT_DOWN);
	outb(CMOS_DATA, SHUT5);
}
