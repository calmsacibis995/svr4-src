/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

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
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ifndef _VM_FAULTCODE_H
#define _VM_FAULTCODE_H

#ident	"@(#)kern-vm:faultcode.h	1.3"

/*
 * This file describes the "code" that is delivered during
 * SIGBUS and SIGSEGV exceptions.  It also describes the data
 * type returned by vm routines which handle faults.
 *
 * If FC_CODE(fc) == FC_OBJERR, then FC_ERRNO(fc) contains the errno value
 * returned by the underlying object mapped at the fault address.
 */
#define	FC_HWERR	0x1	/* misc hardware error (e.g. bus timeout) */
#define	FC_ALIGN	0x2	/* hardware alignment error */
#define	FC_NOMAP	0x3	/* no mapping at the fault address */
#define	FC_PROT		0x4	/* access exceeded current protections */
#define	FC_OBJERR	0x5	/* underlying object returned errno value */

#define	FC_MAKE_ERR(e)	(((e) << 8) | FC_OBJERR)

#define	FC_CODE(fc)	((fc) & 0xff)
#define	FC_ERRNO(fc)	((unsigned)(fc) >> 8)

#ifndef LOCORE
typedef	int	faultcode_t;	/* type returned by vm fault routines */
#endif /* LOCORE */

#endif	/* _VM_FAULTCODE_H */
