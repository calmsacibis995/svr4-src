/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:authu_prot.c	1.3"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)authunix_prot.c 1.4 89/01/11 SMI"
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
 * authunix_prot.c
 * XDR for UNIX style authentication parameters for RPC
 */

#include <rpc/types.h>
#ifdef _KERNEL
#include <sys/param.h>
#include <sys/time.h>
#include <sys/cred.h>
#include <sys/proc.h>
#include <sys/user.h>
#endif

#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/auth_unix.h>
#include <sys/utsname.h>

/*
 * XDR for unix authentication parameters.
 */
bool_t
xdr_authunix_parms(xdrs, p)
	register XDR *xdrs;
	register struct authunix_parms *p;
{

	if (xdr_u_long(xdrs, &(p->aup_time))
	    && xdr_string(xdrs, &(p->aup_machname), MAX_MACHINE_NAME)
	    && xdr_int(xdrs, (int *)&(p->aup_uid))
	    && xdr_int(xdrs, (int *)&(p->aup_gid))
	    && xdr_array(xdrs, (caddr_t *)&(p->aup_gids),
		    &(p->aup_len), NGROUPS_UMAX, sizeof(int), xdr_int) ) {
		return (TRUE);
	}
	return (FALSE);
}

#ifdef _KERNEL
/*
 * XDR kernel unix auth parameters.
 * Goes out of the u struct directly.
 * NOTE: this is an XDR_ENCODE only routine.
 */
xdr_authkern(xdrs)
	register XDR *xdrs;
{

	/* int	*gp; */
	uid_t	 uid = u.u_cred->cr_uid;
	gid_t	 gid = u.u_cred->cr_gid;
	int	 len;
	caddr_t	groups;
	char	*name = (caddr_t)utsname.nodename;

	if (xdrs->x_op != XDR_ENCODE) {
		return (FALSE);
	}

/*
	for (gp = &u.u_groups[NGROUPS_UMAX]; gp > u.u_groups; gp--) {
		if (gp[-1] >= 0) {
			break;
		}
	}
	len = gp - u.u_groups;
*/
	len = u.u_procp->p_cred->cr_ngroups;
	groups = (caddr_t)u.u_procp->p_cred->cr_groups;
        if (xdr_u_long(xdrs, (u_long *)&hrestime.tv_sec)
            && xdr_string(xdrs, &name, MAX_MACHINE_NAME)
            && xdr_int(xdrs, &uid)
            && xdr_int(xdrs, &gid)
	    && xdr_array(xdrs, &groups, (u_int *)&len, NGRPS, sizeof (int), xdr_int) ) {
                return (TRUE);
	}
	return (FALSE);
}
#endif
