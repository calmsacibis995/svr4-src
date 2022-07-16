/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:gen/lconstants.c	1.3"

#ifdef __STDC__
	#pragma weak lzero = _lzero
	#pragma weak lone = _lone
	#pragma weak lten = _lten
#endif
#include	"synonyms.h"
#include	<sys/types.h>
#include	<sys/dl.h>

dl_t	lzero	= { 0, 0};
dl_t	lone	= { 1, 0};
dl_t	lten	= {10, 0};
