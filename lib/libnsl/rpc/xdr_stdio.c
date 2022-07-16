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


#ident	"@(#)librpc:xdr_stdio.c	1.2.2.1"

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
static char sccsid[] = "@(#)xdr_stdio.c 1.26 89/01/31 Copyr 1984 Sun Micro";
#endif

/*
 * xdr_stdio.c, XDR implementation on standard i/o file.
 *
 * This set of routines implements a XDR on a stdio stream.
 * XDR_ENCODE serializes onto the stream, XDR_DECODE de-serializes
 * from the stream.
 */

#include <rpc/types.h>
#include <stdio.h>
#include <rpc/xdr.h>
#include <sys/byteorder.h>

static struct xdr_ops *xdrstdio_ops();

/*
 * Initialize a stdio xdr stream.
 * Sets the xdr stream handle xdrs for use on the stream file.
 * Operation flag is set to op.
 */
void
xdrstdio_create(xdrs, file, op)
	register XDR *xdrs;
	FILE *file;
	enum xdr_op op;
{
	xdrs->x_op = op;
	xdrs->x_ops = xdrstdio_ops();
	xdrs->x_private = (caddr_t)file;
	xdrs->x_handy = 0;
	xdrs->x_base = 0;
}

/*
 * Destroy a stdio xdr stream.
 * Cleans up the xdr stream handle xdrs previously set up by xdrstdio_create.
 */
static void
xdrstdio_destroy(xdrs)
	register XDR *xdrs;
{
	(void)fflush((FILE *)xdrs->x_private);
	/* xx should we close the file ?? */
}

static bool_t
xdrstdio_getlong(xdrs, lp)
	XDR *xdrs;
	register long *lp;
{
	if (fread((caddr_t)lp, sizeof(long), 1, (FILE *)xdrs->x_private) != 1)
		return (FALSE);
#ifndef mc68000
	*lp = ntohl(*lp);
#endif
	return (TRUE);
}

static bool_t
xdrstdio_putlong(xdrs, lp)
	XDR *xdrs;
	long *lp;
{

#ifndef mc68000
	long mycopy = htonl(*lp);
	lp = &mycopy;
#endif
	if (fwrite((caddr_t)lp, sizeof(long), 1, (FILE *)xdrs->x_private) != 1)
		return (FALSE);
	return (TRUE);
}

static bool_t
xdrstdio_getbytes(xdrs, addr, len)
	XDR *xdrs;
	caddr_t addr;
	u_int len;
{
	if ((len != 0) && (fread(addr, (int)len, 1, (FILE *)xdrs->x_private) != 1))
		return (FALSE);
	return (TRUE);
}

static bool_t
xdrstdio_putbytes(xdrs, addr, len)
	XDR *xdrs;
	caddr_t addr;
	u_int len;
{
	if ((len != 0) && (fwrite(addr, (int)len, 1, (FILE *)xdrs->x_private) != 1))
		return (FALSE);
	return (TRUE);
}

static u_int
xdrstdio_getpos(xdrs)
	XDR *xdrs;
{
	return ((u_int) ftell((FILE *)xdrs->x_private));
}

static bool_t
xdrstdio_setpos(xdrs, pos) 
	XDR *xdrs;
	u_int pos;
{ 

	return ((fseek((FILE *)xdrs->x_private, (long)pos, 0) < 0) ?
		FALSE : TRUE);
}

static long *
xdrstdio_inline(xdrs, len)
	XDR *xdrs;
	u_int len;
{

	/*
	 * Must do some work to implement this: must insure
	 * enough data in the underlying stdio buffer,
	 * that the buffer is aligned so that we can indirect through a
	 * long *, and stuff this pointer in xdrs->x_buf.  Doing
	 * a fread or fwrite to a scratch buffer would defeat
	 * most of the gains to be had here and require storage
	 * management on this buffer, so we don't do this.
	 */
	return (NULL);
}

static struct xdr_ops *
xdrstdio_ops()
{
	static struct xdr_ops ops;

	if (ops.x_getlong == NULL) {
		ops.x_getlong = xdrstdio_getlong;
		ops.x_putlong = xdrstdio_putlong;
		ops.x_getbytes = xdrstdio_getbytes;
		ops.x_putbytes = xdrstdio_putbytes;
		ops.x_getpostn = xdrstdio_getpos;
		ops.x_setpostn = xdrstdio_setpos;
		ops.x_inline = xdrstdio_inline;
		ops.x_destroy = xdrstdio_destroy;
	}
	return (&ops);
}
