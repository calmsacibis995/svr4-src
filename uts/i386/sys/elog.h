/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_ELOG_H
#define _SYS_ELOG_H

#ident	"@(#)head.sys:sys/elog.h	11.3.7.1"
/*
 * "True" major device numbers. These correspond
 * to standard positions in the configuration
 * table, but are used for error logging
 * purposes only.
 */

#define CNTL	1
#define SYS	2
#define CAC	3
#define PF	4

/*
 * IO statistics are kept for each physical unit of each
 * block device (within the driver). Primary purpose is
 * to establish a guesstimate of error rates during
 * error logging.
 */

struct iostat {
	long	io_ops;		/* number of read/writes */
	long	io_misc;	/* number of "other" operations */
	long	io_qcnt;	/* number of jobs assigned to drive */
	ushort io_unlog;	/* number of unlogged errors */
};

/*
 * structure for system accounting
 */
struct iotime {
	struct iostat ios;
	long	io_bcnt;	/* total blocks transferred */
	clock_t	io_resp;	/* total block response time */
	clock_t	io_act;		/* total drive active time (cumulative utilization) */
	int	io_pad;		/* round size to 2^n */
};
#define	io_cnt	ios.io_ops
#define io_qc ios.io_qcnt
/* drive utilization times can be calculated by system software as follows */

/* Average drive utilization = (io_cact/io_elapt) */
/* Average drive utilization for last interval = (io_liact/io_intv) */

#endif	/* _SYS_ELOG_H */
