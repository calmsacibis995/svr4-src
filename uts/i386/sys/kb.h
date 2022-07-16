/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/kb.h	1.1.2.1"

#ifndef	_SYS_KB_H
#define	_SYS_KB_H

#define SEND2KBD(port, byte) { \
	while (inb(KB_STAT) & KB_INBF) \
		; \
	outb(port, byte); \
}

#endif /* _SYS_KB_H */
