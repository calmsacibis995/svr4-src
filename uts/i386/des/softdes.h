/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-des:softdes.h	1.1"

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
static char sccsid[] = "@(#)softdes.h 1.2 89/03/10 Copyr 1986 Sun Micro";
#endif
/*
 * softdes.h,  Data types and definition for software DES
 */

/*
 * A chunk is an area of storage used in three different ways
 * - As a 64 bit quantity (in high endian notation)
 * - As a 48 bit quantity (6 low order bits per byte)
 * - As a 32 bit quantity (first 4 bytes)
 */
typedef union {
	struct {
#if defined(vax) || defined(i386)
		u_long	_long1;
		u_long	_long0;
#else
		u_long	_long0;
		u_long	_long1;
#endif
	} _longs;
#define	long0	_longs._long0
#define	long1	_longs._long1
	struct {
#if defined(vax) || defined(i386)
		u_char	_byte7;
		u_char	_byte6;
		u_char	_byte5;
		u_char	_byte4;
		u_char	_byte3;
		u_char	_byte2;
		u_char	_byte1;
		u_char	_byte0;
#else
		u_char	_byte0;
		u_char	_byte1;
		u_char	_byte2;
		u_char	_byte3;
		u_char	_byte4;
		u_char	_byte5;
		u_char	_byte6;
		u_char	_byte7;
#endif
	} _bytes;
#define	byte0	_bytes._byte0
#define	byte1	_bytes._byte1
#define	byte2	_bytes._byte2
#define	byte3	_bytes._byte3
#define	byte4	_bytes._byte4
#define	byte5	_bytes._byte5
#define	byte6	_bytes._byte6
#define	byte7	_bytes._byte7
} chunk_t;

/*
 * Intermediate key storage
 * Created by des_setkey, used by des_encrypt and des_decrypt
 * 16 48 bit values
 */
struct deskeydata {
	chunk_t	keyval[16];
};
