/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libpkg:stubs.c	1.1.2.1"

/* LINTLIBRARY */

#ifdef PRESVR4
#include <sys/types.h>
#include <sys/stat.h>

symlink(a, b) char *a, *b; {return (-1);}
__makedev(a, b, c) int a; major_t b; minor_t c; {return (-1);}
__major(a, b) int a; dev_t b; {return (-1);}
__minor(a, b) int a; dev_t b; {return (-1);}
lstat(a, b) char *a; struct stat *b; {return (-1);}
readlink(a, b, c) char *a, *b; int c; {return (-1);}
#endif /* PRESVR4 */
