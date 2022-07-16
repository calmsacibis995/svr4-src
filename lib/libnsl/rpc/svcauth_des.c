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


#ident	"@(#)librpc:svcauth_des.c	1.2.2.1"

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
static char sccsid[] = "@(#)svcauth_des.c 1.4 89/03/10 Copyr 1987 Sun Micro";
#endif

/*
 * svcauth_des.c, server-side des authentication
 *
 * We insure for the service the following:
 * (1) The timestamp microseconds do not exceed 1 million.
 * (2) The timestamp plus the window is less than the current time.
 * (3) The timestamp is not less than the one previously 
 *     seen in the current session.
 *
 * It is up to the server to determine if the window size is
 * too small .
 *
 */


#include <rpc/des_crypt.h>
#include <rpc/rpc.h>

#ifdef KERNEL
#include <sys/kernel.h>
#define gettimeofday(tvp, tzp)	(*(tvp) = time)
#else
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define LOG_DEBUG 7
#endif /* SYSLOG */
#endif

#ifndef NGROUPS
#define uid_t u_short
#define gid_t u_short
#define NGROUPS 16
#endif

extern char *strcpy();

#define USEC_PER_SEC ((u_long) 1000000L)
#define BEFORE(t1, t2) timercmp(t1, t2, <)

/*
 * LRU cache of conversation keys and some other useful items.
 */
#define AUTHDES_CACHESZ 64
struct cache_entry {
	des_block key;			/* conversation key */
	char *rname;			/* client's name */
	u_int window;			/* credential lifetime window */
	struct timeval laststamp;	/* detect replays of creds */
	char *localcred;		/* generic local credential */
};
struct cache_entry *_rpc_authdes_cache/* [AUTHDES_CACHESZ] */;
static short *authdes_lru/* [AUTHDES_CACHESZ] */;

static void cache_init();		/* initialize the cache */
static short cache_spot();		/* find an entry in the cache */
static void cache_ref(/*short sid*/);	/* note that sid was ref'd */
static void invalidate();		/* invalidate entry in cache */

/*
 * cache statistics 
 */
struct {
	u_long ncachehits;	/* times cache hit, and is not replay */
	u_long ncachereplays;	/* times cache hit, and is replay */
	u_long ncachemisses;	/* times cache missed */
} svcauthdes_stats;

/*
 * Service side authenticator for AUTH_DES
 */
