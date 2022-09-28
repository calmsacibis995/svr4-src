/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/outfile.c	1.41"
/*
 * Write the a.out file.
 *
 * The a.out file format is fully described in the ABI.  Below is a short 
 * summary of format to be produced by ld.
 * 
 * 	EHDR
 * 	[PHDR]			Only present if the file is executable.
 * 	LOADABLE SECTIONS
 *	...
 * 	NON-LOADABLE SECTIONS	Comments, debug, etc.
 * 	...
 * 	SYMTAB
 * 	STRTAB
 * 	RELOC SECTIONS		Only present if the file is relocatable (-r).
 * 	...
 * 	SHSTRTAB
 * 	SHDR
 */

/************************************************************
 * Imports
 ***********************************************************/

#include	"sgs.h"
#include	"link.h"
#include	"globals.h"
#include	"macros.h"
#include	"ccstypes.h"
#include	<fcntl.h>

/************************************************************
 * Local Variable Definitions
 ***********************************************************/

static char *strtab_strptr;	/* used to build the symbol table string table  */
static char *dynstrtab_strptr;	/* used to build the subset symbol table string table  */

/************************************************************
 * Local Functions Declarations
 ***********************************************************/

LPROTO(void build_odynsym, (void));
LPROTO(void build_ophdr, (void));
LPROTO(void build_osymtab, (void));
LPROTO(void finish_dyn, (void));

/************************************************************
 * Local Functions Definitions
 ***********************************************************/


/* Build the output file dynamic symbol table. */
static void 
build_odynsym()
{
	Listnode	*np1;		/* Node pointer. */
	long		bkt;		/* Current bucket in the symbol hash table. */
	Ldsym		*sym;		/* Current symbol in the symbol hash table. */
	register Sym	*dynsymptr;	/* current symbol being entered in output dynsym symtab */
	Word		*hashtab;	/* Pointer to the hash table */
	Word		*bucket;	/* Pointer to the first bucket */
	Word		*chain;		/* Pointer to the hash table chains */
	Word		first_chain;	/* Index of the first free chain slot */
	Word		hashval;	/* Value returned by the hash function */
	
	/* set up the hash table for the dynsym symbol table */
	hashtab = (Word*) (hash_sect->is_rawbits->d_buf);
	bucket = &hashtab[2];
	chain = &hashtab[2 + dynbkts];
	hashtab[0] = dynbkts;
	hashtab[1] = count_dynglobs + 1;

	dynsymptr =  (Sym*)dynsymtab_sect->is_rawbits->d_buf + 1;
	dynstrtab_strptr = (char*)dynstrtab_sect->is_rawbits->d_buf + 1;

	/* the index of the first global symbol in the dynsym table is 1. */

	first_chain = 1;
	dynsymtab_sect->is_outsect_ptr->os_shdr->sh_info = 1;

	/* We must now traverse ld's internal symbol table and add the */
	/* appropriate symbols to the dynsym symbol table we are */
	/* constructing and hash these symbols only. */
	for (bkt = 0; bkt < NBKTS; bkt++)
		for (LIST_TRAVERSE(&(symbucket[bkt]), np1, sym)) {
			if(sym->ls_deftag == REF_DYN || sym->ls_deftag == REF_DEFN){
				
				*dynsymptr = *(sym->ls_syment);
				dynsymptr->st_name = dynstrtab_strptr - (char*)dynstrtab_sect->is_rawbits->d_buf;
				strcpy(dynstrtab_strptr,sym->ls_name);
				dynstrtab_strptr += strlen(sym->ls_name) + 1;
				if( sym->ls_flptr &&
					(sym->ls_flptr->fl_e_type == ET_DYN) &&
					(ELF_ST_TYPE(sym->ls_syment->st_info) == STT_FUNC)
					&& (sym->ls_PLTdone != FALSE) )
					dynsymptr->st_value = plt_sect->is_newVAddr +
						sym->ls_PLTndx * PLTENTSZ;
				else
					dynsymptr->st_value = 
						sym->ls_syment->st_value;
				dynsymptr++;

				hashval = sym->ls_hashval % dynbkts;
				chain[first_chain] = bucket[hashval];
				bucket[hashval] = first_chain;
				first_chain++;
			}

		}

	/* We can now fill in the dynamic string table index for the dynsym symbol table */

	dynsymtab_sect->is_outsect_ptr->os_shdr->sh_link = my_elf_ndxscn(dynstrtab_sect->is_outsect_ptr->os_scn);

}


