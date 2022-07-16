/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_MODE_H
#define _SYS_MODE_H

#ident	"@(#)head.sys:sys/mode.h	1.2.7.1"

/*
 * REQUIRES sys/stat.h
 * REQUIRES sys/vnode.h
 */

/*
 * Conversion between vnode types/modes and encoded type/mode as
 * seen by stat(2) and mknod(2).
 */
extern enum vtype	iftovt_tab[];
extern ushort		vttoif_tab[];
#define IFTOVT(M)	(iftovt_tab[((M) & S_IFMT) >> 12])
#define VTTOIF(T)	(vttoif_tab[(int)(T)])
#define MAKEIMODE(T, M)	(VTTOIF(T) | ((M) & ~S_IFMT))

#endif	/* _SYS_MODE_H */
