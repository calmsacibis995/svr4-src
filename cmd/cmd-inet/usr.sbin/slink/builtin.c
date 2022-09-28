/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/slink/builtin.c	1.3.2.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

/*
 *
 *	Copyright 1987, 1988 Lachman Associates, Incorporated (LAI)
 *	All Rights Reserved.
 *
 *	The copyright above and this notice must be preserved in all
 *	copies of this source code.  The copyright above does not
 *	evidence any actual or intended publication of this source
 *	code.
 *
 *	This is unpublished proprietary trade secret source code of
 *	Lachman Associates.  This source code may not be copied,
 *	disclosed, distributed, demonstrated or licensed except as
 *	expressly authorized by Lachman Associates.
 *
 *	System V STREAMS TCP was jointly developed by Lachman
 *	Associates and Convergent Technologies.
 */
#include <fcntl.h>
#include <sys/types.h>
#include <stropts.h>
#include <sys/stream.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/sockio.h>
#include <net/strioc.h>
/* #ifdef uts */
#include <sys/dlpi.h>
/* #endif */
#include "defs.h"


extern int	pflag;
extern int	uflag;
struct val      val_none = {V_NONE};

/*
 * num - convert string to integer.  Returns 1 if ok, 0 otherwise. Result is
 * stored in *res.
 */
int
num(str, res)
	char           *str;
	int            *res;
{
	int             val;
	char           *p;

	val = strtol(str, &p, 10);
	if (*p || p == str)
		return 0;
	else {
		*res = val;
		return 1;
	}
}

struct val     *
Open(fi, c, argc, argv)
	struct finst   *fi;
	struct cmd     *c;
	int             argc;
	struct val     *argv;
{
	static struct val rval = {V_FD};

	if ((rval.u.val = open(argv[0].u.sval, O_RDWR)) < 0)
		xerr(fi, c, E_SYS, "open \"%s\"", argv[0].u.sval);
	return &rval;
}

static int      open_argtypes[] = {V_STR};
static struct bfunc open_info = {
	Open, 1, 1, open_argtypes
};

struct val     *
Link(fi, c, argc, argv)
	struct finst   *fi;
	struct cmd     *c;
	int             argc;
	struct val     *argv;
{
	static struct val rval = {V_MUXID};

	if (!uflag) {
		if ((rval.u.val = ioctl(argv[0].u.val,
		(pflag ? I_PLINK : I_LINK), argv[1].u.val)) < 0)
			xerr(fi, c, E_SYS, "link");
	} else {
		/*
		 * unlink
		 * ignore return -- since the multiplexor id is lost,
		 * we can only do MUXID_ALL, and we may do it multiple
		 * times for one driver.
		 */
		(void) ioctl(argv[0].u.val, I_PUNLINK, MUXID_ALL);
		rval.u.val = 0;
	}
	close(argv[1].u.val);
	return &rval;
}

static int      link_argtypes[] = {V_FD, V_FD};
static struct bfunc link_info = {
	Link, 2, 2, link_argtypes
};

struct val     *
Push(fi, c, argc, argv)
	struct finst   *fi;
	struct cmd     *c;
	int             argc;
	struct val     *argv;
{
	if (uflag)
		return &val_none;
	if (ioctl(argv[0].u.val, I_PUSH, argv[1].u.sval) < 0)
		xerr(fi, c, E_SYS, "push \"%s\"", argv[1].u.sval);
	return &val_none;
}

static int      push_argtypes[] = {V_FD, V_STR};
static struct bfunc push_info = {
	Push, 2, 2, push_argtypes
};

struct val     *
Sifname(fi, c, argc, argv)
	struct finst   *fi;
	struct cmd     *c;
	int             argc;
	struct val     *argv;
{
	struct strioctl iocb;
	struct ifreq    ifr;

	if (uflag)
		return &val_none;
	strcpy(ifr.ifr_name, argv[2].u.sval);
	ifr.ifr_metric = argv[1].u.val;
	iocb.ic_cmd = SIOCSIFNAME;
	iocb.ic_timout = 15;
	iocb.ic_len = sizeof(ifr);
	iocb.ic_dp = (char *) &ifr;
	if (ioctl(argv[0].u.val, I_STR, &iocb) < 0)
		xerr(fi, c, E_SYS, "sifname");
	return &val_none;
}

