/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mb1:uts/i386/boot/mb1/bload.c	1.3"

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

/*
 *	Real mode section of the stage 2 bootstrap.
 *
 *	The physical memory map currently looks like:
 *		Reserved Memory
 *		Real mode boot loader (6K max)
 *		Free memory under 1M boundary
 *		Free memory over 1M boundary
 *
 *	We want to load and then start off the protected mode loader.
*/
#include "sys/types.h"
#include "sys/bolt.h"
#include "sys/elog.h"
#include "sys/i214.h"
#include "sys/sysmacros.h"
#include "../sys/blb.h"
#include "../sys/dib.h"
#include "../sys/boot.h"
#include "../sys/prot.h"

struct blb blb;
struct dib dib;
struct bsl_mem_used_list mu[3];
POINTER buffp;
POINTER actualp;
POINTER statusp;
ulong actual;
ulong status;

extern ulong data_len;		/* Length of data in bytes */
extern ulong text_len;		/* Length of text in bytes */
extern daddr_t text_addr;	/* Address of text on disk */
extern int dev_gran;		/* Granularity of the disk */
extern ulong debug_ck;		/* Debug flag */

extern void dinit();
extern void dread();
extern void disk_init();
extern void disk_read();
extern paddr_t memsize();
extern void error();

extern ushort unit;
extern void tread();
extern void tread_fm ();
extern void tape_init ();
extern void tape_read ();
extern void tape_rewind ();
extern void tape_read_fm ();

extern void bzero(int, paddr_t, long);
extern void pboot(struct blb *, int, int);
extern void fixup_sel(int, paddr_t, long, int);

/*
 *	Load the stage 2 protected mode (MSA) bootstrap.
*/
unsigned long
bload(path)
caddr_t path;
{	paddr_t mem;

	/* Set up DIB */

	if (unit >= FIRSTTAPE)
	{
		dib.hdr.size = sizeof(dib.hdr) + sizeof(dib.un.tape);
		dib.hdr.device_type = TAPE_DIB;
		dib.un.tape.init = tape_init;
		dib.un.tape.init_sel = CODEDESC;
		dib.un.tape.read = tape_read;
		dib.un.tape.read_sel = CODEDESC;
		dib.un.tape.rewind = tape_rewind;
		dib.un.tape.rewind_sel = CODEDESC;
		dib.un.tape.read_fm = tape_read_fm;
		dib.un.tape.read_fm_sel = CODEDESC;
		dib.un.tape.dev_gran = dev_gran;
	}
	else
	{
		dib.hdr.size = sizeof(dib.hdr) + sizeof(dib.un.disk);
		dib.hdr.device_type = DISK_DIB;
		dib.un.disk.init = disk_init;
		dib.un.disk.init_sel = CODEDESC;
		dib.un.disk.read = disk_read;
		dib.un.disk.read_sel = CODEDESC;
		dib.un.disk.dev_gran = dev_gran;
	}

	/* link DIB into the BLB */

	blb.DIB_off = (char *)&dib;
	blb.DIB_sel = getDS();
	/*
	 *	Link our error handler into the BLB.
	*/
	blb.BEH_off = error;
	blb.BEH_sel = getCS();
	/*
	 *	Set up NIL file handlers (pboot will do fixup)/
	*/
	blb.FOE_off = 0;
	blb.FOE_sel = 0;
	blb.FRE_off = 0;
	blb.FRE_sel = 0;
	blb.FCE_off = 0;
	blb.FCE_sel = 0;
	/*
	 *	First, compute the size of memory and the memory used map.
	*/

	mem = memsize();

	mem = mem - text_len - data_len;
	mu[0].block_start = mem;
	mu[0].block_length = text_len + data_len;
	mu[1].block_start = 0;
	mu[1].block_length = RESERVED_SIZE;
	mu[2].block_start = 0;
	mu[2].block_length = 0;
	blb.BMU_off = (char *)mu;
	blb.BMU_sel = getDS();
	/*
	 *	Load the protected mode bootstrap.
	*/

	buffp.offset = (caddr_t)mem;
	buffp.sel = FLATDESC;
	actualp.offset = (caddr_t)&actual;
	actualp.sel = DATADESC;
	statusp.offset = (caddr_t)&status;
	statusp.sel = DATADESC;

	dinit();

	if (unit >= FIRSTTAPE)
	{
		tread (roundup(text_len,dev_gran)/dev_gran, buffp, actualp, statusp);
		tread_fm ();
	}
	else
		dread((ulong)text_addr/dev_gran,
			roundup(text_len, dev_gran) / dev_gran,
			buffp,
			actualp,
			statusp);

	fixup_sel(CODESEL, mem, text_len, CODE);
	mem += text_len;		/* Set up for data */
	fixup_sel(DATASEL, mem, data_len, DATA);
	bzero(DATASEL, 0, data_len);	/* Zero out the data area */

	/*
	 *	Finish off the BLB and invoke the protected mode loader.
	*/
	blb.flags = debug_ck;
#ifdef MB1
	blb.FNM_off = path;
	blb.FNM_sel = getDS();
#endif

	pboot(&blb, CODESEL, DATASEL);	/* Go spuds! */
	return(0);				/* should never get here */
}
