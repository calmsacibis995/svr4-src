/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/decl.h	1.10"


typedef unsigned char	Byte;
typedef struct Dnode	Dnode;
typedef struct Member	Member;
typedef struct Snode	Snode;


/* Data alignment
 *	An elf file is defined to have its structures aligned on
 *	appropriate boundaries.  The following type lets the
 *	library test whether the file's alignment meets its own
 *	constraints in memory.  This assumes every machine uses
 *	an alignment that is no greater than an object's size.
 *	The pointer isn't relevant for the file, but the code uses
 *	it to get memory alignment.  ANSI C void * holds any pointer,
 *	making it appropriate here.
 */

typedef union
{
	Elf32_Word	w;
	Elf32_Addr	a;
	Elf32_Off	o;
	Elf_Void	*p;
} Elf32;


/* Memory allocation
 *	Structures are obtained several ways: file mapping,
 *	malloc(), from the user.  A status bit in the structures
 *	tells whether an object was obtained with malloc() and
 *	therefore should be released with free().  The bits
 *	named ...ALLOC indicate this.
 */


/* Data descriptor
 *	db_data must be first in the Dnode structure, because
 *	&db_data must == &Dnode.
 *
 *	db_buf is a pointer to an allocated buffer.  The same value
 *	goes into db_data.d_buf originally, but the user can touch
 *	it.  If the data buffer is not to be freed, db_buf is null.
 *
 *	When "reading" an input file's buffer, the data are left
 *	alone until needed.  When they've been converted to internal
 *	form, the READY flag is set.
 *
 *	db_raw points to a parallel raw buffer.  Raw buffers
 *	have null db_raw.
 */

struct	Dnode
{
	Elf_Data	db_data;
	Elf_Scn		*db_scn;	/* section parent */
	Dnode		*db_next;
	Dnode		*db_raw;	/* raw data */
	off_t		db_off;		/* orig file offset, 0 o/w */
	size_t		db_fsz;		/* orig file size, 0 o/w */
	size_t		db_shsz;	/* orig shdr size, 0 o/w */
	size_t		db_osz;		/* output size for update */
	Elf_Void	*db_buf;	/* allocated data buffer */
	unsigned	db_uflags;	/* user flags: ELF_F_... */
	unsigned	db_myflags;	/* internal flags: DBF_... */
};

#define DBF_ALLOC	0x1	/* applies to Dnode itself */
#define DBF_READY	0x2	/* buffer ready */


/* Section descriptor
 *	These are sometimes allocated in a block.  If the SF_ALLOC
 *	bit is set in the flags, the Scn address may be passed to free.
 *	The caller must first follow the s_next list to the next freeable
 *	node, because free can clobber the s_next value in the block.
 */

struct	Elf_Scn
{
	Elf_Scn		*s_next;	/* next section */
	Elf		*s_elf; 	/* parent file */
	Dnode		*s_hdnode;	/* head Dnode */
	Dnode		*s_tlnode;	/* tail Dnode */
	Elf32_Shdr	*s_shdr;	/* section header */
	size_t		s_index;	/* section index */
	int		s_err;		/* for delaying data error */
	unsigned	s_shflags;	/* user shdr flags */
	unsigned	s_uflags;	/* user flags */
	unsigned	s_myflags;	/* SF_... */
	Dnode		s_dnode;	/* every scn needs one */
};

#define SF_ALLOC	0x1	/* applies to Scn */


struct	Snode
{
	Elf_Scn		sb_scn;		/* must be first */
	Elf32_Shdr	sb_shdr;
};


/*	A file's status controls how the library can use file data.
 *	This is important to keep "raw" operations and "cooked"
 *	operations from interfering with each other.
 *
 *	A file's status is "fresh" until something touches it.
 *	If the first thing is a raw operation, we freeze the data
 *	and force all cooking operations to make a copy.  If the
 *	first operation cooks, raw operations use the file system.
 */

typedef enum
{
	ES_FRESH = 0,	/* unchanged */
	ES_COOKED,	/* translated */
	ES_FROZEN	/* raw, can't be translated */
} Status;


/* Elf descriptor
 *	The major handle between user code and the library.
 *
 *	Descriptors can have parents: archive members reference
 *	the archive itself.  Relevant "offsets:"
 *
 *	ed_baseoff	The file offset, relative to zero, to the first
 *			byte in the file.  For all files, this gives
 *			the lseek(fd, ed_baseoff, 0) value.
 *
 *	ed_memoff	The offset from the beginning of the nesting file
 *			to the bytes of a member.  For an archive member,
 *			this is the offset from the beginning of the
 *			archive to the member bytes (not the hdr).  If an
 *			archive member slides, memoff changes.
 *
 *	ed_siboff	Similar to ed_memoff, this gives the offset from
 *			the beginning of the nesting file to the following
 *			sibling's header (not the sibling's bytes).  This
 *			value is necessary, because of archive sliding.
 *
 *	ed_nextoff	For an archive, this gives the offset of the next
 *			member to process on elf_begin.  That is,
 *			(ed_ident + ed_nextoff) gives pointer to member hdr.
 *
 *	Keeping these absolute and relative offsets allows nesting of
 *	files, including archives within archives, etc.  The only current
 *	nesting file is archive, but others might be supported.
 *
 *	ed_image	This is a pointer to the base memory image holding
 *			the file.  Library code assumes the image is aligned
 *			to a boundary appropriate for any object.  This must
 *			be true, because we get an image only from malloc
 *			or mmap, both of which guarantee alignment.
 */

