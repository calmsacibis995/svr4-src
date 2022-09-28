/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/globals.h	1.31"

/*
 * Include file for ld
 */

#ifndef	GLOBALS_DOT_H
#define	GLOBALS_DOT_H

/************************************************************
** Imports
************************************************************/

#ifdef	__STDC__
#include	<stdlib.h>
#endif	/* __STDC__ */

#include	"machdep.h"
#include	"libelf.h"

#ifndef	NULL
#define	NULL	0
#endif

/************************************************************
** C Version Dependencies
************************************************************/

/*
** const is only known in ANSI C
*/
#ifdef __STDC__
#define	CONST	const
#else
#define	CONST
#endif

/*
 * The PROTO macro is used to declare prototypes for ANSI C but consumes its
 * second argument in pre-ANSI C.  This is declared here so that we can use it
 * in declaring prototypes below.  LPROTO is the local prototype version.
 */
#ifdef	__STDC__
#define	PROTO(F, A)	extern F A
#define LPROTO(F, A)	static F A
#else
#define	PROTO(F, A)	extern F()
#define	LPROTO(F, A)	static F()
#endif

/*
 * The OLD_C macro expands to its argument in pre-ANSI C and to
 * nothing in ANSI C.  This macro is used in cases such as the
 * following:
 *
 * 		OLD_C(extern int strlen();)
 *
 * where this function will be declared in the standard C headers,
 * but not the old C headers.
 */
#ifdef	__STDC__
#define	OLD_C(x)
#else
#define	OLD_C(x)	x
#endif

OLD_C(PROTO(int close, (int));)
OLD_C(PROTO(void exit, (int));)
OLD_C(PROTO(char *getenv, (char *));)
OLD_C(PROTO(long lseek, (int, long, int));)
OLD_C(PROTO(int read, (int, CONST char*, unsigned int));)
OLD_C(PROTO(unsigned long umask, (int));)
OLD_C(PROTO(int unlink, (CONST char*));)

/*
 * In ANSI C we can use void* things but in pre-ANSI C we replace them
 * with char*
 */
#ifdef	__STDC__
#define	VOID	void
#else
#define	VOID	char
#endif

/*
/*
 * EXIT_SUCCESS and EXIT_FAILURE are only available in ANSI C
 */
#ifndef	EXIT_SUCCESS
#define	EXIT_SUCCESS	0
#define EXIT_FAILURE	1
#endif

/************************************************************
** Macros
************************************************************/

/*
 * Get a new dynamically-allocated item T
 */
#define	NEWZERO(T)	((T*) new_calloc(sizeof (T)))

/*
 * Rounding
 */

/* round A to next B boundary */
#define ROUND(A, B) (((A) + ((B)?(B):1) - 1) & ~(((B)?(B):1) - 1))

#ifdef	DEBUG
/*
 * Print a debugging message
 * Usage: DPRINTF(DBG_MAIN, (MSG_DEBUG, "I am here"));
 */
#define	DPRINTF(D,M)	if (debug_bits[D]) lderror M
#else
#define	DPRINTF(D,M)
#endif

/*
 * LIST_TRAVERSE(List* L, Listnode* N, VOID* D)
 * is used as the only "argument" of a "for" loop to traverse a linked list.
 * The node pointer N is set to each node in turn and tthe corresponding data
 * pointer is copied to D.
 * The macro is used as in
 *   for (LIST_TRAVERSE(some_list, np, item)) {
 *      process(item);
 *   }
 */
#ifdef __STDC__
#define	LIST_TRAVERSE(L, N, D) \
	(void) (((N) = (L)->head) != NULL && ((D) = (N)->data) != NULL); \
	(N) != NULL; \
	(void) (((N) = (N)->next) != NULL && ((D) = (N)->data) != NULL)
#else
#define LIST_TRAVERSE(L, N, D) \
	(void) (((N) = (L)->head) != NULL && ((VOID*)(D) = (N)->data) != NULL); \
	(N) != NULL; \
	(void) (((N) = (N)->next) != NULL && ((VOID*)(D) = (N)->data) != NULL)
#endif

