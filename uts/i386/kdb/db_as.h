/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:kdb/db_as.h	1.2"

/*
 * Address space codes for the kernel debugger.
 */

#ifndef _DB_AS_H
#define _DB_AS_H

typedef unsigned int	db_as_t;

#define AS_KVIRT	1	/* Kernel virtual */
#define AS_PHYS		2	/* Physical */
#define AS_IO		3	/* I/O ports */
#define AS_UVIRT	4	/* User process virtual */

typedef struct as_addr {
	u_long	a_addr;		/* Address within address space */
	db_as_t	a_as;		/* Relevant address space */
	int	a_mod;		/* Numeric address space modifier */
} as_addr_t;


paddr_t db_uvtop(), db_kvtop();

#endif /* _DB_AS_H */
