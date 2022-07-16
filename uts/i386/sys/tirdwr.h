/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_TIRDWR_H
#define _SYS_TIRDWR_H

#ident	"@(#)head.sys:sys/tirdwr.h	1.1"

struct trw_trw {
	long 	 trw_flags;
	queue_t	*trw_rdq;
	mblk_t  *trw_mp;
};

#endif	/* _SYS_TIRDWR_H */