/************************************************************
** Typedefs
************************************************************/


typedef enum {
	FALSE = 0,
	TRUE = 1
} Boolean;

/*
 * Some flags can have three states
 */
typedef enum {
	NOT_SET = -1,
	SET_FALSE = 0,
	SET_TRUE = 1
} Setstate;

/*
 * Types of symbols
 */
typedef enum {
	REF_UNK,		/* .so symbol */
	REF_SEEN,		/* .so symbol, resolves undef global from */
				/* .so only */
	REF_DEFN,		/* .so symbol, resolves undef global from .o */
	REF_RELOBJ,		/* .o symbol */
	REF_DYN			/* .o symbol, resolves undef global from .so */
} Deflevel;

/*
 * Types of errors
 */
typedef enum {
#ifdef	DEBUG
	MSG_DEBUG,
#endif	/* DEBUG */
	MSG_NOTICE,
	MSG_WARNING,
	MSG_FATAL,
	MSG_ELF,
	MSG_SYSTEM
} Errorlevel;

/*
 * Types of segment attributes
 */
typedef enum {
	SGA_VADDR,
	SGA_PADDR,
	SGA_LENGTH,
	SGA_ALIGN,
	SGA_FLAGS,
	SGA_TYPE
} Segment_attr;

#ifdef	DEBUG
/*
 * Debugging levels
 */
typedef enum {
	DBG_ARGS,
	DBG_FILES,
	DBG_GLOBALS,
	DBG_LIBS,
	DBG_MAIN,
	DBG_MAP,
	DBG_OUTFILE,
	DBG_RELOC,
	DBG_SECTIONS,
	DBG_SYMS,
	DBG_UPDATE,
	DBG_UTIL,
	/* ----------- MUST BE LAST!!! ----------- */
	DBG_MAX
} Dbgset;
#endif	/* DEBUG */

typedef struct ent_crit Ent_crit;
typedef struct infile Infile;
typedef struct insect Insect;
typedef struct list List;
typedef struct listnode Listnode;
typedef struct os_desc Os_desc;
typedef struct sg_desc Sg_desc;
typedef struct ldsym Ldsym;

/****************************************
** Constants
****************************************/

/*
 * Two string compare the same with strcmp
 */
#define	SAME	0

#ifndef	FILENAME_MAX
/* The maximum length of a path name. */
#define	FILENAME_MAX	BUFSIZ
#endif

#define	NBKTS	521		/* Number of buckets in the internal symbol hash table */

#define NOHASH	(~(unsigned long)0)		/* No hash value -- this value will never be returned from elf_hash */

/************************************************************
** Structs
************************************************************/

/*
 * Linked list management
 */
struct list {			/* A linked list */
	Listnode *head;		/* The first element */
	Listnode *tail;		/* The last element */
};

struct listnode {		/* A node on a linked list */
	VOID 	 *data;		/* The data item */
	Listnode *next;		/* The next element */
};

/*
 * Input file processing structures
 */

struct infile {			/* Input file control structure */
	CONST char	
		*fl_name;		/* The file name */
	Half	fl_e_type;		/* The corresponding file type from the file header */
	Ldsym	**fl_oldindex;		/* Original symbol table indices */
	Ldsym	*fl_locals;		/* Ldsym version of locals */
	Word	fl_countlocs;		/* Number of local symbols in file symbol table */
	List	fl_insects;		/* List of input sections */
	Insect	**fl_insect_ndx;	/* insect-ndx[scnhdr tbl ndx] = insects ptr */
};

struct insect {			/* Input section descriptor */
	Elf_Scn *is_scn;	/* The section descriptor */
	char	*is_name;	/* Pseudo-sections do not have secname */
				/* string tables */
	Shdr	*is_shdr;	/* The section header */
	Half	is_index;	/* Index of this section header in the file scn */
				/* header table */
	Infile	*is_file_ptr;	/* File struct for this section */
	Os_desc *is_outsect_ptr;	/* Output section for this input section */
	Elf_Data *is_rawbits;	/* Bits from progbits section */
	List	is_rela_list;	/* Reloc sections for this insect */
	Word	is_displ;	/* Displacement w.r.t. output sections */
	Word	is_newFOffset;	/* Offset of section in output file */
	Addr	is_newVAddr;	/* Relocated virtual address of section */
	Elf_Data *is_outdata;	/* Data buffer pointer in the output */
				/* section */
};