enum auth_stat
_svcauth_des(rqst, msg)
	register struct svc_req *rqst;
	register struct rpc_msg *msg;
{

	register long *ixdr;
	des_block cryptbuf[2];
	register struct authdes_cred *cred;
	struct authdes_verf verf;
	int status;
	register struct cache_entry *entry;
	short sid;
	des_block *sessionkey;
	des_block ivec;
	u_int window;
	struct timeval timestamp;
	u_long namelen;
	struct area {
		struct authdes_cred area_cred;
		char area_netname[MAXNETNAMELEN+1];
	} *area;

	if (_rpc_authdes_cache == NULL) {
		cache_init();
	}

	area = (struct area *)rqst->rq_clntcred;
	cred = (struct authdes_cred *)&area->area_cred;

	/*
	 * Get the credential
	 */
	ixdr = (long *)msg->rm_call.cb_cred.oa_base;
	cred->adc_namekind = IXDR_GET_ENUM(ixdr, enum authdes_namekind);
	switch (cred->adc_namekind) {
	case ADN_FULLNAME:
		namelen = IXDR_GET_U_LONG(ixdr);
		if (namelen > MAXNETNAMELEN) {
			return (AUTH_BADCRED);
		}
		cred->adc_fullname.name = area->area_netname;
		memcpy(cred->adc_fullname.name, (char *)ixdr, (u_int)namelen);
		cred->adc_fullname.name[namelen] = 0;
		ixdr += (RNDUP(namelen) / BYTES_PER_XDR_UNIT);
		cred->adc_fullname.key.key.high = (u_long)*ixdr++;
		cred->adc_fullname.key.key.low = (u_long)*ixdr++;
		cred->adc_fullname.window = (u_long)*ixdr++;
		break;
	case ADN_NICKNAME:
		cred->adc_nickname = (u_long)*ixdr++;
		break;
	default:
		return (AUTH_BADCRED);	
	}

	/*
	 * Get the verifier
	 */
	ixdr = (long *)msg->rm_call.cb_verf.oa_base;
	verf.adv_xtimestamp.key.high = (u_long)*ixdr++;
	verf.adv_xtimestamp.key.low = (u_long)*ixdr++;
	verf.adv_int_u = (u_long)*ixdr++;

	/*
	 * Get the conversation key
 	 */
	if (cred->adc_namekind == ADN_FULLNAME) {
		sessionkey = &cred->adc_fullname.key;
		if (key_decryptsession(cred->adc_fullname.name, 
				sessionkey) < 0) {
			_msgout("key_decryptsessionkey failed");
			return (AUTH_BADCRED);	/* key not found */
		}
	} else { /* ADN_NICKNAME */	
		sid = cred->adc_nickname;
		if (sid >= AUTHDES_CACHESZ) {
			_msgout("bad nickname");
			return (AUTH_BADCRED);	/* garbled credential */
		}
		sessionkey = &_rpc_authdes_cache[sid].key;
	}

	/*
	 * Decrypt the timestamp
	 */
	cryptbuf[0] = verf.adv_xtimestamp;
	if (cred->adc_namekind == ADN_FULLNAME) {
		cryptbuf[1].key.high = cred->adc_fullname.window;
		cryptbuf[1].key.low = verf.adv_winverf;
		ivec.key.high = ivec.key.low = 0;	
		status = cbc_crypt((char *)sessionkey, (char *)cryptbuf,
			2*sizeof(des_block), DES_DECRYPT | DES_HW, 
			(char *)&ivec);
	} else {
		status = ecb_crypt((char *)sessionkey, (char *)cryptbuf,
			sizeof(des_block), DES_DECRYPT | DES_HW);
	}
	if (DES_FAILED(status)) {
		_msgout("decryption failure");
		return (AUTH_FAILED);	/* system error */
	}

	/*
	 * XDR the decrypted timestamp
	 */
	ixdr = (long *)cryptbuf;
	timestamp.tv_sec = IXDR_GET_LONG(ixdr);
	timestamp.tv_usec = IXDR_GET_LONG(ixdr);

	/*
 	 * Check for valid credentials and verifiers.
	 * They could be invalid because the key was flushed
	 * out of the cache, and so a new session should begin.
	 * Be sure and send AUTH_REJECTED{CRED, VERF} if this is the case.
	 */
	{
		struct timeval current;
		int nick;
		int winverf;

		if (cred->adc_namekind == ADN_FULLNAME) {
			window = IXDR_GET_U_LONG(ixdr);
			winverf = IXDR_GET_U_LONG(ixdr);
			if (winverf != window - 1) {
				_msgout("window verifier mismatch");
				return (AUTH_BADCRED);	/* garbled credential */
			}
			sid = cache_spot(sessionkey, cred->adc_fullname.name, 
					&timestamp);
			if (sid < 0) {
				_msgout("replayed credential");
				return (AUTH_REJECTEDCRED);	/* replay */
			}
			nick = 0;
		} else {	/* ADN_NICKNAME */
			window = _rpc_authdes_cache[sid].window;
			nick = 1;
		}

		if ((u_long)timestamp.tv_usec >= USEC_PER_SEC) {
			_msgout("invalid usecs");
			/* cached out (bad key), or garbled verifier */
			return (nick ? AUTH_REJECTEDVERF : AUTH_BADVERF);
		}
		if (nick && BEFORE(&timestamp, 
					&_rpc_authdes_cache[sid].laststamp)) {
			_msgout("timestamp before last seen");
			return (AUTH_REJECTEDVERF);	/* replay */
		}
		(void) gettimeofday(&current, (struct timezone *)NULL);
		current.tv_sec -= window;	/* allow for expiration */
		if (!BEFORE(&current, &timestamp)) {
			_msgout("timestamp expired");
			/* replay, or garbled credential */
			return (nick ? AUTH_REJECTEDVERF : AUTH_BADCRED);
		}
	}

	/*
	 * Set up the reply verifier
	 */
	verf.adv_nickname = sid;

	/*
	 * xdr the timestamp before encrypting
	 */
	ixdr = (long *)cryptbuf;
	IXDR_PUT_LONG(ixdr, timestamp.tv_sec - 1);
	IXDR_PUT_LONG(ixdr, timestamp.tv_usec);

	/*	 
	 * encrypt the timestamp
	 */
	status = ecb_crypt((char *)sessionkey, (char *)cryptbuf,
				sizeof(des_block), DES_ENCRYPT | DES_HW);
	if (DES_FAILED(status)) {
		_msgout("encryption failure");
		return (AUTH_FAILED);	/* system error */
	}
	verf.adv_xtimestamp = cryptbuf[0];

	/*
	 * Serialize the reply verifier, and update rqst
	 */
	ixdr = (long *)msg->rm_call.cb_verf.oa_base;
	*ixdr++ = (long)verf.adv_xtimestamp.key.high;
	*ixdr++ = (long)verf.adv_xtimestamp.key.low;
	*ixdr++ = (long)verf.adv_int_u;

	rqst->rq_xprt->xp_verf.oa_flavor = AUTH_DES;
	rqst->rq_xprt->xp_verf.oa_base = msg->rm_call.cb_verf.oa_base;
	rqst->rq_xprt->xp_verf.oa_length = 
		(char *)ixdr - msg->rm_call.cb_verf.oa_base;

	/*
	 * We succeeded, commit the data to the cache now and
	 * finish cooking the credential.
	 */
	entry = &_rpc_authdes_cache[sid];
	entry->laststamp = timestamp;
	cache_ref(sid);
	if (cred->adc_namekind == ADN_FULLNAME) {
		cred->adc_fullname.window = window;
		cred->adc_nickname = sid;	/* save nickname */
		if (entry->rname != NULL) {
			mem_free(entry->rname, strlen(entry->rname) + 1);
		}
		entry->rname = mem_alloc((u_int)strlen(cred->adc_fullname.name)
					 + 1);
		if (entry->rname != NULL) {
			(void) strcpy(entry->rname, cred->adc_fullname.name);
		} else {
			_msgout("out of memory");
		}
		entry->key = *sessionkey;
		entry->window = window;
		invalidate(entry->localcred);	/* mark any cached cred invalid */
	} else { /* ADN_NICKNAME */
		/*
		 * nicknames are cooked into fullnames
		 */	
		cred->adc_namekind = ADN_FULLNAME;
		cred->adc_fullname.name = entry->rname;
		cred->adc_fullname.key = entry->key;
		cred->adc_fullname.window = entry->window;
	}
	return (AUTH_OK);	/* we made it!*/
}


