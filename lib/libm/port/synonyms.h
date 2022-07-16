/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/synonyms.h	1.2"

/* synonym file for non-ANSI names */

#if defined(__STDC__)

#define	copysign	_copysign
#define	erf	_erf
#define	erfc	_erfc
#define	fpsetmask	_fpsetmask
#define	fpsetround	_fpsetround
#define	fpsetsticky	_fpsetsticky
#define	gamma	_gamma
#define	j0	_j0
#define	j1	_j1
#define	jn	_jn
#define	lgamma	_lgamma
#define logb	_logb
#define	remainder	_remainder
#define	write	_write
#define	y0	_y0
#define	y1	_y1
#define	yn	_yn

#endif
