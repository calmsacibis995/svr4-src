/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Reg.h	1.6"

#include	<sys/reg.h>

enum Fund_type;

//
// NAME
//	Reg.h
//
// ABSTRACT
//	Typedef and struct for register names and attributes
//	(machine independent).
//
// DESCRIPTION
//	Each machine has its own set of register names.  This header
//	is the common interface.
//

#ifndef REG_UNK

typedef int RegRef;			// a register reference (index)

#define	REG_UNK		-1

enum Stype;             // from Itype.h

struct RegAttrs {
	RegRef	 ref;
	char	*name;
	char	 size;	// in bytes
	char	 flags;	// see below
	Stype	 stype;
	int	 offset;
};

// bits in "flags"
#define	FPREG	0x01

extern RegAttrs regs[];	// array, last entry contains reg = REG_UNK
			// first decrease in ref's indicates start of aliases

extern RegRef regref(char *name);	// lookup by name
extern Fund_type  regtype(RegRef);

overload regattrs;
extern RegAttrs  *regattrs(RegRef);
extern RegAttrs  *regattrs(char *name);

#include "Reg1.h"	/* machine specific */

#endif /* REG_UNK */
