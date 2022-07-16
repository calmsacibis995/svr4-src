/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi:ID/scsi/space.c	1.3.2.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/sdi.h"
#include "sys/sdi_edt.h"
#include "sys/immu.h"
#include "sys/had.h"
#include "sys/conf.h"
#include "config.h"

int	scsidevflag = D_NEW | D_DMA ;	/* SVR4 Driver Flags */

struct scsi_ctrl scsi_ctrl[SCSI_CNTLS];
struct scsi_edt sc_edt[MAX_TCS * SCSI_CNTLS];
union cdb_item scsi_cdb_pool[MAX_LUS * MAX_TCS * SCSI_CNTLS];
int scsi_cdbsz = (MAX_LUS * MAX_TCS * SCSI_CNTLS);
int scsi_cnt = SCSI_CNTLS;
long sdi_hacnt = SCSI_CNTLS;
int scsi_major = SCSI_CMAJOR_0;
char dmac[3] = {2, 1, 3};	/* DMA Channels 6, 5, 7	*/

struct scsi_addr scsi_ad[SCSI_CNTLS] = 
{
	SCSI_0_SIOA,	/* HA status register		*/
	SCSI_0_SIOA,	/* HA command register		*/
	SCSI_0_SIOA+1,	/* HA interrupt status register	*/
	SCSI_0_SIOA+1,	/* HA int. acknowledge register	*/
	SCSI_0_SIOA+2,	/* HA control register		*/
	SCSI_0_VECT	/* interrupt vector for HA 0	*/

#ifdef SCSI_1
	,
	SCSI_1_SIOA,	/* HA status register		*/
	SCSI_1_SIOA,	/* HA command register		*/
	SCSI_1_SIOA+1,	/* HA interrupt status register	*/
	SCSI_1_SIOA+1,	/* HA int. acknowledge register	*/
	SCSI_1_SIOA+2,	/* HA control register		*/
	SCSI_1_VECT	/* interrupt vector for HA 1	*/
#endif
#ifdef SCSI_2
	,
	SCSI_2_SIOA,	/* HA status register		*/
	SCSI_2_SIOA,	/* HA command register		*/
	SCSI_2_SIOA+1,	/* HA interrupt status register	*/
	SCSI_2_SIOA+1,	/* HA int. acknowledge register	*/
	SCSI_2_SIOA+2,	/* HA control register		*/
	SCSI_2_VECT	/* interrupt vector for HA 2	*/
#endif
};

int scsi_dbgsize = 11;
char scsi_Debug[11] = { 1, 0,0,0,0,0,0,0,0,0,0};
char scsi_Board[11] = { 1, 1,1,1,0,0,0,0,0,0,0};
