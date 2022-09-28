/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)fmli:inc/exception.h	1.3"

/*  include in .c file not in header file  abs 9/13/88
#include	<termio.h>
#define        _SYS_TERMIO_H
*/
extern struct termio	Echo;
extern struct termio	Noecho;
extern int	Echoit;

#define echo()		(Echoit = TRUE)
#define noecho()	(Echoit = FALSE)
#define restore_tty()	(Echo.c_cflag ? ioctl(0, TCSETAW, &Echo) : -1)

#define LCKPREFIX	".L"
