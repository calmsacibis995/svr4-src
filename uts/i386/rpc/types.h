/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _RPC_TYPES_H
#define _RPC_TYPES_H

#ident	"@(#)kern-rpc:types.h	1.3"

/*      @(#)types.h 1.23 88/10/25 SMI      */

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
 * Rpc additions to <sys/types.h>
 */
#include <sys/types.h>

#define	bool_t	int
#define	enum_t	int
#define __dontcare__	-1

#ifndef FALSE
#	define	FALSE	(0)
#endif

#ifndef TRUE
#	define	TRUE	(1)
#endif

#ifndef NULL
#	define NULL 0
#endif

#ifndef _KERNEL
#define mem_alloc(bsize)	malloc(bsize)
#define mem_free(ptr, bsize)	free(ptr)
#else
#include <sys/kmem.h>		/* XXX */
extern _VOID *kmem_alloc();
#define mem_alloc(bsize)	kmem_alloc((u_int)bsize, KM_SLEEP)
#define mem_free(ptr, bsize)	kmem_free((caddr_t)(ptr), (u_int)(bsize))

#ifdef DEBUG
extern int	rpc_log();
extern int	rpclog;

#define		RPCLOG(A, B, C) ((void)((rpclog) && rpc_log((A), (B), (C))))
#else
#define		RPCLOG(A, B, C)
#endif

#endif

#ifdef _NSL_RPC_ABI
/* For internal use only when building the libnsl RPC routines */
#define select	_abi_select
#define gettimeofday	_abi_gettimeofday
#define syslog	_abi_syslog
#define getgrent	_abi_getgrent
#define endgrent	_abi_endgrent
#define setgrent	_abi_setgrent
#endif

#include <sys/time.h>

#endif	/* _RPC_TYPES_H */
