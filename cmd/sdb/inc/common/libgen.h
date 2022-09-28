/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/libgen.h	1.1"

#ifndef c_plusplus

/*	declarations of functions found in libgen for C
*/

/*	from file num.c */
extern int	num(/* string */)/* 
		register char *string; */;

/*	from file pathfind.c */
extern char *	pathfind(/*path, name, mode*/)/* 
			register char *path, *name;
			char *mode; */;

/*	from file vmprintf.c */
/**
	Given a pointer to a character pointer,
	a printf-stype format and variable argument list pointer,
	get enough memory to hold the result and set *mem to the
	pointer to that memory.
	Return the length of the formated string, not-including the trailing
	'\0' character.
	There is a limit of 4000 (default) on the length of the final
	formatted string.
	Exceeding it causes a abort() by default.
*/

/* VARARGS2 */
extern unsigned	vmprintf(/* mempp, fmtp, args */)/* 
		char		**mempp;
		char		*fmtp;
		va_list		args; */;

/**
	Adjust the behavior of mprintf() and vmprintf().
	maxstring is the maximum number of characters permitted in the string,
	including the trailing '\0' character, produced by the functions.
	failfunc is a pointer to a function returning int that is called when
	the vsprintf() fails or is too long.
	If either argument is 0, the corresponding behavior remains unchanged.
*/
extern void	mprintadj(/* maxstring, failfunc */)/* 
		unsigned	maxstring;
		int		(*failfunc)(); */;

#else	/* c_plusplus defined */

/*	declarations of functions found in libgen for C++
*/
#include	<stdio.h>
#include	<stdarg.h>

/*	from file num.c */
extern int	num( 
		char *string);

/*	from file pathfind.c */
extern char *	pathfind( 
			char *path, char *name,
			char *mode);

/*	from file vmprintf.c */
/**
	Given a pointer to a character pointer,
	a printf-stype format and variable argument list pointer,
	get enough memory to hold the result and set *mem to the
	pointer to that memory.
	Return the length of the formated string, not-including the trailing
	'\0' character.
	There is a limit of 4000 (default) on the length of the final
	formatted string.
	Exceeding it causes a abort() by default.
*/

/* VARARGS2 */
extern unsigned	vmprintf( 
		char		**mempp,
		char		*fmtp,
		va_list		args);

/**
	Adjust the behavior of mprintf() and vmprintf().
	maxstring is the maximum number of characters permitted in the string,
	including the trailing '\0' character, produced by the functions.
	failfunc is a pointer to a function returning int that is called when
	the vsprintf() fails or is too long.
	If either argument is 0, the corresponding behavior remains unchanged.
*/
extern void	mprintadj( 
		unsigned	maxstring,
		int		(*failfunc)());

#endif
