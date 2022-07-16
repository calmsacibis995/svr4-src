/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rtld:common/map.c	1.29"

#include "rtinc.h"


/* map in a shared object or the a.out; takes an open file descriptor
 * for the map object and its pathname
 * returns a pointer to a rt_private_map
 * structure for this object, or 0 on error.
 */

struct rt_private_map *_map_so(fd, pathname)
int fd;
CONST char *pathname;
{
	int is_main = 0;
	register int i; 
	int prot;
	register unsigned long addr, faddr; 
	unsigned long memsize = 0, foff; 
	unsigned long mentry, hsize, fsize, msize;
	unsigned long lastaddr = 0;
	register Elf32_Ehdr *ehdr;
	Elf32_Phdr *last, *first = 0;
	register Elf32_Phdr *phdr, *pptr;
	Elf32_Dyn *mld;
	CONST char *name;
	struct rt_private_map *lm;

	DPRINTF(LIST,(2,"rtld: _map_so(%d, %s)\n",fd, pathname? pathname : (CONST char *)"0"));

	/* is object the a.out? */
	if (pathname == (char *)0) {
		name = _proc_name;
		is_main = 1;
	}
	else
		name = pathname;
	
	/* read ELF and program headers 
	 * we first make a guess as to how big the elf and program
	 * headers will be and attempt to read both in 1 chunk
	 * if the guess was wrong we reread the correct amount
	 */

	hsize = sizeof(Elf32_Ehdr) + (8 * sizeof(Elf32_Phdr));
	if ((ehdr = (Elf32_Ehdr *)_rtmalloc(hsize)) == 0)
		return 0;

	if (_read(fd, (char *)ehdr, hsize) != hsize) {
		if ((_lseek(fd, 0, 0) == -1) ||
	 	   (_read(fd, (char *)ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr))) {
			_rt_lasterr("%s: %s: can't read ELF header for file: %s",(char*) _rt_name,_proc_name,name);
			return 0;
		}
	}

	/* verify information in file header */

	/* check ELF identifier */
	if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
		ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
		ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
		ehdr->e_ident[EI_MAG3] != ELFMAG3) {
		_rt_lasterr("%s: %s: %s is not an ELF file",(char*) _rt_name,_proc_name,name);
		return 0;
	}

	/* check class and encoding */
	if (ehdr->e_ident[EI_CLASS] != M_CLASS ||
		ehdr->e_ident[EI_DATA] != M_DATA) {
		_rt_lasterr("%s: %s: %s has wrong class or data encoding",(char*) _rt_name,_proc_name,name);
		return 0;
	}

	/* check magic number */
	if (is_main) {
		if (ehdr->e_type != ET_EXEC) {
			_rt_lasterr("%s: %s: %s not an executable file",(char*) _rt_name, _proc_name,name);
			return 0;
		}
	}
	else { /* shared object */
		if (ehdr->e_type != ET_DYN) {
			_rt_lasterr("%s: %s: %s not a shared object",(char*) _rt_name,_proc_name,name);
			return 0;
		}
	}

	/* check machine type */
	if (ehdr->e_machine != M_MACH) {
		_rt_lasterr("%s: %s: bad machine type for file: %s",(char*) _rt_name,_proc_name,name);
		return 0;
	}

	/* check flags */
	if (ehdr->e_flags != M_FLAGS) {
		if (_flag_error(ehdr->e_flags, name))
			return 0;
	}

	/* verify ELF version */
	/* ??? is this too restrictive ??? */
	if (ehdr->e_version > EV_CURRENT) {
		_rt_lasterr("%s: %s: bad file version for file: %s",(char*) _rt_name,_proc_name,name);
		return 0;
	}

	/* find and read program header if our original guess was wrong */
	if ((ehdr->e_phoff > ehdr->e_ehsize) ||
		((ehdr->e_ehsize + (ehdr->e_phnum * ehdr->e_phentsize)) > hsize)) {
		hsize = ehdr->e_phnum * ehdr->e_phentsize;
		if (_lseek(fd, ehdr->e_phoff, 0) == -1) {
			_rt_lasterr("%s: %s: cannot seek to program header for file: %s",(char*) _rt_name,_proc_name,name);
			return 0;
		}

		if ((phdr = (Elf32_Phdr *)_rtmalloc(hsize)) == 0)
			return 0;

		if (_read(fd, (char *)phdr, hsize) != hsize) {
			_rt_lasterr("%s: %s: can't read program header for file: %s",(char*) _rt_name,_proc_name,name);
			return 0;
		}
	}
	else 
		phdr = (Elf32_Phdr *)((char *)ehdr + ehdr->e_ehsize);

	/* get entry point */
	mentry = (unsigned long)(ehdr->e_entry);

	/* read segment list and extract needed information */
	for (i = 0,pptr = phdr; i < (int)ehdr->e_phnum; i++) {
		if (pptr->p_type == PT_LOAD) {
			if (first == 0) {
				first = pptr;
			}
			else if (pptr->p_vaddr <= lastaddr) {
				_rt_lasterr("%s: %s: invalid program header - segments out of order: %s",(char*) _rt_name,_proc_name,name);
				return 0;
			}
			lastaddr = pptr->p_vaddr;
			last = pptr;
		}
		else if (pptr->p_type == PT_DYNAMIC)
			mld = (Elf32_Dyn *)(pptr->p_vaddr);
		pptr = (Elf32_Phdr *)((unsigned long)pptr + ehdr->e_phentsize);
	}
	
	/* check that we have at least 1 loadable segment */
	if (first == 0) {
		_rt_lasterr("%s: %s: no loadable segments in %s",(char*) _rt_name,_proc_name,name);
		return 0;
	}

	/* calculate beginning virtual addr == virtual address of
	 * first segment truncated to previous page boundary
	 */
	faddr = STRUNC(first->p_vaddr);

	/* calculate total amount of memory to be mapped ==
	 * virtual addr of last loadable segment plus
	 * memsize of last loadable segment minus first virtual
	 * addr truncated to a page boundary
	 */
	memsize = (last->p_vaddr + last->p_memsz) - faddr;

	/* map in object - first map enough memory to hold
	 * entire object; use /dev/zero.
	 * If shared object, let OS decide where to put object;
	 * if a.out, put object at faddr.
	 */
	if (ehdr->e_type == ET_DYN) {
		if ((faddr = (unsigned long)_mmap(0, memsize,
			PROT_READ, MAP_PRIVATE,
			_devzero_fd, 0)) == -1) {

			_rt_lasterr("%s: %s: can't map enough space for file %s",(char*) _rt_name, _proc_name,name);
			return 0;
		}
	}
	else  { /* a.out */
		if (_mmap((caddr_t)faddr, memsize,
			PROT_READ,
			(MAP_PRIVATE|MAP_FIXED), _devzero_fd, 0) 
			== (caddr_t)-1) {

			_rt_lasterr("%s: %s: can't map enough space for %s",(char*) _rt_name,_proc_name,name);
			return 0;
		}
	}

	DPRINTF(MAP,(2, "rtld: mapped 0x%x bytes from %s at 0x%x\n",
		memsize, DEV_ZERO, faddr));

	/* now map each loadable segment */
	for (i = 0,pptr = phdr; i < (int)ehdr->e_phnum; i++) {
		if (pptr->p_type != PT_LOAD) {
			pptr = (Elf32_Phdr *)((unsigned long)pptr + 
				ehdr->e_phentsize);
			continue;
		}

	/* find file offset to begin mapping from ==
	 * offset truncated to previous page boundary
	 * fsize is filesz in program header plus amount
	 * we just truncated offset.
	 * memory size of segment is fsize plus difference
	 * between fielsz and memsz
	 */
		foff = PTRUNC(pptr->p_offset);
		fsize = pptr->p_filesz + (pptr->p_offset - foff);
		msize = fsize + (pptr->p_memsz - pptr->p_filesz);
			

	/* find beginning virtual address of segment ==
	 * virtual address truncated to previous page boundary
	 */
		if (pptr == first) 
			addr = faddr;
		else
			addr = PTRUNC(((unsigned long)pptr->p_vaddr 
				+ (is_main ? 0 : faddr)));

	/* set up permissions and do mapping 
	 * initial mappings all have read and write permission
	 * this is reset where appropriate before rtld
	 * returns
	 */
		prot = 0;
		if (pptr->p_flags & PF_R)
			prot |= PROT_READ;
		if (pptr->p_flags & PF_W)
			prot |= PROT_WRITE;
		if (pptr->p_flags & PF_X)
			prot |= PROT_EXEC;

		if (_mmap((caddr_t)addr, fsize, prot,
			(MAP_FIXED|MAP_PRIVATE), fd, foff) == (caddr_t)-1) {

			_rt_lasterr("%s: %s: can't map segment with size 0x%x at 0x%x for file %s",(char*) _rt_name,_proc_name,fsize,addr,name);
			goto error;
		}

	DPRINTF(MAP,(2, "rtld: mapped 0x%x bytes from offset 0x%x at 0x%x\n",
		fsize, foff, addr));

	/* unmap hole, if any, between this segment and the next */
	if ((SROUND(addr + msize) > PROUND(addr + msize)) && pptr != last) {
	 	if (_munmap((caddr_t)PROUND(addr + msize), 
			(SROUND(addr + msize) - PROUND(addr + msize)))) {
			_rt_lasterr("%s: %s: can't unmap space for %s",(char*) _rt_name,_proc_name,name);
			goto error;
		}
	DPRINTF(MAP,(2,"rtld: unmapped 0x%x bytes from addr 0x%x\n",
		(SROUND(addr + msize) - PROUND(addr + msize)),
		PROUND(addr + msize)));
	}

	/* If memsz > filesz and
	 * if addr + filesz is less than a page boundary
	 * we must zero out the "dirty" page at the end of the
	 * filesz boundary
	 */
		if (pptr->p_memsz > pptr->p_filesz) {
			unsigned long bmem;
			bmem = PROUND(addr + fsize);
			if (bmem < (addr + msize)) {
				if (_mprotect((caddr_t)bmem,(PROUND(addr+msize)-bmem),
					prot) == -1) {
					_rt_lasterr("%s: %s: can't set protections on segment of length 0x%x at 0x%x",(char*) _rt_name, _proc_name,(addr+msize)-bmem,bmem);
				goto error;
				}
			}
			if (bmem > (addr + fsize)) {
				(void)_clrpage((char *)addr + fsize,
					 bmem - (addr + fsize));
				DPRINTF(MAP,(2,
					"rtld: zeroed 0x%x bytes at 0x%x\n",
					bmem -(addr+fsize), addr+fsize));
				if (!is_main && _rt_nodelete)
					_rtmkspace((char *) (addr + msize),
						PAGESIZE - ((addr + msize)
							% PAGESIZE));
			} 
		} 

		pptr = (Elf32_Phdr *)((unsigned long)pptr + ehdr->e_phentsize);
	} /* end for loop */


	/* close argument file descriptor */
	if (_close(fd) == -1) {
		_rt_lasterr("%s: %s: can't close %s",(char*) _rt_name,_proc_name,name);
		goto error;
	}
	if (!is_main) { /* add base addr to dynamic and entry point */
		mld = (Elf32_Dyn *)((unsigned long)mld + faddr);
		mentry += faddr;
	}

	/* create and return new rt_private_map structure */
	lm = _new_lm(pathname, mld, faddr, memsize, mentry, phdr, 
		ehdr->e_phnum, ehdr->e_phentsize);
	if (!lm)
		goto error;
	if (TEXTREL(lm))
		if (_set_protect(lm, PROT_WRITE) == 0) {
			_rt_cleanup(lm);
			return 0;
		}
	return(lm);
