/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:macros.h	1.3.1.7"
/*
	numeric() is useful in while's, if's, etc., but don't use *p++
	max() and min() depend on the types of the operands
	abs() is absolute value
*/
# define numeric(c)		((c) >= '0' && (c) <= '9')
# define max(a,b) 		((a)<(b) ? (b) : (a))
# define min(a,b) 		((a)>(b) ? (b) : (a))
# define abs(x)			((x)>=0 ? (x) : -(x))

# define compare(str1,str2)	strcmp((str1),(str2))
# define equal(str1,str2)	!strcmp((str1),(str2))
# define length(str)		strlen(str)
# define size(str)		(strlen(str) + 1)

/*
	The global variable Statbuf is available for use as a stat(II)
	structure.  Note that "stat.h" is included here and should
	not be included elsewhere.
	Exists(file) returns 0 if the file does not exist;
	the flags word if it does (the flags word is always non-zero).
*/

#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef _SYS_STAT_H
#include <sys/stat.h>
#endif

extern struct stat Statbuf;
# define exists(file)		(stat(file,&Statbuf)<0 ? 0:Statbuf.st_mode)

extern long itol();

/*
	SAVE() and RSTR() use local data in nested blocks.
	Make sure that they nest cleanly.
*/
# define SAVE(name,place)	{ int place = name;
# define RSTR(name,place)	name = place;}

/*
	Use: DEBUG(sum,d) which becomes fprintf(stderr,"sum = %d\n",sum)
*/
#ifdef __STDC__
# define DEBUG(var,type)	fprintf(stderr,#var "= %" #type "\n", var)
# define SCCSID(arg)		static char Sccsid[]=#arg
#else
# define DEBUG(var,type)	fprintf(stderr,"var = %type\n", var)
# define SCCSID(arg)		static char Sccsid[]="arg"
#endif

/*
	Use of ERRABORT() will cause libS.a internal
	errors to cause aborts
*/
# define ERRABORT()	_error() { abort(); }

/*
	Use of USXALLOC() is required to force all calls to alloc()
	(e.g., from libS.a) to call xalloc().
*/

# define NONBLANK(p)		while (*(p)==' ' || *(p)=='\t') (p)++


/*
	A global null string.
*/
extern char	Null[1];

/*
	A global error message string.
*/
extern char	Error[128];
