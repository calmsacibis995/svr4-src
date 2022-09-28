/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/i386/Machine.h	1.8.1.1"

// Machine dependent manifest constants.

#define TEXT_BYTE_SWAP  

// #define SIZEOF_XFLOAT	12	/* size (in bytes) of MAU extended float */

#define GENERIC_CHAR_SIGNED  	0 
#define	ELFMAGIC	 	0x464c457f
#define COFFMAGIC	 	0x14c
#ifdef PTRACE
#define EXECCNT			4
#else
#define EXECCNT			2
#endif

#define BKPTSIZE		1
#define BKPTSIG			5

#define RTLD_BASE		0x80000000
#ifndef UVUBLK
#define UVUBLK			0xe0000000
#endif
#define BKPTTEXT		"\314"
#define SYSCALL_FAILED()        (goal2 == sg_run && getreg(REG_EAX) != 0)
#define PROCKLUDGE		0
#define	RTOLBYTES
