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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/ics.d/reset.d/reset.c	1.3"

static char reset_copyright[] = "Copyright 1987, 1989 Intel Corp. 462677";

/*
* Cause a local reset through interconnect.  Options are:
* 	-b  don't set the bist complete bit
*		( default is to set the bist complete bit)
*	-r	initialize the bootstrap parameter string
*	-v	verbose mode
*		(displays run-time diagnostic information)
* 	-m  set program table index register to invoke the monitor
*		(same as -i 248)
* 	-n  don't change the contents of program table index register 
* 	-i  set the index of program table index register   
*		( this option is default with value 0 )
*/

#include <sys/types.h>
#include <sys/ics.h>
#include <stdio.h>
#include "reset.h"

extern void snd_parcel();
extern int atoi();
extern void exit();
extern int getopt();
extern char *optarg;
extern int optind;

static FILE *ofile;
unsigned char slot_num;
unsigned char *progname;
static unsigned char ptir_val=0;
static int	ibps_flag=0;
static int	n_flag=0;
static int	b_flag=0;
	

main(argc, argv)
int argc;
char *argv[];
{
	int c;
	static void usage();
	static void do_bisttest();
	extern long	atol();
	extern void catch_interrupt();
	extern void ic_init();
	extern void do_board_reset();
	extern unsigned char get_ic();
	extern void put_ic();
	extern FILE *open_bit_bucket();

	ofile = open_bit_bucket();
	progname = (unsigned char *) argv[0];

	if (argc <= MAX_ARGS) {
		usage();
		exit(1);
	}
	
	catch_interrupt();

	while ((c = getopt(argc, argv, "brvmni:")) != EOF)
		switch(c) {
			case 'i':
				ptir_val = (unsigned char) atol(optarg);
				break;

			case 'n':
				n_flag = 1;
				break;

			case 'm':
				ptir_val = 248;
				break;

			case 'v':
				ofile = stdout;
				break;

			case 'r':
				ibps_flag = 1;
				break;

			case 'b':
				b_flag = 1;
				break;

			case'?':
				usage();
				exit(1);
		}
				
	argc -= optind;
	argv += optind;

	if (argc != MAX_ARGS) {
		usage();
		exit(1);
	}

	slot_num = atoi(argv[0]);

	if (slot_num > ICS_MAX_SLOT) {
		(void) fprintf(stderr,"%s: Invalid Slot Id\n",progname);
		exit(1);
	}

	(void) fprintf(ofile,"Target Board is at slot %02d\n",slot_num);
	
	ic_init();

	if (ibps_flag) {
		/* Tell slave that system testing is not complete. */
		/* Tell slave that it cannot handle parcel service requests. */
		put_ic(slot_num, ICS_BistMasterStatus, BST_RMT_PRCL);
	
		do_board_reset();

		/* Check for IDX support on slave. */
		if ((get_ic(slot_num, ICS_BistSupportLevel) & BST_IDX_SUPPORT) == 0) {
			(void) fprintf(ofile,"Target board is not IDX compatible\n");
			(void) fprintf(ofile,"Resetting target at slot %02d\n",slot_num);

			/* Set the program table index register unless told not to. */
			if (!n_flag)
				put_ic(slot_num, ICS_ProgramTableIndex, ptir_val);

			/* Tell slave that system testing is complete unless told not to. */
			if (b_flag)
				put_ic(slot_num, ICS_BistMasterStatus, 0);
			else
				/* This is the cold reset value. */
				put_ic(slot_num, ICS_BistMasterStatus, BST_CMPLT);


			do_board_reset();

			exit(0);
		}

		do_bisttest(134);
	}

	/* Set the program table index register unless told not to. */
	if (!n_flag)
		put_ic(slot_num, ICS_ProgramTableIndex, ptir_val);

	/* Tell slave that system testing is complete unless told not to. */
	if (b_flag)
		put_ic(slot_num, ICS_BistMasterStatus, 0);
	else
		/* This is the cold reset value. */
		put_ic(slot_num, ICS_BistMasterStatus, BST_CMPLT);
	
	(void) fprintf(ofile,"Resetting target at slot %02d\n",slot_num);

	do_board_reset();

	return(0);
}


static struct x_tst_rq_parcel bisttest = { ExecTstReqParcel, 0, TstLvl, 
		ErrReprtLvl, ErrAction, TstInit };

static void do_bisttest(testid)
unsigned char	testid;
{
	extern int rcv_parcel();
	extern char *malloc();
	unsigned char *data_buf;
	static int process_rcvparcel();

	bisttest.test_id = testid;

	snd_parcel((unsigned char *)&bisttest, sizeof(struct x_tst_rq_parcel));
	
	data_buf = (unsigned char *) malloc(BUF_SIZE);
	if (data_buf == NULL) {
		(void) fprintf(stderr,"%s: Memory Not Available\n",progname);
		exit (1);
	}	

	do {
		if (rcv_parcel(data_buf) < 0) {
			(void) fprintf(stderr,"%s: Error in receiving a parcel from slave\n",progname);
			exit (1);
		}	

		if (data_buf[0] != ExecTstRespParcel)
			if (process_rcvparcel(data_buf) < 0) {
				(void) fprintf(stderr,"%s: Unexpected parcel type received.\n",progname);
				exit (1);
			}	
	} while (data_buf[0] != ExecTstRespParcel);

}


static struct prmpt_rd_resp_parcel prmptrdresp = { 0x20, };

static int process_rcvparcel(buf)
unsigned char *buf;
{

	switch (buf[0]) {
		case PrmptRdReqParcel:
	
			(void) fprintf(ofile, "%s\n", &buf[PRMPTOFFSET]);
			(void) fflush(ofile);

			prmptrdresp.length = sizeof(struct prmpt_rd_resp_parcel) - sizeof(prmptrdresp.parcel_type);
			prmptrdresp.status = OK_STATUS;
			prmptrdresp.param[0] = 'y';
			prmptrdresp.param[1] = '\0';

			(void) fprintf(ofile, "%s\n", prmptrdresp.param);
			(void) fflush(ofile);

			/* Send a response back to initialize the boot parameters. */
			snd_parcel((unsigned char *)&prmptrdresp, sizeof(struct prmpt_rd_resp_parcel));
			break;

		case PadWrtReqParcel:
	
			(void) fprintf(ofile, "%s\n", &buf[sizeof(struct pad_wrt_req_parcel)]);
			(void) fflush(ofile);
			break;

		default:
			return(ERR_STATUS);
	}
	return(OK_STATUS);
}


static void usage()
{
	(void) fprintf(stderr,"Usage: %s [-bv][-m|-n|-i index] slot_number\n", progname);
}
