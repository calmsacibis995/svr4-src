/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libnsl:des/intldes_soft.c	1.1.2.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)des_soft.c 1.2 89/03/10 Copyr 1986 Sun Micro";
#endif
/*
 * Warning!  Things are arranged very carefully in this file to
 * allow read-only data to be moved to the text segment.  The
 * various DES tables must appear before any function definitions
 * (this is arranged by including them immediately below) and partab
 * must also appear before and function definitions
 * This arrangement allows all data up through the first text to
 * be moved to text.
 */

#include <sys/types.h>
#include <des/softdes.h>
#include <des/desdata.h>
#ifdef sun
#	include <sys/ioctl.h>
#	include <sys/des.h>
#else
#include <des/des.h>
#endif

#define btst(k, b)	(k[b >> 3] & (0x80 >> (b & 07)))
#define	BIT28	(1<<28)

static char partab[128] = {
	0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x07, 0x07,
	0x08, 0x08, 0x0b, 0x0b, 0x0d, 0x0d, 0x0e, 0x0e,
	0x10, 0x10, 0x13, 0x13, 0x15, 0x15, 0x16, 0x16,
	0x19, 0x19, 0x1a, 0x1a, 0x1c, 0x1c, 0x1f, 0x1f,
	0x20, 0x20, 0x23, 0x23, 0x25, 0x25, 0x26, 0x26,
	0x29, 0x29, 0x2a, 0x2a, 0x2c, 0x2c, 0x2f, 0x2f,
	0x31, 0x31, 0x32, 0x32, 0x34, 0x34, 0x37, 0x37,
	0x38, 0x38, 0x3b, 0x3b, 0x3d, 0x3d, 0x3e, 0x3e,
	0x40, 0x40, 0x43, 0x43, 0x45, 0x45, 0x46, 0x46,
	0x49, 0x49, 0x4a, 0x4a, 0x4c, 0x4c, 0x4f, 0x4f,
	0x51, 0x51, 0x52, 0x52, 0x54, 0x54, 0x57, 0x57,
	0x58, 0x58, 0x5b, 0x5b, 0x5d, 0x5d, 0x5e, 0x5e,
	0x61, 0x61, 0x62, 0x62, 0x64, 0x64, 0x67, 0x67,
	0x68, 0x68, 0x6b, 0x6b, 0x6d, 0x6d, 0x6e, 0x6e,
	0x70, 0x70, 0x73, 0x73, 0x75, 0x75, 0x76, 0x76,
	0x79, 0x79, 0x7a, 0x7a, 0x7c, 0x7c, 0x7f, 0x7f,
};

void
des_setparity(p)
	char *p;
{
}

_des_crypt(buf, len, desp)
	register char *buf;
	register unsigned len;
	struct desparams *desp;
{
	return (1);
}


static
des_setkey(userkey, kd, dir)
	u_char userkey[8];	
	register struct deskeydata *kd;
	unsigned dir;
{
}


static
des_encrypt(data, kd)
	register u_char *data;
	register struct deskeydata *kd;
{
}
