/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ifndef _NET_STRIOC_H
#define _NET_STRIOC_H

#ident	"@(#)kern-net:strioc.h	1.3"

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


struct iocqp {
	unsigned short iqp_type;
	unsigned short iqp_value;
};

/* Queue types */
#define IQP_RQ		0		/* standard read queue */
#define IQP_WQ		1		/* standard write queue */
#define IQP_HDRQ	2		/* stream head read queue */
#define IQP_MUXRQ	3		/* multiplexor read queue */
#define IQP_MUXWQ	4		/* multiplexor write queue */
#define IQP_NQTYPES	5

#define MODL_INFO_SZ          2
#define DRVR_INFO_SZ          3
#define MUXDRVR_INFO_SZ       5

/* Queue parameter (value) types */
#define IQP_LOWAT	0x10		/* Low water mark */
#define IQP_HIWAT	0x20		/* High water mark */
#define IQP_NVTYPES     2

/* Masks */
#define IQP_QTYPMASK	0x0f
#define IQP_VTYPMASK	0xf0

/* Ioctl */
#define INITQPARMS      ('Q'<<8|0x01)

#ifdef _KERNEL
extern int initqparms();
#endif
#endif	/* _NET_STRIOC_H */
