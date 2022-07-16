/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rtld:i386/reloc.c	1.6"

/* i386 specific routines for performing relocations */

#include "rtinc.h"
#include <sys/elf_386.h>
#include <sgs.h>

/* read and process the relocations for one link object 
 * we assume all relocation sections for loadable segments are
 * stored contiguously in the file
 *
 * accept a pointer to the rt_private_map structure for a given object and
 * the binding mode: if RTLD_LAZY, then don't relocate procedure
 * linkage table entries; if RTLD_NOW, do.
 */

static int do_reloc ARGS((struct rt_private_map *, Elf32_Rel *, unsigned long, int));


int _relocate(lm, mode)
struct rt_private_map *lm;
int mode;
{
	register unsigned long *got_addr;
	register unsigned long base_addr;
	register Elf32_Rel *rel;
	register Elf32_Rel *rend;

	DPRINTF((LIST|DRELOC),(2, "rtld: _relocate(%s, %d), lm::0x%x\n",
		(CONST char *)((NAME(lm) ? NAME(lm) : "a.out")), mode, lm));
	
	/* if lazy binding, initialize first procedure linkage
	 * table entry to go to _rtbinder
	 */
	if (PLTGOT(lm)) {
		if (mode == RTLD_LAZY) {
			/* fix up the first few GOT entries
			 *	GOT[GOT_XLINKMAP] = the address of the link map
			 *	GOT[GOT_XRTLD] = the address of rtbinder
			 */
			got_addr = PLTGOT(lm) + GOT_XLINKMAP;
			*got_addr = (unsigned long)lm;
			got_addr = PLTGOT(lm) + GOT_XRTLD;
			*got_addr = (unsigned long)_rtbinder;

			/* if this is a shared object, we have to step
			 * through the plt entries and add the base address
			 * to the corresponding got entry
			 */
			if (NAME(lm)) {
				base_addr = ADDR(lm);
				rel = JMPREL(lm);
				rend = (Elf32_Rel *)((unsigned long)rel + PLTRELSZ(lm));
		DPRINTF((LIST|DRELOC),(2, "%s:%d: rel::0x%x\n", __FILE__, __LINE__, rel));

				for ( ; rel < rend; ++rel) {
					got_addr = (unsigned long *)((char *)rel->r_offset + base_addr);	
					*got_addr += base_addr;
				}
			}
		}
		else
		{
			if (do_reloc(lm, JMPREL(lm), PLTRELSZ(lm), LOOKUP_NORM) == 0)
				return 0;
		}
	}

	if (RELSZ(lm) && REL(lm))
		return do_reloc(lm, REL(lm), RELSZ(lm), LOOKUP_SPEC);
	return 1;
}


