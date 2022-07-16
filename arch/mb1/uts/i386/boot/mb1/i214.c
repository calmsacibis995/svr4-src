/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mb1:uts/i386/boot/mb1/i214.c	1.3"

#include "sys/types.h"
#include "sys/elog.h"
#include "sys/i214.h"
#ifndef lint
#include "sys/inline.h"
#endif
#include "../sys/farcall.h"

#define WUB	0x1000
#define WUP	(WUB>>4)

extern void iomove();

extern ushort cyls;
extern ushort heads;
extern ushort sectors;
extern int dev_gran;
extern ushort alternates;
extern ushort unit;

struct i214wub wub;
struct i214ccb ccb;
struct i214cib cib;
struct i214iopb iopb;
struct i214drtab drtab;

/*
 *	Near call version of the disk driver routines.
*/
void
tenmicrosec()
{	int i;

	for (i = 0; i < 1000; i++)
		(void)inb(WUP);
}

/*
 * Name:	exec_long_term (funct)
 *
 * Funct:	execute 214/221 long-term commands
 *
 */
void
exec_long (funct)
char funct;
{
	iopb.i_funct = funct;		/* set up function code  */
	outb (WUP, WAKEUP_START);	/* interrupt the controller */

	while (!cib.c_statsem);		/* wait for first notice of status */

	/* check for ERROR and Short-term Ope- */
	/* ration Complete in the status byte  */

	if ((cib.c_stat & (ST_HARD_ERR | ST_ERROR)) || ((cib.c_stat & 0x0f) != 0x09))
		asm ("int $3");

	cib.c_statsem = 0;			/* status has been read, reset semaphore. This will */
								/* allow the controller to post long term status. */
	while (!cib.c_statsem);		/* wait for the long term command to complete */

	/* check for ERROR and Long-term Ope- */
	/* ration Complete in the status byte */

	if ((cib.c_stat & 0xc0) || ((cib.c_stat & 0x0f) != 0x0f))
		asm ("int $3");

	cib.c_statsem = 0;				/* reset command semaphore */

	outb(WUP, WAKEUP_CLEAR_INT);	/* long term command interrupt can not be */
	tenmicrosec();					/* suppressed, so clear intterrupt and delay */
}

trewind()
{	exec_long(REW_OP);
}

/*
 *	Re-initialize the controller.
 *	This is needed since we are moving the CCB and the board has
 *	cached the location from the WUB.  Only the near version (which
 *	is called first) actually needs to do the reset.
*/
void
dinit()
{	ushort tmp;

	if (unit >= FIRSTTAPE)		/* if tape, set up tape drtab */
	{
		drtab.dr_ncyl = 1;		/* tape parameter = drive present */
		drtab.dr_nfhead = 0;	/* the rest of the buffer are all */
		drtab.dr_nrhead = 0;	/* reserved and should be set to 0 */
		drtab.dr_nsec   = 0;
		drtab.dr_lsecsiz = 0;
		drtab.dr_hsecsiz = 0;
		drtab.dr_nalt    = 0;
	}
	else						/* else, set up disk drtab */
	{
		drtab.dr_ncyl = cyls;
		drtab.dr_nfhead = heads;
		drtab.dr_nrhead = 0;
		drtab.dr_nsec   = sectors;
		drtab.dr_lsecsiz = dev_gran & 0xFF;
		drtab.dr_hsecsiz = (dev_gran >> 8);
		drtab.dr_nalt = alternates;
	}

	/* Put param request in iob */

	if (unit >= FIRSTTAPE)
		iopb.i_device = STREAMER;
	else
		iopb.i_device = DEVWINI;

	iopb.i_unit = unit & 3;
	iopb.i_funct = INIT_OP;
	iopb.i_modifier = MOD_24_BIT|MOD_NO_INT;
	iopb.i_sector = 0;
	iopb.i_head = 0;
	iopb.i_cylinder = 0;
	tmp = DATADESC;
	iopb.i_addr = vtophys(tmp, (caddr_t)&drtab);
	iopb.i_xfrcnt = 0;

	cib.c_csa[0] = 0;			/* Point cib to iopb */
	cib.c_csa[1] = 0;
	cib.c_iopb = (int)&iopb;
	cib.c_iopb_b = MY_86SEL;

	ccb.c_ccw1 = 1;				/* Point ccb to cib */
	ccb.c_cib = ((int)&cib)+4;	/* manual says fith byte */
	ccb.c_cib_b = MY_86SEL;
	ccb.c_ccw2 = 1;
	ccb.c_cpp = (int)&ccb.c_cp;
	ccb.c_cpp_b = MY_86SEL;
	ccb.c_cp = 4;

	wub.w_sysop = 1;			/* Point wub to ccb */
	wub.w_rsvd = 0;
	wub.w_ccb = (int)&ccb;
	wub.w_ccb_b = MY_86SEL;
	iomove(&wub, (caddr_t)WUB, FLATDESC, sizeof(wub));

	outb(WUP, WAKEUP_RESET);	/* Required when moving the CCB */
	for (tmp=0; tmp<100; tmp++)
		tenmicrosec();

	outb(WUP, WAKEUP_CLEAR_INT);
	for (tmp=0; tmp<100; tmp++)
		tenmicrosec();

	ccb.c_busy1 = 0xFF;			/* Look busy until this goes clear */
	outb(WUP, WAKEUP_START);	/* Initialize controller */
	while (ccb.c_busy1)
		;						/* Wait for completion */
	cib.c_statsem = 0;

	outb(WUP, WAKEUP_START);	/* Execute INITIALIZE command */
	while (!cib.c_statsem);		/* Wait for completion */
	cib.c_statsem = 0;

	/* If boot from tape, we need to do Tape Init, Tape   */
	/* Reset, and Load Tape, to initialize the controller */
	/* for tape. Then for the controller point to the MSA */
	/* code code, we need to Skip Forward One File Mark.  */

	if (unit >= FIRSTTAPE)
	{
		iopb.i_funct = TAPEINIT_OP;
		outb(WUP, WAKEUP_START);	/* Send command to controller */
		while (!cib.c_statsem);		/* Wait for completion */
		cib.c_statsem = 0;

		iopb.i_funct = TAPERESET_OP;
		outb(WUP, WAKEUP_START);	/* Send command to controller */
		while (!cib.c_statsem);		/* Wait for completion */
		cib.c_statsem = 0;

		exec_long (LOADTAPE_OP);

		exec_long (SFFM_OP);
	}
}

