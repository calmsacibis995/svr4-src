/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)keyserv:setkey.c	1.8.2.1"

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
/*
 * Do the real work of the keyserver.
 * Store secret keys. Compute common keys,
 * and use them to decrypt and encrypt DES keys.
 * Cache the common keys, so the expensive computation is avoided.
 */
#include <stdio.h>
#include "mp.h"
#include <rpc/rpc.h>
#include <rpc/key_prot.h>
#include <rpc/des_crypt.h>
#include <sys/errno.h>

extern char *malloc();
extern char ROOTKEY[];

static MINT *MODULUS;
static char *fetchsecretkey();
static keystatus pk_crypt();
static int nodefaultkeys = 0;

/*
 * prohibit the nobody key on this machine k (the -d flag)
 */
pk_nodefaultkeys()
{
	nodefaultkeys = 1;
}

/*
 * Set the modulus for all our Diffie-Hellman operations
 */
setmodulus(modx)
	char *modx;
{
	MODULUS = xtom(modx);
}

/*
 * Set the secretkey key for this uid
 */
keystatus
pk_setkey(uid, skey)
	uid_t uid;
	keybuf skey;
{
	if (!storesecretkey(uid, skey)) {
		return (KEY_SYSTEMERR);
	}
	return (KEY_SUCCESS);
}

/*
 * Encrypt the key using the public key associated with remote_name and the
 * secret key associated with uid.
 */
keystatus
pk_encrypt(uid, remote_name, key)
	uid_t uid;
	char *remote_name;
	des_block *key;
{
	return (pk_crypt(uid, remote_name, key, DES_ENCRYPT));
}

/*
 * Decrypt the key using the public key associated with remote_name and the
 * secret key associated with uid.
 */
keystatus
pk_decrypt(uid, remote_name, key)
	uid_t uid;
	char *remote_name;
	des_block *key;
{
	return (pk_crypt(uid, remote_name, key, DES_DECRYPT));
}

/*
 * Do the work of pk_encrypt && pk_decrypt
 */
static keystatus
pk_crypt(uid, remote_name, key, mode)
	uid_t uid;
	char *remote_name;
	des_block *key;
	int mode;
{
	char *xsecret;
	char xpublic[HEXKEYBYTES + 1];
	char xsecret_hold[HEXKEYBYTES + 1];
	des_block deskey;
	int err;
	MINT *public;
	MINT *secret;
	MINT *common;
	char zero[8];

	xsecret = fetchsecretkey(uid);
	if (xsecret == NULL || xsecret[0] == 0) {
		memset(zero, 0, sizeof (zero));
		xsecret = xsecret_hold;
		if (nodefaultkeys)
			return (KEY_NOSECRET);

		if (!getsecretkey("nobody", xsecret, zero) || xsecret[0] == 0) {
			return (KEY_NOSECRET);
		}
	}
	if (!getpublickey(remote_name, xpublic)) {

	    if (nodefaultkeys)
			return (KEY_UNKNOWN);
	    if (!getpublickey("nobody", xpublic)) {
			return (KEY_UNKNOWN);
		}
	}

	if (!readcache(xpublic, xsecret, &deskey)) {
		public = xtom(xpublic);
		secret = xtom(xsecret);
		common = itom(0);
		pow(public, secret, MODULUS, common);
		extractdeskey(common, &deskey);
		writecache(xpublic, xsecret, &deskey);
		mfree(secret);
		mfree(public);
		mfree(common);
	}
	err = ecb_crypt(&deskey, key, sizeof (des_block), DES_HW | mode);
	if (DES_FAILED(err)) {
		return (KEY_SYSTEMERR);
	}
	return (KEY_SUCCESS);
}

/*
 * Choose middle 64 bits of the common key to use as our des key, possibly
 * overwriting the lower order bits by setting parity.
 */
static
extractdeskey(ck, deskey)
	MINT *ck;
	des_block *deskey;
{
	MINT *a;
	short r;
	int i;
	short base = (1 << 8);
	char *k;

	a = itom(0);
	_mp_move(ck, a);
	for (i = 0; i < ((KEYSIZE - 64) / 2) / 8; i++) {
		sdiv(a, base, a, &r);
	}
	k = deskey->c;
	for (i = 0; i < 8; i++) {
		sdiv(a, base, a, &r);
		*k++ = r;
	}
	mfree(a);
	des_setparity(deskey);
}

