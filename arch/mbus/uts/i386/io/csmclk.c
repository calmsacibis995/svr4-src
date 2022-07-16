/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1983, 1986, 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/csmclk.c	1.3"

#ifndef lint
static char csmclk_copyright[] = "Copyright 1983,1986,1987 Intel Corp. 463047";
#endif

#include "sys/types.h"
#include "sys/param.h"
#include "sys/file.h"
#include "sys/errno.h"
#include "sys/rtc.h"
#include "sys/clockcal.h"
#include "sys/ics.h"


extern cclkopen();
extern cclkclose();
extern cclkread();
extern cclkwrite();

unsigned int csm_td_comreg;
unsigned int csm_td_secreg;

cclkinit ()
{
	struct clkrtn	clkrtn;
	unsigned csm_td_rec;

	/* find the Time of Day record  */

	if ((csm_td_rec = ics_find_rec (CSM_SLOT, CSM_TD_RECTYPE)) == 0xffff)
		return;

	csm_td_comreg = csm_td_rec + CSM_TD_COMREG_OFF;
	csm_td_secreg = csm_td_rec + CSM_TD_SECREG_OFF;

	/* notify the clk client of existence */

	clkrtn.c_open = cclkopen;
	clkrtn.c_close = cclkclose;
	clkrtn.c_read = cclkread;
	clkrtn.c_write = cclkwrite;
	clk_log (CLK_ICSM, 0, &clkrtn);
}
		

/* ARGSUSED */
cclkopen(dev, flag)
{
	if ((flag & FWRITE))
		return (EACCES);
}


cclkclose()
{
	return (0);
}



int
cclk_cmdreg_isbusy ()
{
	unsigned retry;
	struct ics_struct cmd_ics;

	cmd_ics.slot_id = CSM_SLOT;
	cmd_ics.reg_id = csm_td_comreg;
	cmd_ics.count = 1;
	cmd_ics.buffer[0] = CSM_TD_BUSY;

	retry=0;
	ics_rw(ICS_READ_ICS, &cmd_ics);
	while( ((cmd_ics.buffer[0] & CSM_TD_BUSY) == CSM_TD_BUSY) && 
				(retry++ < CSM_TD_RETRY)) {
		delay(100);
		ics_rw(ICS_READ_ICS, &cmd_ics);
	}

	if ((cmd_ics.buffer[0] & CSM_TD_BUSY) == CSM_TD_BUSY) 
		return (1);
	return (0);
}


int
cclk_cmdreg_isready ()
{
	unsigned retry;
	struct ics_struct cmd_ics;

	cmd_ics.slot_id = CSM_SLOT;
	cmd_ics.reg_id = csm_td_comreg;
	cmd_ics.count = 1;
	cmd_ics.buffer[0] = 0;

	retry = 0;
	ics_rw(ICS_READ_ICS, &cmd_ics);
	while( ((cmd_ics.buffer[0] & CSM_TD_RDY) != CSM_TD_RDY) && 
			(retry++ < CSM_TD_RETRY)) {
		delay(100);
		ics_rw(ICS_READ_ICS, &cmd_ics);
	}
	if ((cmd_ics.buffer[0] & CSM_TD_RDY) != CSM_TD_RDY) 
		  return (1);
	return (0);
}


void
cclkwrite_nop ()
{
	struct ics_struct cmd_ics;

	cmd_ics.slot_id = CSM_SLOT;
	cmd_ics.reg_id= csm_td_comreg;
	cmd_ics.count= 1;
	cmd_ics.buffer[0]= CSM_TD_NOP;
	ics_rw(ICS_WRITE_ICS, &cmd_ics);
}


/* ARGSUSED */
int
cclkread (dev, clk, flag)
dev_t dev;
struct csmclk *clk;
int flag;
{
	unsigned char csm_tdics_rec[ICS_HEADER + sizeof(struct csmclk)]; 
	struct ics_struct *td_ics = (struct ics_struct *)csm_tdics_rec;
	struct ics_struct cmd_ics;

	/*
	 * some sort of mutexing between multiple CPU's and processes in the
	 * same CPU has to be implemented. But for right now we will defer
	 * this process.
	 */
	if (cclk_cmdreg_isbusy() == 1) {
		return (-1);
	}

	/*
	 * write the read command to command reg
	 */
	cmd_ics.slot_id = CSM_SLOT;
	cmd_ics.reg_id= csm_td_comreg;
	cmd_ics.count= 1;
	cmd_ics.buffer[0] = CSM_TD_RDALL;
	ics_rw(ICS_WRITE_ICS, &cmd_ics);

	if (cclk_cmdreg_isready () == 1) {
		return (-1);
	}

	/* read the time/date record */

	td_ics->slot_id = CSM_SLOT;
	td_ics->reg_id = csm_td_secreg;
	td_ics->count = sizeof(struct csmclk); 
	ics_rw(ICS_READ_ICS, td_ics);
	bcopy ((caddr_t)td_ics->buffer, (caddr_t)clk, sizeof(struct csmclk));

	/* write a noop to the command register */

	(void) cclkwrite_nop ();
	return (0);
}

/* ARGSUSED */
int
cclkwrite (dev, clk, flag)
dev_t dev;
struct csmclk *clk;
int flag;
{
	unsigned char csm_tdics_rec[ICS_HEADER + sizeof(struct csmclk)]; 
	struct ics_struct *td_ics = (struct ics_struct *)csm_tdics_rec; 
	struct ics_struct cmd_ics;


	if (cclk_cmdreg_isbusy() == 1) 	
		return (-1);

	/* write the time and date registers */ 
	
	td_ics->slot_id = CSM_SLOT;
	td_ics->reg_id= csm_td_secreg;
	td_ics->count= sizeof (struct csmclk);
	bcopy ((caddr_t)clk, (caddr_t)td_ics->buffer, sizeof (struct csmclk));
	ics_rw(ICS_WRITE_ICS, td_ics);

	/* write the WRITE command to the command register */

	cmd_ics.slot_id = CSM_SLOT;
	cmd_ics.count= 1;
	cmd_ics.buffer[0] = CSM_TD_WRALL;
	cmd_ics.reg_id= csm_td_comreg;
	ics_rw(ICS_WRITE_ICS, &cmd_ics);

	if (cclk_cmdreg_isready () == 1)
		return (-1);
		
	/* write a nop to the command register */

	(void) cclkwrite_nop ();
	return (0);

}
