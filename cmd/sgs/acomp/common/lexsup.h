/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/lexsup.h	1.3"
/* lexsup.h */

/* Definitions of support routines for lexical analysis.
** These routines were split out so there would be one
** source copy whether the compiler was merged with the
** preprocessor or not.
*/

extern unsigned int doescape();
extern wchar_t lx_mbtowc();
extern int lx_keylook();
