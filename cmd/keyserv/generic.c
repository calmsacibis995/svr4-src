/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)keyserv:generic.c	1.1.2.1"

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
#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/file.h>
#include "mp.h"
#include <rpc/key_prot.h>

/*
 * Generate a seed
 */
static
getseed(seed, seedsize, pass)
	char *seed;
	int seedsize;
	unsigned char *pass;
{
	int i;
	int rseed;
	struct timeval tv;

	(void)gettimeofday(&tv, (struct timezone *)NULL);
	rseed = tv.tv_sec + tv.tv_usec;
	for (i = 0; i < 8; i++) {
		rseed ^= (rseed << 8) | pass[i];
	}
	srand(rseed);

	for (i = 0; i < seedsize; i++) {
		seed[i] = (rand() & 0xff) ^ pass[i % 8];
	}
}

/*
 * Generate a random public/secret key pair
 */
genkeys(public, secret, pass)
	char *public;
	char *secret;
	char *pass;
{
	int i;

#   define BASEBITS (8*sizeof (short) - 1)
#	define BASE		(1 << BASEBITS)

	MINT *pk = itom(0);
	MINT *sk = itom(0);
	MINT *tmp;
	MINT *base = itom(BASE);
	MINT *root = itom(PROOT);
	MINT *modulus = xtom(HEXMODULUS);
	short r;
	unsigned short seed[KEYSIZE/BASEBITS + 1];
	char *xkey;

	getseed((char *)seed, sizeof (seed), (u_char *)pass);
	for (i = 0; i < KEYSIZE/BASEBITS + 1; i++) {
		r = seed[i] % BASE;
		tmp = itom(r);
		mult(sk, base, sk);
		madd(sk, tmp, sk);
		mfree(tmp);
	}
	tmp = itom(0);
	mdiv(sk, modulus, tmp, sk);
	mfree(tmp);
	pow(root, sk, modulus, pk);
	xkey = mtox(sk);
	adjust(secret, xkey);
	xkey = mtox(pk);
	adjust(public, xkey);
	mfree(sk);
	mfree(base);
	mfree(pk);
	mfree(root);
	mfree(modulus);
}

/*
 * Adjust the input key so that it is 0-filled on the left
 */
static
adjust(keyout, keyin)
	char keyout[HEXKEYBYTES+1];
	char *keyin;
{
	char *p;
	char *s;

	for (p = keyin; *p; p++)
		;
	for (s = keyout + HEXKEYBYTES; p >= keyin; p--, s--) {
		*s = *p;
	}
	while (s >= keyout) {
		*s-- = '0';
	}
}
