/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/obj.c	1.25"

#include <stdio.h>
#include <string.h>
#include <sgs.h>
#include <syms.h>
#include <libelf.h>
#include "systems.h"
#include "symbols.h"
#include "gendefs.h"
#include "section.h"
#if M32
#include <sys/elf_M32.h>
#else 	/* i386 */
#include <sys/elf_386.h>
#endif


/*
 *
 *	"obj.c" is a file that contains the routines for
 *	creating the object file during the final pass of the assembler.
 *	These include an assortment of routines for putting
 *	out header information, relocation and line number entries, and
 *	other data that is not part of the object program itself.  The
 *	main routines for producing the object program from the
 *	temporary files can be found in the file "codout.c".
 *
 */

/*
 *
 *	"outblock" is a function that outputs a block "block" with size
 *	"size" bytes to the file whose descriptor appears in "file".
 *
 */

#define outblock(a,b,c)	(void) fwrite((char *)(a),b,1,c)

/*
 *
 *	"inblock" is a function that reads a block "block" of size
 *	"size" bytes from the file whose descriptor appears in "file".
 *
 */

#define inblock(a,b,c)	(void) fread((char *)(a),b,1,c)


extern void free();
extern void traverse();
extern unsigned long addstr();
extern unsigned short line;
extern symbol *cfile;

#if	M32
extern int need_mau;	/* indicates that the mau flag should be set */
extern int warlevel;	/* indicates level of workarounds generated */
#endif

#if	M32
#define E_BYTE_ORDER	ELFDATA2MSB
#define E_MACHINE	EM_M32
#endif

extern FILE
	*fdrel;

extern Elf_Scn *elf_string_section;
extern Elf *elffile;

extern symbol *lookup();
extern void aerror();
extern void elferror();


extern int seccnt;



#define STRING_SECTION 1	


symbol	*dot;
static unsigned long first_global;


/*
 *
 * write_elf_header generates the object file header.  It
 * uses the elf access library function elf32_newehdr to
 * generate a file header descriptor and sets the following
 * fields:
 *	e_type		identifies object file to be a
 *			relocatable file
 *	e_machine	specifies the architecture
 *	e_version	identifies the object file version
 *	e_entry		entry point
 *	e_flags		
 *	e_shstrndx	section header table index of the
 *			section name string table
 *
 *	All other fields are provided by the access library.
 *
 */

void 
write_elf_header(elffile)

Elf *elffile;
{
  	Elf32_Ehdr *elf_ehdr;
	if ((elf_ehdr = elf32_newehdr(elffile)) == NULL)
		elferror("libelf error: newehdr:");
	elf_ehdr->e_ident[EI_DATA] = E_BYTE_ORDER;
	elf_ehdr->e_type = ET_REL;
	elf_ehdr->e_machine = E_MACHINE;
	elf_ehdr->e_version = EV_CURRENT;
	elf_ehdr->e_entry = 0;
	elf_ehdr->e_flags = GET_E_FLAGS;
	elf_ehdr->e_shstrndx = STRING_SECTION;
}


/*
 *
 *	"reloc" is a function that reads preliminary relocation entries
 *	from the file whose descriptor appears in "fdrel", processes
 *	them to produce the final relocation entries, and writes them
 *	out to the file whose descriptor appears in "fdout".
 *
 *	This function loops through the section info table, sectab[],
 *   	and  reads preliminary relocation entries in a loop. When it
 *	finds a section with relocation entries, a relocation section
 *	is created.
 *	A preliminary relocation entry consists of an address, followed
 *	by a tab, followed by the symbol with respect to which the
 *	relocation is to take place, followed by a tab, followed by
 *	the relocation type. A symbol table lookup is performed
 * 	determine the symbol table index of the symbol to which 
 * 	the relocation is to take place.
 *
 */

static Elf_Scn *elf_symbol_section;
static Elf_Scn *reloc_section;
static Elf32_Rel *relent_buffer;
static int rel_bufsize;

static void
elf_bldreloctab(relent, syment)
prelent relent;
long syment;
{

	rel_bufsize++;
	(relent_buffer + rel_bufsize)->r_offset = relent.relval;
	(relent_buffer + rel_bufsize)->r_info = ELF32_R_INFO(syment,relent.reltype);
		
}



