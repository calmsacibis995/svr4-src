/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_DL_H
#define _SYS_DL_H

#ident	"@(#)head.sys:sys/dl.h	1.3.6.1"

typedef	struct dl {
	ulong	dl_lop;
	long	dl_hop;
} dl_t;

extern dl_t	ladd();
extern dl_t	lsub();
extern dl_t	lmul();
extern dl_t	ldivide();
extern dl_t	lshiftl();
extern dl_t	llog10();
extern dl_t	lexp10();

extern dl_t	lzero;
extern dl_t	lone;
extern dl_t	lten;

#endif	/* _SYS_DL_H */
