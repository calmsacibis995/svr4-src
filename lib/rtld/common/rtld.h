/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rtld:common/rtld.h	1.7"

/* common header for run-time linker */

/* run-time linker private data maintained for each shared object -
 * connected to link_map structure for that object
 */

struct rt_private_map {
	struct link_map r_public;	/* public data */
	VOID *r_symtab;			/* symbol table */
	unsigned long *r_hash;		/* hash table */
	char *r_strtab;			/* string table */
	VOID *r_reloc;			/* relocation table */
	unsigned long *r_pltgot;	/* addresses for procedure linkage table */
	VOID *r_jmprel;			/* plt relocations */
	unsigned long r_pltrelsize;	/* size of PLT relocation entries */
	void (*r_init)();		/* address of _init */
	void (*r_fini)();		/* address of _fini */
	char *r_runpath;		/* LD_RUN_PATH */
	unsigned long r_relsz;		/* size of relocs */
	unsigned long r_relent;		/* size of base reloc entry */
	unsigned long r_syment;		/* size of symtab entry */
	int r_symbolic;			/* compiled Bsymbolic? */
	unsigned long r_msize;		/* total memory mapped */
	unsigned long r_entry;		/* entry point for file */
	VOID  *r_phdr;			/* program header of object */
	unsigned short r_phnum;		/* number of segments */
	unsigned short r_phentsize;	/* size of phdr entry */
	char r_count;			/* reference count */
	char r_nodelete;		/* can this object be deleted */
	char r_refpermit;		/* permit references */
	char r_refdeny;			/* deny references */
	int  r_textrel;			/* text relocations */
};

/* macros for getting to link_map data */
#define ADDR(X) ((X)->r_public.l_addr)
#define NAME(X) ((X)->r_public.l_name)
#define DYN(X) ((X)->r_public.l_ld)
#define NEXT(X) ((X)->r_public.l_next)
#define PREV(X) ((X)->r_public.l_prev)

/* macros for getting to linker private data */
#define COUNT(X) ((X)->r_count)
#define NODELETE(X) ((X)->r_nodelete)
#define SYMTAB(X) ((X)->r_symtab)
#define HASH(X) ((X)->r_hash)
#define STRTAB(X) ((X)->r_strtab)
#define PLTGOT(X) ((X)->r_pltgot)
#define JMPREL(X) ((X)->r_jmprel)
#define PLTRELSZ(X) ((X)->r_pltrelsize)
#define INIT(X) ((X)->r_init)
#define FINI(X) ((X)->r_fini)
#define RELSZ(X) ((X)->r_relsz)
#define REL(X) ((X)->r_reloc)
#define RELENT(X) ((X)->r_relent)
#define SYMENT(X) ((X)->r_syment)
#define SYMBOLIC(X) ((X)->r_symbolic)
#define MSIZE(X) ((X)->r_msize)
#define ENTRY(X) ((X)->r_entry)
#define RPATH(X) ((X)->r_runpath)
#define PHDR(X) ((X)->r_phdr)
#define PHNUM(X) ((X)->r_phnum)
#define PHSZ(X) ((X)->r_phentsize)
#define PERMIT(X) ((X)->r_refpermit)
#define DENY(X) ((X)->r_refdeny)
#define TEXTREL(X) ((X)->r_textrel)


/* data structure used to keep track of special R1_COPY
 * relocations
 */

struct rel_copy	{
	VOID *r_to;		/* copy to address */
	VOID *r_from;		/* copy from address */
	unsigned long r_size;		/* copy size bytes */
	struct rel_copy *r_next;	/* next on list */
};

/* debugger information version*/
#define LD_DEBUG_VERSION 1

#ifdef DEBUG
#define MAXLEVEL	7	/* maximum debugging level */
#define LIST		1	/* dubgging levels - or'able flags */
#define DRELOC		2
#define MAP		4
#endif

/* flags for lookup routine */
#define	LOOKUP_NORM	0	/* normal action for a.out undefines */
#define	LOOKUP_SPEC	1	/* special action for a.out undefines */
