/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:hdrs/bkstatus.h	1.8.2.1"

/* This file contains definintions about the backup status table */

/* SCHEDULE TABLE DEFINITIONS */
/* Field Names */
#define	ST_JOBID	(unsigned char *)"jobid"
#define	ST_UID	(unsigned char *)"user"
#define	ST_TAG	(unsigned char *)"tag"
#define	ST_ONAME	(unsigned char *)"oname"
#define	ST_ODEVICE	(unsigned char *)"odevice"
#define	ST_STARTTIME	(unsigned char *)"starttime"
#define	ST_DGROUP	(unsigned char *)"dgroup"
#define	ST_DDEVICE	(unsigned char *)"ddevice"
#define	ST_DCHAR	(unsigned char *)"dchar"
#define	ST_STATUS	(unsigned char *)"status"
#define	ST_EXPLANATION	(unsigned char *)"explanation"

/* Rotation Field */
#define ST_ROTATE_MSG	"ROTATION="

/* ENTRY FORMAT */
#define	ST_ENTRY_F	(unsigned char *)\
	"jobid:user:tag:oname:odevice:starttime:dgroup:ddevice:dchar:status:explanation"

/* Values for status field */
#define	ST_ACTIVE	(unsigned char *)"active"
#define	ST_PENDING	(unsigned char *)"pending"
#define	ST_WAITING	(unsigned char *)"waiting"
#define	ST_HALTED	(unsigned char *)"suspended"
#define	ST_FAILED	(unsigned char *)"failed"
#define	ST_SUCCESS	(unsigned char *)"completed"
#define ST_STATES	(unsigned char *)"acfpsw"

/* START and STOP entries */
#define	ST_START	(unsigned char *)"START"
#define	ST_STOP	(unsigned char *)"STOP"
