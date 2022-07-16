/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)boot:boot/at386/initprog/video.c	1.1"

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#include <sys/inline.h>

/*
 * Check video and disable shadow RAM if appropriate.
 */

ck_video()
{
	outb(0x70, 0x14);
	if (!((inb(0x71) >> 4) & 0x3)) {	/* EGA/VGA ? */
		outb(0x3c4, 0x5);
		if (inb(0x3c4) == 0x5)	/* VGA */
			rst_vga();
		else			/* EGA */
			rst_ega();
	}
	shadow();
}
