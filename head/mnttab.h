/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:mnttab.h	1.5.1.2"

#ifndef _MNTTAB_H
#define _MNTTAB_H

#define	NMOUNT	100

/* from time.h */
#ifndef _TIME_T
#define _TIME_T
typedef long	time_t;
#endif

/* Format of the /etc/mnttab file which is set by the mount(1m)
 * command
 */
struct mnttab {
	char	mt_dev[32],
		mt_filsys[32];
		short	mt_ro_flg;
	time_t	mt_time;
};

#endif 	/* _MNTTAB_H */
