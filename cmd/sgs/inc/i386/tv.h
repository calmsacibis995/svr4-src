/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sgs-inc:i386/tv.h	1.1.1.1"
/*
 */

struct tventry {
	long	tv_addr;
	};

#define TVENTRY struct tventry
#define TVENTSZ sizeof(TVENTRY)
#define N3BTVSIZE	0x20000		/* size of 1 segment (128K) */