void
reloc()
{
	register int i;
	register int relentries;
	register struct scninfo	*sect;
	Elf32_Shdr *reloc_sect_hdr;
	Elf_Data *data;
	prelent trelent;
	long syment;
	char *name;


	for (i=2,sect = &sectab[2] ; i <= seccnt;i++, sect++) {
		if (sect->s_nreloc > 0) {

			name = (char *) malloc(strlen(sect->name)+5);
			(void) strcpy(name,".rel");
			(void) strcat(name,sect->name);
			if ((reloc_section = elf_newscn(elffile)) == NULL)
				elferror("libelf error: elf_newscn-relocation section:");
			if ((reloc_sect_hdr = elf32_getshdr(reloc_section)) == NULL)
				elferror("libelf error: elf_getshdr:");
			/* enter section header information */
			GET_SECTION(reloc_sect_hdr,name,SHT_REL,0,sizeof(Elf32_Rel));
			if ((reloc_sect_hdr->sh_link = 
			      elf_ndxscn(elf_symbol_section)) == NULL)
				elferror("libelf error: elf_ndxscn:");
			if ((reloc_sect_hdr->sh_info = 
			      elf_ndxscn(sect->s_scn)) == NULL)
				elferror("libelf error: elf_ndxscn:");

			rel_bufsize = -1;
			if ((relent_buffer = (Elf32_Rel *)calloc(sect->s_nreloc,
					     sizeof(Elf32_Rel))) == NULL)
				aerror("cannot allocate relocation buffer");
			relentries= sect->s_nreloc;
			while (relentries) {
				(void) fread((char *)(&trelent),sizeof(prelent),1,fdrel);
				if (trelent.relname == STN_UNDEF)
				/* absolute symbol -- use 0th entry in symbol table*/
				/* which is predefined */
					syment = 0;
				else if ((syment = lookup
					(trelent.relname,N_INSTALL)->stindex) 
						== NULL) {
					  char errmsg[100];

					  line = trelent.lineno;
					  (void) sprintf(errmsg, "Reference to symbol not in symbol table ...\"%s\"",trelent.relname);
					  aerror(errmsg);
				}
				elf_bldreloctab(trelent,syment);
				relentries--;
			}
			if ((data = elf_newdata(reloc_section)) == NULL)
			  elferror("libelf error: elf_newdata:");
			ENTER_DATA(relent_buffer, ELF_T_REL, 
				   (rel_bufsize + 1)*sizeof(Elf32_Rel), 4);
		}
	}
}








static short	sym_bufsize = -1;
static unsigned long symcnt=0;
static Elf32_Sym *sym_buffer;
static Elf_Data *data;

static void
elf_bldsymtab(symptr,name,info)
register symbol *symptr;
register char *name;
register BYTE info;
{
	Elf32_Sym *new_symbol;

	if (!symcnt) {
		if ((sym_buffer = (Elf32_Sym *) calloc(BUFSIZ,sizeof(Elf32_Sym))) == NULL)
			aerror("cannot alloc data for section");
		sym_bufsize++;
	}

	if (FULL(sym_bufsize)) { /* put out symbol buffer */
		if ((data = elf_newdata(elf_symbol_section)) == NULL)
			elferror("libelf error: elf_newdata:");
		ENTER_DATA(sym_buffer, ELF_T_SYM, BUFSIZ * sizeof(Elf32_Sym), 1);
		if ((sym_buffer = (Elf32_Sym *) calloc(BUFSIZ,sizeof(Elf32_Sym))) == NULL)
			aerror("cannot alloc data for section");
		sym_bufsize = -1;
	}
	/* room in buffer */
	new_symbol = sym_buffer + ++sym_bufsize;
	new_symbol->st_name = addstr(name);
	new_symbol->st_value = symptr->value;
	new_symbol->st_size = symptr->size;
	new_symbol->st_info = info;
	new_symbol->st_other = NULL;
	new_symbol->st_shndx = symptr->sectnum;
}


static void
write_locals(ptr)
register symbol *ptr;
{
	register char *strptr;
	unsigned char symtype;

	strptr = ptr->name;

	if (!(GLOBAL(ptr) || WEAK(ptr))) {
		symtype = (ptr->binding << 4) | (ptr->type & 0xf) ;
		if (SECTION(ptr)) {
			elf_bldsymtab(ptr,"", symtype);
			ptr->stindex = ++symcnt;
			return;
		}
		if (BIT_IS_ON(ptr,GO_IN_SYMTAB)) {
			elf_bldsymtab(ptr, strptr, symtype );
			ptr->stindex = ++symcnt;
		}

	}
}

static void
write_globals(ptr)
register symbol *ptr;
{
	register char *strptr;
	unsigned char symtype;

	strptr = ptr->name;

	if (GLOBAL(ptr) || WEAK(ptr)) {
		symtype = (ptr->binding << 4) | (ptr->type & 0xf) ;
		if (COMMON(ptr))
			 ptr->value = ptr->align;
		elf_bldsymtab(ptr,strptr, symtype );
		ptr->stindex = ++symcnt;

	}
}

/* 	write_symbol_table() creates the symbol table and string
*	table sections by first using the Elf access library routines
*	to generate the sections' descriptors and then calling write_locals()
*	and write_nonlocals() to put out local symbols followed by non-local
*	symbols in the object file symbol table.
*/

void
write_symbol_table()
{
	Elf32_Shdr *symtab_hdr;
	/* generate symbol table section header information */
	if ((elf_symbol_section = elf_newscn(elffile)) == NULL)
		elferror("libelf error: elf_newscn-symbol table section:");
	if ((symtab_hdr = elf32_getshdr(elf_symbol_section)) == NULL)
		elferror("libelf error: elf_getshdr:");
	GET_SECTION(symtab_hdr, ".symtab", SHT_SYMTAB, NULL, sizeof(Elf32_Sym));
	if ((symtab_hdr->sh_link = 
	      elf_ndxscn( elf_string_section)) == NULL)
		elferror("libelf error: elf_ndxscn:");
	if (cfile)
                write_locals(cfile);

	traverse(write_locals);
	first_global = symcnt + 1;
	traverse(write_globals);
	symtab_hdr->sh_info = first_global;
	if (sym_bufsize >= NULL) {
		if ((data = elf_newdata(elf_symbol_section)) == NULL)
			elferror("libelf error: elf_newdata-symbol section data:");
		ENTER_DATA(sym_buffer, ELF_T_SYM, (sym_bufsize + 1)*sizeof(Elf32_Sym), 4);
		
	}
}

