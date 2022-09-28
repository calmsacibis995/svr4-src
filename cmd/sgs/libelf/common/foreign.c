/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/foreign.c	1.5"


#include "syn.h"
#include "libelf.h"
#include "decl.h"
#include "foreign.h"


/* Foreign file conversion
 *	Allow other file formats to be converted to elf.
 *	Change this table to support new or drop old conversions.
 *
 *	A foreign function actually returns Elf_Kind or -1 on error.
 */


int	_elf_coff	_((Elf *));


int	(*const _elf_foreign[]) _((Elf *)) =
{
	_elf_coff,
	0,
};
