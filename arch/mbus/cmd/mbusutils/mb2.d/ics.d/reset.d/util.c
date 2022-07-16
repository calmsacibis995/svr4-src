/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/ics.d/reset.d/util.c	1.3"

#include <sys/types.h>
#include <sys/ics.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include "reset.h"

extern unsigned char slot_num;
extern int close();
extern void nap();
extern int ics_read(), ics_write();
extern void perror(), exit(); 
extern unsigned char *progname;

static int ic_fd;
static FILE *fnull;


void ic_init()
{
	ic_fd = open("/dev/ics", O_RDWR);
	if (ic_fd == -1) {
		(void) fprintf(stderr,"\n%s: Interconnect not Initialized\n",progname);
		perror("open");
		exit(1);
	}
}


unsigned char get_ic(slot, reg)
unsigned char slot;
unsigned short reg;
{
	unsigned char value;
	
	if (ics_read(ic_fd, slot, reg, &value, 1) != 0) {
		(void) fprintf(stderr,"\n%s: Interconnect Read Error\n",progname);
		exit(1);
	}

	return(value);
} 


void put_ic(slot, reg, value)
unsigned char slot;
unsigned short reg;
unsigned char value;
{
	if (ics_write(ic_fd, slot, reg, &value,1) != 0) {
		(void) fprintf(stderr,"\n%s: Interconnect Write Error\n",progname);
		exit(1);
	}
}


FILE *open_bit_bucket()
{
	extern FILE *fopen(); 

	if ((fnull = fopen("/dev/null", "w")) ==NULL) {
		(void) fprintf(stderr,"\n%s: Can't Open /dev/null\n",progname);
		exit(1);
	}
	return(fnull);
}


void catch_interrupt()
{
	static void on_intr();

	(void) signal(SIGINT, on_intr);
}


static void on_intr()
{
	(void) fprintf(stderr,"\n%s: INTERRUPT\n",progname);
	(void) close(ic_fd);
	exit(1);
}


void do_board_reset()
{
	put_ic(slot_num, ICS_GeneralControl, BST_LOCAL_RESET);
	nap(RESET_SLEEP);

	put_ic(slot_num, ICS_GeneralControl, 0);
	nap(RESET_SLEEP);
}


void snd_parcel(buf, bcount)
unsigned char *buf;
unsigned long bcount;
{
	int i;

	/* Indicate Input Pending to slave. */ 
	put_ic(slot_num, ICS_BistMasterStatus,
		(BST_INPT_PNDNG | get_ic(slot_num, ICS_BistMasterStatus)));

	for (i=0; i<bcount; i++) {

		/* Tell slave that byte at data in port no longer valid. */
		put_ic(slot_num,ICS_BistMasterStatus, (~BST_INDATA_VLD & get_ic(slot_num,ICS_BistMasterStatus)));

		/* Wait until slave clears the data byte accepted flag. */
		while (get_ic(slot_num,ICS_BistSlaveStatus) & BST_INDATA_ACPT);

		/* Write data byte to slave. */
		put_ic(slot_num,ICS_BistDataIn, *buf++);

		/* Tell slave that the byte at data in port is valid. */
		put_ic(slot_num,ICS_BistMasterStatus, (BST_INDATA_VLD | get_ic(slot_num,ICS_BistMasterStatus)));

		/* Poll until data byte accepted by slave. */
		while (!(get_ic(slot_num,ICS_BistSlaveStatus) & BST_INDATA_ACPT));
	}

	/* Tell slave that no more input is pending. */
	put_ic(slot_num, ICS_BistMasterStatus,
		(~BST_INPT_PNDNG & get_ic(slot_num, ICS_BistMasterStatus)));

	/* Tell slave that byte at data in port no longer valid. */
	put_ic(slot_num,ICS_BistMasterStatus, (~BST_INDATA_VLD & get_ic(slot_num,ICS_BistMasterStatus)));

	/* Wait until slave clears the data byte accepted flag. */
	while (get_ic(slot_num,ICS_BistSlaveStatus) & BST_INDATA_ACPT);
}


int rcv_parcel(buf)
unsigned char *buf;
{

	/* Poll to see if slave has data to send. */
	while (!(get_ic(slot_num,ICS_BistSlaveStatus) & BST_OUTPUT_PNDNG));

	/* Read in the parcel sent from the slave. */
	do {

		/* Poll until the slave's output data byte is valid. */
		while (!(get_ic(slot_num,ICS_BistSlaveStatus) & BST_OUTDATA_VLD));

		/* Read the data byte from slave. */
		*buf++ = get_ic(slot_num,ICS_BistDataOut);

		/* Tell slave that data byte has been read. */
		put_ic(slot_num, ICS_BistMasterStatus,
			(BST_OUTDATA_ACPT | get_ic(slot_num, ICS_BistMasterStatus)));

		/* Poll until the slave's output data byte is invalid. */
		while (get_ic(slot_num,ICS_BistSlaveStatus) & BST_OUTDATA_VLD);

		/* Indicate that the next data byte has not been read yet. */
		put_ic(slot_num, ICS_BistMasterStatus,
			(~BST_OUTDATA_ACPT & get_ic(slot_num, ICS_BistMasterStatus)));

	/* End of parcel? */
	} while (BST_OUTPUT_PNDNG & get_ic(slot_num,ICS_BistSlaveStatus));
	return(0);
}