error:
	(void) _munmap((caddr_t) faddr, memsize);
	return 0;
}

/* create a new rt_private_map structure and initialize all values. */

struct rt_private_map *_new_lm(pname, ld, addr, msize, entry, phdr, phnum, phsize)
CONST char *pname;
Elf32_Dyn *ld;
unsigned long addr, msize, entry, phnum, phsize;
Elf32_Phdr *phdr;
{
	register struct rt_private_map *lm;
	register unsigned long offset;
	int rpathflag = 0;

	DPRINTF(LIST,(2, "rtld: _new_lm(%s, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, %d, %d)\n",
		(pname ?  pname : (CONST char *)"0"),(unsigned long)ld,addr,msize,entry,
		(unsigned long)phdr,phnum,phsize));

	/* allocate space */
	if ((lm = (struct rt_private_map *)_rtmalloc(sizeof(struct rt_private_map))) == 0) 
		return 0;

	/* all fields not filled in were set to 0 by _rtmalloc */
	NAME(lm) = (char *)pname;
	DYN(lm) = ld;
	ADDR(lm) = addr;
	MSIZE(lm) = msize;
	ENTRY(lm) = entry;
	PHDR(lm) = (VOID *)phdr;
	PHNUM(lm) = (unsigned short)phnum;
	PHSZ(lm) = (unsigned short)phsize;

	/* fill in rest of rt_private entries with info the from
	 * the file's dynamic structure
	 * if shared object, add base address to each address;
	 * if a.out, use address as is
	 */
	if (pname)
		offset = addr;
	else 
		offset = 0;

	/* read dynamic structure into an arry of ptrs to Elf32_Dyn
	 * unions - array[i] is pointer to Elf32_Dyn with tag == i
	 */
	for ( ; ld->d_tag != DT_NULL; ++ld ) {
		switch (ld->d_tag) {
		case DT_SYMTAB:
			SYMTAB(lm) = (char *)ld->d_un.d_ptr + offset;
			break;

		case DT_STRTAB:
			STRTAB(lm) = (char *)ld->d_un.d_ptr + offset;
			break;

		case DT_SYMENT:
			SYMENT(lm) = ld->d_un.d_val;
			break;

		case DT_TEXTREL:
			TEXTREL(lm) = 1;
			break;

	/* at this time we can only handle 1 type of relocation per object */
		case DT_REL:
		case DT_RELA:
			REL(lm) = (char *)ld->d_un.d_ptr + offset;
			break;

		case DT_RELSZ:
		case DT_RELASZ:
			RELSZ(lm) = ld->d_un.d_val;
			break;

		case DT_RELENT:
		case DT_RELAENT:
			RELENT(lm) = ld->d_un.d_val;
			break;

		case DT_HASH:
			HASH(lm) = (unsigned long *)(ld->d_un.d_ptr + offset);
			break;

		case DT_PLTGOT:
			PLTGOT(lm) = (unsigned long *)(ld->d_un.d_ptr + offset);
			break;

		case DT_PLTRELSZ:
			PLTRELSZ(lm) = ld->d_un.d_val;
			break;

		case DT_JMPREL:
			JMPREL(lm) = (char *)(ld->d_un.d_ptr) + offset;
			break;

		case DT_INIT:
			INIT(lm) = (void (*)())((unsigned long)ld->d_un.d_ptr + offset);
			break;

		case DT_FINI:
			FINI(lm) = (void (*)())((unsigned long)ld->d_un.d_ptr + offset);
			break;

		case DT_SYMBOLIC:
			SYMBOLIC(lm) = 1;
			break;

		case DT_RPATH:
			rpathflag = 1;
			RPATH(lm) = (char *) ld->d_un.d_val;
			break;

		case DT_DEBUG:
		/* set pointer to debugging information in a.out's
		 * dynamic structure
		 */
			ld->d_un.d_ptr = (Elf32_Addr)&_r_debug;
			break;
		}
	}
	if (rpathflag)
		RPATH(lm) = (char *)((unsigned long )RPATH(lm) + (char *)STRTAB(lm));

	return(lm);
}

