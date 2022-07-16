/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi:ID/sd01/space.c	1.3.2.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/sdi.h"
#include "sys/scsi.h"
#include "sys/elog.h"
#include "sys/vtoc.h"
#include "sys/sd01.h"
#include "sys/sdi_edt.h"
#include "sys/conf.h"
#include "config.h"

int	sd01devflag = D_NEW | D_DMA ;	/* SVR4 Driver Flags */

struct disk	*Sd01_dp;		/* Disk structure pointer	*/
struct job	*Sd01_jp;		/* Job structure pointer	*/

struct tc_data Sd01_data[] = { 
	"EMULEX  MD21/S2     ESDI", 4,	/* Bridge Cntrl.		*/
	"EMULEX  MD23/S2     ESDI", 4,	/* Bridge Cntrl.		*/
	"AT&T    KS23483         ", 1,	/* Embedded Generic		*/
	"AT&T    KS23483-3       ", 1,	/* Embedded 300MB Generic	*/
	"AT&T    KS23483-6       ", 1,	/* Embedded 600MB Generic	*/
	"AT&T    KS23483-F       ", 1,	/* Embedded Fixed Generic	*/
	"HP      97536D          ", 1,	/* DIF Embedded Cntrl. 		*/
	"HP      97536T          ", 1,	/* SE Embedded Cntrl.		*/
	"HP      7937S           ", 1,	/* DIF Embedded Cntrl.		*/
	"CDC     94161-9         ", 1,	/* SE Embedded Cntrl.		*/
	"CDC     94171-9         ", 1,	/* DIF Embedded Cntrl.		*/
	"CDC     94181-15        ", 1,	/* DIF Embedded Cntrl.		*/
	"MAXTOR  XT-4170S        ", 1,	/* SE Embedded Cntrl.	*/
	"MAXTOR  XT-4280S        ", 1,	/* SE Embedded Cntrl.	*/
	"MAXTOR  XT-4380S        ", 1,	/* SE Embedded Cntrl.	*/
	"MAXTOR  XT-8380S        ", 1,	/* SE Embedded Cntrl.	*/
	"MAXTOR  XT-8760S        ", 1,	/* SE Embedded Cntrl.	*/
	"QUANTUM P40S 940-40-94xx", 1,	/* SE Embedded Cntrl.	*/
	"QUANTUM P80S 980-80-94xx", 1,	/* SE Embedded Cntrl.	*/
	"QUANTUM Q2802022003-REV0", 1,	/* SE Embedded Cntrl.	*/
	"MICROP  1674-07MB1036509", 1,  /* SE Embedded Cntrl.	*/
	"MICROP  1578-15MB1036509", 1,  /* SE Embedded Cntrl.	*/
	"MICROP  1588-15MB1036509", 1,  /* SE Embedded Cntrl.	*/
};

struct drv_majors Sd01_majors[] = {
	SD01_BMAJOR_0, SD01_CMAJOR_0
#if defined(SD01_BMAJOR_1) && defined(SD01_CMAJOR_1)
	,
	SD01_BMAJOR_1, SD01_CMAJOR_1
#endif
#if defined(SD01_BMAJOR_2) && defined(SD01_CMAJOR_2)
        ,
	SD01_BMAJOR_2, SD01_CMAJOR_2
#endif
#if defined(SD01_BMAJOR_3) && defined(SD01_CMAJOR_3)
        ,
	SD01_BMAJOR_3, SD01_CMAJOR_3
#endif
#if defined(SD01_BMAJOR_4) && defined(SD01_CMAJOR_4)
        ,
	SD01_BMAJOR_4, SD01_CMAJOR_4
#endif
#if defined(SD01_BMAJOR_5) && defined(SD01_CMAJOR_5)
        ,
	SD01_BMAJOR_5, SD01_CMAJOR_5
#endif
#if defined(SD01_BMAJOR_6) && defined(SD01_CMAJOR_6)
        ,
	SD01_BMAJOR_6, SD01_CMAJOR_6
#endif
};

char   Sd01_debug[] = {
	0,0,0,0,0,0,0,0,0,0		/* Debug levels			*/
};

int	Sd01_datasz  = DATASZ;		/* Number of supported TC's   	*/

#if defined(SD01_CMAJORS)
int	Sd01_cmajors = SD01_CMAJORS;	/* Number of c major numbers	*/
#else
int	Sd01_cmajors = 1;		/* Default 1 major number	*/
#endif

int	Sd01log_marg = 0;		/* Marginal bad block logging   */
short	Sd01diskinfo = 0x2040;		/* Disk parameter flag		*/

int	Sd01_jobs    = 100;		/* Number of job structures	*/
