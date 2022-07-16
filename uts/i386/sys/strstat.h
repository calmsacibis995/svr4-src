/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_STRSTAT_H
#define _SYS_STRSTAT_H

#ident	"@(#)head.sys:sys/strstat.h	11.5.3.1"

/*
 * Streams Statistics header file.  This file
 * defines the counters which are maintained for statistics gathering
 * under Streams. 
 */

typedef struct {
	int use;	/* current item usage count */
	int total;	/* total item usage count */
	int max;	/* maximum item usage count */
	int fail;	/* count of allocation failures */
	} alcdat;

struct  strstat {
	alcdat stream;		/* stream allocation data */
	alcdat queue;		/* queue allocation data */
	alcdat msgblock;	/* message block allocation data */
	alcdat mdbblock;	/* mesg/data/buffer triplet allocation data */
	alcdat linkblk;		/* linkblk allocation data */
	alcdat strevent;	/* strevent allocation data */
	} ;


/* in the following macro, x is assumed to be of type alcdat */

#define BUMPUP(X)	{(X).use++;  (X).total++;\
			 if ((X).use > (X).max) (X).max = (X).use; }


/* per-module statistics structure */

struct module_stat {
	long ms_pcnt;		/* count of calls to put proc */
	long ms_scnt;		/* count of calls to service proc */
	long ms_ocnt;		/* count of calls to open proc */
	long ms_ccnt;		/* count of calls to close proc */
	long ms_acnt;		/* count of calls to admin proc */
	char *ms_xptr;		/* pointer to private statistics */
	short ms_xsize;		/* length of private statistics buffer */
	};


#endif	/* _SYS_STRSTAT_H */