/*
 * Map file and output file processing structures
 */

struct os_desc {		/* Output section descriptor */
	Elf_Scn *os_scn;		/* The section descriptor */
	CONST char	
		*os_name;		/* The section name */
	Shdr	*os_shdr;		/* The section header */
	Word	os_szinrel;		/* Size of input relocations */
	Insect	*os_outrels;		/* Pointer to output relocation section */
	Word	os_szoutrel;		/* Size of output relocation section */
	Word	os_szcopyrels;		/* Size of output copy relocations */
	Word	os_ndxsectsym;		/* Index in output symtab of Section symbol for this section */
	List	os_insects;		/* List of input sections in this output section */
};

struct sg_desc {		/* Output segment descriptor */
	Phdr	sg_phdr;		/* Segment head for output file */
	char	*sg_name;		/* Segment name */
	Word	sg_length;		/* Maximum length of segment; 0 == not specified */
	List	sg_osectlist;		/* List of output section descriptors */
	unsigned long 
		sg_flags;		/* Bits set to TRUE when attributes are specified */
	Ldsym	*sg_sizesym;		/* Points to the size symbol for this segment. */
};	


struct ent_crit {		/* Entrance criteria */
	List	ec_files;		/* Files from which to accept sections */
	char	*ec_name;		/* Name to match (NULL if none) */
	Word	ec_type;		/* Section type */
	Half	ec_attrmask;		/* Section attribute mask (AWX) */
	Half	ec_attrbits;		/* Sections attribute bits */
	Sg_desc	*ec_segment;		/* Output segment to enter if matched */
};

struct ldsym {			/* Symbol table node */
	Sym	*ls_syment;		/* pointer to file symtab representation of a symbol */
	CONST char	
		*ls_name;		/* Keep a copy of it here for performance */
	unsigned long
		ls_hashval;		/* The pure hash value of this symbol */
	Deflevel
		ls_deftag;		/* `Level' of this definition */
	Infile	*ls_flptr;		/* File in which the symbol is found */
	Insect	*ls_scnptr;		/* Input section of symbol definition */
	Word	ls_outndx;		/* Index in output symbol table */
	Word	ls_GOTndx;		/* Index into global offset table for this symbol */
	Word	ls_PLTndx;		/* Index into procedure linkage table for this symbol */
	Word	ls_PLTGOTndx;		/* index of the GOT entry for the PLT indirection */
	Boolean	ls_COPYalloc;		/* has copy reloc entry been allocate for this symbol */
	Boolean	ls_PLTdone;		/* plt entry created for this symbol */
	Boolean	ls_GOTdone;		/* got entry created for this symbol */
	Boolean	ls_COPYdone;		/* copy reloc entry created for this symbol */
};

/************************************************************
** Global Variables
************************************************************/

#ifdef	DEBUG
extern Boolean debug_bits[DBG_MAX]; /* Debugging flag bits array */
#endif	/* DEBUG */

/* command line flags */
extern Boolean
	aflag,			/* Make the output executable */
	bflag,			/* Special handling for PIC reloc */
	dmode,			/* Default to dynamic mode */
	mflag,			/* Generate a memory map */
	rflag,			/* Retain relocation */
	sflag,			/* Strip output debugging information */
	tflag,			/* No warnings about multiple defs */
	zdflag,			/* Fatal error on undefined symbols */
	znflag,			/* Undefined symbols allowed */
	ztflag,			/* Error on text reloc */
	Gflag,			/* Shared object */
	Bflag_symbolic,		/* -B Symbolic seen */
	Bflag_dynamic;		/* Type of lib {.a,.so} to search for */
				/* (TRUE = dynamic, FALSE = static) */

extern Setstate	Qflag;			/* Add version stamp */

extern Word	bss_align;		/* Maximum alignments of tentatives */ 

