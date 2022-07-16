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


#ident	"@(#)librpc:auth_sys.c	1.2.2.1"

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
static char sccsid[] = "@(#)auth_sys.c 1.34 89/02/28 Copyr 1984 Sun Micro";
#endif

/*
 * auth_sys.c, Implements UNIX (system) style authentication parameters. 
 *  
 * The system is very weak.  The client uses no encryption for its
 * credentials and only sends null verifiers.  The server sends backs
 * null verifiers or optionally a verifier that suggests a new short hand
 * for the credentials.
 *
 */

#ifdef KERNEL
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#else
#include <stdio.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define LOG_ERR 3
#endif /* SYSLOG */
#endif /* _KERNEL */

#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/auth_sys.h>

static struct auth_ops *authsys_ops();

/*
 * This struct is pointed to by the ah_private field of an auth_handle.
 */
struct audata {
	struct opaque_auth	au_origcred;	/* original credentials */
	struct opaque_auth	au_shcred;	/* short hand cred */
	u_long			au_shfaults;	/* short hand cache faults */
	char			au_marshed[MAX_AUTH_BYTES];
	u_int			au_mpos;	/* xdr pos at end of marshed */
};
#define	AUTH_PRIVATE(auth)	((struct audata *)auth->ah_private)

static bool_t marshal_new_auth();

#ifndef KERNEL
/*
 * Create a (sys) unix style authenticator.
 * Returns an auth handle with the given stuff in it.
 */
AUTH *
authsys_create(machname, uid, gid, len, aup_gids)
	char *machname;
	uid_t uid;
	gid_t gid;
	register int len;
	gid_t *aup_gids;
{
	struct authsys_parms aup;
	char mymem[MAX_AUTH_BYTES];
	struct timeval now;
	XDR xdrs;
	register AUTH *auth;
	register struct audata *au;

	/*
	 * Allocate and set up auth handle
	 */
	auth = (AUTH *)mem_alloc(sizeof(*auth));
	if (auth == NULL) {
		(void) syslog(LOG_ERR, "authsys_create: out of memory");
		return (NULL);
	}
	au = (struct audata *)mem_alloc(sizeof(*au));
	if (au == NULL) {
		(void) syslog(LOG_ERR, "authsys_create: out of memory");
		(void) mem_free((char *) auth, sizeof(*auth));
		return (NULL);
	}
	auth->ah_ops = authsys_ops();
	auth->ah_private = (caddr_t)au;
	auth->ah_verf = au->au_shcred = _null_auth;
	au->au_shfaults = 0;

	/*
	 * fill in param struct from the given params
	 */
	(void) gettimeofday(&now,  (struct timezone *)0);
	aup.aup_time = now.tv_sec;
	aup.aup_machname = machname;
	aup.aup_uid = uid;
	aup.aup_gid = gid;
	aup.aup_len = (u_int)len;
	aup.aup_gids = aup_gids;

	/*
	 * Serialize the parameters into origcred
	 */
	xdrmem_create(&xdrs, mymem, MAX_AUTH_BYTES, XDR_ENCODE);
	if (! xdr_authsys_parms(&xdrs, &aup)) {
		(void) syslog(LOG_ERR, "authsys_create:  xdr_authsys_parms failed");
		abort();
	}
	au->au_origcred.oa_length = len = XDR_GETPOS(&xdrs);
	au->au_origcred.oa_flavor = AUTH_SYS;
	if ((au->au_origcred.oa_base = mem_alloc((u_int) len)) == NULL) {
		(void) syslog(LOG_ERR, "authsys_create: out of memory");
		(void) mem_free((char *) au, sizeof(*au));
		(void) mem_free((char *) auth, sizeof(*auth));
		return (NULL);
	}
	(void) memcpy(au->au_origcred.oa_base, mymem, (u_int)len);

	/*
	 * set auth handle to reflect new cred.
	 */
	auth->ah_cred = au->au_origcred;
	marshal_new_auth(auth);
	return (auth);
}

/*
 * Returns an auth handle with parameters determined by doing lots of
 * syscalls.
 */
AUTH *
authsys_create_default()
{
	register int len;
	char machname[MAX_MACHINE_NAME + 1];
	register uid_t uid;
	register gid_t gid;
	gid_t gids[NGRPS];

	if (gethostname(machname, MAX_MACHINE_NAME) == -1) {
		(void) syslog(LOG_ERR, "authsys_create_default:  gethostname failed:  %m");
		abort();
	}
	machname[MAX_MACHINE_NAME] = 0;
	uid = geteuid();
	gid = getegid();
	if ((len = getgroups(NGRPS, gids)) < 0) {
		(void) syslog(LOG_ERR, "authsys_create_default:  getgroups failed:  %m");
		abort();
	}
	return (authsys_create(machname, uid, gid, len, gids));
}
#endif /* !KERNEL */

/*
 * authsys operations
 */

static void
authsys_nextverf(auth)
	AUTH *auth;
{
	/* no action necessary */
}

