/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	_RESOLV_RESOLVABI_H
#define _RESOLV_RESOLVABI_H

#ident	"@(#)nametoaddr:resolv/resolvabi.h	1.1"

#ifdef _RESOLV_ABI
/* For internal use only when building the resolv routines */
#define	select	_abi_select
#define	syslog	_abi_syslog
#define	seteuid	_abi_seteuid
#endif /* _RESOLV_ABI */

#endif /* _RESOLV_RESOLVABI_H */