/* function to correct protection settings 
 * segments are all mapped initially with  permissions as given in
 * the segment header, but we need to turn on write permissions
 * on a text segment if there are any relocations against that segment,
 * and them turn write permission back off again before returning control
 * to the program.  This function turns the permission on or off depending
 * on the value of the argument
 */

int _set_protect(lm, permission)
struct rt_private_map *lm;
int permission;
{
	register int i, prot;
	register Elf32_Phdr *phdr;
	unsigned long msize, addr;

	DPRINTF(LIST,(2, "rtld: _set_protect(%s, %d)\n",(NAME(lm) ? NAME(lm)
		:"a.out"), permission));

	phdr = (Elf32_Phdr *)PHDR(lm);
	/* process all loadable segments */
	for (i = 0; i < (int)PHNUM(lm); i++) {
		if ((phdr->p_type == PT_LOAD) && ((phdr->p_flags & PF_W) == 0)) {
			prot = PROT_READ | permission;
			if (phdr->p_flags & PF_X)
				prot |=  PROT_EXEC;
			addr = (unsigned long)phdr->p_vaddr + NAME(lm) ?
				ADDR(lm) : 0;
			msize = phdr->p_memsz + (addr - PTRUNC(addr));
			if (_mprotect((caddr_t)PTRUNC(addr), msize, prot) == -1){
				_rt_lasterr("%s: %s: can't set protections on segment of length 0x%x at 0x%x",(char*) _rt_name, _proc_name,msize, PTRUNC(addr));
				return(0);
			}
		}
		phdr = (Elf32_Phdr *)((unsigned long)phdr + PHSZ(lm));
	} /* end for phdr loop */
	return(1);
}
