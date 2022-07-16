/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_EUC_H
#define _SYS_EUC_H

#ident	"@(#)head.sys:sys/euc.h	1.2.7.1"

#ifndef	NOTASCII
#define	SS2	0x8e
#define	SS3	0x8f

	/* NOTE: c of following macros must be the 1st byte of characters */
#define	ISASCII(c)	(!((c) & ~0177))
#define	NOTASCII(c)	((c) & ~0177)
#define	ISSET2(c)	((c) == SS2)
#define	ISSET3(c)	((c) == SS3)
#define ISPRINT(c, wp)	(wp._multibyte && !ISASCII(c) || isprint(c))
			/* eucwidth_t wp; */

typedef struct {
	short int _eucw1, _eucw2, _eucw3;	/*	EUC width	*/
	short int _scrw1, _scrw2, _scrw3;	/*	screen width	*/
	short int _pcw;		/*	WIDE_CHAR width	*/
	char _multibyte;	/*	1=multi-byte, 0=single-byte	*/
} eucwidth_t;
#endif


#endif	/* _SYS_EUC_H */
