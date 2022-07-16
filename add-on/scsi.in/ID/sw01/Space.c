/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989 TOSHIBA CORPORATION		*/
/*		All Rights Reserved			*/

/*	Copyright (c) 1989 SORD COMPUTER CORPORATION	*/
/*		All Rights Reserved			*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF		*/
/*	TOSHIBA CORPORATION and SORD COMPUTER CORPORATION	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi.in:ID/sw01/Space.c	1.3.2.1"



#include "sys/types.h"
#include "sys/scsi.h"
#include "sys/sdi_edt.h"
#include "sys/conf.h"
#include "config.h"

int	sw01devflag = D_NEW | D_DMA ;	/* SVR4 Driver Flags */

struct tc_data Sw01_data[] = { 
	"TOSHIBA WM-D070         ", 1,	/* Toshiba WORM		*/
	"TOSHIBA WM-C050         ", 1,	/* Toshiba WORM		*/
	"MAXTOR  RXT-800HS       ", 1,	/* Maxtor WORM		*/
};

int	Sw01_datasz = (sizeof(Sw01_data) / sizeof(struct tc_data));
					/* Number of supported TC's   	*/
int	Sw01_cmajor = SW01_CMAJOR_0;		/* Character major number	*/
int	Sw01_bmajor = SW01_BMAJOR_0;		/* Block major number		*/

int	Sw01_jobs = 100;		/* Allocation per LU device 	*/
