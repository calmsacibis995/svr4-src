/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/foreign.h	1.2"


/*	This file declares functions for handling foreign (non-ELF)
 *	file formats.
 */


int		_elf_coff _((Elf *));

extern int	(*const _elf_foreign[]) _((Elf *));
