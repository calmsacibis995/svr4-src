/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)kern-io:osm.c	1.3"

/*
 *	OSM - operating system messages, allows system printf's to
 *	be read via special file.
 *	minor 0: starts from beginning of buffer and waits for more.
 *	minor 1: starts from beginning of buffer but doesn't wait.
 *	minor 2: starts at current buffer position and waits.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/file.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/debug.h"

extern	char	putbuf[];	/* system putchar circular buffer */
extern	int	putbufsz;		/* size of above */
extern	int	putbufndx;	/* next position for system putchar */

osmopen(dev)
{
	struct file *fp;
	int error;

	dev = minor(dev);
	if (error = getf(u.u_rval1, &fp))
		return(error);
	if (dev == 2)
		fp->f_offset = putbufndx;
	else if (dev > 2)
		u.u_error = ENODEV;
	else if (putbufndx > putbufsz)
		fp->f_offset = putbufndx - putbufsz;
}

osmread(dev)
{
	register int	index;
	register int	offset;
	register int	count;

	dev = minor(dev);
	if (dev == 1 && u.u_offset >= putbufndx)
		return;

	spl6();
	while (u.u_offset == putbufndx)
		sleep(putbuf, PWAIT);
	spl0();

	ASSERT(u.u_offset < putbufndx);

	while (u.u_count  &&  u.u_offset < putbufndx  && u.u_error == 0) {
		offset = u.u_offset % putbufsz;
		index = putbufndx % putbufsz;
		if (offset < index) {
			count = min(index - offset, u.u_count);
		} else {
			count = min(putbufsz - offset, u.u_count);
		}
		iomove(&putbuf[offset], count, B_READ);
	}
}

osmwrite()
{
	register int	cc;

	while ((cc = cpass()) >= 0)
		putbuf[putbufndx++ % putbufsz] = cc;
	wakeup(putbuf);
}