/* Build the output file program header. */
static void 
build_ophdr()
{
	register Listnode	*np;		/* Node pointer. */
	register Sg_desc	*sgp;		/* Segment descriptor pointer. */
	int			nseg = 0;	/* Number of segments seen so far. */
	Ldsym			*entry;		/* ld symbol table entry for entry_point */
	
	/* fill in the entry point */

	if(entry_point != NULL) {
		if((entry = sym_find(entry_point, NOHASH)) == NULL )
			lderror(MSG_FATAL,
				" entry point symbol `%s` is undefined", entry_point);
		outfile_ehdr->e_entry = entry->ls_syment->st_value;
	} else if (( entry = sym_find("_start", NOHASH)) != NULL) {
		outfile_ehdr->e_entry = entry->ls_syment->st_value;
	} else if (( entry = sym_find("main", NOHASH)) != NULL) {
		outfile_ehdr->e_entry = entry->ls_syment->st_value;
	} else {
		outfile_ehdr->e_entry = firstexec_seg;
	}

	/* Traverse the segment list and add a phdr table entry for each */
	/* non-PT_NULL segment found. */
	if( aflag || dmode ){
		for (LIST_TRAVERSE(&seg_list, np, sgp)) {
			if ( (dmode && 
				    ((sgp->sg_phdr.p_type == PT_DYNAMIC)
				    || (sgp->sg_phdr.p_type == PT_PHDR && !Gflag)
				    ||(sgp->sg_phdr.p_type == PT_INTERP &&!Gflag)))
				|| (sgp->sg_phdr.p_type != PT_NULL && sgp->sg_osectlist.head)
				|| ( (aflag || (dmode && !Gflag)) && sgp->sg_phdr.p_type == PT_SHLIB
					&& sgp->sg_osectlist.head))
				outfile_phdr[nseg++] = sgp->sg_phdr;
		}
	}
}

