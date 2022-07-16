/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _NETINET_INSREM_H
#define _NETINET_INSREM_H

#ident	"@(#)kern-inet:insrem.h	1.3"

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


/* simulate vax insque and remque instructions. */

typedef struct vq {
	caddr_t         fwd, back;
} vq_t;

#define dequenxt(e)     (e)->fwd = ((vq_t *)(e)->fwd)->fwd

#define enque(e, p)     (e)->fwd = (p)->fwd; (p)->fwd = (caddr_t)(e)

#if defined(u3b2) || defined(i386)

#define	insque(e, p)	(e)->back = (caddr_t)(p); \
			(e)->fwd = (caddr_t)((vq_t *)(p)->fwd); \
			((vq_t *)(p)->fwd)->back = (caddr_t)(e); \
			(p)->fwd = (caddr_t)(e);

#define	remque(e)	((vq_t *)(e)->back)->fwd =  \
					(caddr_t)(e)->fwd; \
			((vq_t *)(e)->fwd)->back = \
					(caddr_t)(e)->back; \
			(e)->fwd = (caddr_t) 0; \
			(e)->back = (caddr_t)0;
#endif /* u3b2 */

#endif	/* _NETINET_INSREM_H */
