/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:siginfo.h	1.1"

#include <sys/types.h>
#include <sys/siginfo.h>

struct siginfolist {
	int nsiginfo;
	char **vsiginfo;
};

extern char * _sys_illlist[];
extern char * _sys_fpelist[];
extern char * _sys_segvlist[];
extern char * _sys_buslist[];
extern char * _sys_traplist[];
extern char * _sys_cldlist[];
extern struct siginfolist _sys_siginfolist[];

#if defined(__STDC__)

extern void psiginfo(siginfo_t *, char *);

#else

extern void psiginfo();

#endif
