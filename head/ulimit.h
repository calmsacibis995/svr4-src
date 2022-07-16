/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _ULIMIT_H
#define _ULIMIT_H

#ident	"@(#)head:ulimit.h	1.4"

#include <sys/ulimit.h>

#ifdef __STDC__
extern long ulimit(int, ...);
#else
extern long ulimit();
#endif

#endif	/* _ULIMIT_H */
