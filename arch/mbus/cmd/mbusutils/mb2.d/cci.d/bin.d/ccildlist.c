/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/bin.d/ccildlist.c	1.3"

static char ccildlist_copyright[] = "Copyright 1987 Intel Corp. 461779";

#include <stdio.h>
#include <signal.h>
#include "common.h"
#include "msg.h"
#include "cci.h"
#include "main.h"
#include "utils.h"

	/* Global Variables */
	
main(argc, argv)

int 	argc;
char	*argv[];


{
	unsigned long			actual;
	unsigned short			dest_host;
	int				status;
	unsigned int			i;
	req_rec				req_buf;
	cci_line_disc_list_resp_rec	ldlist_rep_buf;
	unsigned char	line_disc_id[MAX_LINE_DISC];
	char 					*ptr;
	short					errflag = FALSE;
	short					c;
	extern char 			*optarg;
	extern short			optind;
	unsigned short			port_id = DEF_PORT_ID;
	
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, on_intr);
	
	if ((c = getopt(argc,argv,"p:")) != -1) 
		switch (c) {
		case 'p' : 
				port_id = (unsigned short) strtol(optarg,&ptr,BASE_TEN);
				if (*ptr != NULL) {
					printf("%s error: port_id <%s> not an integer\n",argv[0],optarg);
					errflag = TRUE;
				}
				break;
		case '?' : 
		default	 :	
				errflag = TRUE;
				break;
		}
	
	if ((argc - optind) != 1 || errflag) {
		printf("Usage : %s [-p portid] <dest host> \n",argv[0]);
		exit(-1);
	}
	
	dest_host  = (unsigned short) strtol(argv[optind],&ptr,BASE_TEN);
	if (*ptr != NULL) {
		printf("%s error: dest_host <%s> not an integer\n",argv[0],argv[optind]);
		errflag = TRUE;
	}
	optind++;

	if (cci_init(dest_host,port_id) != E_OK) {
		perror("cci init");
		myexit(1);
	}

	req_buf.buf[0] = CCI_GET_LINE_DISC_LIST;

	actual = cci_send_cmd(CCI_PORTID, &req_buf, &ldlist_rep_buf, NULL, 0L, 
					&line_disc_id[0],
						(unsigned long)(sizeof(line_disc_id)), &status);

	if (status != E_OK) {
		printf("CCI SEND COMMAND Error : %x\n", status);
		myexit(1);
	}

	if (ldlist_rep_buf.status != E_OK) {
		printf("CCI Line Disc List Error : %x\n", ldlist_rep_buf.status);
		myexit(1);
	}

	printf("Number of line disciplines = %d\n", ldlist_rep_buf.num_line_disc);
	
	if (ldlist_rep_buf.num_line_disc > 0) {
			printf("	Line Disc Id -> ");
			for (i=0; i < ldlist_rep_buf.num_line_disc; i++) 
				printf("%d ", line_disc_id[i]);
			printf("\n");
	}
		
}
