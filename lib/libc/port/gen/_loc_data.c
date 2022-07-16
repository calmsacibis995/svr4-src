/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/_loc_data.c	1.10"
#include "synonyms.h"
#include <locale.h>
#include "_locale.h"
#include <ctype.h>

char _cur_locale[LC_ALL][LC_NAMELEN] = /* current locale names */
{
	"C", "C", "C", "C", "C", "C"		/* need a repeat count feature */
};

unsigned char _numeric[SZ_NUMERIC] =
{
	'.',	'\0',
};
