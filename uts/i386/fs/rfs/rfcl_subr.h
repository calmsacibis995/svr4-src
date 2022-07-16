/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FS_RFS_RFCL_SUBR_H
#define _FS_RFS_RFCL_SUBR_H

#ident	"@(#)kern-fs:rfs/rfcl_subr.h	1.3"

extern int	rfcl_op();
extern int	rfcl_xac();
extern void	rfcl_giftfree();
extern int	rf_stime();
extern int	rfcl_findsndd();
extern int	rfcl_readmove();
extern int	rfcl_writemove();
extern void	rfcl_signal();
extern void	rfcl_sdrele();
extern int	rfcl_copysync();
extern int	rf_falloc();
extern int	rfcl_strategy();
extern int	rfcl_read_op();
extern int	rfcl_write_op();
extern int	rfcl_uioclone();
extern void	rfcl_reqsetup();

extern union rq_arg	init_rq_arg;

/*
 * the standard number of times to send a request in the presence of
 * flow control replies
 */
#if !defined(RFCL_MAXTRIES)
#define RFCL_MAXTRIES 10
#endif

#endif /* _FS_RFS_RFCL_SUBR_H */
