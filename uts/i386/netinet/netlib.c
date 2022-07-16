/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:netlib.c	1.3"

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
 * System V STREAMS TCP - Release 3.0 
 *
 * Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI) 
 * All Rights Reserved. 
 *
 * The copyright above and this notice must be preserved in all copies of this
 * source code.  The copyright above does not evidence any actual or intended
 * publication of this source code. 
 *
 * This is unpublished proprietary trade secret source code of Lachman
 * Associates.  This source code may not be copied, disclosed, distributed,
 * demonstrated or licensed except as expressly authorized by Lachman
 * Associates. 
 *
 * System V STREAMS TCP was jointly developed by Lachman Associates and
 * Convergent Technologies. 
 */

#define STRNET

#ifdef INET
#include <netinet/symredef.h>
#endif INET

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stream.h>
#include <sys/errno.h>
#include <sys/sysmacros.h>
#include <sys/debug.h>
#include <sys/signal.h>
#ifdef SYSV
#include <sys/cred.h>
#endif /* SYSV */
#include <sys/proc.h>
#include <sys/user.h>
#ifdef SYSV
#include <sys/tihdr.h>
#include <sys/tiuser.h>
#else
#include <nettli/tihdr.h>
#include <nettli/tiuser.h>
#endif SYSV
#include <sys/socket.h>
#include <net/strioc.h>
#include <sys/socketvar.h>
#include <netinet/in.h>
#include <net/route.h>
#include <netinet/in_pcb.h>

#if  defined(vax)
u_long
htonl(hl)
	u_long          hl;
{
	char            nl[4];

	nl[0] = ((char *) &hl)[3];
	nl[1] = ((char *) &hl)[2];
	nl[2] = ((char *) &hl)[1];
	nl[3] = ((char *) &hl)[0];
	return (*(u_long *) nl);
}

u_short
htons(hs)
	u_short         hs;
{
	char            ns[2];

	ns[0] = ((char *) &hs)[1];
	ns[1] = ((char *) &hs)[0];
	return (*(u_short *) ns);
}

u_long
ntohl(nl)
	u_long          nl;
{
	char            hl[4];

	hl[0] = ((char *) &nl)[3];
	hl[1] = ((char *) &nl)[2];
	hl[2] = ((char *) &nl)[1];
	hl[3] = ((char *) &nl)[0];
	return (*(u_long *) hl);
}

u_short
ntohs(ns)
	u_short         ns;
{
	char            hs[2];

	hs[0] = ((char *) &ns)[1];
	hs[1] = ((char *) &ns)[0];
	return (*(u_short *) hs);
}
#endif
initqparms(bp, minfo, infosz)
	mblk_t         *bp;
	struct module_info *minfo;
	int             infosz;
{
	register struct iocqp *iqpp;
	int             i;

	if (((struct iocblk *) bp->b_rptr)->ioc_uid != 0)
		return (EPERM);
	for (bp = bp->b_cont; bp; bp = bp->b_cont) {
		for (iqpp = (struct iocqp *) bp->b_rptr;
		     (unsigned char *) iqpp < bp->b_wptr;
		     iqpp++) {
			if ((i = iqpp->iqp_type & IQP_QTYPMASK) < infosz)
				switch (iqpp->iqp_type & IQP_VTYPMASK) {

				case IQP_LOWAT:
					minfo[i].mi_lowat = iqpp->iqp_value;
					break;

				case IQP_HIWAT:
					minfo[i].mi_hiwat = iqpp->iqp_value;
					break;

				default:
					return (EINVAL);
				}
		}
	}
	return (0);
}

mblk_t         *reallocb();
#define CHECKSIZE(bp,size) if (((bp) = reallocb((bp), (size),0)) == NULL) {\
			return;\
			}
/*
 * A common subroutine for positive acknowledgement to user initiated
 * transport events. Performance-wise, it could be worthwhile to make this a
 * macro at some point.
 */