static bool_t
authsys_marshal(auth, xdrs)
	AUTH *auth;
	XDR *xdrs;
{
	register struct audata *au = AUTH_PRIVATE(auth);

	return (XDR_PUTBYTES(xdrs, au->au_marshed, au->au_mpos));
}

static bool_t
authsys_validate(auth, verf)
	register AUTH *auth;
	struct opaque_auth verf;
{
	register struct audata *au;
	XDR xdrs;

	if (verf.oa_flavor == AUTH_SHORT) {
		au = AUTH_PRIVATE(auth);
		xdrmem_create(&xdrs, verf.oa_base, verf.oa_length, XDR_DECODE);

		if (au->au_shcred.oa_base != NULL) {
			mem_free(au->au_shcred.oa_base,
			    au->au_shcred.oa_length);
			au->au_shcred.oa_base = NULL;
		}
		if (xdr_opaque_auth(&xdrs, &au->au_shcred)) {
			auth->ah_cred = au->au_shcred;
		} else {
			xdrs.x_op = XDR_FREE;
			(void) xdr_opaque_auth(&xdrs, &au->au_shcred);
			au->au_shcred.oa_base = NULL;
			auth->ah_cred = au->au_origcred;
		}
		marshal_new_auth(auth);
	}
	return (TRUE);
}

static bool_t
authsys_refresh(auth)
	register AUTH *auth;
{
	register struct audata *au = AUTH_PRIVATE(auth);
	struct authsys_parms aup;
#ifndef KERNEL
	struct timeval now;
#endif
	XDR xdrs;
	register int stat;

	if (auth->ah_cred.oa_base == au->au_origcred.oa_base) {
		/* there is no hope.  Punt */
		return (FALSE);
	}
	au->au_shfaults ++;

	/* first deserialize the creds back into a struct authsys_parms */
	aup.aup_machname = NULL;
	aup.aup_gids = (gid_t *)NULL;
	xdrmem_create(&xdrs, au->au_origcred.oa_base,
	    au->au_origcred.oa_length, XDR_DECODE);
	stat = xdr_authsys_parms(&xdrs, &aup);
	if (! stat) 
		goto done;

	/* update the time and serialize in place */
#ifdef KERNEL
	aup.aup_time = time.tv_sec;
#else
	(void) gettimeofday(&now, (struct timezone *)0);
	aup.aup_time = now.tv_sec;
#endif
	xdrs.x_op = XDR_ENCODE;
	XDR_SETPOS(&xdrs, 0);
	stat = xdr_authsys_parms(&xdrs, &aup);
	if (! stat)
		goto done;
	auth->ah_cred = au->au_origcred;
	marshal_new_auth(auth);
done:
	/* free the struct authsys_parms created by deserializing */
	xdrs.x_op = XDR_FREE;
	(void) xdr_authsys_parms(&xdrs, &aup);
	XDR_DESTROY(&xdrs);
	return (stat);
}

static void
authsys_destroy(auth)
	register AUTH *auth;
{
	register struct audata *au = AUTH_PRIVATE(auth);

	mem_free(au->au_origcred.oa_base, au->au_origcred.oa_length);
	if (au->au_shcred.oa_base != NULL)
		mem_free(au->au_shcred.oa_base, au->au_shcred.oa_length);
	mem_free(auth->ah_private, sizeof(struct audata));
	if (auth->ah_verf.oa_base != NULL)
		mem_free(auth->ah_verf.oa_base, auth->ah_verf.oa_length);
	mem_free((caddr_t)auth, sizeof(*auth));
}

/*
 * Marshals (pre-serializes) an auth struct.
 * sets private data, au_marshed and au_mpos
 */
static bool_t
marshal_new_auth(auth)
	register AUTH *auth;
{
	XDR		xdr_stream;
	register XDR	*xdrs = &xdr_stream;
	register struct audata *au = AUTH_PRIVATE(auth);

	xdrmem_create(xdrs, au->au_marshed, MAX_AUTH_BYTES, XDR_ENCODE);
	if ((! xdr_opaque_auth(xdrs, &(auth->ah_cred))) ||
	    (! xdr_opaque_auth(xdrs, &(auth->ah_verf)))) {
#ifdef KERNEL
		printf("marshal_new_auth - Fatal marshalling problem");
#else
		(void) syslog(LOG_ERR, "auth_none.c - Fatal marshalling problem");
#endif
	} else {
		au->au_mpos = XDR_GETPOS(xdrs);
	}
	XDR_DESTROY(xdrs);
}

static struct auth_ops *
authsys_ops()
{
	static struct auth_ops ops;

	if (ops.ah_nextverf == NULL) {
		ops.ah_nextverf = authsys_nextverf;
		ops.ah_marshal = authsys_marshal;
		ops.ah_validate = authsys_validate;
		ops.ah_refresh = authsys_refresh;
		ops.ah_destroy = authsys_destroy;
	}
	return (&ops);
}