static int      sifname_argtypes[] = {V_FD, V_MUXID, V_STR};
static struct bfunc sifname_info = {
	Sifname, 3, 3, sifname_argtypes
};

struct val     *
Unitsel(fi, c, argc, argv)
	struct finst   *fi;
	struct cmd     *c;
	int             argc;
	struct val     *argv;
{
	struct strioctl iocb;
	int             unit;

	if (uflag)
		return &val_none;
	if (!(num(argv[1].u.sval, &unit)))
		xerr(fi, c, 0, "unitsel: bad unit number specification");
	iocb.ic_cmd = IF_UNITSEL;
	iocb.ic_timout = 0;
	iocb.ic_len = sizeof(int);
	iocb.ic_dp = (char *) &unit;
	if (ioctl(argv[0].u.val, I_STR, &iocb) < 0)
		xerr(fi, c, E_SYS, "unitsel");
	return &val_none;
}

static int      unitsel_argtypes[] = {V_FD, V_STR};
static struct bfunc unitsel_info = {
	Unitsel, 2, 2, unitsel_argtypes
};

struct val     *
Initqp(fi, c, argc, argv)
	struct finst   *fi;
	struct cmd     *c;
	int             argc;
	struct val     *argv;
{
	/* the order of these must agree with the IQP_XX defines */
	static char    *qname[] = {"rq", "wq", "hdrq", "muxrq", "muxwq"};
	static int      vtval[] = {IQP_LOWAT, IQP_HIWAT};
	static char    *vname[] = {"lowat", "hiwat"};
	struct iocqp    iocqp[IQP_NQTYPES * IQP_NVTYPES];
	int             niocqp;
	char           *dev;
	int             fd;
	struct strioctl iocb;
	int             i, qtype, vtype, val;

	if (uflag)
		return &val_none;
	dev = argv[0].u.sval;
	niocqp = 0;
	for (i = 1; i < argc;) {
		for (qtype = 0; qtype < IQP_NQTYPES; qtype++) {
			if (strcmp(argv[i].u.sval, qname[qtype]) == 0)
				break;
		}
		if (qtype == IQP_NQTYPES) {
			xerr(fi, c, 0, "initqp: bad queue type \"%s\"",
			     argv[i].u.sval);
		}
		i++;
		if (i + IQP_NVTYPES > argc) {
			xerr(fi, c, 0, "initqp: incomplete specification for %s\n",
			     qname[qtype]);
		}
		for (vtype = 0; vtype < IQP_NVTYPES; vtype++, i++) {
			if (num(argv[i].u.sval, &val)) {
				if (val < 0 || val > 65535) {
					xerr(fi, c, 0, "initqp: %s %s out of range",
					     qname[qtype], vname[vtype]);
				}
				iocqp[niocqp].iqp_type = qtype | vtval[vtype];
				iocqp[niocqp++].iqp_value = val;
			} else if (strcmp(argv[i].u.sval, "-")) {
				xerr(fi, c, 0, "initqp: illegal value for %s %s",
				     qname[qtype], vname[vtype]);
			}
		}
	}
	if ((fd = open(dev, O_RDWR)) < 0)
		xerr(fi, c, E_SYS, "initqp: open \"%s\"", dev);
	iocb.ic_cmd = INITQPARMS;
	iocb.ic_timout = 0;
	iocb.ic_len = niocqp * sizeof(struct iocqp);
	iocb.ic_dp = (char *) iocqp;
	if (ioctl(fd, I_STR, &iocb) < 0)
		xerr(fi, c, E_SYS, "initqp: ioctl INITQPARMS");
	close(fd);
	return &val_none;
}

static int      initqp_argtypes[] = {
	V_STR, V_STR, V_STR, V_STR, V_STR, V_STR, V_STR, V_STR,
	V_STR, V_STR, V_STR, V_STR, V_STR, V_STR, V_STR, V_STR
};
static struct bfunc initqp_info = {
	Initqp, 4, 16, initqp_argtypes
};

