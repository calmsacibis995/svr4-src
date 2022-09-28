/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/version.h	1.1"
/* SCCSID(@(#)version.h	3.8	LCC);	/* Modified: 17:45:42 6/14/89 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

/******************************************************************************
*
*  This file is included by all of the main files that make up the unix
*  server portions of PCI.
*
*****************************************************************************/

#ifdef IBM_SERCHK
#define VERS_MAJOR	1
#define VERS_MINOR	0
#define VERS_SUBMINOR	0

#else

#define VERS_MAJOR	2
#define VERS_MINOR	8
#define VERS_SUBMINOR	9	/* 2_x after 2_8 but before 2_9 */
#endif

struct version { 
	short 	vers_major,
		vers_minor,
		vers_subminor;
	};



extern char server_version[];
extern char *bridge_version;

#ifdef FAST_LSEEK
#define	SERVER_VERSION	"F=1,PCI=3.0.0,FAST_LSEEK=1.0"
#else
#define	SERVER_VERSION	"F=1,PCI=3.0.0"
#endif	/* FAST_LSEEK */

extern unsigned short bridge_ver_flags;

/* flags for bridge_ver_flags: */
#define	V_FAST_LSEEK	0x0001
#define	V_ERR_FILTER	0x0002

