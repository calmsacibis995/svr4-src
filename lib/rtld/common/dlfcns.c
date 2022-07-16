/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rtld:common/dlfcns.c	1.10"

#include "rtinc.h"



/* return pointer to string describing last occurring error
 * the notion of the last occurring error is cleared
 */

#ifdef __STDC__

char *_dlerror()
#else
char *dlerror()
#endif
{
	char * etmp;

	DPRINTF(LIST,(2,"rtld: dlerror()\n"));
	etmp = _rt_error;
	_rt_error = (char *)0;

	return(etmp);
}

/* open a shared object - uses rtld to map the object into
 * the process' address space - maintains list of
 * known objects; on success, returns a pointer to the structure
 * containing information about the newly added object;
 * on failure, returns a null pointer
 */

static DLLIB *dl_head;
static DLLIB *dl_tail;

static DLLIST *dl_makelist	ARGS((struct rt_private_map *lm));
static int dl_delete		ARGS((DLLIB *dptr));

#ifdef __STDC__
VOID *_dlopen(pathname, mode)
#else
VOID *dlopen(pathname, mode)
#endif
char *pathname;
int mode;
{
	struct rt_private_map *lm;
	register DLLIB *dlptr;
	register DLLIST *lptr;
	register int i;
	DLLIB *dl_old_tail;
	Elf32_Dyn *retval;
	Elf32_Dyn interface[3];

	DPRINTF(LIST,(2,"rtld: dlopen(%s, %d)\n",pathname?pathname:(CONST char *)"0",mode));

	if (!dl_head) {  /* set up DLLIB structure for main */
		/* mark each object already on rt_private_map list as
		 * non-deletable so we do not remove those
		 * objects mapped in on startup
		 */
		for (lm = _ld_loaded; lm; lm = (struct rt_private_map *)NEXT(lm)) {
			NODELETE(lm) = 1;
		}
		NODELETE(_rtld_map) = 1;
		if ((dl_head = (DLLIB *)_rtmalloc(sizeof(DLLIB))) == 0) 
			return 0;
		dl_head->dl_name = (char *)0; /* main has null name */
		dl_head->dl_status = DL_CLOSED;
		dl_tail = dl_head;
		if ((dl_head->dl_list = dl_makelist(_ld_loaded)) ==
			(DLLIST *)0) {
			dl_head = 0;
			return 0;
		}
	}

	/* determine whether we already have a DLLIB structure 
	 * for this object
	 */
	for (dlptr = dl_head; dlptr; dlptr = dlptr->dl_next) {
		if (pathname == 0) {
			if (dlptr->dl_name == 0)
				break;
			else continue;
		}
		else if (dlptr->dl_name == 0)
			continue;
		if (!_rtstrcmp(pathname, dlptr->dl_name))
			break;
	}

	/* if we don't have a dllib structure 
	 * or the dllib structure has status closed or deleted
	 * (and it's not the a.out)
	 * we call rtld to load the shared object
	 * and any objects dependent upon it.
	 */
	if (!dlptr || (dlptr->dl_status != DL_OPEN && dlptr->dl_name)) {

		if (mode != RTLD_LAZY && mode != RTLD_NOW) {
			_rt_lasterr("%s: %s: illegal mode to dlopen: %d",(char*) _rt_name,_proc_name,mode);
			return 0;
		}
		/* set up rtld interface */
		interface[0].d_tag = DT_FPATH;
		interface[0].d_un.d_ptr = (Elf32_Addr)pathname;
		interface[1].d_tag = DT_MODE;
		interface[1].d_un.d_val = mode;
		interface[2].d_tag = DT_NULL;
	
		if (_rtld(interface, &retval) != 0) 
			return 0;
		
		/* if we already have a dllib structure for this object,
		 * update it; otherwise create a new one
		 */
		 if (!dlptr) {
			if ((dlptr = (DLLIB *)_rtmalloc(sizeof(DLLIB))) == 0) 
				goto error;
			if ((dlptr->dl_name = (char *) _rtmalloc(_rtstrlen
				(pathname) +1)) == 0) 
				goto error;
			(void)_rtstrcpy(dlptr->dl_name, pathname);
			dl_tail->dl_next = dlptr;
			dl_old_tail = dl_tail;
			dl_tail = dlptr;
		}
	
	
		/* find pointer to rt_private_map of shared object in Elf32_Dyn
		 * structure returned by rtld
		 */
		while(retval->d_tag != DT_NULL) {
			if (retval->d_tag == DT_MAP ||
				retval->d_tag == DT_MAP_NODELETE)
				break;
			retval++;
		}
		if (retval->d_tag == DT_NULL) {
			_rt_lasterr("%s: %s: interface error: bad return value to dlopen",(char*) _rt_name,_proc_name);
			return 0;
		}
			
		/* create list of dependent shared objects */
		if ((dlptr->dl_list = dl_makelist((struct rt_private_map *)(retval->d_un.d_ptr))) == (DLLIST *)0) {
			goto error_unhook;
		}
	}
	
	/* increment reference count of each object on list 
	 * set refdeny bit for any object whose refpermit bit is not set
	 */
	dlptr->dl_status = DL_OPEN;
	for (lptr = dlptr->dl_list; lptr; lptr = lptr->l_next) {
		for (i = 0; i < DL_LISTSIZE; i++) {
			if (!lptr->l_list[i])
				break;
			else {
				COUNT(lptr->l_list[i]) += 1;
				if (!PERMIT(lptr->l_list[i]))
					DENY(lptr->l_list[i]) = 1;
			}
		}
	}
	return((VOID *)dlptr);
error_unhook:		/* must unhook DLLIB structure from dl_head list */
	(dl_tail = dl_old_tail)->dl_next = 0;
	/* fall through */
error:
	if (retval->d_tag != DT_MAP_NODELETE)
		_rt_cleanup((struct rt_private_map *) (retval->d_un.d_ptr));
	return 0;
}

