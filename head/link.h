/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sgs-inc:common/link.h	1.9"

#ifndef _LINK_H
#define _LINK_H

#include <sys/elftypes.h>

/* declarations for structures contained in the dynamic
 * section of executable files and shared objects
 */

/* The following data structure provides a self-identifying
 * union consisting of a tag from a known list and a value.
 */

typedef struct {
	Elf32_Sword d_tag;		/* how to interpret value */
	union {
		Elf32_Word	d_val;
		Elf32_Addr	d_ptr;
		Elf32_Off	d_off;
	} d_un;
} Elf32_Dyn;

/* valid tag values */

#define	DT_NULL		0	/* last entry in list */
#define	DT_NEEDED	1	/* list of needed objects */
#define	DT_PLTRELSZ	2	/* size of relocations for the PLT */
#define	DT_PLTGOT	3	/* addresses used by procedure linkage table */
#define	DT_HASH		4	/* hash table */
#define	DT_STRTAB	5	/* string table */
#define	DT_SYMTAB	6	/* symbol table */
#define	DT_RELA		7	/* addr of relocation entries */
#define	DT_RELASZ	8	/* size of relocation table */
#define	DT_RELAENT	9	/* base size of relocation entry */
#define	DT_STRSZ	10	/* size of string table */
#define	DT_SYMENT	11	/* size of symbol table entry */
#define	DT_INIT		12	/* _init addr */
#define	DT_FINI		13	/* _fini addr */
#define	DT_SONAME	14	/* name of this shared object */
#define	DT_RPATH	15	/* run-time search path */
#define DT_SYMBOLIC	16	/* shared object linked -Bsymbolic */
#define	DT_REL		17	/* addr of relocation entries */
#define	DT_RELSZ	18	/* size of relocation table */
#define	DT_RELENT	19	/* base size of relocation entry */
#define	DT_PLTREL	20	/* relocation type for PLT entry */
#define	DT_DEBUG	21	/* pointer to r_debug structure */
#define	DT_TEXTREL	22	/* text relocations for this object */
#define DT_JMPREL	23	/* pointer to the PLT relocation entries */

#define	DT_FPATH	-1	/* pathname of file to be opened */
#define	DT_MODE		-2	/* function binding mode */
#define	DT_MAP		-3	/* pointer to link_map */
#define	DT_MAP_NODELETE		-4	/* pointer to link_map */

#define DT_MAXPOSTAGS	24	/* number of positive tags */
#define DT_MAXNEGTAGS	3	/* number of negative tags */

#define DT_LOPROC	0x70000000	/* processor specific range */
#define DT_HIPROC	0x7fffffff



/* public structure defined and maintained within the run-time 
 * linker
 */

struct link_map {
	unsigned long l_addr;	/* address at which object is mapped */
	char *l_name;		/* full name of loaded object */
	Elf32_Dyn *l_ld;	/* dynamic structure of object */
	struct link_map *l_next;	/* next link object */
	struct link_map *l_prev;	/* previous link object */
};

struct r_debug {
	int r_version;		/* debugging info version no. */
	struct link_map *r_map;	/* address of link_map */
	unsigned long r_brk;	/* address of update routine */
	enum { RT_CONSISTENT, RT_ADD, RT_DELETE } r_state;
	unsigned long r_ldbase;	/* base addr of ld.so */
};

#endif  /* _LINK_H */
