#ident	"@(#)Space.c	1.2	92/02/17	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi.in:ID/st01/Space.c	1.3.2.1"

#include "sys/types.h"
#include "sys/scsi.h"
#include "sys/sdi_edt.h"
#include "sys/conf.h"
#include "config.h"

int	st01devflag = D_NEW | D_DMA | D_TAPE; /* SVR4 Driver Flags */

struct tc_data St01_data[] = { 
	"ARCHIVE Python 25501-003", 1,
	"ARCHIVE Python 25501-005", 1,
	"ARCHIVE VIPER 150  21247", 1,	/* SE Embedded Cntrl.	*/
	"ARCHIVE VIPER 60   21116", 1,	/* SE Embedded Cntrl.	*/
	"AT&T    KS22762         ", 1,
	"AT&T    KS23495         ", 1,
	"CDC     92181           ", 1,
	"CDC     92185           ", 1,
	"CIPHER  ST150S2/90      ", 1,
	"EMULEX  MT02/S1 +CCS INQ", 1,
	"EXABYTE EXB-8200        ", 1,
	"HP      88780           ", 1,
	"HPCIPHER            M990", 1,
	"NCR H6210-STD1-01-46C632", 1,
	"SONY    SDT-1000        ", 1,
	"TANDBERG TDC 3600       ", 1,
	"WANGTEK 5125ES SCSI ES41", 1,	/* SE Embedded Cntrl.	*/
	"WANGTEK 5150ES SCSI ES41", 1,
	"WANGTEK 5150ES SCSI-36  ", 1,
	"WANGTEK 5525ES      ES41", 1,
	"WANGTEK 6130-FS         ", 1,
	"WANGTEK KS23417         ", 1,
	"WANGTEK KS23465         ", 1,
	"WANGTEK KS23569         ", 1,
};

int	St01_datasz = (sizeof(St01_data) / sizeof(struct tc_data));	
				/* Number of supported TC's   	*/

int	St01_cmajor = ST01_CMAJOR_0;	/* Character major number	*/

int	St01_jobs = 20;		/* Allocation per LU device	*/