extern Word	copyrels;		/* counter for copy relocs */

extern Word	countGOT;		/* Total number of GOT entries in output file */

extern Word	countPLT;		/* Total number of PLT entries in output file */

extern Word	count_dynglobs	;	/* Number of subset symtab global symbols */

extern Word	count_dynstrsize;	/* Size in bytes of output subset symtab strtab */

extern Word	count_namelen;		/* Length of strings making up names of output sections */

extern Word	count_osect;		/* Number of output sections */

extern Word	count_outglobs;		/* Number of output global symbols */

extern Word	count_outlocs;		/* Number of output local symbols */

extern Word	count_rela;		/* Total number of relocation entries in output file */

extern Word	count_strsize;		/* Size in bytes of output symtab strtab */

extern Ehdr	*cur_file_ehdr; 	/* Ehdr of current file */

extern int	cur_file_fd;		/* File descriptor of current file */

extern char	*cur_file_name;		/* Name of the current input file */

extern Elf	*cur_file_ptr;		/* Pointer to ELF descriptor of current file */

extern Infile	*cur_infile_ptr;	/* Current Infile pointer */

extern char	*dynoutfile_name;	/* (-h option) output file name for dynamic structure */

extern Word	ehdr_flags;		/* flags required for the a.out elf header */

extern char	*entry_point;		/* The user-supplied program entry point */

extern Addr	firstexec_seg;		/* The start address of the first loadable executable segment */

extern Addr	firstseg_origin;	/* The start address of the first loadable segment */

extern Word	grels;			/* counter for GOT relocations */

extern List	infile_list;		/* list of infile structures */

extern char	*interp_path;		/* Path of interpreter to be passed to RTL */

extern char	*ld_run_path;		/* runtime library search paths */

extern char	*libdir;		/* pointer to the string from -YL*/

extern Word	libver;			/* Maximum version that libelf can handle */

extern char	*llibdir;		/* pointer to the string from -YU*/

extern char 	*libpath;		/* pointer to string for LIBPATH */

extern Word	orels;			/* counter for output relocations */

extern Ehdr	*outfile_ehdr;		/* Output file elf header descriptor. */

extern Elf	*outfile_elf;		/* Output file elf descriptor. */

extern int	outfile_fd;		/* Output file descriptor. */

extern char	*outfile_name;		/* Name of the output file */

extern Phdr	*outfile_phdr;		/* Output file program header descriptor. */

extern Word	prels;			/* counter for PLT relocations */

extern List	seg_list;		/* list of segments */

extern Word	sizePHDR;		/* Size of the output file PHDR */

extern List	soneeded_list;		/* list of shared object names needed by this a.out */

extern List	symbucket[];		/* the internal hash table */

extern Elf_Data	*symname_bits;		/* the names of symbol table entries */

extern Boolean	textrel;		/* relocations against allocatable non-writable output sections = TRUE */

/************************************************************
** Globals variables defined sections.c
************************************************************/

extern Insect 	*dynamic_sect;		/* pointer to dynamic section */

extern Insect	*first_rel_sect;	/* First output relocation section */

extern Insect	*hash_sect;		/* pointer to hash table section */

extern Insect	*interp_sect;		/* pointer to interp section */

extern Insect	*pltrel_sect;		/* insect struct for plt relocations */

extern Insect	*bss_sect;		/* insect struct for bss */

extern Word	dynbkts;		/* the number of buckets in the hashed dynsym table */
					/* or, for a shared object, in the hashed symbol table */

extern Insect	*dynstrtab_sect;	/* insect struct for subset symtab strtab */

extern Insect	*dynsymtab_sect;	/* insect struct for output file subset symbol table */

extern Insect	*got_sect;		/* insect struct for got section */

extern Insect	*plt_sect;		/* insect struct for plt section */

extern Insect	*shstrtab_sect;		/* insect struct for section header strtab */

extern Insect	*strtab_sect;		/* insect struct for symtab strtab */

extern Insect	*symtab_sect;		/* insect struct for output file symbol table */

/************************************************************
** Function Declarations
************************************************************/