/* Build the output file symbol table. */
static void 
build_osymtab()
{
	Listnode	*np1;		/* Node pointer. */
	Sg_desc		*sgp;		/* Segment descriptor pointer. */
	Listnode	*np2;		/* Node pointer. */
	Os_desc		*osp;		/* Output section pointer. */
	long		bkt;		/* Current bucket in the symbol hash table. */
	Ldsym		*sym;		/* Current symbol in the symbol hash table. */
	register Sym	*symptr;	/* current symbol being entered in output symtab */
	Infile		*ifp;		/* pointer used to traverse the list of input files */
	Word		*hashtab;	/* Pointer to the hash table */
	Word		*bucket;	/* Pointer to the first bucket */
	Word		 *chain;	/* Pointer to the hash table chains */
	Word		first_chain;	/* Index of the first free chain slot */
	
	if (Gflag) { /* will hash the large symbol table */ 
		hashtab = (Word*) (hash_sect->is_rawbits->d_buf);
		bucket = &hashtab[2];
		chain = &hashtab[2 + dynbkts];
		hashtab[0] = dynbkts;
		hashtab[1] = count_outglobs + count_outlocs + 3;
	}

	symptr =  (Sym*)symtab_sect->is_rawbits->d_buf + 1;
	strtab_strptr = (char*)strtab_sect->is_rawbits->d_buf + 1;

	/* Add a file symbol to the symtab. */

	symptr->st_info = ELF_ST_INFO(STB_LOCAL, STT_FILE);
	symptr->st_shndx = SHN_ABS;

	/* Add its character string to the string table for the symbol table */

	symptr->st_name = strtab_strptr - (char *)strtab_sect->is_rawbits->d_buf;
	strcpy(strtab_strptr,outfile_name);
	strtab_strptr += strlen(outfile_name) + 1;
	symptr++;

	/* Add a section symbol for each section to the symtab. */
	for (LIST_TRAVERSE(&seg_list, np1, sgp))
		for (LIST_TRAVERSE(&(sgp->sg_osectlist), np2, osp)) {
			symptr->st_info = ELF_ST_INFO(STB_LOCAL, STT_SECTION);
			symptr->st_shndx = my_elf_ndxscn(osp->os_scn);
			symptr->st_value = osp->os_shdr->sh_addr;
			symptr++;
		}

	/* We must now traverse each file's local symbols and add the */
	/* local to the output symbol table */
	/* Only input .o files will have local symbols saved --- */
	/* they were ignored for .so inputs */

	for (LIST_TRAVERSE(&infile_list, np1, ifp)) {

		register Ldsym	*locsym, *els;

		if (ifp->fl_e_type == ET_DYN)
			continue;
		els = &(ifp->fl_locals[1]) + ifp->fl_countlocs;
		for (locsym = &(ifp->fl_locals[1]);locsym < els; ++locsym) {
			if (ELF_ST_TYPE(locsym->ls_syment->st_info) == STT_SECTION)
				continue;

			symptr->st_info = locsym->ls_syment->st_info;
			symptr->st_value = locsym->ls_syment->st_value;
			symptr->st_size = locsym->ls_syment->st_size;
			symptr->st_other = locsym->ls_syment->st_other;
			if (ELF_ST_TYPE(locsym->ls_syment->st_info) == STT_FILE)
				symptr->st_shndx = SHN_ABS;
			else {
				if (locsym->ls_syment->st_shndx == SHN_ABS)
					symptr->st_shndx = SHN_ABS;
				else if (locsym->ls_syment->st_shndx == SHN_UNDEF)
					symptr->st_shndx = SHN_UNDEF;
				else
					symptr->st_shndx = my_elf_ndxscn(locsym->
						ls_scnptr->is_outsect_ptr->os_scn);
			}
			symptr->st_name = strtab_strptr - (char*)strtab_sect->is_rawbits->d_buf;
			strcpy(strtab_strptr, locsym->ls_name);
			strtab_strptr += strlen(locsym->ls_name) + 1;
			symptr++;
		}
	}

	if (Gflag)
		first_chain = symptr - (Sym*) (symtab_sect->is_rawbits->d_buf);

	/* We can now fill in the index of the first global symbol. */
	symtab_sect->is_outsect_ptr->os_shdr->sh_info = count_outlocs+2;

	/* We must now traverse ld's internal symbol table and add the */
	/* appropriate symbols to the output symbol table we are */
	/* constructing. */
	for (bkt = 0; bkt < NBKTS; bkt++)
		for (LIST_TRAVERSE(&(symbucket[bkt]), np1, sym)) {
			if( sym->ls_deftag >= REF_DEFN) {
				*symptr = *(sym->ls_syment);

				symptr->st_name = strtab_strptr - (char*)strtab_sect->is_rawbits->d_buf;
                        	strcpy(strtab_strptr,sym->ls_name);
                        	strtab_strptr += strlen(sym->ls_name) + 1;
				if( dmode && !Gflag && sym->ls_flptr &&
					(sym->ls_flptr->fl_e_type == ET_DYN) &&
					(ELF_ST_TYPE(sym->ls_syment->st_info) == STT_FUNC)
					&& (sym->ls_PLTdone != FALSE) )
					symptr->st_value = plt_sect->is_newVAddr +
						sym->ls_PLTndx * PLTENTSZ;
				else
					symptr->st_value = sym->ls_syment->st_value;
				
				symptr++;

				if (Gflag) {
					Word hashval = sym->ls_hashval % dynbkts;
					chain[first_chain] = bucket[hashval];
					bucket[hashval] = first_chain;
					first_chain++;
				}
			}
		}
	
	/* We can now fill in the string table index for this symbol table. */

	symtab_sect->is_outsect_ptr->os_shdr->sh_link = my_elf_ndxscn(strtab_sect->is_outsect_ptr->os_scn);
}





/*
 * Finish building the dynamic section
 */