/* #ifdef uts */
struct val     *
Dlattach(fi, c, argc, argv)
	struct finst   *fi;
	struct cmd     *c;
	int             argc;
	struct val     *argv;
{
	struct strbuf   ctlbuf;
	dl_attach_req_t att_req;
	dl_error_ack_t *error_ack;
	union DL_primitives	dl_prim;
	int             flags = 0;
	int             fd, unit;

	if (uflag)
		return &val_none;
	if (!(num(argv[1].u.sval, &unit)))
		xerr(fi, c, 0, "unitsel: bad unit number specification");
	fd = argv[0].u.val;
	att_req.dl_primitive = DL_ATTACH_REQ;
	att_req.dl_ppa = unit;
	ctlbuf.len = sizeof(dl_attach_req_t);
	ctlbuf.buf = (char *) &att_req;
	if (putmsg(fd, &ctlbuf, NULL, 0) < 0)
		xerr(fi, c, E_SYS, "dlattach: putmsg");
	ctlbuf.maxlen = sizeof(union DL_primitives);
	ctlbuf.len = 0;
	ctlbuf.buf = (char *) &dl_prim;
	if (getmsg(fd, &ctlbuf, NULL, &flags) < 0)
		xerr(fi, c, E_SYS, "dlattach: getmsg");
	switch (dl_prim.dl_primitive) {
	case DL_OK_ACK:
		if (ctlbuf.len < sizeof(dl_ok_ack_t) ||
		    ((dl_ok_ack_t *) & dl_prim)->dl_correct_primitive
		    != DL_ATTACH_REQ)
			xerr(fi, c, 0, "dlattach: protocol error");
		else
			return &val_none;

	case DL_ERROR_ACK:
		if (ctlbuf.len < sizeof(dl_error_ack_t))
			xerr(fi, c, 0, "dlattach: protocol error");
		else {
			error_ack = (dl_error_ack_t *) & dl_prim;
			switch (error_ack->dl_errno) {
			case DL_BADPPA:
				xerr(fi, c, 0, "dlattach: bad PPA");

			case DL_ACCESS:
				xerr(fi, c, 0, "dlattach: access error");

			case DL_SYSERR:
				xerr(fi, c, 0, "dlattach: system error %d",
				     error_ack->dl_unix_errno);

			default:
				xerr(fi, c, 0, "dlattach: protocol error");
			}
		}

	default:
		xerr(fi, c, 0, "dlattach: protocol error");
	}
	/* NOTREACHED */
}

static int      dlattach_argtypes[] = {V_FD, V_STR};
static struct bfunc dlattach_info = {
	Dlattach, 2, 2, dlattach_argtypes
};
/* #endif */

struct val     *
Strcat(fi, c, argc, argv)
	struct finst   *fi;
	struct cmd     *c;
	int             argc;
	struct val     *argv;
{
	static struct val rval = {V_STR};
	int             len;
	char           *newstr;

	len = strlen(argv[0].u.sval) + strlen(argv[1].u.sval) + 1;
	newstr = xmalloc(len);
	strcpy(newstr, argv[0].u.sval);
	strcat(newstr, argv[1].u.sval);
	rval.u.sval = newstr;
	return &rval;
}

static int      strcat_argtypes[] = {V_STR, V_STR};
static struct bfunc strcat_info = {
	Strcat, 2, 2, strcat_argtypes
};

binit()
{
	deffunc("return", F_RETURN);
	deffunc("open", F_BUILTIN, &open_info);
	deffunc("link", F_BUILTIN, &link_info);
	deffunc("push", F_BUILTIN, &push_info);
	deffunc("sifname", F_BUILTIN, &sifname_info);
	deffunc("unitsel", F_BUILTIN, &unitsel_info);
/* #ifdef uts */
	deffunc("dlattach", F_BUILTIN, &dlattach_info);
/* #endif */
	deffunc("strcat", F_BUILTIN, &strcat_info);
	deffunc("initqp", F_BUILTIN, &initqp_info);
}
