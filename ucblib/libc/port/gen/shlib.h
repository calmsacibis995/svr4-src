/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/gen/shlib.h	1.1.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
 * This header file contains all the macros and definitons
 *  needed for importing symbols for libc_s
 * 
 */
#if SHLIB

#define __ctype	(* _libc__ctype)
#define _numeric	(* _libc__numeric)

#define _getflthw	(* _libc__getflthw)

#define _cleanup (* _libc__cleanup)
#define _environ  (* _libc__environ)		
#define end	 (* _libc_end)
#define _exithandle	 (* _libc__exithandle)

#define malloc	(* _libc_malloc)		
#define free	(* _libc_free)	
#define realloc (* _libc_realloc)


#define _sibuf  (* _libc__sibuf)
#define _sobuf  (* _libc__sobuf)
#define _smbuf  (* _libc__smbuf)
#define __iob 	(* _libc__iob)
#define _lastbuf	(* _libc__lastbuf)
#define _bufendtab	(* _libc__bufendtab)

#define _lib_version	(* _libc__lib_version)

#define strspn	(* _libc_strspn)


#endif
