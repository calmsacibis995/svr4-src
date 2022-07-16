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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/410fw.d/ttyswitch.c	1.3"

static char ttyswitch_copyright[] = "Copyright 1987 Intel Corp.  461797";

#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/param.h"
#include "sys/conf.h"
#include "sys/immu.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/file.h"
#include "sys/sysinfo.h"
#include "sys/mps.h"
#include "sys/atcs.h"
#include "sys/fcntl.h"
#include <stdio.h>

#define BASE_TEN	10


main(argc, argv)

int argc;
char *argv[];

{
	int 	fd;
	int	status;
	int new_host;
	struct termio term_param;
	char 	*ptr;

	if (argc != 2) {
		fprintf(stderr,"Usage:%s <newhost>\n", argv[0]);
		exit(-1);
	}

	
	new_host  =  strtol(argv[1],&ptr,BASE_TEN);
	if (*ptr != NULL) {
		fprintf(stderr,"%s: new_host <%s> not an integer\n",argv[0],argv[1]);
		exit(-1);
	}
	
	fd = open("/dev/tty", O_RDWR);
	if (fd == -1) {
		perror("open");
		exit(-1);
	}
	
	ioctl (fd, TCGETA, &term_param);

	status = ioctl(fd, ATCS_SWITCH,  new_host);

	if (status == -1) {
		perror("switch");
		exit(-1);
	}

	ioctl (fd, TCSETA, &term_param);

	exit(0);
}
