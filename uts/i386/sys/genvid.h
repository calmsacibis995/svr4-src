/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/genvid.h	1.1.2.1"

typedef struct gvid {
	unsigned long gvid_num;
	dev_t *gvid_buf;
	major_t gvid_maj;
} gvid_t;

#define	GVID_SET	1
#define	GVID_ACCESS	2

#define	GVIOC	('G'<<8|'v')
#define	GVID_SETTABLE	((GVIOC << 16)|1)
#define	GVID_GETTABLE	((GVIOC << 16)|2)