static void
finish_dyn()
{
	Listnode	*np1;
	char		*lib;
	int		dyn_entries = 0;
	register Dyn	*dynbits;
	Ldsym		*sym;

	dynbits = (Dyn *)dynamic_sect->is_rawbits->d_buf;

	dynamic_sect->is_outsect_ptr->os_shdr->sh_link = Gflag ?
		my_elf_ndxscn(strtab_sect->is_outsect_ptr->os_scn) :
		my_elf_ndxscn(dynstrtab_sect->is_outsect_ptr->os_scn);

	for (LIST_TRAVERSE(&soneeded_list, np1, lib)) {
		dynbits[dyn_entries].d_tag = DT_NEEDED;
		if(Gflag) {
			dynbits[dyn_entries].d_un.d_val = strtab_strptr - (char *)(strtab_sect->is_rawbits->d_buf);
			(void)strcpy(strtab_strptr, lib);
			strtab_strptr += strlen(lib) + 1;
		} else {
			dynbits[dyn_entries].d_un.d_val = dynstrtab_strptr - (char *)(dynstrtab_sect->is_rawbits->d_buf);
			(void)strcpy(dynstrtab_strptr, lib);
			dynstrtab_strptr += strlen(lib) + 1;
		}
		dyn_entries++;
	}

	if ((sym = sym_find(INIT_SYM, NOHASH)) != NULL) {
		dynbits[dyn_entries].d_tag = DT_INIT;
		dynbits[dyn_entries].d_un.d_ptr = sym->ls_syment->st_value;
		dyn_entries++;
	}

	if ((sym = sym_find(FINI_SYM, NOHASH)) != NULL) {
		dynbits[dyn_entries].d_tag = DT_FINI;
		dynbits[dyn_entries].d_un.d_ptr = sym->ls_syment->st_value;
		dyn_entries++;
	}

	if (Gflag && dynoutfile_name != NULL){
		dynbits[dyn_entries].d_tag = DT_SONAME;
		dynbits[dyn_entries].d_un.d_val = strtab_strptr - (char *)(strtab_sect->is_rawbits->d_buf);
		(void)strcpy(strtab_strptr, dynoutfile_name);
		strtab_strptr += strlen(dynoutfile_name)+1;
		dyn_entries++;
	}

	if (!Gflag && ld_run_path != NULL){
		dynbits[dyn_entries].d_tag = DT_RPATH;
		dynbits[dyn_entries].d_un.d_val = dynstrtab_strptr - (char*)(dynstrtab_sect->is_rawbits->d_buf);
                (void)strcpy(dynstrtab_strptr, ld_run_path);
                dynstrtab_strptr += strlen(ld_run_path)+1;
                dyn_entries++;
        }

	dynbits[dyn_entries].d_tag = DT_HASH;
	dynbits[dyn_entries].d_un.d_ptr = hash_sect->is_newVAddr;
	dyn_entries++;

	dynbits[dyn_entries].d_tag = DT_STRTAB;
	dynbits[dyn_entries].d_un.d_ptr = Gflag ? strtab_sect->is_newVAddr
						: dynstrtab_sect->is_newVAddr;
	dyn_entries++;

	dynbits[dyn_entries].d_tag = DT_SYMTAB;
	dynbits[dyn_entries].d_un.d_ptr = Gflag ? symtab_sect->is_newVAddr
						: dynsymtab_sect->is_newVAddr;
	dyn_entries++;

	dynbits[dyn_entries].d_tag = DT_STRSZ;
	dynbits[dyn_entries].d_un.d_ptr = Gflag ? strtab_sect->is_shdr->sh_size
						: dynstrtab_sect->is_shdr->sh_size;
	dyn_entries++;

	dynbits[dyn_entries].d_tag = DT_SYMENT;
	dynbits[dyn_entries].d_un.d_ptr = Gflag ? symtab_sect->is_shdr->sh_entsize
						: dynsymtab_sect->is_shdr->sh_entsize;
	dyn_entries++;

	if(dmode && !Gflag){
		dynbits[dyn_entries].d_tag = DT_DEBUG;
		dynbits[dyn_entries].d_un.d_ptr = 0;
		dyn_entries++;
	}

	if(textrel){
		dynbits[dyn_entries].d_tag = DT_TEXTREL;
		/* only the presence of this entry is used in this implementation, not the value stored */
		dynbits[dyn_entries].d_un.d_val = 0;
		dyn_entries++;
	}

	if (countPLT != PLT_XNumber) {
		dynbits[dyn_entries].d_tag = DT_PLTGOT;
		dynbits[dyn_entries].d_un.d_ptr = got_sect->is_newVAddr;
		dyn_entries++;
		dynbits[dyn_entries].d_tag = DT_PLTRELSZ;
		dynbits[dyn_entries].d_un.d_ptr = pltrel_sect->is_shdr->sh_size;
		dyn_entries++;
		dynbits[dyn_entries].d_tag = DT_PLTREL;
		dynbits[dyn_entries].d_un.d_ptr = DT_REL_TYPE;
		dyn_entries++;
		dynbits[dyn_entries].d_tag = DT_JMPREL;
		dynbits[dyn_entries].d_un.d_ptr = pltrel_sect->is_newVAddr;
		dyn_entries++;
	}

	if (count_rela != 0) {
		dynbits[dyn_entries].d_tag = DT_REL_TYPE;
		dynbits[dyn_entries].d_un.d_ptr = first_rel_sect->is_newVAddr;
		dyn_entries++;
		dynbits[dyn_entries].d_tag = (DT_REL_TYPE == DT_REL?DT_RELSZ:DT_RELASZ);
		dynbits[dyn_entries].d_un.d_ptr = count_rela;
		dyn_entries++;
		dynbits[dyn_entries].d_tag = (DT_REL_TYPE == DT_REL?DT_RELENT:DT_RELAENT);
		dynbits[dyn_entries].d_un.d_ptr = sizeof(Rel);
		dyn_entries++;
	}

	if (Bflag_symbolic) {
		dynbits[dyn_entries].d_tag = DT_SYMBOLIC;
		dyn_entries++;
	}
	
	dynbits[dyn_entries].d_tag = DT_NULL;
	dynbits[dyn_entries].d_un.d_val = 0;
	dyn_entries++;
}



