/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:auth_des.c	1.3"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)auth_des.c 1.2 89/01/11 SMI"
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
 * auth_des.c, client-side implementation of DES authentication
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/tiuser.h>
#include <sys/errno.h>
#include <rpc/des_crypt.h>
#include <rpc/types.h>
#include <rpc/auth.h>
#include <rpc/auth_des.h>
#include <rpc/xdr.h>
#include <rpc/clnt.h>
#include <netinet/in.h>	/* XXX: just to get htonl() and ntohl() */
#include <sys/cmn_err.h>
#include <sys/debug.h>
#ifndef	_KERNEL
#include <sys/syslog.h>
#endif

#define MILLION		1000000L
#define RTIME_TIMEOUT 5		/* seconds to wait for sync */

#define AUTH_PRIVATE(auth)	(struct ad_private *) auth->ah_private
#define ALLOC(object_type)	(object_type *) mem_alloc(sizeof(object_type))
#define FREE(ptr, size)		mem_free((char *)(ptr), (int) size)
#define ATTEMPT(xdr_op)		if (!(xdr_op)) return (FALSE)

#ifdef _KERNEL
#define gettimeofday(tvp, tzp)	uniqtime(tvp)	/* fake system call */
#endif

static struct auth_ops *authdes_ops();

/*
 * This struct is pointed to by the ah_private field of an "AUTH *"
 */
struct ad_private {
	char *ad_fullname; 		/* client's full name */
	u_int ad_fullnamelen;		/* length of name, rounded up */
	char *ad_servername; 		/* server's full name */
	u_int ad_servernamelen;		/* length of name, rounded up */
	u_int ad_window;	  	/* client specified window */
	bool_t ad_dosync;		/* synchronize? */		
	struct netbuf ad_syncaddr;	/* remote host to synch with */
	dev_t ad_synctp;		/* Maj/Min of host transport device */
	int   ad_calltype;		/* use rpc or straight call for sync */
	struct timeval ad_timediff;	/* server's time - client's time */
	u_long ad_nickname;		/* server's nickname for client */
	struct authdes_cred ad_cred;	/* storage for credential */
	struct authdes_verf ad_verf;	/* storage for verifier */
	struct timeval ad_timestamp;	/* timestamp sent */
	des_block ad_xkey;		/* encrypted conversation key */
};
	

/*
 * Create the client des authentication object
 */	
