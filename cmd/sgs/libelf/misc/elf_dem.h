/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*ident	"@(#)libelf:common/elf_dem.h	1.3"*/
/* ELF interface header file for the C++ demangler.

   The C++ demangler is shared by the ELF library and the C++
   translator.  Because the translator is used on many different
   operating systems, not all of which use ELF, we have set up this
   library so that it may be compiled to work with or without libelf.
   The default is to compile for ELF.  Undefine the macro "ELF" in the
   makefile before compiling for the translator.

   These macros tack an _elf_ on to the beginning of all global names
   which should hidden outside of the demangler module.  The only
   visible name at this time is the function "demangle" which is
   mapped to "elf_demangle" for the ELF library.  Note that we use
   "_elf_" to hide names even for the translator.  We could have used
   just "_" but since the names are hidden why should we bother.
*/

#define	app_String	_elf_app_String
#define	demangle_doarg	_elf_demangle_doarg
#define	demangle_doargs	_elf_demangle_doargs
#define	findop		_elf_findop
#define	free_String	_elf_free_String
#define	mk_String	_elf_mk_String
#define	napp_String	_elf_napp_String
#define	nplist		_elf_nplist
#define	nprep_String	_elf_nprep_String
#define	prep_String	_elf_prep_String
#define	set_String	_elf_set_String
#define trunc_String	_elf_trunc_String
#define jbuf		_elf_jbuf

#if defined(ELF)

#define	demangle	elf_demangle


/* Make sure that realloc isn't called inadvertantly.
*/
#define realloc		__can_not_use_realloc_in_elf__

#else /* defined(ELF) */

#include <malloc.h>

#endif	/* defined(ELF) */

#ifdef __STDC__
#	include <limits.h>
#	define ID_NAME_MAX	INT_MAX
#else
	/* The only requirement on this is that it must be greater
	   than the length of any symbol.
	*/
#	define ID_NAME_MAX	30000
#endif
