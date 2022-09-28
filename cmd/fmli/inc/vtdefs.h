/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 *
 */

#ident	"@(#)fmli:inc/vtdefs.h	1.2"

/* see vt.h for previous "VT_" defines !!! */
#define VT_UPSARROW	 0100	
#define VT_DNSARROW	 0200	
#define VT_UPPARROW	 0400	
#define VT_DNPARROW	01000	
#define VT_NONUMBER	16384
#define VT_NOBORDER	32768

#define VT_UNDEFINED	((vt_id) -1)

/* indicates cost function to use when creating a new vt */
#define VT_NOOVERLAP	0
#define VT_CENTER	1
#define VT_COVERCUR	2
#define VT_NOCOVERCUR	3
#define NUMCOSTS	4
#define VT_COSTS	3	/* AND off the COST part of the flags */

#define STATUS_WIN	0
#define CMD_WIN		1
#define MESS_WIN	2

/* "funny" characters */
#define MENU_MARKER	'\1'	/* RIGHT ARROW */