/*ARGSUSED*/
int
authdes_create(servername, window, syncaddr, synctp, ckey, calltype, retauth)
	char *servername;		/* network name of server */
	u_int window;			/* time to live */
	struct netbuf *syncaddr;	/* optional addr of host to sync with */
	des_block *ckey;		/* optional conversation key to use*/
	dev_t synctp;			/* Device of tp to sync with. */
	int   calltype;
	AUTH **retauth;			/* set to new auth handle if success */
{
	AUTH *auth;
	struct ad_private *ad;
	char namebuf[MAXNETNAMELEN+1];
	void getnetname();
	int error = 0;

	if (retauth == NULL)
		return (EINVAL);
	*retauth = NULL;

	/*
 	 * Allocate everything now
	 */
	auth = ALLOC(AUTH);
	ad = ALLOC(struct ad_private);
	getnetname(namebuf);

	ad->ad_fullnamelen = RNDUP(strlen(namebuf));
	ad->ad_fullname = (char *)mem_alloc(ad->ad_fullnamelen + 1);

	ad->ad_servernamelen = strlen(servername);
	ad->ad_servername = (char *)mem_alloc(ad->ad_servernamelen + 1);

	if (auth == NULL || ad == NULL || ad->ad_fullname == NULL ||
	    ad->ad_servername == NULL) {
#ifdef	_KERNEL
		cmn_err(CE_NOTE, "authdes_create: out of memory");
#else
		(void) syslog(LOG_ERR, "authdes_create: out of memory");
#endif
		error = ENOMEM;
		goto failed;
	}

	/* 
	 * Set up private data
	 */
	bcopy(namebuf, ad->ad_fullname, (int)(ad->ad_fullnamelen + 1));
	bcopy(servername, ad->ad_servername, (int)(ad->ad_servernamelen + 1));
	if (syncaddr != NULL) {
		ad->ad_syncaddr = *syncaddr;
		ad->ad_synctp = synctp;
		ad->ad_dosync = TRUE;
		ad->ad_calltype = calltype;
	} else {
		ad->ad_timediff.tv_sec = 0;
		ad->ad_timediff.tv_usec = 0;
		ad->ad_dosync = FALSE;
	}
	ad->ad_window = window;
	if (ckey == NULL) {
		enum clnt_stat stat;
		if ((stat = key_gendes(&auth->ah_key)) != RPC_SUCCESS) {
#ifdef	_KERNEL
			cmn_err(CE_NOTE, "authdes_create: unable to gen conversation key");
#else
			(void) syslog(LOG_ERR, "authdes_create: unable to gen conversation key");
#endif
			if (stat == RPC_INTR)
				error = EINTR;
			else if (stat == RPC_TIMEDOUT)
				error = ETIMEDOUT;
			else
				error = EINVAL;		/* XXX */
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
	*retauth = auth;
	return (0);

failed:
	if (ad != NULL && ad->ad_fullname != NULL) 
		FREE(ad->ad_fullname, ad->ad_fullnamelen + 1);
	if (ad != NULL && ad->ad_servername != NULL) 
		FREE(ad->ad_servername, ad->ad_servernamelen + 1);
	if (ad != NULL) 
		FREE(ad, sizeof(struct ad_private));
	if (auth != NULL) 
		FREE(auth, sizeof(AUTH)); 
	return ((error == 0) ? EINVAL : error);		/* XXX */
}

/*
 * Implement the five authentication operations
 */


/*
 * 1. Next Verifier
 */	
/*ARGSUSED*/
STATIC void
authdes_nextverf(auth)
	AUTH *auth;
{
	/* what the heck am I supposed to do??? */
}
		


/*
 * 2. Marshal
 */
STATIC bool_t
authdes_marshal(auth, xdrs)
	AUTH *auth;
	XDR *xdrs;
{
	/* LINTED pointer alignment */
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
			2*sizeof(des_block), DES_ENCRYPT, (char *)&ivec);
	} else {
		status = ecb_crypt((char *)&auth->ah_key, (char *)cryptbuf, 
			sizeof(des_block), DES_ENCRYPT);
	}
	if (DES_FAILED(status)) {
#ifdef	_KERNEL
		cmn_err(CE_NOTE, "authdes_marshal: DES encryption failure");
#else
		(void) syslog(LOG_ERR, "authdes_marshal: DES encryption failure");
#endif
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
STATIC bool_t
authdes_validate(auth, rverf)
	AUTH *auth;
	struct opaque_auth *rverf;
{
	/* LINTED pointer alignment */
	struct ad_private *ad = AUTH_PRIVATE(auth);
	struct authdes_verf verf;
	int status;
	register u_long *ixdr;

	if (rverf->oa_length != (2 + 1) * BYTES_PER_XDR_UNIT) {
		return (FALSE);
	}
	/* LINTED pointer alignment */
	ixdr = (u_long *)rverf->oa_base;
	verf.adv_xtimestamp.key.high = (u_long)*ixdr++;
	verf.adv_xtimestamp.key.low = (u_long)*ixdr++;
	verf.adv_int_u = IXDR_GET_U_LONG(ixdr);

	/*
	 * Decrypt the timestamp
	 */
	status = ecb_crypt((char *)&auth->ah_key, (char *)&verf.adv_xtimestamp,
		sizeof(des_block), DES_DECRYPT);

	if (DES_FAILED(status)) {
#ifdef	_KERNEL
		cmn_err(CE_NOTE, "authdes_validate: DES decryption failure");
#else
		(void) syslog(LOG_ERR, "authdes_validate: DES decryption failure");
#endif
		return (FALSE);
	}

	/*
	 * xdr the decrypted timestamp 
	 */
	/* LINTED pointer alignment */
	ixdr = (u_long *)verf.adv_xtimestamp.c;
	verf.adv_timestamp.tv_sec = IXDR_GET_LONG(ixdr) + 1;
	verf.adv_timestamp.tv_usec = IXDR_GET_LONG(ixdr);

	/*
	 * validate
	 */
	if (bcmp((char *)&ad->ad_timestamp, (char *)&verf.adv_timestamp,
		 sizeof(struct timeval)) != 0) {
#ifdef	_KERNEL
		cmn_err(CE_NOTE, "authdes_validate: verifier mismatch");
#else
		(void) syslog(LOG_ERR, "authdes_validate: verifier mismatch");
#endif
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
STATIC bool_t
authdes_refresh(auth)
	AUTH *auth;
{
	/* LINTED pointer alignment */
	struct ad_private *ad = AUTH_PRIVATE(auth);
	struct authdes_cred *cred = &ad->ad_cred;

	if (ad->ad_dosync && 
			!synchronize(ad->ad_synctp, &ad->ad_syncaddr,
					ad->ad_calltype, &ad->ad_timediff)) {
		/*
		 * Hope the clocks are synced!
		 */
		ad->ad_timediff.tv_sec = ad->ad_timediff.tv_usec = 0;
#ifdef	_KERNEL
		cmn_err(CE_NOTE, "authdes_refresh: unable to synchronize with server");
#else
		(void) syslog(LOG_ERR, "authdes_refresh: unable to synchronize with server");
#endif
	}
	ad->ad_xkey = auth->ah_key;
	if (key_encryptsession(ad->ad_servername, &ad->ad_xkey) != RPC_SUCCESS) {
#ifdef	_KERNEL
		cmn_err(CE_NOTE, "authdes_refresh: unable to encrypt conversation key");
#else
		(void) syslog(LOG_ERR, "authdes_refresh: unable to encrypt conversation key");
#endif
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
STATIC void
authdes_destroy(auth)
	AUTH *auth;
{
	/* LINTED pointer alignment */
	struct ad_private *ad = AUTH_PRIVATE(auth);

	FREE(ad->ad_fullname, ad->ad_fullnamelen + 1);
	FREE(ad->ad_servername, ad->ad_servernamelen + 1);
	FREE(ad, sizeof(struct ad_private));
	FREE(auth, sizeof(AUTH));
}
	


/*
 * Synchronize with the server at the given address, that is,
 * adjust timep to reflect the delta between our clocks
 */
STATIC bool_t
synchronize(synctp, syncaddr, calltype, timep)
	dev_t	       synctp;
	struct netbuf  *syncaddr;
	struct timeval *timep;
	int		calltype;
{
	struct timeval mytime;
	struct timeval timeout;

	timeout.tv_sec = RTIME_TIMEOUT;
	timeout.tv_usec = 0;
	if (rtime(synctp, syncaddr, calltype, timep, &timeout) < 0) {
		return (FALSE);
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