T_okack(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	struct T_ok_ack *ack;
	int             prim;

	prim = ((union T_primitives *) bp->b_rptr)->type;

	CHECKSIZE(bp, sizeof(struct T_ok_ack));
	freemsg(bp->b_cont);
	bp->b_cont = NULL;
	bp->b_rptr = bp->b_datap->db_base;
	bp->b_datap->db_type = M_PCPROTO;
	ack = (struct T_ok_ack *) bp->b_rptr;
	bp->b_wptr = bp->b_rptr + sizeof(struct T_ok_ack);
	ack->CORRECT_prim = prim;
	ack->PRIM_type = T_OK_ACK;
	qreply(q, bp);
}

/*
 * This subroutine returns tranport errors found during the processing of
 * requests from user level.
 */

T_errorack(q, bp, terr, serr)
	queue_t        *q;
	mblk_t         *bp;
	int             terr, serr;
{
	struct T_error_ack *tea;
	int             prim;

	prim = ((union T_primitives *) bp->b_rptr)->type;

	CHECKSIZE(bp, sizeof(struct T_error_ack));

	if (bp->b_cont)
		freemsg(bp->b_cont);
	bp->b_cont = NULL;
	bp->b_rptr = bp->b_datap->db_base;
	tea = (struct T_error_ack *) bp->b_rptr;
	bp->b_wptr = bp->b_rptr + sizeof(struct T_error_ack);
	bp->b_datap->db_type = M_PCPROTO;
	tea->ERROR_prim = prim;
	tea->PRIM_type = T_ERROR_ACK;
	tea->TLI_error = terr;
	tea->UNIX_error = serr;
	qreply(q, bp);
	return;
}

/*
 * check to see if data block is big enough.  If it isn't, allocate one that
 * is and free it.
 */

mblk_t         *
reallocb(bp, size, copy)
	mblk_t         *bp;
	int             size, copy;
{
	mblk_t         *newbp;

	if ((bp->b_datap->db_lim - bp->b_datap->db_base) >= size) {
		return (bp);
	}
	newbp = allocb(size, BPRI_HI);
	if (newbp == NULL) {
		freeb(bp);
		return (NULL);
	}
	if (copy) {
		bcopy((caddr_t) bp->b_rptr, (caddr_t) newbp->b_rptr,
		      (unsigned) msgblen(bp));
		newbp->b_wptr += msgblen(bp);
	}
	newbp->b_cont = bp->b_cont;
	newbp->b_datap->db_type = bp->b_datap->db_type;
	freeb(bp);
	return (newbp);
}

T_conn_con(inp)
	struct inpcb   *inp;
{
	mblk_t         *bp;
	struct sockaddr_in *sin;
	struct T_conn_con *conn_con;

	if (!(bp = allocb(sizeof(struct T_conn_con) + inp->inp_addrlen,
			  BPRI_HI))) {
		bufcall(sizeof(struct T_conn_con) + inp->inp_addrlen,
			BPRI_HI, T_conn_con, inp);
		return;
	}
	bp->b_wptr += sizeof(struct T_conn_con) + inp->inp_addrlen;
	bp->b_datap->db_type = M_PROTO;
	conn_con = (struct T_conn_con *) bp->b_rptr;
	sin = (struct sockaddr_in *) (bp->b_rptr + sizeof(struct T_conn_con));
	conn_con->PRIM_type = T_CONN_CON;
	conn_con->RES_length = inp->inp_addrlen;
	conn_con->RES_offset = sizeof(struct T_conn_con);
	conn_con->OPT_length = 0;
	conn_con->OPT_offset = 0;
	bzero((caddr_t) sin, inp->inp_addrlen);
	sin->sin_family = inp->inp_family;
	sin->sin_addr = inp->inp_faddr;
	sin->sin_port = inp->inp_fport;
	inp->inp_state &= ~(SS_ISCONNECTING | SS_ISDISCONNECTING);
	inp->inp_state |= SS_ISCONNECTED;
	putnext(inp->inp_q, bp);
	return;
}

