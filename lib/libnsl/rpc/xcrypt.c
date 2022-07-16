/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libnsl:rpc/xcrypt.c	1.1"

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
static char sccsid[] = "@(#)xcrypt.c 1.3 89/03/24 Copyr 1986 Sun Micro";
#endif

/* This file is moved from librpcsvc to libnsl together 
 * with secretkey.c and publickey.c to be in conformance 
 * with ABI document specifications. This file is moved 
 * because secretkey.c invokes function xdecrypt(). All 
 * three files could be moved back at a later time.
 */

/*
 * xcrypt.c: Hex encryption/decryption and utility routines
 */

#include <stdio.h>
#include <rpc/des_crypt.h>
#include <memory.h>

extern char *malloc();

extern char hex[];	/* forward */
static char hexval();

/*
 * Encrypt a secret key given passwd
 * The secret key is passed and returned in hex notation.
 * Its length must be a multiple of 16 hex digits (64 bits).
 */
xencrypt(secret, passwd)
	char *secret;
	char *passwd;
{
	char key[8];
	char ivec[8];
	char *buf;
	int err;
	int len;

	len = strlen(secret) / 2;
	buf = malloc((unsigned)len);
	hex2bin(len, secret, buf);
	passwd2des(passwd, key);
	memset(ivec, 0, 8);

	err = cbc_crypt(key, buf, len, DES_ENCRYPT | DES_HW, ivec);
	if (DES_FAILED(err)) {	
		free(buf);
		return (0);
	}
	bin2hex(len, (unsigned char *) buf, secret);
	free(buf);
	return (1);
}

/*
 * Decrypt secret key using passwd
 * The secret key is passed and returned in hex notation.
 * Once again, the length is a multiple of 16 hex digits
 */
xdecrypt(secret, passwd)
	char *secret;
	char *passwd;
{
	char key[8];
	char ivec[8];
	char *buf;
	int err;
	int len;

	len = strlen(secret) / 2;
	buf = malloc((unsigned)len);

	hex2bin(len, secret, buf);
	passwd2des(passwd, key);	
	memset(ivec, 0, 8);

	err = cbc_crypt(key, buf, len, DES_DECRYPT | DES_HW, ivec);
	if (DES_FAILED(err)) {
		free(buf);
		return (0);
	}
	bin2hex(len, (unsigned char *) buf, secret);
	free(buf);
	return (1);
}

/*
 * Turn password into DES key
 */
passwd2des(pw, key)
	char *pw;
	char *key;
{
	int i;

	memset(key, 0, 8);
	for (i = 0; *pw; i = (i+1) % 8) {
		key[i] ^= *pw++ << 1;
	}
	des_setparity(key);
}

/*
 * Hex to binary conversion
 */
static
hex2bin(len, hexnum, binnum)
	int len;
	char *hexnum;
	char *binnum;
{
	int i;

	for (i = 0; i < len; i++) {
		*binnum++ = 16 * hexval(hexnum[2 * i]) +
					hexval(hexnum[2 * i + 1]);	
	}
}

/*
 * Binary to hex conversion
 */
static
bin2hex(len, binnum, hexnum)
	int len;
	unsigned char *binnum;
	char *hexnum;
{
	int i;
	unsigned val;

	for (i = 0; i < len; i++) {
		val = binnum[i];
		hexnum[i*2] = hex[val >> 4];
		hexnum[i*2+1] = hex[val & 0xf];
	}
	hexnum[len*2] = 0;
}

static char hex[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

static char
hexval(c)
	char c;
{
	if (c >= '0' && c <= '9') {
		return (c - '0');
	} else if (c >= 'a' && c <= 'z') {
		return (c - 'a' + 10);
	} else if (c >= 'A' && c <= 'Z') {
		return (c - 'A' + 10);
	} else {
		return (-1);
	}
}
