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

#ident	"@(#)librpc:rpc_callmsg.c	1.3.2.1"

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
static char sccsid[] = "@(#)rpc_callmsg.c 1.12 89/01/31 Copyr 1984 Sun Micro";
#endif

/*
 * rpc_callmsg.c
 *
 */

#include <sys/param.h>

#ifdef KERNEL
#include <rpc/types.h>		/* spell 'em out for make depend */
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <rpc/rpc_msg.h>
#else
#include <rpc/rpc.h>
#endif
#include <sys/byteorder.h>

/*
 * XDR a call message
 */
bool_t
xdr_callmsg(xdrs, cmsg)
	register XDR *xdrs;
	register struct rpc_msg *cmsg;
{
	register long *buf;
	register struct opaque_auth *oa;

	if (xdrs->x_op == XDR_ENCODE) {
		if (cmsg->rm_call.cb_cred.oa_length > MAX_AUTH_BYTES) {
			return (FALSE);
		}
		if (cmsg->rm_call.cb_verf.oa_length > MAX_AUTH_BYTES) {
			return (FALSE);
		}
		buf = XDR_INLINE(xdrs, 8 * BYTES_PER_XDR_UNIT
			+ RNDUP(cmsg->rm_call.cb_cred.oa_length)
			+ 2 * BYTES_PER_XDR_UNIT
			+ RNDUP(cmsg->rm_call.cb_verf.oa_length));
		if (buf != NULL) {
			IXDR_PUT_LONG(buf, cmsg->rm_xid);
			IXDR_PUT_ENUM(buf, cmsg->rm_direction);
			if (cmsg->rm_direction != CALL) {
				return (FALSE);
			}
			IXDR_PUT_LONG(buf, cmsg->rm_call.cb_rpcvers);
			if (cmsg->rm_call.cb_rpcvers != RPC_MSG_VERSION) {
				return (FALSE);
			}
			IXDR_PUT_LONG(buf, cmsg->rm_call.cb_prog);
			IXDR_PUT_LONG(buf, cmsg->rm_call.cb_vers);
			IXDR_PUT_LONG(buf, cmsg->rm_call.cb_proc);
			oa = &cmsg->rm_call.cb_cred;
			IXDR_PUT_ENUM(buf, oa->oa_flavor);
			IXDR_PUT_LONG(buf, oa->oa_length);
			if (oa->oa_length) {
				(void) memcpy((caddr_t)buf, oa->oa_base,
							(int)oa->oa_length);
				buf += RNDUP(oa->oa_length) / sizeof (long);
			}
			oa = &cmsg->rm_call.cb_verf;
			IXDR_PUT_ENUM(buf, oa->oa_flavor);
			IXDR_PUT_LONG(buf, oa->oa_length);
			if (oa->oa_length) {
				(void) memcpy((caddr_t)buf, oa->oa_base,
							(int)oa->oa_length);
				/* no real need....
				buf += RNDUP(oa->oa_length) / sizeof (long);
				*/
			}
			return (TRUE);
		}
	}
	if (xdrs->x_op == XDR_DECODE) {
		buf = XDR_INLINE(xdrs, 8 * BYTES_PER_XDR_UNIT);
		if (buf != NULL) {
			cmsg->rm_xid = IXDR_GET_LONG(buf);
			cmsg->rm_direction = IXDR_GET_ENUM(buf, enum msg_type);
			if (cmsg->rm_direction != CALL) {
				return (FALSE);
			}
			cmsg->rm_call.cb_rpcvers = IXDR_GET_LONG(buf);
			if (cmsg->rm_call.cb_rpcvers != RPC_MSG_VERSION) {
				return (FALSE);
			}
			cmsg->rm_call.cb_prog = IXDR_GET_LONG(buf);
			cmsg->rm_call.cb_vers = IXDR_GET_LONG(buf);
			cmsg->rm_call.cb_proc = IXDR_GET_LONG(buf);
			oa = &cmsg->rm_call.cb_cred;
			oa->oa_flavor = IXDR_GET_ENUM(buf, enum_t);
			oa->oa_length = IXDR_GET_LONG(buf);
			if (oa->oa_length) {
				if (oa->oa_length > MAX_AUTH_BYTES) {
					return (FALSE);
				}
				if (oa->oa_base == NULL) {
					oa->oa_base = (caddr_t)
						mem_alloc(oa->oa_length);
				}
				buf = XDR_INLINE(xdrs, RNDUP(oa->oa_length));
				if (buf == NULL) {
					if (xdr_opaque(xdrs, oa->oa_base,
					    oa->oa_length) == FALSE) {
						return (FALSE);
					}
				} else {
					(void) memcpy(oa->oa_base,
					   (caddr_t)buf, (int) oa->oa_length);
					/* no real need....
					buf += RNDUP(oa->oa_length) /
						sizeof (long);
					*/
				}
			}
			oa = &cmsg->rm_call.cb_verf;
			buf = XDR_INLINE(xdrs, 2 * BYTES_PER_XDR_UNIT);
			if (buf == NULL) {
				if (xdr_enum(xdrs, &oa->oa_flavor) == FALSE ||
				    xdr_u_int(xdrs, &oa->oa_length) == FALSE) {
					return (FALSE);
				}
			} else {
				oa->oa_flavor = IXDR_GET_ENUM(buf, enum_t);
				oa->oa_length = IXDR_GET_LONG(buf);
			}
			if (oa->oa_length) {
				if (oa->oa_length > MAX_AUTH_BYTES) {
					return (FALSE);
				}
				if (oa->oa_base == NULL) {
					oa->oa_base = (caddr_t)
						mem_alloc(oa->oa_length);
				}
				buf = XDR_INLINE(xdrs, RNDUP(oa->oa_length));
				if (buf == NULL) {
					if (xdr_opaque(xdrs, oa->oa_base,
					    oa->oa_length) == FALSE) {
						return (FALSE);
					}
				} else {
					(void) memcpy(oa->oa_base,
					   (caddr_t)buf, (int)oa->oa_length);
					/* no real need...
					buf += RNDUP(oa->oa_length) /
						sizeof (long);
					*/
				}
			}
			return (TRUE);
		}
	}
	if (xdr_u_long(xdrs, &(cmsg->rm_xid)) &&
	    xdr_enum(xdrs, (enum_t *)&(cmsg->rm_direction)) &&
	    (cmsg->rm_direction == CALL) &&
	    xdr_u_long(xdrs, &(cmsg->rm_call.cb_rpcvers)) &&
	    (cmsg->rm_call.cb_rpcvers == RPC_MSG_VERSION) &&
	    xdr_u_long(xdrs, &(cmsg->rm_call.cb_prog)) &&
	    xdr_u_long(xdrs, &(cmsg->rm_call.cb_vers)) &&
	    xdr_u_long(xdrs, &(cmsg->rm_call.cb_proc)) &&
	    xdr_opaque_auth(xdrs, &(cmsg->rm_call.cb_cred)) )
	    return (xdr_opaque_auth(xdrs, &(cmsg->rm_call.cb_verf)));
	return (FALSE);
}
