/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_PTEM_H
#define _SYS_PTEM_H

#ident	"@(#)head.sys:sys/ptem.h	11.7.3.1"

#ifndef _SYS_TTOLD_H
#ifndef _SYS_TERMIOS_H

/* Windowing structure to support TIOCSWINSZ/TIOCGWINSZ */
struct winsize {
	unsigned short ws_row;       /* rows, in characters*/
	unsigned short ws_col;       /* columns, in character */
	unsigned short ws_xpixel;    /* horizontal size, pixels */
	unsigned short ws_ypixel;    /* vertical size, pixels */
};

#endif /* end _SYS_TERMIOS_H */
#endif /* end _SYS_TTOLD_H */
/*
 * The ptem data structure used to define the global data
 * for the psuedo terminal emulation streams module
 */
struct ptem
{
	long cflags;		/* copy of c_cflags */
	mblk_t *dack_ptr;	/* pointer to preallocated message block used to ACK disconnect */
	queue_t *q_ptr;		/* pointer to the ptem read queue */
	struct winsize wsz;	/* struct to hold the windowing info. */
	unsigned short state;	/* state of ptem entry, free or not */
};
/*
 * state flags
 * if state is zero then ptem entry is free to be allocated
 */
#define INUSE 		0x1	/* Internal ptem entry in use */
#define OFLOW_CTL 	0x2	/* Outflow control on */
/*
 * Constants used to distinguish between a common function invoked
 * from the read or write side put procedures
 */
#define	RDSIDE	1
#define	WRSIDE	2

#endif	/* _SYS_PTEM_H */
