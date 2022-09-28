/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)acomp:common/host.h	55.1"
/* host.h */

/* Declarations for the host system. */

/* These are good for a host with 32-bit ints. */

typedef	unsigned char I7;	/* has at least 7 bits */
typedef int I16;		/* has at least 16 bits */
typedef unsigned short U16;	/* has at least 16 bits, unsigned */
typedef int I32;		/* has at least 32 bits */
typedef unsigned long U32;	/* has at least 32 bits, unsigned */
typedef unsigned int SIZE;	/* type (presumed unsigned) that can
				** hold the size (number of elements),
				** and, consequently the index number
				** for any host array
				*/
/* typedef long BITOFF;		/* holds bit offsets for the target machine */
typedef long CONVAL;		/* type in which integral constants get folded */
typedef unsigned long UCONVAL;	/* type in which unsigned integral constants
				** get folded
				*/

/* Limits for constant folding.  Minimum/maximum values for
** CONVAL object.
*/
#ifdef	__STDC__
#include <limits.h>
#define	H_CON_MIN	LONG_MIN
#define	H_CON_MAX	LONG_MAX
#else
#define	H_CON_MIN	(-2147483647-1)
#define	H_CON_MAX	2147483647
#endif

/* Assume the host supports IEEE format floating point and
** ieee.h trapping if not a VAX or Amdahl.
*/
#if !defined(IEEE_HOST) && !(defined(vax) || defined(uts))
#define	IEEE_HOST
#endif
