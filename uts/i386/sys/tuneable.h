/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_TUNEABLE_H
#define _SYS_TUNEABLE_H

#ident	"@(#)head.sys:sys/tuneable.h	11.8.3.1"

typedef struct tune {
	int	t_gpgslo;	/* If freemem < t_getpgslow, then start	*/
				/* to steal pages from processes.	*/
	int	t_gpgshi;	/* Once we start to steal pages, don't	*/
				/* stop until freemem > t_getpgshi.	*/
	int	t_padd;		/* Padding for driver compatibility	*/
	int	t_notused;	/* Not used				*/
	int	t_ageinterval;	/* Age process every so many seconds	*/
	int	t_padd1[3];	/* Padding for driver compatibility 	*/
	int	t_fsflushr;	/* The rate at which fsflush is run in	*/
				/* seconds.				*/
	int	t_minarmem;	/* The minimum available resident (not	*/
				/* swapable) memory to maintain in 	*/
				/* order to avoid deadlock.  In pages.	*/
	int	t_minasmem;	/* The minimum available swapable	*/
				/* memory to maintain in order to avoid	*/
				/* deadlock.  In pages.			*/
	int	t_dmalimit;	/* Last (exclusive) Dmaable Page No.	*/
	int t_flckrec;		/* max number of active frlocks */
	int	t_minakmem;	/* Threshhold of the minimum number of  */
				/* pages reserved as the soft kmem_alloc*/
				/* threshold to avoid deadlock 		*/
} tune_t;

extern tune_t	tune;

/*	The following is the default value for t_gpgsmsk.  It cannot be
 *	defined in /etc/master or /etc/system due to limitations of the
 *	config program.
 */

#define	GETPGSMSK	PG_REF|PG_NDREF

#endif	/* _SYS_TUNEABLE_H */
