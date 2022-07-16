/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-des:intldes_soft.c	1.1"

#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)des_soft.c 1.1 88/12/14 SMI"
#endif

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

/*
 * Warning!  Things are arranged very carefully in this file to
 * allow read-only data to be moved to the text segment.  The
 * various DES tables must appear before any function definitions
 * (this is arranged by including them immediately below) and partab
 * must also appear before and function definitions
 * This arrangement allows all data up through the first text to
 * be moved to text.
 */

/*
 * Fast (?) software implementation of DES
 * Has been seen going at 2000 bytes/sec on a Sun-2
 * Works on a VAX too.
 * Won't work without 8 bit chars and 32 bit longs
 */

#include <sys/types.h>
#include <des/des.h>
#include <des/softdes.h>
#include <des/desdata.h>
#include <sys/debug.h>

STATIC void des_setkey();
STATIC void des_encrypt();

#define btst(k, b)	(k[b >> 3] & (0x80 >> (b & 07)))
#define	BIT28	(1<<28)

/*
 * Software encrypt or decrypt a block of data (multiple of 8 bytes)
 * Do the CBC ourselves if needed.
 */
_des_crypt(buf, len, desp)
	register char *buf;
	register unsigned len;
	struct desparams *desp;
{
}


/*
 * Set the key and direction for an encryption operation
 * We build the 16 key entries here
 */
STATIC void
des_setkey(userkey, kd, dir)
	u_char userkey[8];	
	register struct deskeydata *kd;
	unsigned dir;
{
}



/*
 * Do an encryption operation
 * Much pain is taken (with preprocessor) to avoid loops so the compiler
 * can do address arithmetic instead of doing it at runtime.
 * Note that the byte-to-chunk conversion is necessary to guarantee
 * processor byte-order independence.
 */
STATIC void
des_encrypt(data, kd)
	register u_char *data;
	register struct deskeydata *kd;
{
}
