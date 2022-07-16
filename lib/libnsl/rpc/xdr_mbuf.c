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


#ident	"@(#)librpc:xdr_mbuf.c	1.2.2.1"

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
#ifdef	KERNEL
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)xdr_mbuf.c 1.29 89/01/31 Copyr 1984 Sun Micro";
#endif

/*
 * xdr_mbuf.c, XDR implementation on kernel mbufs.
 *
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <sys/uio.h>
#include <sys/byteorder.h>

bool_t	xdrmbuf_getlong(), xdrmbuf_putlong();
bool_t	xdrmbuf_getbytes(), xdrmbuf_putbytes();
u_int	xdrmbuf_getpos();
bool_t	xdrmbuf_setpos();
long *	xdrmbuf_inline();
void	xdrmbuf_destroy();

/*
 * Xdr on mbufs operations vector.
 */
struct	xdr_ops xdrmbuf_ops = {
	xdrmbuf_getlong,
	xdrmbuf_putlong,
	xdrmbuf_getbytes,
	xdrmbuf_putbytes,
	xdrmbuf_getpos,
	xdrmbuf_setpos,
	xdrmbuf_inline,
	xdrmbuf_destroy
};

/*
 * Initailize xdr stream.
 */
void
xdrmbuf_init(xdrs, m, op)
	register XDR		*xdrs;
	register struct mbuf	*m;
	enum xdr_op		 op;
{

	xdrs->x_op = op;
	xdrs->x_ops = &xdrmbuf_ops;
	xdrs->x_base = (caddr_t)m;
	xdrs->x_private = mtod(m, caddr_t);
	xdrs->x_public = (caddr_t)0;
	xdrs->x_handy = m->m_len;
}

void
/* ARGSUSED */
xdrmbuf_destroy(xdrs)
	XDR	*xdrs;
{
	/* do nothing */
}

bool_t
xdrmbuf_getlong(xdrs, lp)
	register XDR	*xdrs;
	long		*lp;
{

	if ((xdrs->x_handy -= sizeof(long)) < 0) {
		if (xdrs->x_handy != -sizeof(long))
			printf("xdr_mbuf: long crosses mbufs!\n");
		if (xdrs->x_base) {
			register struct mbuf *m =
			    ((struct mbuf *)xdrs->x_base)->m_next;

			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_private = mtod(m, caddr_t);
			xdrs->x_handy = m->m_len - sizeof(long);
		} else {
			return (FALSE);
		}
	}
	*lp = ntohl(*((long *)(xdrs->x_private)));
	xdrs->x_private += sizeof(long);
	return (TRUE);
}

bool_t
xdrmbuf_putlong(xdrs, lp)
	register XDR	*xdrs;
	long		*lp;
{

	if ((xdrs->x_handy -= sizeof(long)) < 0) {
		if (xdrs->x_handy != -sizeof(long))
			printf("xdr_mbuf: putlong, long crosses mbufs!\n");
		if (xdrs->x_base) {
			register struct mbuf *m =
			    ((struct mbuf *)xdrs->x_base)->m_next;

			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_private = mtod(m, caddr_t);
			xdrs->x_handy = m->m_len - sizeof(long);
		} else {
			return (FALSE);
		}
	}
	*(long *)xdrs->x_private = htonl(*lp);
	xdrs->x_private += sizeof(long);
	return (TRUE);
}

bool_t
xdrmbuf_getbytes(xdrs, addr, len)
	register XDR	*xdrs;
	caddr_t		 addr;
	register u_int	 len;
{

	while ((xdrs->x_handy -= len) < 0) {
		if ((xdrs->x_handy += len) > 0) {
			(void) memcpy(addr, xdrs->x_private, (u_int)xdrs->x_handy);
			addr += xdrs->x_handy;
			len -= xdrs->x_handy;
		}
		if (xdrs->x_base) {
			register struct mbuf *m =
			    ((struct mbuf *)xdrs->x_base)->m_next;

			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_private = mtod(m, caddr_t);
			xdrs->x_handy = m->m_len;
		} else {
			return (FALSE);
		}
	}
	(void) memcpy(addr, xdrs->x_private, (u_int)len);
	xdrs->x_private += len;
	return (TRUE);
}

