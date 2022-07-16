/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rtld:i386/machdep.h	1.6"

/* i386 machine dependent macros, constants and declarations */

/* object file macros */
#define ELF_TARGET_386 1
#define M_MACH	EM_386
#define M_CLASS	ELFCLASS32
#define M_DATA	ELFDATA2LSB
#define M_FLAGS	_flags

/* page size */

#define PAGESIZE	_syspagsz

/* segment boundary */

#ifdef	ELF_386_MAXPGSZ
#define SEGSIZE		ELF_386_MAXPGSZ
#else
#define SEGSIZE		0x1000	/* 4k */
#endif

/* macro to truncate to previous page boundary */

#define PTRUNC(X)	((X) & ~(PAGESIZE - 1))

/* macro to truncate to previous segment boundary */

#define STRUNC(X)	((X) & ~(SEGSIZE - 1))

/* macro to round to next page boundary */

#define PROUND(X)	(((X) + PAGESIZE - 1) & ~(PAGESIZE - 1))

/* macro to round to next segment boundary */

#define SROUND(X)	(((X) + SEGSIZE - 1) & ~(SEGSIZE - 1))

/* macro to round to next double word boundary */

#define DROUND(X)	(((X) + sizeof(double) - 1) & ~(sizeof(double) - 1))

/* generic bit mask */

#define MASK(N)	((1 << (N)) -1)

/* is V in the range supportable in N bits ? */

#define IN_RANGE(V, N)  ((-(1 << ((N) - 1))) <= (V) && (V) < (1 << ((N) - 1)))

/* macro to determine if relocation is a PC-relative type */

#define PCRELATIVE(T)	((T) == R_386_PC32)

/* default library search directory */

#define LIBDIR	"/usr/lib"
#define LIBDIRLEN	8

/* /dev/zero */
#define DEV_ZERO "/dev/zero"