/*
 * Key storage management
 */
struct secretkey_list {
	uid_t uid;
	char secretkey[HEXKEYBYTES+1];
	struct secretkey_list *next;
};

static struct secretkey_list *g_secretkeys;

/*
 * Fetch the secret key for this uid
 */
static char *
fetchsecretkey(uid)
	uid_t uid;
{
	struct secretkey_list *l;

	for (l = g_secretkeys; l != NULL; l = l->next) {
		if (l->uid == uid) {
			return (l->secretkey);
		}
	}
	return (NULL);
}

/*
 * Store the secretkey for this uid
 */
storesecretkey(uid, key)
	uid_t uid;
	keybuf key;
{
	struct secretkey_list *new;
	struct secretkey_list **l;
	int nitems;

	nitems = 0;
	for (l = &g_secretkeys; *l != NULL && (*l)->uid != uid;
			l = &(*l)->next) {
		nitems++;
	}
	if (*l == NULL) {
		new = (struct secretkey_list *)malloc(sizeof (*new));
		if (new == NULL) {
			return (0);
		}
		new->uid = uid;
		new->next = NULL;
		*l = new;
	} else {
		new = *l;
	}
	memcpy(new->secretkey, key, HEXKEYBYTES);
	new->secretkey[HEXKEYBYTES] = 0;
	return (1);
}

static
hexdigit(val)
	int val;
{
	return ("0123456789abcdef"[val]);
}

bin2hex(bin, hex, size)
	unsigned char *bin;
	unsigned char *hex;
	int size;
{
	int i;

	for (i = 0; i < size; i++) {
		*hex++ = hexdigit(*bin >> 4);
		*hex++ = hexdigit(*bin++ & 0xf);
	}
}

static
hexval(dig)
	char dig;
{
	if ('0' <= dig && dig <= '9') {
		return (dig - '0');
	} else if ('a' <= dig && dig <= 'f') {
		return (dig - 'a' + 10);
	} else if ('A' <= dig && dig <= 'F') {
		return (dig - 'A' + 10);
	} else {
		return (-1);
	}
}

hex2bin(hex, bin, size)
	unsigned char *hex;
	unsigned char *bin;
	int size;
{
	int i;

	for (i = 0; i < size; i++) {
		*bin = hexval(*hex++) << 4;
		*bin++ |= hexval(*hex++);
	}
}

/*
 * Exponential caching management
 */
struct cachekey_list {
	keybuf secret;
	keybuf public;
	des_block deskey;
	struct cachekey_list *next;
};
static struct cachekey_list *g_cachedkeys;

/*
 * cache result of expensive multiple precision exponential operation
 */
static
writecache(pub, sec, deskey)
	char *pub;
	char *sec;
	des_block *deskey;
{
	struct cachekey_list *new;

	new = (struct cachekey_list *) malloc(sizeof (struct cachekey_list));
	if (new == NULL) {
		return;
	}
	memcpy(new->public, pub, sizeof (keybuf));
	memcpy(new->secret, sec, sizeof (keybuf));
	new->deskey = *deskey;
	new->next = g_cachedkeys;
	g_cachedkeys = new;
}

/*
 * Try to find the common key in the cache
 */
static
readcache(pub, sec, deskey)
	char *pub;
	char *sec;
	des_block *deskey;
{
	struct cachekey_list *found;
	register struct cachekey_list **l;

#define	cachehit(pub, sec, list)	\
		(memcmp(pub, (list)->public, sizeof (keybuf)) == 0 && \
		 memcmp(sec, (list)->secret, sizeof (keybuf)) == 0)

	for (l = &g_cachedkeys; (*l) != NULL && !cachehit(pub, sec, *l);
		l = &(*l)->next)
		;
	if ((*l) == NULL) {
		return (0);
	}
	found = *l;
	(*l) = (*l)->next;
	found->next = g_cachedkeys;
	g_cachedkeys = found;
	*deskey = found->deskey;
	return (1);
}
