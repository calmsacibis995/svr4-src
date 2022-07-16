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

#ident	"@(#)scsi.in:ID/sc01/Space.c	1.3.1.1"


#include "sys/types.h"
#include "sys/scsi.h"
#include "sys/sdi_edt.h"
#include "sys/conf.h"
#include "config.h"

int	sc01devflag = D_NEW | D_DMA ;	/* SVR4 Driver Flags */

struct tc_data Sc01_data[] = { 
	"TOSHIBA CD-ROM DRIVE:XM ", 1,	/* TOSHIBA CD-ROM.		*/
};

int	Sc01_datasz = (sizeof(Sc01_data) / sizeof(struct tc_data));
					/* Number of supported TC's   	*/
int	Sc01_cmajor = SC01_CMAJOR_0;		/* Character major number	*/
int	Sc01_bmajor = SC01_BMAJOR_0;		/* Block major number		*/

int	Sc01_jobs = 100;		/* Allocation per LU device 	*/
