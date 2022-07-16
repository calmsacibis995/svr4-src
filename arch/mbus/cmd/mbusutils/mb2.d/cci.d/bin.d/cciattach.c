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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/bin.d/cciattach.c	1.3"

static char cciattach_copyright[] = "Copyright 1987 Intel Corp. 461775";

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
	unsigned char			line;
	unsigned short			subch;
	int						status;
	req_rec					req_buf;
	cci_attach_resp_rec		att_reply_buf;
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
		printf("Usage : %s [-p portid] <dest host> <line> <sub channel>\n",
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

	req_buf.att_req.line = line;
	req_buf.att_req.subchannel = subch;
	req_buf.att_req.def_portid = port_id;
	req_buf.buf[0] = CCI_ATTACH;

	actual = cci_send_cmd(CCI_PORTID, &req_buf, &att_reply_buf, NULL, 0L, 
				NULL, 0L, &status);
	if (status != E_OK) {
		printf("CCI SEND COMMAND Error : %x\n", status);
		myexit(1);
	}
	if (att_reply_buf.status == 0x80) {
		printf("ATTACH Error : %x\n", att_reply_buf.status);
		myexit(1);
	} else if (att_reply_buf.status == 0x01) 
		printf("SWITCHed from : %d\n", att_reply_buf.prev_host);

	printf("ATTACHed Sub Channel %d on Line %d\n", subch, line);

	printf("Subchannel %d Portid = %x\n", subch, att_reply_buf.portid);
	printf("	Session Status[0] : %x\n", att_reply_buf.session_stat[0]);
	printf("	Session Status[1] : %x\n", att_reply_buf.session_stat[1]);
	printf("	Session Status[2] : %x\n", att_reply_buf.session_stat[2]);
	printf("	Session Status[3] : %x\n", att_reply_buf.session_stat[3]);

	printf("	Script Info[0] : %x\n", att_reply_buf.script_info[0]);
	printf("	Script Info[1] : %x\n", att_reply_buf.script_info[1]);
	printf("	Script Info[2] : %x\n", att_reply_buf.script_info[2]);
	printf("	Script Info[3] : %x\n", att_reply_buf.script_info[3]);
	

}
