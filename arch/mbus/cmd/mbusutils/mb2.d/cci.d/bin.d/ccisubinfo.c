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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/bin.d/ccisubinfo.c	1.3"

static char ccisubinfo_copyright[] = "Copyright 1987 Intel Corp. 461782";

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
	unsigned char			line;
	unsigned short			subch;
	req_rec				req_buf;
	cci_subch_info_resp_rec	sinfo_rep_buf;
	unsigned short	host_id[MAX_HOSTS];
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
	
	if ((argc - optind) != 3 || errflag) {
		printf("Usage : %s [-p portid] <dest host> <line> <subchannel>\n",
				argv[0]);
		exit(1);
	}
	
	dest_host  = (unsigned short) strtol(argv[optind],&ptr,BASE_TEN);
	if (*ptr != NULL) {
		printf("%s error: dest_host <%s> not an integer\n",argv[0],argv[optind]);
		errflag = TRUE;
	}
	optind++;
	line  = (unsigned short) strtol(argv[optind],&ptr,BASE_TEN);
	if (*ptr != NULL) {
		printf("%s error: line <%s> not an integer\n",argv[0],argv[optind]);
		errflag = TRUE;
	}
	optind++;
	subch  = (unsigned short) strtol(argv[optind],&ptr,BASE_TEN);
	if (*ptr != NULL) {
		printf("%s error: subch <%s> not an integer\n",argv[0],argv[optind]);
		errflag = TRUE;
	}
	optind++;

	if (errflag == TRUE)
		exit(1);


	if (cci_init(dest_host,port_id) != E_OK) {
		perror("cci init");
		exit(1);
	}

	req_buf.buf[0] = CCI_GET_SUBCH_INFO;
	req_buf.buf[2] = line;
	req_buf.subch_req.subchannel = subch;

	actual = cci_send_cmd(CCI_PORTID, 
						&req_buf, &sinfo_rep_buf, NULL, 0L, &host_id[0],
						(unsigned long)(sizeof(host_id)), &status);

	if (status != E_OK) {
		printf("CCI SEND COMMAND Error : %x\n", status);
		myexit(1);
	}

	if (sinfo_rep_buf.status != E_OK) {
		printf("CCI Get Subchannel Info Error : %x\n", sinfo_rep_buf.status);
		myexit(1);
	}

	printf("Subchannel %d on ", sinfo_rep_buf.subchannel);
	printf("Line %d is ", sinfo_rep_buf.line);
	if (sinfo_rep_buf.state == 0)
		printf("not attached\n");

	else if (sinfo_rep_buf.state == 1) {
		printf("attached to ");
		printf("Host %d\n", sinfo_rep_buf.active_host);
		printf("	Portid of subchannel = %d\n", sinfo_rep_buf.portid);
	}
	
	else 
		printf("in an illegal state\n");

	if (sinfo_rep_buf.state == 1) {
		printf("	Number of Hosts queued = %d\n", sinfo_rep_buf.num_hosts);
		if (sinfo_rep_buf.num_hosts > 0) {
			printf("	Host Queue -> ");
			for (i=0; i < sinfo_rep_buf.num_hosts; i++) 
				printf("%d ", host_id[i]);
			printf("\n");
		}
		
	}		
}
