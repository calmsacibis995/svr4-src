/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)acomp:common/target.h	52.5"
/* target.h */

/* Definitions for numeric limits and other characteristics
** in the target machine.  The names here deliberately mimic
** those in limits.h.
*/

/* These values are suitable for a machine with:
**	32 bit ints/longs
*/

#define	T_SCHAR_MIN	-128
#define	T_SCHAR_MAX	127
#define	T_UCHAR_MAX	255
#ifdef	C_CHSIGN
#  define T_CHAR_MIN	-128
#  define T_CHAR_MAX	127
#else
#  define T_CHAR_MIN	0
#  define T_CHAR_MAX	255
#endif

#define	T_SHRT_MIN	-32768
#define	T_SHRT_MAX	32767
#define	T_USHRT_MAX	65535

#define	T_INT_MIN	(-2147483647-1)
#define	T_INT_MAX	2147483647
#define	T_UINT_MAX	4294967295

#define	T_LONG_MIN	(-2147483647-1)
#define	T_LONG_MAX	2147483647
#define	T_ULONG_MAX	4294967295

#define	T_ptrdiff_t	TY_INT		/* type for pointer differences */
#define	T_ptrtype	TY_INT		/* integral type equivalent to ptr */
#define	T_size_t	TY_UINT		/* type for sizeof() */
#define	T_wchar_t	TY_LONG		/* type for wide characters */
#define	T_UWCHAR_MAX	T_ULONG_MAX	/* maximum unsigned wchar_t value */
