/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:hdrs/bkmsgs.h	1.9.4.1"

/*
	This file contains definitions, etc. for messaging in the backup service.
*/

#include <sys/types.h>

/* Message types */
#define	START	1
#define	ESTIMATE	2
#define	FAILED	3
#define	DONE	4
#define	GET_VOLUME	5
#define	VOLUME	6
#define	DISCONNECTED	7
#define	SUSPEND	8
#define	RESUME	9
#define	CANCEL	10
#define	TEXT	11
#define	SUSPENDED	12
#define	RESUMED	13
#define	CANCELED	14
#define	HISTORY	15
#define	DOT	16
#define	RSRESULT	17
#define	RSTOC	18
#define	INVL_LBLS	19

/* Individual structures for each type of message having data with it. */

typedef struct start_s {
	int options;
	uid_t	my_uid;	/* UID of calling process */
	gid_t my_gid;	/* GID of calling process */
	int week, day;	/* Do backups for a particular week/day */
	char table[ BKFNAME_SZ ];
	char fname[ BKFNAME_SZ ];
	char user[ BKMAIL_SZ ];
} start_m;

/* Start message options */
#define	unused	0x1
#define	S_ESTIMATE	0x2
#define	S_NO_EXECUTE	0x4
#define	S_INTERACTIVE	0x8
#define	S_DEMAND	0x10
#define	S_SEND_DOTS	0x20
#define	S_SEND_FILENAMES	0x40

typedef struct failed_s {
	int reason;
	char errmsg[ BKTEXT_SZ ];
	method_id_t method_id;
} failed_m;

typedef struct getvolume_s {
	int flags;
	char label[ BKLABEL_SZ + 1 ];
} getvolume_m;

/* Values for flags */
#define	GV_OVERRIDE	0x1

typedef struct volume_s {
	char label[ BKLABEL_SZ + 1 ];
	method_id_t method_id;
} volume_m;

typedef struct operator_s {
	int operator;
} operator_m;

typedef struct estimate_s {
	method_id_t method_id;
	int volumes;
	int blocks;
} estimate_m;

typedef struct inv_lbls_s {
	char label[ BKTEXT_SZ ];
} inv_lbls_m;

typedef	struct text_s {
	char text[ BKTEXT_SZ ];
} text_m;

typedef struct control_s {
	uid_t uid;
	pid_t pid;
	int flags;
} control_m;

/* Values for flags in control messages */
#define	CTL_ALL	0x1
#define	CTL_UID	0x2
#define	CTL_PID	0x4

typedef	struct done_s {
	int nblocks;
} done_m;

typedef	struct	history_s {
	int nvolumes;
	int size;
	time_t time;
	char oname[ BKDEVICE_SZ + 1 ];
	char odevice[ BKDEVICE_SZ + 1 ];
	char labels[ BKTEXT_SZ + 1 ];
	char tocname[ BKFNAME_SZ + 1 ];
	int	flags;
} history_m;

/* Values for flags in history message */
#define	HST_DO_ARCHIVE_TOC	0x1	/* send the TOC out to media? */
#define	HST_MODIFY	0x2	/* modify an existing entry */
#define	HST_IS_TOC	0x4	/* Labels refer to table of contents labels */
#define	HST_CONTINUE 0x8 /* Continuation message */

typedef struct	rsreturn_s {
	char jobid[ BKJOBID_SZ + 1 ];
	int retcode;
	char errmsg[ BKTEXT_SZ ];
} rsreturn_m;

typedef struct	rstoc_s {
	char tocname[ BKFNAME_SZ ];
} rstoc_m;

/* Union of all message data types */
typedef	union bkdata_u {
	control_m	control;
	done_m done;
	estimate_m estimate;
	failed_m failed;
	getvolume_m getvolume;
	history_m history;
	inv_lbls_m inv_lbls;
	operator_m operator;
	rsreturn_m rsret;
	rstoc_m	rst_o_c;
	start_m start;
	volume_m volume;
	text_m	text;
} bkdata_t;

/* A newly-arrived message is put into this structure */
typedef	struct queued_msg_s {
	struct queued_msg_s *next;
	pid_t originator;
	int type;
	bkdata_t	data;
} queued_msg_t;

