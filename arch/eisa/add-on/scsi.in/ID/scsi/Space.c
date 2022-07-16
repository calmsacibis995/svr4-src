/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)eisa:add-on/scsi.in/ID/scsi/Space.c	1.3"

#include "sys/types.h"
#include "sys/sdi_edt.h"
#include "sys/sdi.h"
#include "sys/scsi.h"
#include "sys/esc.h"
#include "config.h"

#define SCSI_BLKS   (SCSI_CNTLS * 100)

int	sc_major = SCSI_CMAJOR_0;	/* SCSI driver major number	*/
int	sc_hacnt = SCSI_CNTLS;	/* Total number of controllers	*/
int	sc_sbcnt = SCSI_BLKS;	/* Total number of SCSI Blocks	*/
int	sc_hiwat = 8;		/* LU queue high water mark	*/
int	sc_lowat = 2;		/* LU queue low water mark	*/

long	sdi_hacnt = SCSI_CNTLS;
long	sdi_started = 0;
struct	ver_no    sdi_ver;

struct  scsi_ha	  sc_ha[SCSI_CNTLS];
struct  scsi_edt  sc_edt[SCSI_CNTLS * MAX_TCS];
struct  srb	  sc_sbtab[SCSI_BLKS];

struct  scsi_cfg  sc_cfg[SCSI_CNTLS] = 
{
	SCSI_ID,	/* HA 0 SCSI identifier	 */
	SCSI_0_VECT,	/* HA 0 interrupt vector */
	SCSI_0_SIOA	/* HA 0 base I/O address */

#ifdef SCSI_1
	,
	SCSI_ID,	/* HA 1 SCSI identifier	 */
	SCSI_1_VECT,	/* HA 1 interrupt vector */
	SCSI_1_SIOA	/* HA 1 base I/O address */
#endif
};
