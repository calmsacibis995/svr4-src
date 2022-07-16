/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _USTAT_H
#define _USTAT_H

#ident	"@(#)head:ustat.h	1.3.1.6"

#include <sys/types.h>
#include <sys/ustat.h>

#if defined(__STDC__)
extern int ustat(dev_t, struct ustat *);
#else
extern int ustat();
#endif	/* end defined(_STDC) */

#endif /* _USTAT_H */
