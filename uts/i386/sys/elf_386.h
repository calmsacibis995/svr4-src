/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_ELF_I386_H
#define _SYS_ELF_I386_H

#ident	"@(#)head.sys:sys/elf_386.h	1.2.3.1"
#define	EF_I386_NONE		0
#define	EF_I386_FP		1	/* Floating point chip saved state */
#define	EF_I386_WEITEK		2	/* Weitek chip saved state */

#define R_386_NONE		0	/* relocation type */
#define R_386_32		1
#define R_386_PC32		2
#define R_386_GOT32		3
#define R_386_PLT32		4
#define R_386_COPY		5
#define R_386_GLOB_DAT		6
#define R_386_JMP_SLOT		7
#define R_386_RELATIVE		8
#define R_386_GOTOFF		9
#define R_386_GOTPC		10
#define R_386_NUM		11	/* must be > last */

#define ELF_386_MAXPGSZ		0x1000	/* maximum page size */

#endif