/* create a list of pointers to the rt_private_map structures
 * of this shared object and all shared objects dependent on
 * this one; the list is built of DL_LISTSIZE length arrays
 * of pointers, allocated dynamically
 */
static DLLIST *dl_makelist(lm)
struct rt_private_map *lm;
{
	DLLIST *lhead, *lcur, *lread, *lptr;
	Elf32_Dyn *lneed;
	register int i_cur, i_rd, j;
	int found;
	char *name;
	struct rt_private_map *nlm;

	DPRINTF(LIST,(2,"rtld: dl_makelist(0x%x)\n",lm));
	/* allocate first list chunk */
	if ((lhead = (DLLIST *)_rtmalloc(sizeof(DLLIST))) == 0) 
		return 0;
	lhead->l_list[0] = lm;
	lcur = lread = lhead;
	i_rd = 0;
	i_cur = 1;
	while (lread->l_list[i_rd]) {
		/* process each object on needed list */
		lneed = (Elf32_Dyn *)DYN(lread->l_list[i_rd]);
		while (lneed->d_tag != DT_NULL) {
			if (lneed->d_tag != DT_NEEDED) {
				lneed++;
				continue;
			}
			name = (char *)STRTAB(lm) + lneed->d_un.d_val;
			nlm = _so_loaded(name);

			/* make sure we haven't already put this 
			 * map structure on our list
			 * this is to prevent circularity
			 */
			found = 0;
			for (lptr = lhead; lptr; lptr = lptr->l_next) {
				for (j = 0; j < DL_LISTSIZE; j++) {
					if (!lptr->l_list[j])
						break;
					else
						if (nlm == lptr->l_list[j]) {
							found = 1;
							break;
						}
				}
			}
			if (!found) {
				lcur->l_list[i_cur++] = nlm;
				if (i_cur >= DL_LISTSIZE) { 
				/* allocate new chunk */
					if ((lcur->l_next = (DLLIST *)_rtmalloc(sizeof(DLLIST))) == 0) 
						return 0;
					lcur = lcur->l_next;
					i_cur = 0;
				}
			}
			lneed++;
		} /* end for each item on needed list */
		i_rd++;
		if (i_rd >= DL_LISTSIZE) { 
			if (!lread->l_next)
				break; /* end of list */
			else {
				lread = lread->l_next;
				i_rd = 0;
			}
		}
	}
	return(lhead);
}

/* takes the name of a symbol and a pointer to a dllib structure;
 * search for the symbol in the shared object specified
 * and in all objects in the specified object's needed list
 * returns the address of the symbol if found; else 0
 */

#ifdef __STDC__

VOID *_dlsym(handle, name)
#else
char *dlsym(handle, name)
#endif
char *name;
VOID *handle;
{
	struct rt_private_map *nlm;
	Elf32_Sym *sym;
	unsigned int addr;
	register int i;
	register DLLIST *lptr;

	DPRINTF(LIST,(2,"rtld: dlsym(0x%x, %s)\n",handle,name?name:(CONST char *)"0"));

	if (!name) {
		_rt_lasterr("%s: %s: null symbol name to dlsym",(char*) _rt_name,_proc_name);
		return(0);
	}
	if (((DLLIB *)handle)->dl_status != DL_OPEN) {
		_rt_lasterr("%s: %s: dlsym: attempt to find symbol %s in closed object",(char*) _rt_name,_proc_name,name);
		return(0);
	}

	/* for each object on list, lookup name in that object's 
	 * symbol table
	 */
	for (lptr = ((DLLIB *)handle)->dl_list; lptr; lptr = lptr->l_next) {
		for (i = 0; i < DL_LISTSIZE; i++) {
			if (!lptr->l_list[i])
				break;
			else
				if ((sym = _lookup(name,  lptr->l_list[i],
					0, &nlm, LOOKUP_NORM)) != 
					(Elf32_Sym *)0)
					goto symfound;
		}
	}
	if (!sym) {
		_rt_lasterr("%s: %s: dlsym: can't find symbol: %s",(char*) _rt_name,_proc_name,name);
		return(0);
	}
symfound:
	addr = sym->st_value;
	if (NAME(nlm))
		addr += ADDR(nlm);
	return((VOID *)addr);
}

