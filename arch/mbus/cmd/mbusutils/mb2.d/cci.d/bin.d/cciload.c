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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/bin.d/cciload.c	1.3"

static char cciload_copyright[] = "Copyright 1987 Intel Corp. 461771";

#include <stdio.h>
#include <signal.h>
#include "common.h"
#include "msg.h"
#include "cci.h"
#include "main.h"

	/* Global Variables */
	
main(argc, argv)

int 	argc;
char	*argv[];

{
	unsigned short			dest_host;
	unsigned char			line_disc;
	unsigned long			buf_len;
	unsigned long			offset;
	unsigned long			actual;
	int						status;
	req_rec					req_buf;
	cci_create_resp_rec		crt_reply_buf;
	cci_free_resp_rec		free_reply_buf;
	cci_download_resp_rec	dl_reply_buf;
	char					*buf_p;
	FILE					*fp;
	char 					*ptr;
	short					errflag = FALSE;
	short					temp;
	short					c;
	extern char 			*optarg;
	extern short			optind;
	short					verbose = FALSE;
	unsigned short			port_id = DEF_PORT_ID;

	
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, on_intr);

	while ((c = getopt(argc,argv,"vp:")) != -1) 
		switch (c) {
		case 'v' : 
				verbose = TRUE;
				break;
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

	if ((argc - optind) != 3  ||  errflag) {
printf("Usage : %s  [-v] [-p <port id>] <dest host> <line disc ID> <file>\n",
				argv[0]);
		exit(1);
	}
	
	dest_host  = (unsigned short) strtol(argv[optind],&ptr,BASE_TEN);
	if (*ptr != NULL) {
		printf("%s error: dest_host <%s> not an integer\n",argv[0],argv[optind]);
		errflag = TRUE;
	}
	optind++;
	line_disc = temp  = (unsigned short) strtol(argv[optind],&ptr,BASE_TEN);
	if (*ptr != NULL) {
		printf("%s error: line_disc_ID <%s> is not an integer\n",argv[0],argv[optind]);
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



	fp = fopen(argv[optind++], "rb");
	if (fp == NULL) {
		perror("open");
		exit(1);
	}

	if (cci_init(dest_host,port_id) != E_OK) {
		perror("cci init");
		myexit(1);
	}

	req_buf.create_req.line_disc_id = line_disc;
	req_buf.create_req.mem_size = 1024L;

	req_buf.buf[0] = CCI_CREATE;

	buf_p =	(char *)malloc(1024);
	if (buf_p == NULL) {
		perror("malloc");
		myexit(1);
	}

	actual = cci_send_cmd(CCI_PORTID, &req_buf, &crt_reply_buf, NULL, 0L, 
							NULL, 0L, &status);
	if (crt_reply_buf.status == CCI_ERROR) {
		printf("CCI CREATE Error\n");
		myexit(1);
	}

	if (crt_reply_buf.status != CCI_LINE_DISC_LOADED) {
		buf_len = crt_reply_buf.buf_size;
		offset  = crt_reply_buf.offset;

		while (buf_len > 0) {

			if (verbose) {
				printf("DOWNLOADing Line Disc %2d : %6d Bytes At Offset %6d\n",
						line_disc, buf_len, offset);
				}

			actual = myread(fp, buf_p, offset, buf_len);

			if (actual != buf_len) {
				printf("READ Error, FREEing Up Line Discipline %d\n", 
						line_disc);
				req_buf.buf[0] = CCI_FREE;
				req_buf.buf[2] = line_disc;
				actual = cci_send_cmd(CCI_PORTID, &req_buf, &free_reply_buf,
								NULL, 0L, NULL, 0L, &status);
				if ((status != E_OK) || (free_reply_buf.status != E_OK))
					printf("FREE Error\n");
				myexit(1);
			}

			req_buf.buf[0] = CCI_DOWNLOAD;
			req_buf.buf[2] = line_disc; 
			actual = cci_send_cmd(CCI_PORTID, &req_buf, &dl_reply_buf,
								buf_p, buf_len, NULL, 0L, &status);
			if (status != E_OK) {
				printf("SEND CMD Error : %x\n", status);
				myexit(1);
			}

			if (dl_reply_buf.status != E_OK) {
				printf("DOWNLOAD Error, FREEing Up Line Discipline %d\n", 
						line_disc);
				req_buf.buf[0] = CCI_FREE;
				req_buf.buf[2] = line_disc;
				actual = cci_send_cmd(CCI_PORTID, &req_buf, &free_reply_buf, 
									NULL, 0L, NULL, 0L, &status);
				if ((status != E_OK) || (free_reply_buf.status != E_OK))
					printf("FREE Error\n");

				myexit(1);
			}
				

				
			buf_len = dl_reply_buf.buf_size;
			offset  = dl_reply_buf.offset;

		}				

	printf("LOADed Line Discipline %d\n",line_disc);
	} else 
		printf("Line Discipline %d Is Already Loaded\n", line_disc);
		

}
