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


#ident	"@(#)librpc:auth_des.c	1.5.1.1"

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
static char sccsid[] = "@(#)auth_des.c 1.9 89/05/02 Copyr 1986 Sun Micro";
#endif

/*
 * auth_des.c, client-side implementation of DES authentication
 *
 */

#include <rpc/rpc.h>
#include <rpc/des_crypt.h>
#ifndef	_KERNEL
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define	LOG_ERR 1
#endif /* SYSLOG */
#endif /* _KERNEL */

#define	MILLION			1000000L
#define	RTIME_TIMEOUT		5	/* seconds to wait for sync */

#define	AUTH_PRIVATE(auth)	(struct ad_private *) auth->ah_private
#define	ALLOC(object_type)	(object_type *) mem_alloc(sizeof (object_type))
#define	FREE(ptr, size)		mem_free((char *)(ptr), (int) size)
#define	ATTEMPT(xdr_op)		if (!(xdr_op)) return (FALSE)

#ifdef _KERNEL
#define	gettimeofday(tvp, tzp)	uniqtime(tvp)	/* fake system call */
#endif

static struct auth_ops *authdes_ops();
extern char *malloc();

/*
 * This struct is pointed to by the ah_private field of an "AUTH *"
 */
struct ad_private {
	char *ad_fullname;		/* client's full name */
	u_int ad_fullnamelen;		/* length of name, rounded up */
	char *ad_servername;		/* server's full name */
	u_int ad_servernamelen;		/* length of name, rounded up */
	u_int ad_window;		/* client specified window */
	bool_t ad_dosync;		/* synchronize? */
	char *ad_timehost;		/* remote host to sync with */
	struct timeval ad_timediff;	/* server's time - client's time */
	u_long ad_nickname;		/* server's nickname for client */
	struct authdes_cred ad_cred;	/* storage for credential */
	struct authdes_verf ad_verf;	/* storage for verifier */
	struct timeval ad_timestamp;	/* timestamp sent */
	des_block ad_xkey;		/* encrypted conversation key */
};

AUTH *
authdes_seccreate(servername, window, timehost, ckey)
	char *servername;		/* network name of server */
	u_int window;			/* time to live */
	char *timehost;			/* optional hostname to sync with */
	des_block *ckey;		/* optional conversation key to use*/
{
	AUTH *auth;
	struct ad_private *ad;
	char namebuf[MAXNETNAMELEN+1];

	/*
	 * Allocate everything now
	 */
	auth = ALLOC(AUTH);
	if (auth == NULL) {
		msgout("authdes_seccreate: out of memory");
		return (NULL);
	}
	ad = ALLOC(struct ad_private);
	if (ad == NULL) {
		msgout("authdes_seccreate: out of memory");
		goto failed;
	}
	ad->ad_fullname = ad->ad_servername = NULL; /* Sanity reasons */
	ad->ad_timehost = NULL;
	ad->ad_timediff.tv_sec = 0;
	ad->ad_timediff.tv_usec = 0;
	(void) getnetname(namebuf);
	ad->ad_fullnamelen = RNDUP(strlen(namebuf));
	ad->ad_fullname = mem_alloc(ad->ad_fullnamelen + 1);
	ad->ad_servernamelen = strlen(servername);
	ad->ad_servername = mem_alloc(ad->ad_servernamelen + 1);

	if (ad->ad_fullname == NULL || ad->ad_servername == NULL) {
		msgout("authdes_seccreate: out of memory");
		goto failed;
	}
	if (timehost != NULL) {
		ad->ad_timehost = mem_alloc(strlen(timehost) + 1);
		if (ad->ad_timehost == NULL) {
			msgout("authdes_seccreate: out of memory");
			goto failed;
		}
		memcpy(ad->ad_timehost, timehost, strlen(timehost) + 1);
		ad->ad_dosync = TRUE;
	} else {
		ad->ad_dosync = FALSE;
	}
	memcpy(ad->ad_fullname, namebuf, ad->ad_fullnamelen + 1);
	memcpy(ad->ad_servername, servername, ad->ad_servernamelen + 1);
	ad->ad_window = window;
	if (ckey == NULL) {
		if (key_gendes(&auth->ah_key) < 0) {
			msgout(
	"authdes_seccreate: unable to gen conversation key");
			goto failed;
		}
	} else {
		auth->ah_key = *ckey;
	}

	/*
	 * Set up auth handle
	 */
	auth->ah_cred.oa_flavor = AUTH_DES;
	auth->ah_verf.oa_flavor = AUTH_DES;
	auth->ah_ops = authdes_ops();
	auth->ah_private = (caddr_t)ad;

	if (!authdes_refresh(auth)) {
		goto failed;
	}
	return (auth);

failed:
	if (auth)
		FREE(auth, sizeof (AUTH));
	if (ad) {
		if (ad->ad_fullname)
			FREE(ad->ad_fullname, ad->ad_fullnamelen + 1);
		if (ad->ad_servername)
			FREE(ad->ad_servername, ad->ad_servernamelen + 1);
		if (ad->ad_timehost)
			FREE(ad->ad_timehost, strlen(ad->ad_timehost) + 1);
		FREE(ad, sizeof (struct ad_private));
	}
	return (NULL);
}