/* close the shared object associated with handle;
 * reference counts are decremented - we check reference
 * counts of all objects on each dllib list; if any list
 * has all objects with ref count == 0, those objects are
 * unmapped from the address space
 * Returns 0 on success, 1 on failure.
 */

#ifdef __STDC__

int _dlclose(handle)
#else 
int dlclose(handle)
#endif
VOID *handle;
{
	register int i, ref;
	register DLLIST *lptr;
	register DLLIB *dptr;

	DPRINTF(LIST,(2,"rtld: dlclose(0x%x)\n",handle));

	if (((DLLIB *)handle)->dl_status != DL_OPEN) {
		_rt_lasterr("%s: %s: dlclose: attempt to close already closed object",(char*) _rt_name,_proc_name);
		return(1);
	}
	/* decrement reference counts of all objects associated
	 * with this object
	 */
	
	for (lptr = ((DLLIB *)handle)->dl_list; lptr; lptr = lptr->l_next) {
		for (i = 0; i < DL_LISTSIZE; i++) {
			if (!lptr->l_list[i])
				break;
			else
				COUNT(lptr->l_list[i]) -= 1;
		}
	}

	/* if ref count of 1st object on list <= 0, status of this
	 * dllib goes to DL_CLOSED
	 */
	if (COUNT(((DLLIB *)handle)->dl_list->l_list[0]) <= 0)
		((DLLIB *)handle)->dl_status = DL_CLOSED;
	
	/* go through dllib list - for all entries with status
	 * closed, check if all members have ref count == 0;
	 * if so, delete those members 
	 */
	/* - skip a.out. */
	for (dptr = dl_head->dl_next; dptr; dptr = dptr->dl_next) {
		if (dptr->dl_status != DL_CLOSED)
			continue;
		ref = 0;
		for (lptr = dptr->dl_list; lptr; lptr = lptr->l_next) {
			for (i = 0; i < DL_LISTSIZE; i++) {
				if (!lptr->l_list[i])
					goto testref;
				else
					if (COUNT(lptr->l_list[i]) >= 1) {
						ref = 1;
						goto testref;
					}
				}
		}
	testref:
		if (!ref) {
			if (!dl_delete(dptr)) 
			/* ??? is this what we want ??? */
				return(1);
			else
				dptr->dl_status = DL_DELETED;
		}
	}
	return(0);
}

/* unmap a set of shared objects from the process' address space;
 * unlink associated link maps from list - returns 1 on success,
 * 0 on failure.
 */

static int dl_delete(dptr)
DLLIB *dptr;
{
	register DLLIST *lptr;
	register int i;
	register struct rt_private_map *lm;
	void (*fptr)();
	int j;
	Elf32_Phdr *phdr;

	DPRINTF(LIST,(2,"rtld: dl_delete(0x%x)\n",dptr));
	/* alert debuggers that link_map list is shrinking */
	_r_debug.r_state = RT_DELETE;
	_r_debug_state();

	for (lptr = dptr->dl_list; lptr; lptr = lptr->l_next) {
		for (i = 0; i < DL_LISTSIZE; i++) {
			if (!lptr->l_list[i])
				break;
			else {
				lm = lptr->l_list[i];
				if (!NODELETE(lm)) {
					/* invoke _fini, if present */
					fptr = FINI(lm);
					if (fptr)
						(*fptr)();
					/* unmap each segment */
					phdr = (Elf32_Phdr *)PHDR(lm);
					for (j = 0; j < (int)PHNUM(lm); j++) {
						unsigned long addr, msize;
						if (phdr->p_type == PT_LOAD) {
							addr = (unsigned long)
								phdr->p_vaddr
								+ ADDR(lm);
							msize = phdr->p_memsz +
								(addr - PTRUNC
								(addr));
							if (_munmap((caddr_t)
								PTRUNC(addr), 
								msize) == -1) {
								/*  ??? or should we continue ??? */
								_rt_lasterr("%s: %s: dlclose: failure unmapping %s",(char*) _rt_name,_proc_name,NAME(lm));
								return(0);
							}
						}
						phdr = (Elf32_Phdr *)
							((unsigned long)phdr 
								+ PHSZ(lm));
					} /* end for phdr loop */
					NODELETE(lm) = 1;
					/* unlink lm from chain 
					 * we never unlink the
					 * 1st item on the chain
					 * (the a.out)
					 */
					NEXT((struct rt_private_map *)PREV(lm)) = NEXT(lm);
					if (!NEXT(lm))
						_ld_tail = (struct rt_private_map *)PREV(lm);
					else
						PREV((struct rt_private_map *)NEXT(lm)) = PREV(lm);
						
				} /* not nodelete */
			} 
		}
	}
	
	/* alert debuggers that link_map is consistent again */
	_r_debug.r_state = RT_CONSISTENT;
	_r_debug_state();

	return(1);
}

