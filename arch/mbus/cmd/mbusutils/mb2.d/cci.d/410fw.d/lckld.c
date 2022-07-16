/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1990  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/410fw.d/lckld.c	1.3"

static char lckld_copyright[] = "Copyright 1987 Intel Corp. 461796";

#include <stdio.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/mb2taiusr.h>
#include <sys/ics.h>
#include <ctype.h>
#include "common.h"

int my_slotid;
int	param_flg = 0;

char	cmd_buf[512];
char	*cmd_name = "/usr/lib/cci/download";

extern unsigned short optind;
extern char	*optarg;

main(argc, argv)

	int argc;
	char *argv[];

{
	int i;
	int fd;
	int slot;
	int fw_reg;
	int fw_rec;
	int lock_reg;
	char *file_name;


	while ((i = getopt(argc, argv, "p:")) != EOF) {
		switch (i) {
			case 'p':
				param_flg++;
				break;
			case '?':
				fprintf(stderr,
						"Usage:%s [-p options for download] <file> <slotid>\n", 
						argv[0]);
				exit(1);
				break;
			default:
				fprintf(stderr,
						"Usage:%s [-p options for download] <file> <slotid>\n", 
						argv[0]);
				exit(1);
				break;
		}
	}
	
	if(argc -optind != 2) {
		fprintf(stderr, "Usage: %s [-p options for download] <file> <slotid>\n", argv[0]);
		exit(1);
	}
	
	file_name = argv[optind++];
	slot = atoi(argv[optind++]);
	
	if (slot > ICS_MAX_SLOT) {
		fprintf(stderr, "%s: Slot number %d out of range\n",argv[0], slot);
		exit(1);
	}

	if (param_flg != 0)
		sprintf(cmd_buf, "%s -%s %s %d", cmd_name, optarg, file_name, slot);
	else
		sprintf(cmd_buf, "%s %s %d", cmd_name, file_name, slot);
		
	
	if (init() == -1) {
		fprintf(stderr, "%s: Initialization failed\n",argv[0]);
		exit(1);
	}
	
	if (get_ic(slot, VENDOR_ID_LO) != 1 || get_ic(slot, VENDOR_ID_HI) != 0) {
		fprintf(stderr, "%s: No Controller in slot %d\n",argv[0], slot);
		exit(1);
	}
	

	my_slotid = get_slot_id();
	if (my_slotid == -1) {
		fprintf(stderr, "%s: Error while getting my slotid\n",argv[0]);
		exit(1);
	}

		
	fw_rec  = find_rec(slot, IC_FW_REC);
	if (fw_rec < 0) {
		fprintf(stderr, "%s: Firmware Comm Rec not found\n", argv[0]);
		exit(1);
	} else
		lock_reg = fw_rec + IC_LOCK_OFFSET;
	
	if (lock_reg < 0 || lock_reg > 511) {
		fprintf(stderr, "%s: Lock Register number %d out of range\n",
															argv[0], lock_reg);
		exit(1);
	}

	fw_reg = fw_rec + IC_FW_OFFSET;
	if (fw_reg < 0 || fw_reg > 511) {
		fprintf(stderr, "%s: Download Register number %d out of range\n",
														argv[0], fw_reg);
		exit(1);
	}

	printf("Locking Controller in slot %d...\n", slot);

	lock(slot, lock_reg);

	printf("Controller Locked\n");
	
	i = 0;
	if (get_ic(slot, fw_reg) == INIT_NOT_DONE) {
		printf("Downloading\n");
		if (execute(cmd_buf) == 0) {
			while (get_ic(slot, fw_reg) != INIT_DONE && i++ <= MAX_WAIT) 
				sleep(1);
			if (i <= MAX_WAIT)
				printf("Controller Initialized\n");
			else
				printf("Controller Not Initialized\n");
		
		} else {
			printf("Download failed\n");
			unlock(slot, lock_reg);
			printf("Controller Unlocked\n");
			exit(1);
		}

	} else if (get_ic(slot, fw_reg) == INIT_DONE) 
		printf("Controller Already Initialized\n");

	else
		printf("Controller Not Initialized\n");
		

	unlock(slot, lock_reg);
	printf("Controller Unlocked\n");

}

lock(slot, reg)

unsigned char	slot;
unsigned short	reg;

{
	for (;;) {
		if (get_ic(slot, reg) == (my_slotid + 1))
			break;
		while (get_ic(slot, reg) != 0)
			do_sleep(1);

		put_ic(slot, reg, (my_slotid + 1));

		do_sleep(1);

	}

}

unlock(slot, reg)

unsigned char	slot;
unsigned short	reg;

{
	put_ic(slot, reg, 0);

}