/*
 * Implement the five authentication operations
 */


/*
 * 1. Next Verifier
 */
/*ARGSUSED*/
static void
authdes_nextverf(auth)
	AUTH *auth;
{
	/* what the heck am I supposed to do??? */
}


/*
 * 2. Marshal
 */
static bool_t
authdes_marshal(auth, xdrs)
	AUTH *auth;
	XDR *xdrs;
{
	struct ad_private *ad = AUTH_PRIVATE(auth);
	struct authdes_cred *cred = &ad->ad_cred;
	struct authdes_verf *verf = &ad->ad_verf;
	des_block cryptbuf[2];
	des_block ivec;
	int status;
	int len;
	register long *ixdr;

	/*
	 * Figure out the "time", accounting for any time difference
	 * with the server if necessary.
	 */
	(void) gettimeofday(&ad->ad_timestamp, (struct timezone *)NULL);
	ad->ad_timestamp.tv_sec += ad->ad_timediff.tv_sec;
	ad->ad_timestamp.tv_usec += ad->ad_timediff.tv_usec;
	if (ad->ad_timestamp.tv_usec >= MILLION) {
		ad->ad_timestamp.tv_usec -= MILLION;
		ad->ad_timestamp.tv_sec += 1;
	}

	/*
	 * XDR the timestamp and possibly some other things, then
	 * encrypt them.
	 */
	ixdr = (long *)cryptbuf;
	IXDR_PUT_LONG(ixdr, ad->ad_timestamp.tv_sec);
	IXDR_PUT_LONG(ixdr, ad->ad_timestamp.tv_usec);
	if (ad->ad_cred.adc_namekind == ADN_FULLNAME) {
		IXDR_PUT_U_LONG(ixdr, ad->ad_window);
		IXDR_PUT_U_LONG(ixdr, ad->ad_window - 1);
		ivec.key.high = ivec.key.low = 0;
		status = cbc_crypt((char *)&auth->ah_key, (char *)cryptbuf,
			2 * sizeof (des_block),
			DES_ENCRYPT | DES_HW, (char *)&ivec);
	} else {
		status = ecb_crypt((char *)&auth->ah_key, (char *)cryptbuf,
			sizeof (des_block), DES_ENCRYPT | DES_HW);
	}
	if (DES_FAILED(status)) {
		msgout("authdes_marshal: DES encryption failure");
		return (FALSE);
	}
	ad->ad_verf.adv_xtimestamp = cryptbuf[0];
	if (ad->ad_cred.adc_namekind == ADN_FULLNAME) {
		ad->ad_cred.adc_fullname.window = cryptbuf[1].key.high;
		ad->ad_verf.adv_winverf = cryptbuf[1].key.low;
	} else {
		ad->ad_cred.adc_nickname = ad->ad_nickname;
		ad->ad_verf.adv_winverf = 0;
	}

	/*
	 * Serialize the credential and verifier into opaque
	 * authentication data.
	 */
	if (ad->ad_cred.adc_namekind == ADN_FULLNAME) {
		len = ((1 + 1 + 2 + 1)*BYTES_PER_XDR_UNIT + ad->ad_fullnamelen);
	} else {
		len = (1 + 1)*BYTES_PER_XDR_UNIT;
	}

	if (ixdr = xdr_inline(xdrs, 2*BYTES_PER_XDR_UNIT)) {
		IXDR_PUT_LONG(ixdr, AUTH_DES);
		IXDR_PUT_LONG(ixdr, len);
	} else {
		ATTEMPT(xdr_putlong(xdrs, &auth->ah_cred.oa_flavor));
		ATTEMPT(xdr_putlong(xdrs, &len));
	}
	ATTEMPT(xdr_authdes_cred(xdrs, cred));

	len = (2 + 1)*BYTES_PER_XDR_UNIT;
	if (ixdr = xdr_inline(xdrs, 2*BYTES_PER_XDR_UNIT)) {
		IXDR_PUT_LONG(ixdr, AUTH_DES);
		IXDR_PUT_LONG(ixdr, len);
	} else {
		ATTEMPT(xdr_putlong(xdrs, &auth->ah_verf.oa_flavor));
		ATTEMPT(xdr_putlong(xdrs, &len));
	}
	ATTEMPT(xdr_authdes_verf(xdrs, verf));
	return (TRUE);
}


/*
 * 3. Validate
 */
