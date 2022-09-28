/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)xt:set_enc.c	1.3.1.1"

/*
 * Routine to change ENCODING option remotely
 *
 * Usage:
 *
 *	wtinit set_encoding.j 1		# or 0
 *
 */

#include	<dmd.h>
#include	<setup.h>

#define	BRAM_SIZE	2048
char *itox();

main()
{
	int value;

	wait(RCV);
	value = (rcvchar() & 0x7f) - '0';
	spl7();
	ENCODE_ENABLE = value;
	setbram();
	spl0();
}
