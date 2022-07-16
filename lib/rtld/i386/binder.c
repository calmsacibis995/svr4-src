/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rtld:i386/binder.c	1.10"


/* function binding routine - invoked on the first call
 * to a function through the procedure linkage table;
 * passes first through an assembly language interface
 *
 *
 * Takes the address of the PLT entry where the call originated,
 * the offset into the relocation table of the associated
 * relocation entry and the address of the rt_private_map for the entry
 *
 * Returns the address of the function referenced after
 * re-writing the PLT entry to invoke the function
 * directly
 * 
 * On error, causes process to terminate with a SIGKILL
 */

#include <signal.h>
#include "rtinc.h"

 unsigned long _binder(lm, reloc)
 struct rt_private_map *lm;
 unsigned long reloc;
 {
	struct rt_private_map *nlm;
	char *symname;
	Elf32_Rel *rptr;
	Elf32_Sym *sym, *nsym;
	unsigned long value;
	unsigned long *got_addr;

	DPRINTF((LIST|DRELOC),(2, "rtld: _binder(0x%x, 0x%x)\n", reloc, lm));

	if (!lm) {
		_rtfprintf(2, "%s: %s: unidentifiable procedure reference\n",(char*) _rt_name,_proc_name);
		(void)_kill(_getpid(), SIGKILL);
	}
	
	/* use relocation entry to get symbol table entry and symbol name */
	rptr = (Elf32_Rel *)((char *)JMPREL(lm) + reloc);
	sym = (Elf32_Sym *)((unsigned long)SYMTAB(lm) + 
		(ELF32_R_SYM(rptr->r_info) * SYMENT(lm)));
	symname = (char *)(STRTAB(lm) + sym->st_name);

	/* find definition for symbol */
	if ((nsym = _lookup(symname, 0, _ld_loaded, &nlm, LOOKUP_NORM)) ==
		(Elf32_Sym *)0 &&
		(!DENY(lm) ||
		(nsym = _lookup(symname, lm, 0, &nlm, LOOKUP_NORM)) ==
		(Elf32_Sym *)0)) {
		_rtfprintf(2, "%s: %s: symbol not found: %s\n",(char*) _rt_name,_proc_name,symname);
		(void)_kill(_getpid(), SIGKILL);
	}
	
	/* get definition address and rebuild PLT entry */
	value = nsym->st_value + (NAME(nlm) ? ADDR(nlm) : 0);

	DPRINTF(DRELOC,(2, "rtld: relocating function %s to 0x%x\n",
		symname, value));

	got_addr = (unsigned long *)((int)rptr->r_offset + (NAME(lm) ? ADDR(lm) : 0));
	*got_addr = value;
	DPRINTF(DRELOC,(2, "got_addr = 0x%x, *got_addr = 0x%x\n", got_addr, *got_addr));
	return(value);
}
