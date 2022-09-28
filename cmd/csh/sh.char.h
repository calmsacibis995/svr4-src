/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)csh:sh.char.h	1.2.3.1"

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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley Software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Macros to classify characters.
 */

#ifdef MBCHAR
#include <wctype.h>
#define	isauxspZ	(!isascii(Z)&&!(Z&QUOTE)&&iswspace(Z))
#define	isauxsp(c) 	(Z=((unsigned)(c)), isauxspZ)
/* Regocnizes non-ASCII space characters. */
#else
#include <ctype.h>
/* macros of macros to reduce further #ifdef MBCHAR later. Please be patient!*/
#define	iswdigit(c)	isdigit(c)
#define	iswalpha(c)	isalpha(c)
#define	isphonogram(c)	0	
#define	isideogram(c)	0
#define	isauxsp(c) 	0
#define	isauxspZ 	0
#endif
extern unsigned short _cmap[];/* Defined in sh.char.c */
unsigned int	Z; 	/* A place to save macro arg to avoid side-effect!*/

#define _Q	0x01		/* '" */
#define _Q1	0x02		/* ` */
#define _SP	0x04		/* space and tab */
#define _NL	0x08		/* \n */
#define _META	0x10		/* lex meta characters, sp #'`";&<>()|\t\n */
#define _GLOB	0x20		/* glob characters, *?{[` */
#define _ESC	0x40		/* \ */
#define _DOL	0x80		/* $ */
#define _DIG    0x100		/* 0-9 */
#define _LET    0x200		/* a-z, A-Z, _ 	NO LONGER OF REAL USE. */


#define	cmapZ(bits)	(isascii(Z)?(_cmap[Z] & (bits)):0)
#define cmap(c, bits)	(Z=((unsigned)(c)), cmapZ(bits))

#define isglob(c)	cmap(c, _GLOB)
#define ismeta(c)	cmap(c, _META)
#define digit(c)	cmap(c, _DIG)
#define issp(c)		(Z=((unsigned)(c)), cmapZ( _SP)||isauxspZ) 
/*WAS isspace(c)*/
#define isspnl(c)	(Z=((unsigned)(c)), cmapZ( _SP|_NL)||isauxspZ)
#define letter(c)	(Z=((unsigned)(c)), iswalpha(Z)||((Z)=='_')\
			||isphonogram(Z)||isideogram(Z))
#define alnum(c)	(Z=((unsigned)(c)), iswalpha(Z)||((Z)=='_')\
			||iswdigit(Z)||isphonogram(Z)||isideogram(Z))
