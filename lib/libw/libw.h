/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libw:libw.h	1.1.1.1"

#ifndef _LIBW_H
#define _LIBW_H
#include	<stdlib.h>

#ifndef _EUCWIDTH_T
#define _EUCWIDTH_T
typedef struct {
	short int _eucw1, _eucw2, _eucw3;	/*	EUC width	*/
	short int _scrw1, _scrw2, _scrw3;	/*	screen width	*/
	short int _pcw;		/*	WIDE_CHAR width	*/
	char _multibyte;	/*	1=multi-byte, 0=single-byte	*/
} eucwidth_t;
#endif

#ifdef __STDC__
void getwidth(eucwidth_t *);
int mbftowc(char *, wchar_t *, int (*)(), int *);
int scrwidth(wchar_t);
int wisprint(wchar_t);
#else
void getwidth();
int mbftowc();
int scrwidth();
int wisprint();
#endif /* __STDC__ */
#endif /* _LIBW_H */
