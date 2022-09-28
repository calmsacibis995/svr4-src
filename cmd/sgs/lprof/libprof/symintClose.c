/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lprof:libprof/symintClose.c	1.3"

#include "hidelibc.h"		/* uses "_" to hide libc functions */
#include "hidelibelf.h"		/* uses "_" to hide libelf functions */

#include "symint.h"
#include "debug.h"

/* * * * * *
 * symintFcns.c -- symbol information interface routines.
 * 
 * these routines form a symbol information access
 * interface, for the profilers to get at object file
 * information.  this interface was designed to aid
 * in the COFF to ELF conversion of prof, lprof and friends.
 * 
 */




/* * * * * *
 * _symintClose(profPtr)
 * profPtr	- structure allocated by _symintOpen(),
 * 		  indicating structures to free and
 * 		  object file to close.
 * 
 * specifically, elf_end() and fclose() are called for the object file,
 * and the PROF_SYMBOL and section hdr arrays are freed.
 * 
 * 
 * No Returns.
 */

void
_symintClose(profPtr)
PROF_FILE *profPtr; {

	DEBUG_LOC("_symintClose: top");
	if (profPtr) {
		(void) elf_end(profPtr->pf_elf_p);
		(void) close(profPtr->pf_fildes);

		(void) free(profPtr->pf_shdarr_p);
		(void) free(profPtr->pf_symarr_p);
	}
	DEBUG_LOC("_symintClose: bottom");
}
