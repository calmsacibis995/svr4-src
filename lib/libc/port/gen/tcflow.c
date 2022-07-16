/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/tcflow.c	1.1"

#ifdef __STDC__
	#pragma weak tcflow = _tcflow
#endif
#include "synonyms.h"
#include <sys/termios.h>

/* 
 *suspend transmission or reception of input or output
 */

/*
 * TCOOFF (0) -> suspend output 
 * TCOON  (1) -> restart suspend output
 * TCIOFF (2) -> suspend input 
 * TCION  (3) -> restart suspend input
 */

int tcflow(fildes,action)
int fildes;
int action;
{
	return(ioctl(fildes,TCXONC,action));

}