static bool_t
authdes_validate(auth, rverf)
	AUTH *auth;
	struct opaque_auth *rverf;
{
	struct ad_private *ad = AUTH_PRIVATE(auth);
	struct authdes_verf verf;
	int status;
	register u_long *ixdr;

	if (rverf->oa_length != (2 + 1) * BYTES_PER_XDR_UNIT) {
		return (FALSE);
	}
	ixdr = (u_long *)rverf->oa_base;
	verf.adv_xtimestamp.key.high = (u_long)*ixdr++;
	verf.adv_xtimestamp.key.low = (u_long)*ixdr++;
	verf.adv_int_u = IXDR_GET_U_LONG(ixdr);

	/*
	 * Decrypt the timestamp
	 */
	status = ecb_crypt((char *)&auth->ah_key, (char *)&verf.adv_xtimestamp,
		sizeof (des_block), DES_DECRYPT | DES_HW);

	if (DES_FAILED(status)) {
		msgout("authdes_validate: DES decryption failure");
		return (FALSE);
	}

	/*
	 * xdr the decrypted timestamp
	 */
	ixdr = (u_long *)verf.adv_xtimestamp.c;
	verf.adv_timestamp.tv_sec = IXDR_GET_LONG(ixdr) + 1;
	verf.adv_timestamp.tv_usec = IXDR_GET_LONG(ixdr);

	/*
	 * validate
	 */
	if (memcmp((char *)&ad->ad_timestamp, (char *)&verf.adv_timestamp,
		sizeof (struct timeval)) != 0) {
		msgout("authdes_validate: verifier mismatch");
		return (FALSE);
	}

	/*
	 * We have a nickname now, let's use it
	 */
	ad->ad_nickname = verf.adv_nickname;
	ad->ad_cred.adc_namekind = ADN_NICKNAME;
	return (TRUE);
}

/*
 * 4. Refresh
 */
static bool_t
authdes_refresh(auth)
	AUTH *auth;
{
	struct ad_private *ad = AUTH_PRIVATE(auth);
	struct authdes_cred *cred = &ad->ad_cred;

	if (ad->ad_dosync &&
		!synchronize(ad->ad_timehost, &ad->ad_timediff)) {
		/*
		 * Hope the clocks are synced!
		 */
		ad->ad_timediff.tv_sec = ad->ad_timediff.tv_usec = 0;
		msgout("authdes_refresh: unable to synchronize with server");
	}
	ad->ad_xkey = auth->ah_key;
	if (key_encryptsession(ad->ad_servername, &ad->ad_xkey) < 0) {
		msgout("authdes_refresh: unable to encrypt conversation key");
		return (FALSE);
	}
	cred->adc_fullname.key = ad->ad_xkey;
	cred->adc_namekind = ADN_FULLNAME;
	cred->adc_fullname.name = ad->ad_fullname;
	return (TRUE);
}


/*
 * 5. Destroy
 */
static void
authdes_destroy(auth)
	AUTH *auth;
{
	struct ad_private *ad = AUTH_PRIVATE(auth);

	FREE(ad->ad_fullname, ad->ad_fullnamelen + 1);
	FREE(ad->ad_servername, ad->ad_servernamelen + 1);
	FREE(ad, sizeof (struct ad_private));
	FREE(auth, sizeof (AUTH));
}

/*
 * Synchronize with the server at the given address, that is,
 * adjust timep to reflect the delta between our clocks
 */
static bool_t
synchronize(timehost, timep)
	char *timehost;
	struct timeval *timep;
{
	struct timeval mytime;

	if (! rpcb_gettime(timehost, timep)) {
		/* try to contact INET time server */
		struct timeval timeout;

		timeout.tv_sec = RTIME_TIMEOUT;
		timeout.tv_usec = 0;
		if (rtime_tli(timehost, timep, &timeout) < 0) {
			msgout("synchronize time failed");
			return (FALSE);
		}
	}
	(void) gettimeofday(&mytime, (struct timezone *)NULL);
	timep->tv_sec -= mytime.tv_sec;
	if (mytime.tv_usec > timep->tv_usec) {
		timep->tv_sec -= 1;
		timep->tv_usec += MILLION;
	}
	timep->tv_usec -= mytime.tv_usec;
	return (TRUE);
}

static struct auth_ops *
authdes_ops()
{
	static struct auth_ops ops;

	if (ops.ah_nextverf == NULL) {
		ops.ah_nextverf = authdes_nextverf;
		ops.ah_marshal = authdes_marshal;
		ops.ah_validate = authdes_validate;
		ops.ah_refresh = authdes_refresh;
		ops.ah_destroy = authdes_destroy;
	}
	return (&ops);
}

static
msgout(str)
	char *str;
{
#ifdef	_KERNEL
		(void) printf("%s", str);
#else
		(void) syslog(LOG_ERR, "%s", str);
#endif
}
