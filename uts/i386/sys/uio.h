/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_UIO_H
#define _SYS_UIO_H

#ident	"@(#)head.sys:sys/uio.h	1.6.3.1"

/*
 * I/O parameter information.  A uio structure describes the I/O which
 * is to be performed by an operation.  Typically the data movement will
 * be performed by a routine such as uiomove(), which updates the uio
 * structure to reflect what was done.
 */

typedef struct iovec {
	caddr_t	iov_base;
	int	iov_len;
} iovec_t;

typedef struct uio {
	iovec_t	*uio_iov;	/* pointer to array of iovecs */
	int	uio_iovcnt;	/* number of iovecs */
	off_t	uio_offset;	/* file offset */
	short	uio_segflg;	/* address space (kernel or user) */
	short	uio_fmode;	/* file mode flags */
	daddr_t	uio_limit;	/* u-limit (maximum "block" offset) */
	int	uio_resid;	/* residual count */
} uio_t;

/*
 * I/O direction.
 */
typedef enum uio_rw { UIO_READ, UIO_WRITE } uio_rw_t;

/*
 * Segment flag values.
 */
typedef enum uio_seg { UIO_USERSPACE, UIO_SYSSPACE, UIO_USERISPACE } uio_seg_t;

int	uiomove();
int	ureadc();	/* should be errno_t in future */
int	uwritec();
int	uiomvuio();
void	uioskip();

#endif	/* _SYS_UIO_H */