/************************************************************
 * Global Functions
 ***********************************************************/

/* Finish building the executable file. */
void
finish_out()
{
	if(aflag || dmode)
		build_ophdr();
	if( Gflag || !sflag )
		build_osymtab();
	if( dmode && !Gflag)
		build_odynsym();
	if (dmode)
		finish_dyn();
	outfile_ehdr->e_shstrndx = my_elf_ndxscn(shstrtab_sect->is_outsect_ptr->os_scn);
	my_elf_update(outfile_elf, ELF_C_WRITE);
} 

/* Open the a.out and create an EHDR and PHDR. */
void
open_out()
{
	Half			out_type;	/* Output file type (EXEC, REL, ...) */
	Listnode		*np1;		/* Node pointer. */
	Sg_desc			*sgp;		/* Segment descriptor pointer. */
	Listnode		*np2;		/* Node pointer. */
	register Os_desc	*osp;		/* Output section pointer. */
	Listnode		*np3;		/* Node pointer. */
	register Insect		*isp;		/* Input section pointer. */
	Elf_Scn			*scn;		/* A new section descriptor. */
	Shdr			*shdr;		/* A new section header. */
	Shdr			*hashshdr;	/* Save new hash section header. */
	Elf_Data		*data;		/* A new data descriptor. */
	char			*strptr;	/* ptr into the section header string table */
	unsigned int		nseg = 0;	/* number of segments, used to create program header */

	strptr = (char*)shstrtab_sect->is_rawbits->d_buf + 1;

	/* Copy outfile_name to cur_file_name for lderror purposes */
	cur_file_name = outfile_name;

	/* Check the state of the file outfile_name */

	if ((outfile_fd = open(outfile_name, O_RDWR | O_CREAT | O_TRUNC,
		(mode_t)0666)) < 0) {
		outfile_name = NULL;
		lderror(MSG_FATAL, "cannot create output file");
	}

	/* Determine the type of output file we are creating. */
	if (dmode) {
		if (Gflag)
			out_type = ET_DYN;
		else
			out_type = ET_EXEC;
	} else {
		if (aflag)
			out_type = ET_EXEC;
		else
			out_type = ET_REL;
	}

	if(out_type == ET_EXEC || out_type == ET_DYN){
		mode_t mask;
		mask = umask(0);
		(void) umask(mask);
		(void) chmod(outfile_name,(mode_t)0777 & ~mask);
	}

	/* Tell the access library about our new temporary file. */
	outfile_elf = my_elf_begin(outfile_fd, ELF_C_WRITE, NULL);

	/* Build the EHDR and fill the fields for which we have information. */
	/* After filling the appropriate fields, calculate the EHDR size using */
	/* the library function "elf_fsize". */
	if ((outfile_ehdr = elf_newehdr(outfile_elf)) == NULL)
		lderror(MSG_FATAL, "cannot open output file elf header");
	outfile_ehdr->e_ident[EI_DATA] = BYTE_ORDER;
	outfile_ehdr->e_type = out_type;
	outfile_ehdr->e_machine = MACHINE_TYPE;
	outfile_ehdr->e_version = libver;
	outfile_ehdr->e_flags = ehdr_flags;

	/* For each output section in the map file... */
	for (LIST_TRAVERSE(&seg_list, np1, sgp)) {
		/* count the number of segments that will go in the program header. */
		/* If a segment is empty, ignore it */
		if ( (dmode &&
			    ((sgp->sg_phdr.p_type == PT_DYNAMIC)
			    || (sgp->sg_phdr.p_type == PT_PHDR && !Gflag)
			    ||(sgp->sg_phdr.p_type == PT_INTERP &&!Gflag)))
			|| (sgp->sg_phdr.p_type != PT_NULL && sgp->sg_osectlist.head))
			nseg++;

		if ( (aflag || (dmode && !Gflag)) && sgp->sg_phdr.p_type == PT_SHLIB
				&& sgp->sg_osectlist.head){
			nseg++;
			firstseg_origin = LIBFIRSTSEG_ORIGIN;
		}

		for (LIST_TRAVERSE(&(sgp->sg_osectlist), np2, osp)) {
			off_t old_offset = 0;

			/* Get a section descriptor for the section. */
			scn = my_elf_newscn(outfile_elf);

			/* Set the section decscriptor of the new output section */
			osp->os_scn = scn;

			/* Get a new section header table entry and copy the */
			/* pertinent information from the in-core */
			/* descriptor. */
			shdr = my_elf_getshdr(scn);
			*shdr = *(osp->os_shdr);
			if(dmode && osp->os_shdr->sh_type == SHT_HASH)
				hashshdr = shdr;

			free(osp->os_shdr);
			osp->os_shdr = shdr;

			/* Create an SHSTRTAB entry for this output section. */
			shdr->sh_name = strptr - (char*)shstrtab_sect->is_rawbits->d_buf;
			strcpy(strptr,osp->os_name);
			strptr += strlen(osp->os_name) + 1;

			/* Traverse the list of input sections that will be */
			/* merged to form this output section. */
			for (LIST_TRAVERSE(&(osp->os_insects), np3, isp)) {
				if ((sgp->sg_phdr.p_type == PT_LOAD) &&
				    (osp->os_shdr->sh_type == SHT_PROGBITS) &&
				    ((osp->os_shdr->sh_flags & SHF_EXECINSTR) ==
				     SHF_EXECINSTR)) {
					off_t fill_bytes =
						ROUND(old_offset,
						      isp->is_shdr->sh_addralign) -
							      old_offset;
					if (fill_bytes)
						FILL_TEXT();
					old_offset += fill_bytes +
						isp->is_shdr->sh_size;
				}
				data = my_elf_newdata(scn);
				*data = *(isp->is_rawbits);
				isp->is_outdata = data;
			}
		}
	}

	if (dmode)
		hashshdr->sh_link = hash_sect->is_shdr->sh_link =
			 Gflag ? my_elf_ndxscn(symtab_sect->is_outsect_ptr->os_scn)
				: my_elf_ndxscn(dynsymtab_sect->is_outsect_ptr->os_scn);
	/* Build an empty PHDR. We just traversed the seg_list data structure and counted the */
	/* entries, to get the size. */
	if (aflag || dmode) {
		outfile_phdr = my_elf_newphdr(outfile_elf, nseg);
		sizePHDR = my_elf_fsize(ELF_T_PHDR, nseg, libver);
	} else
		sizePHDR = 0;

	my_elf_update(outfile_elf, ELF_C_NULL);
}