/*
 * Sort of like getbytes except that instead of getting
 * bytes we return the mbuf that contains all the rest
 * of the bytes.
 */
bool_t
xdrmbuf_getmbuf(xdrs, mm, lenp)
	register XDR	*xdrs;
	register struct mbuf **mm;
	register u_int *lenp;
{
	register struct mbuf *m;
	register int len, used;

	if (! xdr_u_int(xdrs, lenp)) {
		return (FALSE);
	}
	m = (struct mbuf *)xdrs->x_base;
	used = m->m_len - xdrs->x_handy;
	m->m_off += used;
	m->m_len -= used;
	*mm = m;
	/*
	 * Consistency check.
	 */
	len = 0;
	while (m) {
		len += m->m_len;
		m = m->m_next;
	}
	if (len < *lenp) {
		printf("xdrmbuf_getmbuf failed\n");
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdrmbuf_putbytes(xdrs, addr, len)
	register XDR	*xdrs;
	caddr_t		 addr;
	register u_int		 len;
{

	while ((xdrs->x_handy -= len) < 0) {
		if ((xdrs->x_handy += len) > 0) {
			(void) memcpy(xdrs->x_private, addr, (u_int)xdrs->x_handy);
			addr += xdrs->x_handy;
			len -= xdrs->x_handy;
		}
		if (xdrs->x_base) {
			register struct mbuf *m =
			    ((struct mbuf *)xdrs->x_base)->m_next;

			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_private = mtod(m, caddr_t);
			xdrs->x_handy = m->m_len;
		} else {
			return (FALSE);
		}
	}
	(void) memcpy(xdrs->x_private, addr, len);
	xdrs->x_private += len;
	return (TRUE);
}

/*
 * Like putbytes, only we avoid the copy by pointing a type 2
 * mbuf at the buffer.  Not safe if the buffer goes away before
 * the mbuf chain is deallocated.
 */
bool_t
xdrmbuf_putbuf(xdrs, addr, len, func, arg)
	register XDR	*xdrs;
	caddr_t		 addr;
	u_int		 len;
	int		 (*func)();
	int		 arg;
{
	register struct mbuf *m;
	struct mbuf *mclgetx();
	long llen = len;

	if (len & 3) {  /* can't handle roundup problems */
		return (FALSE);
	}
	if (! xdrmbuf_putlong(xdrs, &llen)) {
		return(FALSE);
	}
	((struct mbuf *)xdrs->x_base)->m_len -= xdrs->x_handy;
	m = mclgetx(func, arg, addr, (int)len, M_WAIT);
	if (m == NULL) {
		printf("xdrmbuf_putbuf: mclgetx failed\n");
		return (FALSE);
	}
	((struct mbuf *)xdrs->x_base)->m_next = m;
	xdrs->x_handy = 0;
	return (TRUE);
}

u_int
xdrmbuf_getpos(xdrs)
	register XDR	*xdrs;
{

	return (
	    (u_int)xdrs->x_private - mtod(((struct mbuf *)xdrs->x_base), u_int));
}

bool_t
xdrmbuf_setpos(xdrs, pos)
	register XDR	*xdrs;
	u_int		 pos;
{
	register caddr_t	newaddr =
	    mtod(((struct mbuf *)xdrs->x_base), caddr_t) + pos;
	register caddr_t	lastaddr =
	    xdrs->x_private + xdrs->x_handy;

	if ((int)newaddr > (int)lastaddr)
		return (FALSE);
	xdrs->x_private = newaddr;
	xdrs->x_handy = (int)lastaddr - (int)newaddr;
	return (TRUE);
}

long *
xdrmbuf_inline(xdrs, len)
	register XDR	*xdrs;
	int		 len;
{
	long *buf = 0;

	if (xdrs->x_handy >= len) {
		xdrs->x_handy -= len;
		buf = (long *) xdrs->x_private;
		xdrs->x_private += len;
	}
	return (buf);
}
#endif	/* KERNEL */
