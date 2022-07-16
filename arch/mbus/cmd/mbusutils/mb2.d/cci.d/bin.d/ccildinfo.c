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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/bin.d/ccildinfo.c	1.3"

static char ccildinfo_copyright[] = "Copyright 1987 Intel Corp. 461780";

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
	unsigned char			line_disc;
	req_rec					req_buf;
	cci_line_disc_info_resp_rec	ldinfo_rep_buf;
	char 					*ptr;
	short					errflag = FALSE;
	short					temp;
	
	struct {
		unsigned short	host_id[MAX_HOSTS];
		unsigned short	lines[MAX_LINES];
	}	info;
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
	
	if ((argc - optind) != 2 || errflag) {
		printf("Usage : %s [-p portid] <dest host> <line disc ID> \n",
				argv[0]);
		exit(1);
	}
	
	
	dest_host  = (unsigned short) strtol(argv[optind],&ptr,BASE_TEN);
	if (*ptr != NULL) {
		printf("%s error: dest_host <%s> not an integer\n",
					argv[0],argv[optind]);
		errflag = TRUE;
	}
	optind++;
	line_disc  = temp = (unsigned short) strtol(argv[optind],&ptr,BASE_TEN);
	if (*ptr != NULL) {
		printf("%s error: line_disc_ID <%s> not an integer\n",
					argv[0],argv[optind]);
		errflag = TRUE;
	}
	if (temp < 0 || temp > 255) {
		printf("%s error: line_disc_ID <%s> is outside the range\n",
					argv[0],argv[optind]);
		errflag = TRUE;
	}
	optind++;

	if (errflag == TRUE)
		exit(1);


	if (cci_init(dest_host,port_id) != E_OK) {
		printf("cci init");
		myexit(1);
	}

	req_buf.buf[0] = CCI_GET_LINE_DISC_INFO;
	req_buf.buf[2] = line_disc;

	actual = cci_send_cmd(CCI_PORTID, &req_buf, &ldinfo_rep_buf, NULL, 0L, 
							&info, 
						(unsigned long)(sizeof(info)), &status);

	if (status != E_OK) {
		printf("CCI SEND COMMAND Error : %x\n", status);
		myexit(1);
	}

	if (ldinfo_rep_buf.status != E_OK) {
		printf("CCI Error : %x\n", ldinfo_rep_buf.status);
		myexit(1);
	}

	printf("Line Discipline %d is ", ldinfo_rep_buf.line_disc_id);
	if (ldinfo_rep_buf.state == 0)
		printf("not present\n");

	else if (ldinfo_rep_buf.state == 1)
		printf("being downloaded\n");

	else if (ldinfo_rep_buf.state == 2)
		printf("present\n");

	else 
		printf("in an illegal state\n");

	if (ldinfo_rep_buf.state != 0) {
		printf("	Num Created Hosts = %d\n", ldinfo_rep_buf.num_hosts);
		if (ldinfo_rep_buf.num_hosts > 0) {
			printf("	Create Host List -> ");
			for (i=0; i < ldinfo_rep_buf.num_hosts; i++) 
				printf("%d ",info.host_id[i]);
			printf("\n");
		}
		
		printf("	Num Lines Bound = %d\n", ldinfo_rep_buf.num_lines);
		if (ldinfo_rep_buf.num_lines > 0) {
			printf("	Line Number -> ");
			for (i=0; i < ldinfo_rep_buf.num_lines; i++) 
				printf("%d ",info.lines[i]);
			printf("\n");
		}
	}		
}
