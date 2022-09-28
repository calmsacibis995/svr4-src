/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:hdrs/backup.h	1.9.2.1"

/* This file contains definintions common to backup commands */

/* rotation period stuff */
#define	SUNDAY_F	(1<<0)
#define	MONDAY_F	(1<<1)
#define	TUESDAY_F	(1<<2)
#define	WEDNESDAY_F	(1<<3)
#define	THURSDAY_F	(1<<4)
#define	FRIDAY_F	(1<<5)
#define SATURDAY_F	(1<<6)

#define WK_PER_YR	52
#define	TMDAY_TO_F( t )	(1<<((t)->tm_wday))
#define	BK_DAYSET(r,w,d)	r[w] |= (1<<(d))
#define	BK_DAYUNSET(r,w,d)	r[w] &= ~(1<<(d))

#define	IS_DEMAND(d)	(d[WK_PER_YR] != 0)
#define	DEFAULT_PERIOD	1

/* Method that does table of contents */
#define	TOC_METHOD	"incfile"

/* Demand backups are recorded in offset WK_PER_YR */
typedef unsigned char bkrotate_t[WK_PER_YR + 1];

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

/* General defines */
#define	BEGIN_CRITICAL_REGION	if( bklevels == 0 ) (void) sighold( SIGUSR1 ); bklevels++
#define	END_CRITICAL_REGION		if( --bklevels == 0 ) (void) sigrelse( SIGUSR1 )
#define	BEGIN_SIGNAL_HANDLER	bklevels++
#define	END_SIGNAL_HANDLER	bklevels--

#define	DEFAULT_PRIORITY	0
#define MAX_PRIORITY		100

/* Types for bkspawn() */
#define	BKARGS	1
#define	BKARGV	2

/* Name for IPC system */
#define	BKNAME	"backup"
#define	BKLOGFILE	"bklog"

/* Names for various tables */
#define	BK_HISTLOG	"bkhist.tab"
#define	BK_STATLOG	"bkstatus.tab"
#define RS_NTFYLOG	"rsnotify.tab"
#define BK_EXTAB	"bkexcept.tab"

/* Sizes of things */
#define	BKFNAME_SZ	1024	/* When PATH_MAX is straightened out, we'll use it */
#define	BKTEXT_SZ	150
#define	BKLABEL_SZ	50
#define	BKJOBID_SZ	15
#define	BKTAG_SZ	25
#define	BKMAIL_SZ	75
#define	BKDEVICE_SZ	125

/* Method Identifier Structure */
typedef struct method_id_s {
	char jobid[ BKJOBID_SZ + 1 ];
	char tag[ BKTAG_SZ ];
} method_id_t;

/* Stuff that belongs in devmgmt.h when bkgetvol() -> getvol() */
#define	DM_CHKLBL	0x40
#define	DM_AUTO	0x80

/* These return codes should go there, too. */
#define	DMR_SUCCESS	0
#define DMR_BADDEVICE	1
#define	DMR_UNKDEVICE	2
#define	DMR_QUIT	3
#define	DMR_DIFFLABEL	4

