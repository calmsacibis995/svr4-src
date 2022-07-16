/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:mon.h	1.8.2.2"

#ifndef _MON_H
#define _MON_H

struct hdr {
	char	*lpc;
	char	*hpc;
	int	nfns;
};

struct cnt {
	char	*fnpc;
	long	mcnt;
};

typedef unsigned short WORD;

#define MON_OUT	"mon.out"
#define MPROGS0	(150 * sizeof(WORD))	/* 300 for pdp11, 600 for 32-bits */
#define MSCALE0	4
#define NULL	0

#if defined(__STDC__)
extern void monitor(int (*)(), int (*)(), WORD *, int, int);
#else
extern void monitor();
#endif

#endif /* _MON_H */
