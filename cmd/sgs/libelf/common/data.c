/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/data.c	1.3"


#include "syn.h"
#include "libelf.h"
#include "decl.h"


/* Global data
 * _elf_byte		Fill byte for file padding.  See elf_fill().
 * _elf32_ehdr_init	Clean copy for to initialize new headers.
 * _elf_encode		Host/internal data encoding.  If the host has
 *			an encoding that matches one known for the
 *			ELF format, this changes.  An machine with an
 *			unknown encoding keeps ELFDATANONE and forces
 *			conversion for host/target translation.
 * _elf_err		Global error number.  See elf_errno(), elf_errmsg().
 * _elf_work		Working version given to the lib by application.
 *			See elf_version().
 */

int			_elf_byte = 0;
const Elf32_Ehdr	_elf32_ehdr_init = { 0 };
unsigned		_elf_encode = ELFDATANONE;
int			_elf_err = 0;
const Snode		_elf_snode_init = { 0 };
unsigned		_elf_work = EV_NONE;