struct Elf
{
	Elf		*ed_parent;	/* archive parent */
	size_t		ed_activ;	/* activation count */
	int		ed_fd;		/* file descriptor */
	Status		ed_status;	/* file's memory status */
	off_t		ed_baseoff;	/* base file offset, zero based */
	size_t		ed_memoff;	/* offset within archive */
	size_t		ed_siboff;	/* sibling offset with archive */
	size_t		ed_nextoff;	/* next archive member hdr offset */
	char		*ed_image;	/* pointer to file image */
	size_t		ed_imagesz;	/* # bytes in ed_image */
	char		*ed_ident;	/* file start, getident() bytes */
	size_t		ed_identsz;	/* # bytes for getident() */
	char		*ed_raw;	/* raw file ptr */
	size_t		ed_fsz;		/* file size */
	unsigned	*ed_vm;		/* virtual memory map */
	size_t		ed_vmsz;	/* # regions in vm */
	unsigned	ed_encode;	/* data encoding */
	unsigned	ed_version;	/* file version */
	int		ed_class;	/* file class */
	Elf_Kind	ed_kind;	/* file type */
	Elf32_Ehdr	*ed_ehdr;	/* elf header */
	Elf_Void	*ed_phdr;	/* phdr table */
	size_t		ed_phdrsz;	/* sizeof phdr table */
	Elf32_Shdr	*ed_shdr;	/* shdr table */
	Elf_Scn		*ed_hdscn;	/* head scn */
	Elf_Scn		*ed_tlscn;	/* tail scn */
	Member		*ed_memlist;	/* list of archive member nodes */
	Member		*ed_armem;	/* archive member header */
	Elf_Void	*ed_arsym;	/* archive symbol table */
	size_t		ed_arsymsz;	/* archive symbol table size */
	size_t		ed_arsymoff;	/* archive symbol table hdr offset */
	char		*ed_arstr;	/* archive string table */
	size_t		ed_arstrsz;	/* archive string table size */
	size_t		ed_arstroff;	/* archive string table hdr offset */
	unsigned	ed_myflags;	/* EDF_... */
	unsigned	ed_ehflags;	/* ehdr flags */
	unsigned	ed_phflags;	/* phdr flags */
	unsigned	ed_uflags;	/* elf descriptor flags */
};

#define EDF_ASALLOC	0x1	/* applies to ed_arsym */
#define EDF_EHALLOC	0x2	/* applies to ed_ehdr */
#define EDF_PHALLOC	0x4	/* applies to ed_phdr */
#define EDF_SHALLOC	0x8	/* applies to ed_shdr */
#define EDF_COFFAOUT	0x10	/* original file was coff a.out */
#define EDF_RAWALLOC	0x20	/* applies to ed_raw */
#define EDF_READ	0x40	/* file can be read */
#define EDF_WRITE	0x80	/* file can be written */


typedef enum
{
	OK_YES = 0,
	OK_NO = ~0
} Okay;


#undef _
#undef const
#ifdef __STDC__
#	define _(a)	a
#	undef ftruncate
#	undef lseek
#	undef mmap
#	undef msync
#	undef munmap
#	undef read
#	undef uname
#	undef write

#	define ftruncate	_ftruncate
#	define lseek		_lseek
#	define mmap		_mmap
#	define msync		_msync
#	define munmap		_munmap
#	define read		_read
#	define uname		_uname
#	define write		_write
#else
#	define _(a)	()
#	define const
#endif

Member		*_elf_armem	_((Elf *, char *, size_t));
void		_elf_arinit	_((Elf *));
Okay		_elf_cook	_((Elf *));
Okay		_elf_cookscn	_((Elf *, size_t));
Dnode		*_elf_dnode	_((void));
size_t		_elf32_entsz	_((Elf32_Word, unsigned));
Okay		_elf_inmap	_((Elf *));
char		*_elf_outmap	_((int, size_t, unsigned *));
size_t		_elf_outsync	_((int, char *, size_t, unsigned));
size_t		_elf32_msize	_((Elf_Type, unsigned));
Elf_Type	_elf32_mtype	_((Elf32_Word, unsigned));
char		*_elf_read	_((int, off_t, size_t));
Snode		*_elf_snode	_((void));
int		_elf_svr4	_((void));
void		_elf_unmap	_((char *, size_t));
Okay		_elf_vm		_((Elf *, size_t, size_t));

extern int		_elf_byte;
extern const Elf32_Ehdr	_elf32_ehdr_init;
extern unsigned		_elf_encode;
extern int		_elf_err;
extern const Snode	_elf_snode_init;
extern unsigned		_elf_work;


/* C, OS stuff
 *	Some of the following declarations should come from standard
 *	header files, but those header files don't exist everywhere.
 */

Elf_Void	free		_((void *));
int		ftruncate	_((int, off_t));
long		lseek		_((int, long, int));
Elf_Void	*malloc		_((size_t));
int		memcmp		_((const void *, const void *, size_t));
Elf_Void	*memcpy		_((void *, const void *, size_t));
Elf_Void	*memset		_((void *, int, size_t));
char		*mmap		_((char *, size_t, int, int, int, off_t));
int		msync		_((char *, size_t, int));
int		munmap		_((char *, size_t));
int		read		_((int, void *, int));
int		uname		_((void *));
int		write		_((int, const void *, int));
