/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _CTYPE_H
#define _CTYPE_H

#ident	"@(#)head:ctype.h	1.18"

#define	_U	01	/* Upper case */
#define	_L	02	/* Lower case */
#define	_N	04	/* Numeral (digit) */
#define	_S	010	/* Spacing character */
#define	_P	020	/* Punctuation */
#define	_C	040	/* Control character */
#define	_B	0100	/* Blank */
#define	_X	0200	/* heXadecimal digit */

#if defined(__STDC__)

extern int isalnum(int);        
extern int isalpha(int);        
extern int iscntrl(int);        
extern int isdigit(int);        
extern int isgraph(int);        
extern int islower(int);        
extern int isprint(int);        
extern int ispunct(int);        
extern int isspace(int);        
extern int isupper(int);        
extern int isxdigit(int);       
extern int tolower(int);
extern int toupper(int);

#if (__STDC__ == 0 && !defined(_POSIX_SOURCE)) || defined(_XOPEN_SOURCE)

extern int isascii(int);        
extern int toascii(int);        
extern int _tolower(int);
extern int _toupper(int);

#endif

extern unsigned char	__ctype[];

#if !#lint(on)

#define	isalpha(c)	((__ctype + 1)[c] & (_U | _L))
#define	isupper(c)	((__ctype + 1)[c] & _U)
#define	islower(c)	((__ctype + 1)[c] & _L)
#define	isdigit(c)	((__ctype + 1)[c] & _N)
#define	isxdigit(c)	((__ctype + 1)[c] & _X)
#define	isalnum(c)	((__ctype + 1)[c] & (_U | _L | _N))
#define	isspace(c)	((__ctype + 1)[c] & _S)
#define	ispunct(c)	((__ctype + 1)[c] & _P)
#define	isprint(c)	((__ctype + 1)[c] & (_P | _U | _L | _N | _B))
#define	isgraph(c)	((__ctype + 1)[c] & (_P | _U | _L | _N))
#define	iscntrl(c)	((__ctype + 1)[c] & _C)

#if (__STDC__ == 0 && !defined(_POSIX_SOURCE)) || defined(_XOPEN_SOURCE)

#define	isascii(c)	(!((c) & ~0177))
#define	_toupper(c)     ((__ctype + 258)[c])
#define	_tolower(c)	((__ctype + 258)[c])
#define	toascii(c)	((c) & 0177)

#endif

#endif	/* lint */

#else

extern unsigned char	_ctype[];

#ifndef lint

#define	isalpha(c)	((_ctype + 1)[c] & (_U | _L))
#define	isupper(c)	((_ctype + 1)[c] & _U)
#define	islower(c)	((_ctype + 1)[c] & _L)
#define	isdigit(c)	((_ctype + 1)[c] & _N)
#define	isxdigit(c)	((_ctype + 1)[c] & _X)
#define	isalnum(c)	((_ctype + 1)[c] & (_U | _L | _N))
#define	isspace(c)	((_ctype + 1)[c] & _S)
#define	ispunct(c)	((_ctype + 1)[c] & _P)
#define	isprint(c)	((_ctype + 1)[c] & (_P | _U | _L | _N | _B))
#define	isgraph(c)	((_ctype + 1)[c] & (_P | _U | _L | _N))
#define	iscntrl(c)	((_ctype + 1)[c] & _C)
#define	isascii(c)	(!((c) & ~0177))
#define	_toupper(c)     ((_ctype + 258)[c])
#define	_tolower(c)	((_ctype + 258)[c])
#define	toascii(c)	((c) & 0177)

#endif	/* lint */

#endif
#endif	/* _CTYPE_H */
