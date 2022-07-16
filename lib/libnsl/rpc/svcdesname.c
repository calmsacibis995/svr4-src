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


#ident	"@(#)librpc:svcdesname.c	1.2.2.1"

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
static char sccsid[] = "@(#)svcdesname.c 1.2 89/03/10 Copyr 1987 Sun Micro";
#endif

/*
 * svcdesname.c, server-side des authentication
 * netname conversion dependent stuff
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

#define USEC_PER_SEC ((u_long) 1000000L)
#define BEFORE(t1, t2) timercmp(t1, t2, <)

extern char *strcpy();

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
extern struct cache_entry *_rpc_authdes_cache/* [AUTHDES_CACHESZ] */;

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
	short grouplen;		/* length of cached groups */
	short groups[NGROUPS];	/* cached groups */
};

/*
 * Map a des credential into a unix cred.
 * We cache the credential here so the application does
 * not have to make an rpc call every time to interpret
 * the credential.
 */
authdes_getucred(adc, uid, gid, grouplen, groups)
	struct authdes_cred *adc;
	uid_t *uid;
	gid_t *gid;
	short *grouplen;
	register int *groups;
{
	unsigned sid;
	register int i;
	int i_uid;	
	int i_gid;
	int i_grouplen;
	struct bsdcred *cred;

	sid = adc->adc_nickname;
	if (sid >= AUTHDES_CACHESZ) {
		_msgout("authdes_getucred: invalid nickname");
		return (0);
	}
	cred = (struct bsdcred *)_rpc_authdes_cache[sid].localcred;
	if (cred == NULL) {
		cred = (struct bsdcred *)mem_alloc(sizeof(struct bsdcred));
		_rpc_authdes_cache[sid].localcred = (char *)cred;
		cred->grouplen = INVALID;
	}
	if (cred->grouplen == INVALID) {
		/*
		 * not in cache: lookup
		 */
		if (!netname2user(adc->adc_fullname.name, &i_uid, &i_gid, 
			&i_grouplen, groups))
		{
			_msgout("authdes_getucred: unknown netname");
			/* mark as lookup up, but not found */
			cred->grouplen = UNKNOWN;
			return (0);
		}
		_msgout("authdes_getucred: missed ucred cache");
		*uid = cred->uid = i_uid;
		*gid = cred->gid = i_gid;
		*grouplen = cred->grouplen = i_grouplen;
		for (i = i_grouplen - 1; i >= 0; i--) {
			cred->groups[i] = groups[i];	/* int to short */
		}
		return (1);
	} else if (cred->grouplen == UNKNOWN) {
		/*
		 * Already lookup up, but no match found
		 */	
		return (0);
	}

	/*
	 * cached credentials
	 */
	*uid = cred->uid;
	*gid = cred->gid;
	*grouplen = cred->grouplen;
	for (i = cred->grouplen - 1; i >= 0; i--) {
		groups[i] = cred->groups[i];	/* short to int */
	}
	return (1);
}

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
