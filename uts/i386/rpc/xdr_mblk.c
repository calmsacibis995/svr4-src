/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:xdr_mblk.c	1.3"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)xdr_mblk.c 1.3 89/01/11 SMI"
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

#ifdef _KERNEL
/*
 * xdr_mblk.c, XDR implementation on kernel streams mblks.
 *
 * 
 *
 */

#include <sys/param.h>
#include <sys/types.h>
#include <rpc/types.h>
#include <sys/stream.h>
#include <rpc/xdr.h>
#include <netinet/in.h>

bool_t	xdrmblk_getlong(), xdrmblk_putlong();
bool_t	xdrmblk_getbytes(), xdrmblk_putbytes();
u_int	xdrmblk_getpos();
bool_t	xdrmblk_setpos();
long *	xdrmblk_inline();
void	xdrmblk_destroy();

/*
 * Xdr on mblks operations vector.
 */
struct	xdr_ops xdrmblk_ops = {
	xdrmblk_getlong,
	xdrmblk_putlong,
	xdrmblk_getbytes,
	xdrmblk_putbytes,
	xdrmblk_getpos,
	xdrmblk_setpos,
	xdrmblk_inline,
	xdrmblk_destroy
};

/*
 * Initialize xdr stream.
 */
void
xdrmblk_init(xdrs, m, op)
	register XDR		*xdrs;
	register mblk_t		*m;
	enum xdr_op		 op;
{
	xdrs->x_op = op;
	xdrs->x_ops = &xdrmblk_ops;
	xdrs->x_base = (caddr_t)m;
	xdrs->x_public = (caddr_t)0;

	if (op == XDR_DECODE) {
		/* mblk has data, we want to
		 * read it.
		 */
		xdrs->x_private = (char *)m->b_rptr;
		xdrs->x_handy = m->b_wptr - m->b_rptr;
	}
	else	{
		/* mblk is for writing into.
		 */
		xdrs->x_private = (char *)m->b_wptr;
		xdrs->x_handy = m->b_datap->db_lim - m->b_datap->db_base;
	}
}

void
/* ARGSUSED */
xdrmblk_destroy(xdrs)
	XDR	*xdrs;
{
	/* do nothing */
}

bool_t
xdrmblk_getlong(xdrs, lp)
	register XDR	*xdrs;
	long		*lp;
{
	if ((xdrs->x_handy -= sizeof(long)) < 0) {
		if (xdrs->x_handy != -sizeof(long))
			printf("xdr_mblk: long crosses mblks!\n");
		if (xdrs->x_base) {
			/* LINTED pointer alignment */
			register mblk_t *m = ((mblk_t *)xdrs->x_base)->b_cont;

			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_private = (char *)m->b_rptr;
			xdrs->x_handy = m->b_wptr-m->b_rptr - sizeof(long);
		} else {
			return (FALSE);
		}
	}
	/* LINTED pointer alignment */
	*lp = ntohl(*((long *)(xdrs->x_private)));
	xdrs->x_private += sizeof(long);
	return (TRUE);
}

bool_t
xdrmblk_putlong(xdrs, lp)
	register XDR	*xdrs;
	long		*lp;
{
#ifdef XDRDEBUG
printf("xdrmblk_putlong: Entered %x\n", xdrs->x_base);
#endif

	if ((xdrs->x_handy -= sizeof(long)) < 0) {
		if (xdrs->x_handy != -sizeof(long))
			printf("xdr_mblk: putlong, long crosses mblks!\n");
		if (xdrs->x_base) {
			/* LINTED pointer alignment */
			register mblk_t *m = ((mblk_t *)xdrs->x_base)->b_cont;

			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_private = (char *)m->b_wptr;
			xdrs->x_handy = m->b_datap->db_lim - m->b_datap->db_base;
		} else {
			return (FALSE);
		}
	}
	/* LINTED pointer alignment */
	*(long *)xdrs->x_private = htonl(*lp);
	xdrs->x_private += sizeof(long);
	return (TRUE);
}

bool_t
xdrmblk_getbytes(xdrs, addr, len)
	register XDR	*xdrs;
	caddr_t		 addr;
	register u_int	 len;
{
#ifdef XDRDEBUG
printf("xdrmblk_getbytes: Entered %x\n", xdrs->x_base);
#endif

	while ((xdrs->x_handy -= len) < 0) {
		if ((xdrs->x_handy += len) > 0) {
			bcopy(xdrs->x_private, addr, (u_int)xdrs->x_handy);
			addr += xdrs->x_handy;
			len -= xdrs->x_handy;
		}
		if (xdrs->x_base) {
			/* LINTED pointer alignment */
			register mblk_t *m = ((mblk_t *)xdrs->x_base)->b_cont;

			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_private = (char *)m->b_rptr;
			xdrs->x_handy = m->b_wptr-m->b_rptr;
		} else {
			return (FALSE);
		}
	}
	bcopy(xdrs->x_private, addr, (u_int)len);
	xdrs->x_private += len;
	return (TRUE);
}

/*
 * Sort of like getbytes except that instead of getting
 * bytes we return the mblk that contains all the rest
 * of the bytes.
 */
