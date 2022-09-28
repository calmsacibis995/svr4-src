/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:mac.h	1.8.2.1"

/*
 *	UNIX shell
 */

#define TRUE	(-1)
#define FALSE	0
#define LOBYTE	0377
#define QUOTE	0200

#define EOF	0
#define NL	'\n'
#define SP	' '
#define LQ	'`'
#define RQ	'\''
#define MINUS	'-'
#define COLON	':'
#define TAB	'\t'


#define MAX(a,b)	((a)>(b)?(a):(b))

#define blank()		prc(SP)
#define	tab()		prc(TAB)
#define newline()	prc(NL)

