/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:auth.h	1.3"

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
 * auth.h, Authentication interface.
 *
 * The data structures are completely opaque to the client. The client
 * is required to pass a AUTH * to routines that create rpc
 * "sessions".
 */
#ifndef _RPC_AUTH_H
#define _RPC_AUTH_H

#define MAX_AUTH_BYTES	400
#define MAXNETNAMELEN	255	/* maximum length of network user's name */

/*
 * Status returned from authentication check
 */
enum auth_stat {
	AUTH_OK=0,
	/*
	 * failed at remote end
	 */
	AUTH_BADCRED=1,			/* bogus credentials (seal broken) */
	AUTH_REJECTEDCRED=2,		/* client should begin new session */
	AUTH_BADVERF=3,			/* bogus verifier (seal broken) */
	AUTH_REJECTEDVERF=4,		/* verifier expired or was replayed */
	AUTH_TOOWEAK=5,			/* rejected due to security reasons */
	/*
	 * failed locally
	*/
	AUTH_INVALIDRESP=6,		/* bogus response verifier */
	AUTH_FAILED=7			/* some unknown reason */
};

#if (mc68000 || sparc || vax || i386 || u3b2)
typedef u_long u_int32;	/* 32-bit unsigned integers */
#endif

union des_block {
	struct {
		u_int32 high;
		u_int32 low;
	} key;
	char c[8];
};
typedef union des_block des_block;
extern bool_t xdr_des_block();

/*
 * Authentication info. Opaque to client.
 */
struct opaque_auth {
	enum_t	oa_flavor;		/* flavor of auth */
	caddr_t	oa_base;		/* address of more auth stuff */
	u_int	oa_length;		/* not to exceed MAX_AUTH_BYTES */
};


/*
 * Auth handle, interface to client side authenticators.
 */
typedef struct {
	struct	opaque_auth	ah_cred;
	struct	opaque_auth	ah_verf;
	union	des_block	ah_key;
	struct auth_ops {
		void	(*ah_nextverf)();
		int	(*ah_marshal)();	/* nextverf & serialize */
		int	(*ah_validate)();	/* validate varifier */
		int	(*ah_refresh)();	/* refresh credentials */
		void	(*ah_destroy)();	/* destroy this structure */
	} *ah_ops;
	caddr_t ah_private;
} AUTH;


/*
 * Authentication ops.
 * The ops and the auth handle provide the interface to the authenticators.
 *
 * AUTH	*auth;
 * XDR	*xdrs;
 * struct opaque_auth verf;
 */
#define AUTH_NEXTVERF(auth)		\
		((*((auth)->ah_ops->ah_nextverf))(auth))
#define auth_nextverf(auth)		\
		((*((auth)->ah_ops->ah_nextverf))(auth))

#define AUTH_MARSHALL(auth, xdrs)	\
		((*((auth)->ah_ops->ah_marshal))(auth, xdrs))
#define auth_marshall(auth, xdrs)	\
		((*((auth)->ah_ops->ah_marshal))(auth, xdrs))

#define AUTH_VALIDATE(auth, verfp)	\
		((*((auth)->ah_ops->ah_validate))((auth), verfp))
#define auth_validate(auth, verfp)	\
		((*((auth)->ah_ops->ah_validate))((auth), verfp))

#define AUTH_REFRESH(auth)		\
		((*((auth)->ah_ops->ah_refresh))(auth))
#define auth_refresh(auth)		\
		((*((auth)->ah_ops->ah_refresh))(auth))

#define AUTH_DESTROY(auth)		\
		((*((auth)->ah_ops->ah_destroy))(auth))
#define auth_destroy(auth)		\
		((*((auth)->ah_ops->ah_destroy))(auth))


extern struct opaque_auth _null_auth;


/*
 * These are the various implementations of client side authenticators.
 */

/*
 * System style authentication
 * AUTH *authsys_create(machname, uid, gid, len, aup_gids)
 *	char *machname;
 *	uid_t uid;
 *	gid_t gid;
 *	int len;
 *	gid_t *aup_gids;
 */
#ifdef _KERNEL
extern AUTH *authkern_create();		/* takes no parameters */
#else
extern AUTH *authsys_create();
extern AUTH *authsys_create_default();	/* takes no parameters */
extern AUTH *authnone_create();		/* takes no parameters */

/* Will get obsolete in near future */
#define authunix_create(machname, uid, gid, len, aup_gids) \
	authsys_create(machname, uid, gid, len, aup_gids)
#define authunix_create_default() authsys_create_default()

#endif

/*
 * DES style authentication
 * AUTH *authdes_seccreate(servername, window, timehost, ckey)
 *	char *servername;		- network name of server
 *	u_int window;			- time to live
 *	char *timehost;			- optional hostname to sync with
 *	des_block *ckey;		- optional conversation key to use
 */
/* Will get obsolete in near future */
#ifdef _KERNEL
extern int authdes_create();
#else
extern AUTH *authdes_create();
#endif
extern AUTH *authdes_seccreate();

#define AUTH_NONE	0		/* no authentication */
#define	AUTH_NULL	0		/* backward compatibility */
#define	AUTH_SYS	1		/* unix style (uid, gids) */
#define AUTH_UNIX	AUTH_SYS
#define	AUTH_SHORT	2		/* short hand unix style */
#define AUTH_DES	3		/* des style (encrypted timestamps) */

#endif /* !_RPC_AUTH_H */
