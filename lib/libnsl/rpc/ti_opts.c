/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libnsl:rpc/ti_opts.c	1.2.1.1"

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
#include <stdio.h>
#include <sys/types.h>
#include <tiuser.h>
#include <rpc/rpc.h>
#include <sys/ticlts.h>

int
_rpc_negotiate_uid(fd)
int fd;
{
	struct t_optmgmt *options;
	struct t_optmgmt *optionsret;
	struct {
		struct tcl_opt_hdr	uid_hdr;
		struct tcl_opt_setid	uid_opt;
	} send_in, *opt_out;

	extern int t_errno;
	extern int errno;

	options = (struct t_optmgmt *)t_alloc(fd, T_OPTMGMT, 0);
	optionsret = (struct t_optmgmt *)t_alloc(fd, T_OPTMGMT, T_OPT);
	if ((options == NULL) || (optionsret == NULL)) {
#ifdef DEBUG
		fprintf(stderr, "No memory\n");
#endif
		return (-1);
	}
	send_in.uid_hdr.hdr_thisopt_off = sizeof (struct tcl_opt_hdr);
	send_in.uid_hdr.hdr_nexthdr_off = TCL_OPT_NOHDR;
	send_in.uid_opt.setid_type= TCL_OPT_SETID;
	send_in.uid_opt.setid_flg= TCL_IDFLG_UID;
	options->opt.maxlen = sizeof (send_in);
	options->opt.len = sizeof (send_in);
	options->opt.buf = (char *) &send_in;
	options->flags = T_NEGOTIATE;
	if (t_optmgmt(fd, options, optionsret) == -1) {
#ifdef DEBUG
		fprintf(stderr, "No initial negotiate t_error %d err %d\n",
				t_errno, errno);
#endif
		return (-1);
	}

	options->opt.buf = 0;

	(void) t_free((char *)options, T_OPTMGMT);
	(void) t_free((char *)optionsret, T_OPTMGMT);
	return (0);
}

int
_rpc_get_local_uid(trans, uid_out)
SVCXPRT *trans;
uid_t *uid_out;
{
	struct t_optmgmt *optionsret;

	struct netbuf * abuf;
	struct myopts{
		struct tcl_opt_hdr uid_hdr;
		struct tcl_opt_uid uid_opt;
	}  *opt_out;

	abuf = (struct netbuf *) trans->xp_p2;
	if (abuf == 0) {
#ifdef DEBUG
		fprintf(stderr, "get_local_uid: null xp_p2\n");
#endif
		return (-1);
	}
	if (abuf->len != sizeof (*opt_out)) {
#ifdef DEBUG
		fprintf(stderr, "get_local_uid: len %d is wrong want %d\n",
			abuf->len, sizeof (*opt_out));
#endif
		return (-1);
	}
	if (abuf->buf == 0) {
#ifdef DEBUG
		fprintf(stderr, "get_local_uid: null data\n");
#endif
		return (-1);
	}
	opt_out= (struct myopts *) abuf->buf;
	if (opt_out->uid_opt.uid_type != TCL_OPT_UID) {
#ifdef DEBUG
		fprintf(stderr, "get_local_uid: type %d is wrong want %d\n",
			opt_out->uid_opt.uid_type, TCL_OPT_UID);
#endif
		return (-1);
	}
	*uid_out= opt_out->uid_opt.uid_val;
	return (0);
}
