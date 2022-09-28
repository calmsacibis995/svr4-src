/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ldd:i386/machdep.h	1.2"

/* 386 machine dependent definitions */

#define M_TYPE EM_386
#define M_DATA ELFDATA2LSB
#define M_CLASS ELFCLASS32
#define ELF_EHDR Elf32_Ehdr
#define ELF_PHDR Elf32_Phdr
#define elf_getehdr elf32_getehdr
#define elf_getphdr elf32_getphdr
