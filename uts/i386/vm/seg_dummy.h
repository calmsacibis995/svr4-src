/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _VM_SEG_DUMMY_H
#define _VM_SEG_DUMMY_H

#ident	"@(#)kern-vm:seg_dummy.h	1.3"

#ifdef _KERNEL

#if defined(__STDC__)
extern int segdummy_create(struct seg *, void *);
#else
extern int segdummy_create();
#endif

#endif /* _KERNEL */

#endif	/* _VM_SEG_DUMMY_H */
