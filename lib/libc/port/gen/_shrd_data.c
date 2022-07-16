/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/_shrd_data.c	1.3"
/*LINTLIBRARY*/

#include "synonyms.h"
#include <locale.h>
#include "_locale.h"

unsigned char _shrd_numeric[SZ_NUMERIC] =
{
	'.',	'\0',
};
