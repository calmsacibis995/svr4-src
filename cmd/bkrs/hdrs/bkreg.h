/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:hdrs/bkreg.h	1.3.2.1"

/* This file contains definintions about the backup schedule table */

/* Default table names */
#define BKREGTAB	"bkreg.tab"

/* SCHEDULE TABLE DEFINITIONS */
/* Field Names */
#define	R_TAG		(unsigned char *)"tag"
#define R_ONAME	(unsigned char *)"oname"
#define	R_ODEVICE	(unsigned char *)"odevice"
#define	R_WEEK	(unsigned char *)"week"
#define R_DAY		(unsigned char *)"day"
#define	R_OLABEL	(unsigned char *)"olabel"
#define	R_OPTIONS	(unsigned char *)"options"
#define	R_PRIORITY	(unsigned char *)"priority"
#define R_METHOD	(unsigned char *)"method"
#define	R_DGROUP	(unsigned char *)"dgroup"
#define R_DDEVICE	(unsigned char *)"ddevice"
#define	R_DCHAR	(unsigned char *)"dchar"
#define R_DMNAME	(unsigned char *)"dmname"
#define R_DEPEND	(unsigned char *)"depend"

/* Rotation Field */
#define R_ROTATE_MSG	"ROTATION="

/* Rotation Start Msg */
#define R_ROTATE_START_MSG	"ROTATION STARTED="

/* Schedule table format */
#define R_BKREG_F	(unsigned char *)\
	"tag:oname:odevice:olabel:week:day:method:options:priority:dgroup:ddevice:dchar:dmname:depend:other"