/*
 *	Read the disk.
*/
/* ARGSUSED */
void
dread(blk, num_blks, buff, actual, status)
ulong blk;
ulong num_blks;
POINTER buff;
POINTER actual;
POINTER status;
{
	iopb.i_funct = READ_OP;
	iopb.i_sector = blk % sectors;
	blk /= sectors;
	iopb.i_head = blk % heads;
	blk /= heads;
	iopb.i_cylinder = blk;
	iopb.i_addr = vtophys(buff.sel, buff.offset);
	iopb.i_xfrcnt = num_blks * dev_gran;
	if ((iopb.i_addr<=0x1000) && (0x1000<=iopb.i_addr+iopb.i_xfrcnt))
		asm("int $3");
	ccb.c_busy1 = 0xFF;			/* Look busy until this goes clear */
	outb(WUP, WAKEUP_START);	/* Initiate I/O */
	while (ccb.c_busy1)
		;						/* Wait for completion */
	far_store(actual.sel, actual.offset, iopb.i_actual);
	far_store(status.sel, status.offset, 0);
	cib.c_statsem = 0;
}

/*
 * Name:	tread
*/
void
tread(num_blks, buff, actual, status)
ulong num_blks;
POINTER buff;
POINTER actual;
POINTER status;
{
	iopb.i_funct = READ_OP;
	iopb.i_addr = vtophys(buff.sel, buff.offset);
	iopb.i_xfrcnt = num_blks * dev_gran;

	if ((iopb.i_addr<=0x1000) && (0x1000<=iopb.i_addr+iopb.i_xfrcnt))
		asm("int $3");

	outb(WUP, WAKEUP_START);	/* Initiate I/O */
	while (!cib.c_statsem);		/* Wait for completion */

	far_store(actual.sel, actual.offset, iopb.i_actual);
	far_store(status.sel, status.offset, 0);

	cib.c_statsem = 0;
}

/*
 * Name:	tread_fm()
 *
 * Funct:	thie routine moves the tape forward to a file mark or
 *			EOM(the end of media mark).
 *
 *			Since we are having problem getting Space Forward One File
 *			Mark command, this routine is implemented by reading data
 *			512 bytes at a time until it reach End of File Mark Error.
*/
char junk[512];
void
tread_fm()
{
	ushort tmp;

	/* set up iopb fields */

	tmp = DATADESC;
	iopb.i_addr = vtophys(tmp, (caddr_t) junk);
	iopb.i_funct = READ_OP;
	iopb.i_xfrcnt = 512;

	/* keep reading data 512 bytes at a time until error */

	do {
		cib.c_stat = 0;
		cib.c_statsem = 0;
		outb(WUP, WAKEUP_START);
		while (!cib.c_statsem);
	} while (!(cib.c_stat & (ST_HARD_ERR | ST_ERROR)));

	cib.c_statsem = 0;				/* clear semaphore */
}