/*
 * Initialize the cache
 */
static void
cache_init()
{
	register int i;

	_rpc_authdes_cache = (struct cache_entry *)
		mem_alloc(sizeof(struct cache_entry) * AUTHDES_CACHESZ);	
	memset((char *)_rpc_authdes_cache, 0,
		sizeof(struct cache_entry) * AUTHDES_CACHESZ);

	authdes_lru = (short *)mem_alloc(sizeof(short) * AUTHDES_CACHESZ);
	/*
	 * Initialize the lru list
	 */
	for (i = 0; i < AUTHDES_CACHESZ; i++) {
		authdes_lru[i] = i;
	}
}

/*
 * Find the lru victim
 */
static short
cache_victim()
{
	return (authdes_lru[AUTHDES_CACHESZ-1]);
}

/*
 * Note that sid was referenced
 */
static void
cache_ref(sid)
	register short sid;
{
	register int i;
	register short curr;
	register short prev;

	prev = authdes_lru[0];
	authdes_lru[0] = sid;
	for (i = 1; prev != sid; i++) {
		curr = authdes_lru[i];
		authdes_lru[i] = prev;
		prev = curr;
	}
}

/*
 * Find a spot in the cache for a credential containing
 * the items given. Return -1 if a replay is detected, otherwise
 * return the spot in the cache.
 */
static short
cache_spot(key, name, timestamp)
	register des_block *key;
	char *name;
	struct timeval *timestamp;
{
	register struct cache_entry *cp;
	register int i;
	register u_long hi;

	hi = key->key.high;
	for (cp = _rpc_authdes_cache, i = 0; i < AUTHDES_CACHESZ; i++, cp++) {
		if (cp->key.key.high == hi && 
		    cp->key.key.low == key->key.low &&
		    cp->rname != NULL &&
		    memcmp(cp->rname, name, strlen(name) + 1) == 0) {
			if (BEFORE(timestamp, &cp->laststamp)) {
				svcauthdes_stats.ncachereplays++;
				return (-1);	/* replay */
			}
			svcauthdes_stats.ncachehits++;
			return (i);	/* refresh */
		}
	}
	svcauthdes_stats.ncachemisses++;
	return (cache_victim());	/* new credential */
}


/*
 * Local credential handling stuff.
 * NOTE: bsd unix dependent.
 * Other operating systems should put something else here.
 */
#define UNKNOWN 	-2	/* grouplen, if cached cred is unknown user */
#define INVALID		-1 	/* grouplen, if cache entry is invalid */

struct bsdcred {
	uid_t uid;		/* cached uid */
	gid_t gid;		/* cached gid */
	short grouplen;	/* length of cached groups */
	short groups[NGROUPS];	/* cached groups */
};

static void
invalidate(cred)
	char *cred;
{
	if (cred == NULL) {
		return;
	}
	((struct bsdcred *)cred)->grouplen = INVALID;
}
/*#endif*/

static
_msgout(str)
	char *str;
{
#ifdef	KERNEL
		printf("%s", str);
#else
		(void) syslog(LOG_DEBUG, "%s", str);
#endif
}
