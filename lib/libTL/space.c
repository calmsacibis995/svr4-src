/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libTL:space.c	1.3.3.1"
#include <table.h>
#include <internal.h>
#include <setjmp.h>

tbl_t TLtables[ TL_MAXTABLES ];
int TLinitialized = 0;
unsigned char TLinbuffer[ 2 * TL_MAXLINESZ + 1 ];
unsigned char TLgenbuf[ 2 * TL_MAXLINESZ + 1 ];
jmp_buf TLenv;