setuerror(errno)
	unsigned short  errno;
{
	u.u_error = errno;
}

/*
 * convert integer to ascii hex
 */
itox(val, buf)
	int             val;
	char           *buf;
{
	int             shift;
	static char     hexdig[] = "0123456789abcdef";

	for (shift = 28; shift >= 4; shift -= 4)
		if ((val >> shift) & 0xf)
			break;
	for (; shift >= 0; shift -= 4)
		*buf++ = hexdig[(val >> shift) & 0xf];
	*buf = '\0';
}

#ifdef STRINGS
/*
 * Strings functions being include for the sake of building without RFS
 * streams. The code is taken from the System V libc strings routines. This
 * is a kludge... it would be better to have string functions in a separate
 * library to save a few butes.
 */

/*
 * Compare strings:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */

int
in_strcmp(s1, s2)
	register char  *s1, *s2;
{

	if (s1 == s2)
		return (0);
	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return (0);
	return (*s1 - *--s2);
}


/*
 * Copy string s2 to s1.  s1 must be large enough. return s1
 */

char           *
in_strcpy(s1, s2)
	register char  *s1, *s2;
{
	register char  *os1;

	os1 = s1;
	while (*s1++ = *s2++);
	return (os1);
}

/*
 * Copy s2 to s1, truncating or null-padding to always copy n bytes return s1
 */

char           *
in_strncpy(s1, s2, n)
	register char  *s1, *s2;
	register int    n;
{
	register char  *os1 = s1;

	n++;
	while ((--n > 0) && ((*s1++ = *s2++) != '\0'));
	if (n > 0)
		while (--n > 0)
			*s1++ = '\0';
	return (os1);
}

/*
 * Returns the number of non-NULL bytes in string argument.
 */

int
in_strlen(s)
	register char  *s;
{
	register char  *s0 = s + 1;

	while (*s++ != '\0');
	return (s - s0);
}


/*
 * Compare strings (at most n bytes) returns: s1>s2; >0  s1==s2; 0  s1<s2; <0
 */

int
in_strncmp(s1, s2, n)
	register char  *s1, *s2;
	register int    n;
{
	n++;
	if (s1 == s2)
		return (0);
	while (--n > 0 && *s1 == *s2++)
		if (*s1++ == '\0')
			return (0);
	return ((n == 0) ? 0 : (*s1 - *--s2));
}
#endif				/* STRINGS */

/* routines used for options processing */

/*
 * makeopt - format option in message The specified option & value are copied
 * into the last mblk at b_wptr.  b_wptr is incremented accordingly. If there
 * is not enough space in the last mblk, a new mblk is allocated.  Returns 1
 * if ok, 0 if unable to allocate an mblk.
 */
int
makeopt(bp, level, name, ptr, len)
	mblk_t         *bp;
	int             level, name, len;
	char           *ptr;
{
	struct opthdr  *opt;
	int             rlen, tlen;

	for (; bp->b_cont; bp = bp->b_cont);
	rlen = OPTLEN(len);
	tlen = sizeof(struct opthdr) + rlen;
	if ((bp->b_datap->db_lim - bp->b_wptr) < tlen) {
		if (!(bp->b_cont = allocb(max(tlen, 64), BPRI_MED)))
			return 0;
		bp = bp->b_cont;
		bp->b_datap->db_type = M_PROTO;
	}
	opt = (struct opthdr *) bp->b_wptr;
	opt->level = level;
	opt->name = name;
	opt->len = rlen;
	if (rlen)
		bzero(OPTVAL(opt), rlen);
	if (len)
		bcopy(ptr, OPTVAL(opt), len);
	bp->b_wptr += tlen;
	return 1;
}

