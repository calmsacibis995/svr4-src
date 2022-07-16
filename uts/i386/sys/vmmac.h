/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_VMMAC_H
#define _SYS_VMMAC_H

#ident	"@(#)head.sys:sys/vmmac.h	1.9.3.2"

#include <sys/sysmacros.h>


/* Average new into old with aging factor time */
#define	ave(smooth, cnt, time) \
	smooth = ((time - 1) * (smooth) + (cnt)) / (time)

/* XXX - this doesn't really belong here */
#define	outofmem()	wakeprocs((caddr_t)proc_pageout, PRMPT)

#endif	/* _SYS_VMMAC_H */
