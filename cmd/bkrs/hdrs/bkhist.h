/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:hdrs/bkhist.h	1.3.2.1"

/* This file contains definintions about the backup history table */

/* SCHEDULE TABLE DEFINITIONS */
/* Field Names */
#define	H_TAG	(unsigned char *)"tag"
#define	H_DATE	(unsigned char *)"date"
#define	H_ONAME	(unsigned char *)"oname"
#define	H_ODEVICE	(unsigned char *)"odevice"
#define	H_METHOD	(unsigned char *)"method"
#define	H_OPTIONS	(unsigned char *)"options"
#define	H_DGROUP	(unsigned char *)"dgroup"
#define	H_DDEVICE	(unsigned char *)"ddevice"
#define	H_DCHAR	(unsigned char *)"dchar"
#define	H_DNVOL	(unsigned char *)"dnvol"
#define	H_SIZE	(unsigned char *)"size"
#define	H_DMNAME	(unsigned char *)"dmname"
#define	H_ARCHTOC	(unsigned char *)"archivetoc"
#define	H_TMNAME	(unsigned char *)"tmname"
#define	H_TOCNAME	(unsigned char *)"tocname"

/* Rotation Field */
#define H_ROTATE_MSG	"ROTATION="

/* Entry Format */
#define	H_ENTRY_F	(unsigned char *)\
	"tag:date:oname:odevice:method:options:dgroup:ddevice:dchar:dnvol:size:dmname:archivetoc:tmname:tocname"
