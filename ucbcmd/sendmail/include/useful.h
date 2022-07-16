/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbsendmail:include/useful.h	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
**  USEFUL.H -- Some useful stuff.
**
*/

# ifndef makedev
# include <sys/types.h>
# endif

/* support for bool type */
typedef char	bool;
# define TRUE	1
# define FALSE	0

# ifndef NULL
# define NULL	0
# endif NULL

/* bit hacking */
# define bitset(bit, word)	(((word) & (bit)) != 0)

/* some simple functions */
# ifndef max
# define max(a, b)	((a) > (b) ? (a) : (b))
# define min(a, b)	((a) < (b) ? (a) : (b))
# endif max

/* assertions */
# ifndef NASSERT
# define ASSERT(expr, msg, parm)\
	if (!(expr))\
	{\
		fprintf(stderr, "assertion botch: %s:%d: ", __FILE__, __LINE__);\
		fprintf(stderr, msg, parm);\
	}
# else NASSERT
# define ASSERT(expr, msg, parm)
# endif NASSERT

/* sccs id's */
# ifndef lint
#ifdef __STDC__
# define SCCSID(arg) static char sccsid[]=#arg
#else
# define SCCSID(arg) static char sccsid[]="arg"
#endif
# else lint
# define SCCSID(arg)
# endif lint

/* define the types of some common functions */
extern char	*malloc();
#ifndef SYSV
extern char	*strcpy(), *strncpy();
extern char	*strcat(), *strncat();
extern int	errno;
extern char	*index(), *rindex();
extern char	*sprintf();
#else
#include <string.h>
#include <errno.h>
#define index(a,b) strchr(a,b)
#define rindex(a,b) strrchr(a,b)
#define bcopy(a,b,c) memcpy(b,a,c)
#define bzero(a,b)  memset(a,0,b)
#define sigmask(m)      (1 << ((m)-1))
#endif
extern time_t	time();
extern char	*ctime();
# ifndef V6
extern char	*getenv();
# endif V6
# ifndef SYSV
# ifndef VMUNIX
typedef unsigned short	u_short;
typedef long		u_long;
typedef char		u_char;
typedef int		void;
# endif VMUNIX
# endif SYSV
