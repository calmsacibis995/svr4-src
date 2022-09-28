/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)fmli:inc/detabdefs.h	1.1"

/* Note: this file created with tabstops set to 4.
 *
 * Definitions for the Object Detection Function Table (ODFT, pronounced
 * "oddfoot").  On of these will exist per system, and it defines a set
 * of functions which can be used to detect objects on the system.
 */

#define MAXMAGIC	256			/* max num of magic numbers detectable*/
#define MAXODFT		50			/* max detect functions */

#define IDF_ZLASC	0
#define IDF_ASC		1
#define IDF_PCTRANS	2
#define IDF_TRANS	3
#define IDF_CORE	4
#define IDF_ARCH	5
#define IDF_ENCRYPT	6
/* 7 is not used now */
#define IDF_UNKNOWN	8
#define IDF_MAIL_IN	9
#define IDF_MAIL_OUT	10

struct odft_entry {
	char objtype[OTYPESIZ];			/* the object this detects */
	char *defodi;					/* default odi */
	long defmask;					/* addition to the mask when detected*/
	int	 func_type;					/* what kind of function */
	int  intern_func;				/* index into internal function table*/
	char *extern_func;				/* name of a unix program to detect */
	long *magic_offset;				/* offset into file of magic number*/
	char *magic_bytes;				/* byte of the magic number */
};