static int
do_reloc(lm, reladdr, relsz, flag)
	struct rt_private_map *lm;
	register Elf32_Rel *reladdr;
	unsigned long relsz;
	int flag;
{
	unsigned long baseaddr, stndx;
	register unsigned long off;
	register unsigned int rtype;
	register Elf32_Rel *rend;
	long value;
	Elf32_Sym *symref, *symdef;
	char *name;
	struct rt_private_map *def_lm, *first_lm, *list_lm;

	DPRINTF((LIST|DRELOC),(2, "rtld: do_reloc(%s, 0x%x, 0x%x)\n",
		(CONST char *)(NAME(lm) ? NAME(lm) : "a.out"), reladdr, relsz));
	
	baseaddr = ADDR(lm);
	rend = (Elf32_Rel *)((unsigned long)reladdr + relsz);

	/* loop through relocations */
	for ( ; reladdr < rend; ++reladdr) { 
		rtype = ELF32_R_TYPE(reladdr->r_info);
		off = (unsigned long)reladdr->r_offset;
		stndx = ELF32_R_SYM(reladdr->r_info);

		if (rtype == R_386_NONE)
			continue;

		/* if not a.out, add base address to offset */
		if (NAME(lm))
			off += baseaddr;


		/* if R_386_RELATIVE, simply add base addr 
		 * to reloc location 
		 */

		if (rtype == R_386_RELATIVE)
			value = baseaddr;

		/* get symbol table entry - if symbol is local
		 * value is base address of this object
		 */
		 else {
			symref = (Elf32_Sym *)((unsigned long)SYMTAB(lm) + (stndx * SYMENT(lm)));
	
			/* if local symbol, just add base address 
			 * we should have no local relocations in the
			 * a.out
			 */
			if (ELF32_ST_BIND(symref->st_info) == STB_LOCAL) {
				value = baseaddr;
			}
			else {	/* global or weak 
				 * lookup symbol definition - error 
				 * if name not found and reference was 
				 * not to a weak symbol - weak 
				 * references may be unresolved
				 */
		
				name = (char *)(STRTAB(lm) + symref->st_name);

			DPRINTF(DRELOC,(2, "rtld: relocating %s\n",name));

				first_lm = 0;
				if (rtype == R_386_COPY) {
					/* don't look in the a.out */
					list_lm = (struct rt_private_map *)NEXT(_ld_loaded);
				} else {
					list_lm = _ld_loaded;
					/* look in the current object first */
					if (SYMBOLIC(lm))
						first_lm = lm;
				}
					
				if (((symdef = _lookup(name, first_lm, list_lm, &def_lm, flag))
					== (Elf32_Sym *)0)
					&& (ELF32_ST_BIND(symref->st_info)
					!= STB_WEAK)) {
					if (_rt_warn) {
						_rtfprintf(2, "%s: %s: relocation error: symbol not found: %s\n",(char*) _rt_name, _proc_name,name);
						continue;
					}
					else {
						_rt_lasterr("%s: %s: relocation error: symbol not found: %s",(char*) _rt_name, _proc_name, name);
						return(0);
					}
				}
				else { /* symbol found  - relocate */
					if (symdef == (Elf32_Sym *)0)
						/* undefined weak global */
						continue;  
					/* calculate location of definition 
					 * - symbol value plus base address of
					 * containing shared object
					 */
					value = symdef->st_value;
					if (NAME(def_lm) && 
						(symdef->st_shndx != SHN_ABS))
						value += ADDR(def_lm);
		
		
					/* for R_386_COPY, just make an entry 
					 * in the rt_copy_entries array
					 */
					if (rtype == R_386_COPY) {
						struct rel_copy *rtmp;
		
						if ((rtmp = (struct rel_copy *) _rtmalloc(sizeof(struct rel_copy))) == 0) {
							if (!_rt_warn)
								return(0);
							else continue;
						}
						else {
							rtmp->r_to = (char *)off;
							rtmp->r_size = symdef->st_size;
							rtmp->r_from = (char *) value;
							if (!_rt_copy_entries) {
								/* 1st entry */
								_rt_copy_entries = rtmp;
								_rt_copy_last = rtmp;
							}
							else {
								_rt_copy_last-> r_next = rtmp;
								_rt_copy_last = rtmp;
							}
						}
						continue;
					} /* end R_386_COPY */
					
					/* calculate final value - 
					 * if PC-relative, subtract ref addr
					 */
					if (PCRELATIVE(rtype))
						value -= off;
					
				} /* end else symbol found */
			} /* end global or weak */
		} /* end not R_386_RELATIVE */
		/* insert value calculated at reference point
		 * 3 cases - normal byte order aligned, normal byte
		 * order unaligned, and byte swapped
		 * for the swapped and unaligned cases we insert value 
		 * a byte at a time
		 */
		switch(rtype) {
		case R_386_GLOB_DAT:  /* word aligned */
			DPRINTF(DRELOC,(2,"rtld: sym value is 0x%x, offset is 0x%x\n",value, off));
			value += *(unsigned long *)off;
			*(unsigned long *)off = value;
			break;
		case R_386_32:	/* unaligned */
		case R_386_RELATIVE:
		case R_386_PC32:
			*(unsigned long *)off += value;
			break;
		case R_386_JMP_SLOT: 
			/* for plt-got do not add ref contents */
			*(unsigned long *)off = value;
			break;
		default:
			if (_rt_warn)
				_rtfprintf(2, "%s: %s: invalid relocation type %d at 0x%x\n",(char*) _rt_name,_proc_name,rtype,off);
			else {
				_rt_lasterr("%s: %s: invalid relocation type %d at 0x%x",(char*) _rt_name,_proc_name,rtype,off);
				return(0);
			}
			break;
		}

	} /* end of while loop */
	return(1);
}