bool_t
xdrmblk_getmblk(xdrs, mm, lenp)
	register XDR	*xdrs;
	register mblk_t **mm;
	register u_int *lenp;
{
	register mblk_t *m;
	register int len, used;
#ifdef XDRDEBUG
printf("xdrmblk_getmblk: Entered %x\n", xdrs->x_base);
#endif

	if (! xdr_u_int(xdrs, lenp)) {
		return (FALSE);
	}
	/* LINTED pointer alignment */
	m = (mblk_t *)xdrs->x_base;
	used = m->b_wptr-m->b_rptr - xdrs->x_handy;
	m->b_rptr += used;
	*mm = m;
	/*
	 * Consistency check.
	 */
	len = 0;
	while (m) {
		len += (m->b_wptr-m->b_rptr);
		m = m->b_cont;
	}
	if (len < *lenp) {
		printf("xdrmblk_getmblk failed\n");
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdrmblk_putbytes(xdrs, addr, len)
	register XDR	*xdrs;
	caddr_t		 addr;
	register u_int		 len;
{
#ifdef XDRDEBUG
printf("xdrmblk_putbytes: Entered %x\n", xdrs->x_base);
#endif

	while ((xdrs->x_handy -= len) < 0) {
		if ((xdrs->x_handy += len) > 0) {
			bcopy(addr, xdrs->x_private, (u_int)xdrs->x_handy);
			addr += xdrs->x_handy;
			len -= xdrs->x_handy;
		}
		if (xdrs->x_base) {
			register mblk_t *m =
			    /* LINTED pointer alignment */
			    ((mblk_t *)xdrs->x_base)->b_cont;

			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_private = (char *)m->b_wptr;
			xdrs->x_handy = m->b_datap->db_lim - m->b_datap->db_base;
		} else {
			return (FALSE);
		}
	}
	bcopy(addr, xdrs->x_private, len);
	xdrs->x_private += len;
	return (TRUE);
}

/*
 * Like putbytes, only we avoid the copy by pointing a type 2
 * mblk at the buffer.  Not safe if the buffer goes away before
 * the mblk chain is deallocated.
 */
bool_t
xdrmblk_putbuf(xdrs, addr, len, func, arg)
	register XDR	*xdrs;
	caddr_t		 addr;
	u_int		 len;
	void		 (*func)();
	int		 arg;
{
	register mblk_t *m;
	mblk_t *esballoc();
	long llen = len;
	frtn_t	frtn;
#ifdef XDRDEBUG
printf("xdrmblk_putbuf: Entered %x\n", xdrs->x_base);
#endif

	if (len & 3) {  /* can't handle roundup problems */
		return (FALSE);
	}
	if (! xdrmblk_putlong(xdrs, &llen)) {
		return(FALSE);
	}

	/* make the current one zero length
	 */
	/* LINTED pointer alignment */
	((mblk_t *)xdrs->x_base)->b_rptr += xdrs->x_handy;
	frtn.free_func = func;
	frtn.free_arg = (char *)arg;
	m = esballoc(addr, len, BPRI_LO, &frtn);
	if (m == NULL) {
		printf("xdrmblk_putbuf: esballoc failed\n");
		return (FALSE);
	}
	/* link the new one to the zero length original.
	 */
	/* LINTED pointer alignment */
	((mblk_t *)xdrs->x_base)->b_cont = m;
	xdrs->x_handy = 0;	/* makes other routines go look at the next
				 * mblk.
				 */
	return (TRUE);
}

u_int
xdrmblk_getpos(xdrs)
	register XDR	*xdrs;
{
	u_int tmp;

	/* LINTED pointer alignment */
	tmp =  (u_int)(((mblk_t *)xdrs->x_base)->b_rptr) - 
			/* LINTED pointer alignment */
			(u_int)(((mblk_t *)xdrs->x_base)->b_datap->db_base);
#ifdef XDRDEBUG
printf("xdrmblk_getpos: pos = %d\n", tmp);
#endif
	return tmp;
		
}

bool_t
xdrmblk_setpos(xdrs, pos)
	register XDR	*xdrs;
	u_int		 pos;
{
	/* calculate the new address from the base */
	/* LINTED pointer alignment */
	register caddr_t newaddr = (caddr_t)((((mblk_t *)xdrs->x_base)->b_rptr) + pos);

	/* calculate the last valid address in the mblk */
	register caddr_t	lastaddr = 
		xdrs->x_private + xdrs->x_handy;
#ifdef XDRDEBUG
printf("xdrmblk_setpos: Entered %x\n", xdrs->x_base);
#endif

	if ((int)newaddr > (int)lastaddr)
		return (FALSE);
	xdrs->x_private = newaddr;
	xdrs->x_handy = (int)lastaddr - (int)newaddr;
	return (TRUE);
}

long *
xdrmblk_inline(xdrs, len)
	register XDR	*xdrs;
	int		 len;
{
	long *buf = 0;

#ifdef XDRDEBUG
printf("xdrmblk_inline: Entered %x\n", xdrs->x_base);
#endif

	if (xdrs->x_handy >= len) {
		xdrs->x_handy -= len;
		/* LINTED pointer alignment */
		buf = (long *) xdrs->x_private;
		xdrs->x_private += len;
	}
	return (buf);
}
#endif	/* _KERNEL */
