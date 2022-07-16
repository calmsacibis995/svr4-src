/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc:inc/shlib.h	1.10"
/*
 * This header file contains all the macros and definitons
 *  needed for importing symbols for dynamic shared library
 * 
 */
#ifdef DSHLIB

#define malloc	(* _libc_malloc)		
#define free	(* _libc_free)	
#define realloc (* _libc_realloc)
#define calloc (* _libc_calloc)

#endif
