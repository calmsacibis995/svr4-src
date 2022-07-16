/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbplot:libplot/imagen/open.c	1.1.3.1"

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved. The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */


#include "imPcodes.h"
#include "imp.h"

openpl(){

	putch(imP_SET_HV_SYSTEM);
	  putch((3<<3)|5);
	putch(imP_SET_FAMILY);
	  putch(2);
	setfont(imP_charset,imPcsize);
	putch(imP_SET_IL);
	  putwd(imPcsize+3);
	putch(imP_SET_SP);
	  putwd(imPcsize);
	putch(imP_SET_PEN);
	  putch(2);
	putch(imP_SET_ABS_H);
	  putwd(0);
	putch(imP_SET_ABS_V);
	  putwd(0);
}
setfont(c, sz) char *c; int sz;
{
	imPcsize = sz;
	putch(imP_CREATE_FAMILY_TABLE);
	  putch(2);
	  putch(1);
	  putch(0);
	  fprintf(stdout, c);
	  putch(0);
}
