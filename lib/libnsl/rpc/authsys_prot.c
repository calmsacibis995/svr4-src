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


#ident	"@(#)librpc:authsys_prot.c	1.3.2.1"

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
static char sccsid[] = "@(#)authsys_prot.c 1.24 89/02/07 Copyr 1984 Sun Micro";
#endif

/*
 * authsys_prot.c
 * XDR for UNIX style authentication parameters for RPC
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#ifdef KERNEL
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#endif

#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/auth_sys.h>

/*
 * XDR for unix authentication parameters.
 */
bool_t
xdr_authsys_parms(xdrs, p)
	register XDR *xdrs;
	register struct authsys_parms *p;
{
	if (xdr_u_long(xdrs, &(p->aup_time)) &&
	    xdr_string(xdrs, &(p->aup_machname), MAX_MACHINE_NAME) &&
	    xdr_uid_t(xdrs, (uid_t *)&(p->aup_uid)) &&
	    xdr_gid_t(xdrs, (gid_t *)&(p->aup_gid)) &&
	    xdr_array(xdrs, (caddr_t *)&(p->aup_gids),
		    &(p->aup_len), NGRPS, sizeof (gid_t), xdr_gid_t)) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * XDR user id types (uid_t)
 */
bool_t
xdr_uid_t(xdrs, ip)
	XDR *xdrs;
	int *ip;
{
#ifdef lint
	(void) (xdr_short(xdrs, (short *)ip));
	return (xdr_long(xdrs, (long *)ip));
#else
	if (sizeof (uid_t) == sizeof (long)) {
		return (xdr_long(xdrs, (long *)ip));
	} else {
		return (xdr_short(xdrs, (short *)ip));
	}
#endif
}

/*
 * XDR group id types (gid_t)
 */
bool_t
xdr_gid_t(xdrs, ip)
	XDR *xdrs;
	int *ip;
{
#ifdef lint
	(void) (xdr_short(xdrs, (short *)ip));
	return (xdr_long(xdrs, (long *)ip));
#else
	if (sizeof (gid_t) == sizeof (long)) {
		return (xdr_long(xdrs, (long *)ip));
	} else {
		return (xdr_short(xdrs, (short *)ip));
	}
#endif
}

#ifdef KERNEL
/*
 * XDR kernel unix auth parameters.
 * Goes out of the u struct directly.
 * NOTE: this is an XDR_ENCODE only routine.
 */
xdr_authkern(xdrs)
	register XDR *xdrs;
{
	int	*gp;
	uid_t	uid = getuid();
	gid_t	gid = getgid();
	int	len;
	caddr_t	groups;
	char	*name = hostname;

	if (xdrs->x_op != XDR_ENCODE) {
		return (FALSE);
	}

	for (gp = &u.u_groups[NGROUPS]; gp > u.u_groups; gp--) {
		if (gp[-1] >= 0) {
			break;
		}
	}
	len = gp - u.u_groups;
	groups = (caddr_t)u.u_groups;
	if (xdr_u_long(xdrs, (u_long *)&time.tv_sec) &&
	    xdr_string(xdrs, &name, MAX_MACHINE_NAME) &&
	    xdr_uid_t(xdrs, &uid) &&
	    xdr_gid_t(xdrs, &gid) &&
	    xdr_array(xdrs, &groups, (u_int *)&len, NGRPS,
			sizeof (gid_t), xdr_gid_t)) {
		return (TRUE);
	}
	return (FALSE);
}
#endif