PROTO(void add_libdir, (CONST char *));
PROTO(void add_usym, (CONST char *));
PROTO(void build_specsym, (CONST char *, CONST char *));
PROTO(void check_flags, (int));
PROTO(void count_relentries, (void));
PROTO(void count_sect, (Insect*, Insect*, Os_desc*));
PROTO(void ecrit_setup, (void));
PROTO(void fillin_gotplt, (void));
PROTO(Infile *find_infile, (char *));
PROTO(void find_library, (CONST char *));
PROTO(void finish_out, (void));
PROTO(void init_signals, (void));
PROTO(void lderror, (Errorlevel, char *, ...));
PROTO(void ldmap_out, (void));
PROTO(void lib_setup, (void));
PROTO(Listnode *list_append, (List *, CONST VOID *));
PROTO(Listnode *list_insert, (Listnode *, CONST VOID *));
PROTO(Listnode *list_prepend, (List *, CONST VOID *));
PROTO(void make_bss, (void));
PROTO(void make_comment, (void));
PROTO(void make_dyn, (void));
PROTO(void make_got, (Word));
PROTO(void make_hash, (void));
PROTO(void make_interp, (void));
PROTO(void make_plt, (Word));
PROTO(void make_shstrtab, (void));
PROTO(void make_strtab, (void));
PROTO(void make_symtab, (void));
PROTO(void make_dynstrtab, (void));
PROTO(void make_dynsymtab, (void));
PROTO(void map_parse, (char *));
PROTO(Elf *my_elf_begin, (int, Elf_Cmd, Elf *));
PROTO(void my_elf_cntl, (Elf *, Elf_Cmd));
PROTO(int my_elf_end, (Elf *));
PROTO(size_t my_elf_fsize, (Elf_Type, size_t, unsigned));
PROTO(Elf_Arhdr *my_elf_getarhdr, (Elf *));
PROTO(Elf_Arsym *my_elf_getarsym, (Elf *, size_t *));
PROTO(Elf_Data *my_elf_getdata, (Elf_Scn *, Elf_Data *));
PROTO(Elf_Data *my_elf_newdata, (Elf_Scn *));
PROTO(Ehdr *my_elf_getehdr, (Elf *));
PROTO(char *my_elf_getident, (Elf *, size_t *));
PROTO(Phdr *my_elf_newphdr, (Elf *, size_t));
PROTO(Elf_Scn *my_elf_getscn, (Elf *,size_t));
PROTO(size_t my_elf_ndxscn, (Elf_Scn *));
PROTO(Elf_Scn *my_elf_newscn, (Elf *));
PROTO(Shdr *my_elf_getshdr, (Elf_Scn *));
PROTO(size_t my_elf_rand, (Elf *,size_t));
PROTO(char* my_elf_strptr, (Elf *, size_t, size_t));
PROTO(void my_elf_update, (Elf *, Elf_Cmd));
PROTO(VOID *mycalloc, (unsigned int));
PROTO(VOID *mymalloc, (unsigned int));
PROTO(VOID *new_calloc, (unsigned int));
PROTO(void open_out, (void));
PROTO(void place_section, (Insect *));
PROTO(void process_archive, (void));
PROTO(void process_files, (int, char **));
PROTO(void process_flags, (int, char **));
PROTO(void process_infile, (char *));
PROTO(void process_relobj, (void));
PROTO(void process_shobj, (char *));
PROTO(void process_symtab, (Elf_Scn *));
PROTO(void relocate, (void));
PROTO(void reloc_sect, (Insect*, Insect*, Os_desc*));
PROTO(void set_off_addr, (void));
PROTO(Ldsym *sym_enter, (Sym *, CONST char *, unsigned long));
PROTO(Ldsym *sym_find, (CONST char *, unsigned long ));
PROTO(void update_syms, (void));
PROTO(void add_undefs_to_dynsymtab, (void));

#ifdef	DEBUG
PROTO(void libdir_list_print, (void));
PROTO(void print_globals, (void));
PROTO(void mapprint, (char*));
#endif	/* DEBUG */

#endif	/* GLOBALS_DOT_H */