/*
 * dooptions - do options processing this function processes a T_OPTMGMT_REQ.
 * funcs points to a list of options processing functions.
 */
dooptions(q, bp, funcs)
	queue_t        *q;
	mblk_t         *bp;
	struct opproc  *funcs;
{
	struct T_optmgmt_req *req = (struct T_optmgmt_req *) bp->b_rptr;
	int             flags;
	int             error = 0;
	struct opproc  *f;
	struct opthdr  *opt, *nopt, *eopt;
	int             level;
	mblk_t         *mp = NULL;

	switch (flags = req->MGMT_flags) {

	case T_CHECK:
	case T_NEGOTIATE:
		if (!(mp = allocb(64, BPRI_MED))) {
			error = -ENOSR;
			goto done;
		}
		mp->b_datap->db_type = M_PROTO;
		opt = (struct opthdr *) (bp->b_rptr + req->OPT_offset);
		eopt = (struct opthdr *) ((char *) opt + req->OPT_length);
		if ((char *) eopt > (char *) bp->b_wptr) {
			error = -EINVAL;
			goto done;
		}
		do {
			nopt = (struct opthdr *) ((char *) (opt + 1) + opt->len);
			if (nopt > eopt) {
				error = TBADOPT;
				goto done;
			}
			level = opt->level;
			for (f = funcs; f->func; f++) {
				if (f->level == level) {
					if (error = (*f->func) (q, req, opt, mp))
						goto done;
					break;
				}
			}
			if (!f->func) {
				if (flags == T_CHECK)
					req->MGMT_flags = T_FAILURE;
				else
					error = TBADOPT;
				goto done;
			}
			if (flags == T_CHECK && req->MGMT_flags == T_FAILURE)
				goto done;
		} while ((opt = nopt) < eopt);
		if (flags == T_CHECK)
			req->MGMT_flags = T_SUCCESS;
		break;

	case T_DEFAULT:
		if (!(mp = allocb(256, BPRI_MED))) {
			error = -ENOSR;
			break;
		}
		mp->b_datap->db_type = M_PROTO;
		for (f = funcs; f->func; f++) {
			if (error = (*f->func) (q, req, 0, mp))
				break;
		}
		break;

	default:
		error = TBADFLAG;
		break;
	}

done:
	if (error && mp)
		freemsg(mp);
	if (error < 0)
		T_errorack(q, bp, TSYSERR, -error);
	else if (error > 0)
		T_errorack(q, bp, error, 0);
	else {
		int             size = 0;
		mblk_t         *mp1;

		for (mp1 = mp; mp1; mp1 = mp1->b_cont)
			size += mp1->b_wptr - mp1->b_rptr;
		req->PRIM_type = T_OPTMGMT_ACK;
		req->OPT_offset = sizeof(struct T_optmgmt_ack);
		req->OPT_length = size;
		bp->b_wptr = bp->b_rptr + sizeof(struct T_optmgmt_ack);
		if (bp->b_cont)
			freemsg(bp->b_cont);
		bp->b_cont = mp;
		/*
		 * A pullupmsg is necessary because timod is stupid (it only
		 * considers the first block of the message)
		 */
		pullupmsg(bp, -1);
		qreply(q, bp);
	}
}

/*
 * Overlapped bcopy.
 *
 * Copy `from' to `to' taking into account the operands may represent
 * overlapping buffers.  If `from' is less then `to', we can safely
 * copy from right to left.  If `from' is greater than `to', then we
 * can safely copy from left to right.
 */
in_ovbcopy(from, to, len)
	char *from, *to;
	int len;
{
	register char *fp, *tp;

	if (from < to) {	/* copy right to left */
		fp = from + len;
		tp = to + len;
		while (len-- > 0)
			*--tp = *--fp;
	} else {		/* copy left to right */
		fp = from;
		tp = to;
		while (len-- > 0)
			*tp++ = *fp++;
	}
}
