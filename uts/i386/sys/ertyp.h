/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_ERTYP_H
#define _SYS_ERTYP_H

#ident	"@(#)head.sys:sys/ertyp.h	1.2.7.1"
/* bit position definintions for errno */

/* standard non-fatal messages (high byte of errno) */

#define	NON_FATALOC	24	/* location of non-fatal info in errno */
#define	NON_FATAL	1L << NON_FATALOC	/* smallest non-fatal error */

#define	SELF_CHECK	(0x80L << NON_FATALOC)
#define	NVRAM_WARN	(0x40L << NON_FATALOC)
#define	NVRAM_FAIL	(0x20L << NON_FATALOC)

/* additional information for disk sanity failures (2nd byte of errno) */

#define	DSK_ERLOC	16	/* location of disk error info in errno */

#define	DSK_IDLOC	(DSK_ERLOC + 7)		/* high bit is which disk */
#define	DSK_PHYS	(1L << DSK_ERLOC)
#define	DSK_WORD	(2L << DSK_ERLOC)
#define	DSK_COPY	(3L << DSK_ERLOC)
#define	DSK_INIT	(4L << DSK_ERLOC)
#define	DSK_PATTERN	(5L << DSK_ERLOC)
#define	DSK_MAP		6L		/* 6 & up for bad map sect 0 & up */

#define	DSK_ERTYP	(1L << DSK_ERLOC)	/* smallest disk error */

/* standard fatal errors (low short of errno) */

#define	CONFIG_FAIL	0x1L
#define	DISK_FAIL	0x2L
#define	BOOT_FAIL	0x4L
#define	MEM_FAIL	0x8L
#define	INCOMPATIBLE	0x10L
#define	FKEY_FAIL	0x20L
#define	EXC_FAIL	0x40L
#define	INT_FAIL	0x80L

/* sane flag for FWERR_NVR info described in nvram.h */
#define GOODERROR	0x600dbeef

#endif	/* _SYS_ERTYP_H */
