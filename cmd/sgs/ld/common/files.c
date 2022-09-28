/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/files.c	1.26"
/*
** Module files
** Processing of objects and shared objects.
*/

/****************************************
** Imports
****************************************/

#include	<string.h>
#include	"link.h"
#include	"globals.h"

#ifndef	__STDC__
extern char*	memcpy();
#endif


/****************************************
** Local Variables
****************************************/

static char	*scn_name;

/****************************************
** Local Function Declarations
****************************************/

LPROTO(void infile_setup, (void));
LPROTO(Insect *insect_setup, (Elf_Scn *, Shdr *));
LPROTO(void process_strtab, (Elf_Scn *, Shdr *));

/****************************************
** Local function definitions
****************************************/

/*
** void
** infile_setup()
**
** Check sanity of file header and allocate an infile structure
** for the file being processed.
**
*/

static void
infile_setup()
{

	if(cur_file_ptr == NULL)
		lderror(MSG_SYSTEM,"ld internal error:  null cur_file_ptr");

	/* check version, etc. */
	if (cur_file_ehdr->e_machine != MACHINE_TYPE)
		lderror(MSG_FATAL, "%s: wrong machine type", cur_file_name);

	if (cur_file_ehdr->e_version > libver)
		lderror(MSG_FATAL,
			"%s: creator ELF library version is higher than the maximum version known to ld",
			cur_file_name);

	if (cur_file_ehdr->e_ident[EI_CLASS] != MACHINE_CLASS)
		lderror(MSG_FATAL, "%s: wrong machine class", cur_file_name);

	if (cur_file_ehdr->e_ident[EI_DATA] != BYTE_ORDER)
		lderror(MSG_FATAL, "%s: wrong machine byte order for data encoding", cur_file_name);
	cur_infile_ptr = NEWZERO(Infile);
	cur_infile_ptr->fl_name = cur_file_name;
	cur_infile_ptr->fl_e_type = cur_file_ehdr->e_type;
	cur_infile_ptr->fl_insect_ndx = (Insect** ) mycalloc(cur_file_ehdr->e_shnum * sizeof(Insect*));
	return;
}


/*
** Insect*
** insect_setup(Elf_Scn* scn, Shdr* shdr);
** Allocate and initialize a new Insect structure for this input section.
*/

static Insect*
insect_setup(scn,shdr)
	Elf_Scn		*scn;
	Shdr		*shdr;
{
	register Insect	*tip;	/*temporary input section structure pointer */

	tip = NEWZERO(Insect);
	tip->is_scn = scn;
	tip->is_shdr = shdr;
	tip->is_index = my_elf_ndxscn(scn);
	tip->is_file_ptr = cur_infile_ptr;
	tip->is_outsect_ptr = NULL;
	tip->is_name = scn_name;
	return(tip);
}


/*
** void
** process_strtab(scn,shdr)
**
** read the string table bytes of a string table section
*/

static void
process_strtab(scn,shdr)
	Elf_Scn		*scn;
        Shdr		*shdr;
{
        Elf_Data	*data;
	Insect		*tip;

        if (shdr->sh_type != SHT_STRTAB){
        	lderror(MSG_SYSTEM, "section %s: string table for symbol names not of type stringtable",
				scn_name);
		return;
	}
        data = NULL;
        data = my_elf_getdata(scn,data);
	
	if (data->d_size != 0 && (((char *)(data->d_buf))[0] != '\0'
		|| *((char*)(data->d_buf) + data->d_size - 1) !='\0'))
		lderror(MSG_WARNING,"section %s: malformed string table, initial or final byte",
			scn_name);

	tip = insect_setup(scn,shdr);
        tip->is_rawbits = data;
	/* this assignment assumes 1 symbol-string table pair per file in .so's */
	/* should this change, keep a pointer to the stringtab bits in the symtab insect */
	symname_bits = data;
        (void) list_append(&(cur_infile_ptr->fl_insects), tip);
	return;
}

/****************************************
** global function definitions
****************************************/

/*
** Infile *
** find_infile(char*)
** searches the infile_list for the current input file (cur_file_name)
** and returns it Infile* or NULL if not found.
*/
Infile*
find_infile(thisname)
	char	*thisname;
{
	register Listnode
		*ip;		/* temporary pointer to input file */

	Infile	*dp;		/* pointer to Infile data */

	for (LIST_TRAVERSE(&infile_list, ip, dp)) {
		if (strcmp(thisname, dp->fl_name) == SAME)
			return(dp);
	}
	return NULL;
}

/*
** void
** process_infile(char *fname)
** determine the type of and
** process the current input file (cur_file_name).
*/
void
process_infile(fname)
	char	*fname;
{
	Elf_Kind
		kind;		/* file type */
	char	*tempname;	/* copy cur_file_name into protected memory */

	tempname = (char *)mymalloc(strlen(cur_file_name)+1);
	strcpy(tempname,cur_file_name);
	cur_file_name = tempname;
	cur_file_ptr = my_elf_begin(cur_file_fd,ELF_C_READ,NULL);

	kind = elf_kind(cur_file_ptr);
	switch(kind) {
	case ELF_K_AR:
		process_archive();
		break;
	case ELF_K_COFF:
		lderror(MSG_NOTICE,"internal conversion of COFF file to ELF");
		my_elf_update(cur_file_ptr,ELF_C_NULL);
		/*FALLTHROUGH*/
	case ELF_K_ELF:
		cur_file_ehdr = my_elf_getehdr(cur_file_ptr);
		switch (cur_file_ehdr->e_type) {
		case ET_REL:
			if(find_infile(cur_file_name) != NULL)
				lderror(MSG_WARNING,"attempted multiple inclusion of file");
			else
				process_relobj();
			break;
		case ET_DYN:
			if (!dmode || !Bflag_dynamic )
				lderror(MSG_FATAL,"input shared object in static mode");
			else
				process_shobj(fname);
			break;
		default:
			lderror(MSG_WARNING, "is an illegal elfld file type");
			break;
		}
		break;
	case ELF_K_NONE:
		lderror(MSG_FATAL, "file type unknown to the library used by ld");
		break;
	default:
		break;
	}
	my_elf_cntl(cur_file_ptr,ELF_C_FDDONE);
	close(cur_file_fd);
	cur_file_fd = -1;
	cur_file_ptr = NULL;
        cur_infile_ptr = NULL;
        cur_file_ehdr = NULL;
	cur_file_name = NULL;

}

/*
** void
** process_relobj()
** reads the sections from an object file (.o or .o from an archive).
*/

void
process_relobj()
{
	register Elf_Scn
		*scn; 		/* the section */
	Elf_Scn	*nscn;
	register Shdr		/* current section header table entry */
		*shp;
	Shdr	*nshp;
	Elf_Scn
		*sym_scn;	/* the symbol table */
	Insect	*tip, *ttip;	/* temp input section ptr */
	size_t	scn_ndx;
	char	*save_scn_name;
	char	*save_symscn_name;
	Elf_Data
		*scn_name_bits;

	infile_setup();
	(void) list_append(&infile_list, cur_infile_ptr);

	/* process the section header string table */
	/* this assignment means we have seen the section but it has no corresponding insect struct */
	cur_infile_ptr->fl_insect_ndx[cur_file_ehdr->e_shstrndx] = (Insect*)(-1);
	scn = my_elf_getscn(cur_file_ptr,cur_file_ehdr->e_shstrndx);
	shp = my_elf_getshdr(scn);
	scn_name = my_elf_strptr(cur_file_ptr,cur_file_ehdr->e_shstrndx,(size_t)(shp->sh_name));
	symname_bits = scn_name_bits = my_elf_getdata(scn,NULL);
	if (scn_name_bits->d_size != 0 && (((char *)(scn_name_bits->d_buf))[0] != '\0'
		|| *((char*)(scn_name_bits->d_buf) + scn_name_bits->d_size - 1) !='\0'))
		lderror(MSG_WARNING,"section %s: malformed string table, initial or final byte",
			scn_name);

	SETEHDR_FLAGS();

	scn = NULL;
	scn_ndx = 0;
	DPRINTF(DBG_FILES,(MSG_DEBUG,"%s: cur_file_name",cur_file_name));	
	while((scn = elf_nextscn(cur_file_ptr,scn)) != NULL)
	{
		scn_ndx++;
		shp = my_elf_getshdr(scn);
		scn_name = (char *)scn_name_bits->d_buf + (size_t)(shp->sh_name);

		switch (shp->sh_type) {
		case SHT_STRTAB:
			if((strcmp(scn_name,".strtab") != SAME) && (strcmp(scn_name,".shstrtab") != SAME) )
				goto savesec;
			break;
		case SHT_NULL:
			break;
		case SHT_SYMTAB:
			if( cur_infile_ptr->fl_insect_ndx[shp->sh_link] == 0 ){
				cur_infile_ptr->fl_insect_ndx[shp->sh_link] = (Insect*)(-1);
				nscn = my_elf_getscn(cur_file_ptr,shp->sh_link);
				nshp = my_elf_getshdr(nscn);
				save_scn_name = scn_name;
				scn_name = (char *)scn_name_bits->d_buf + (size_t)(nshp->sh_name);
				process_strtab(nscn,nshp);
			}
			sym_scn = scn;
			break;
		case SHT_SHLIB:
			if ( (dmode && Gflag) || (!dmode && rflag) ){
				lderror(MSG_FATAL,
					"static shared library section `%s` valid only when building an executable",
					scn_name);
			} else
				goto savesec;
			/*FALLTHROUGH*/
		case SHT_PROGBITS:
			/* strip debug sections if -s seen */
			if(sflag && ((strcmp(scn_name,".debug") == SAME) ||
				(strcmp(scn_name,".line") == SAME)))
				break;
			/*FALLTHROUGH*/
		case SHT_NOBITS:
		case SHT_NOTE:
	savesec:
			if( cur_infile_ptr->fl_insect_ndx[scn_ndx] == 0 ){
				tip = insect_setup(scn,shp);
				cur_infile_ptr->fl_insect_ndx[scn_ndx] = tip;
				if (shp->sh_type == SHT_NOBITS) {
					tip->is_rawbits = NEWZERO(Elf_Data);
					tip->is_rawbits->d_buf = NULL;
					tip->is_rawbits->d_type = ELF_T_BYTE;
					tip->is_rawbits->d_size = shp->sh_size;
					tip->is_rawbits->d_off = shp->sh_offset;
					tip->is_rawbits->d_align = shp->sh_addralign;
					tip->is_rawbits->d_version = libver;
				}
				else {
                                	tip->is_rawbits = my_elf_getdata(scn,NULL);
					tip->is_rawbits->d_align = shp->sh_addralign;
				}
				(void) list_append(&(cur_infile_ptr->fl_insects), tip);
				place_section(tip);
			}
			break;
		case SHT_RELA:
		case SHT_REL:
			if((tip = cur_infile_ptr->fl_insect_ndx[shp->sh_info])== 0) {
				nscn = my_elf_getscn(cur_file_ptr,shp->sh_info);
				nshp = my_elf_getshdr(nscn);
				save_scn_name = scn_name;
                                scn_name = (char *)scn_name_bits->d_buf + (size_t)(nshp->sh_name);
				if(sflag && ((strcmp(scn_name,".debug") == SAME) ||
					(strcmp(scn_name,".line") == SAME)))
					break;
				tip = insect_setup(nscn,nshp);
				cur_infile_ptr->fl_insect_ndx[my_elf_ndxscn(nscn)] = tip;
				if (nshp->sh_type == SHT_NOBITS) {
					tip->is_rawbits = NEWZERO(Elf_Data);
					tip->is_rawbits->d_buf = NULL;
					tip->is_rawbits->d_type = ELF_T_BYTE;
					tip->is_rawbits->d_size = shp->sh_size;
					tip->is_rawbits->d_off = shp->sh_offset;
					tip->is_rawbits->d_align =
						shp->sh_addralign;
					tip->is_rawbits->d_version = libver;
				}
				else {
					tip->is_rawbits = my_elf_getdata(nscn,NULL);
					tip->is_rawbits->d_align = nshp->sh_addralign;
				}
				(void) list_append(&(cur_infile_ptr->fl_insects), tip);
				place_section(tip);
				scn_name = save_scn_name;
			}
			ttip = insect_setup(scn,shp);
			cur_infile_ptr->fl_insect_ndx[scn_ndx] = ttip;
			ttip->is_rawbits = my_elf_getdata(scn,NULL);
			(void) list_append(&(tip->is_rela_list),ttip);
			break;
		case SHT_DYNAMIC:
			lderror(MSG_FATAL, "found a DYNAMIC section in relocatable object file");
			/*NOTREACHED*/
		default:
			if(shp->sh_type < SHT_LOUSER || shp->sh_type > SHT_HIUSER){
				lderror(MSG_WARNING, "unknown section type 0x%x", shp->sh_type);
				break;
			} else
				goto savesec;
		}
	}
	if (sym_scn){
		scn_name = save_symscn_name;
		process_symtab(sym_scn);
	}
}


/* void
** process_shobj(char *fname)
**
** reads the symbol table and dynamic sections from a shared object file
**
*/

void
process_shobj(fname)
	char* fname;
{
	register Elf_Scn
		*scn; 		/* the section */
	Elf_Scn	*nscn;
	register Shdr		/* current section header table entry */
		*shp;
	Shdr	*nshp;
	Elf_Scn
		*sym_scn;	/* the symbol table */
	Insect	*tip;		/* temp input section ptr */
	size_t	scn_ndx;
	char	*save_scn_name;
	Elf_Data
		*scn_name_bits;
	Elf_Data	
		*dyndata;	/* Input dynamic section data */
	Dyn	*tdyn;		/* temp dynamic element pointer */
	char	*name_recorded = NULL;	/* override name of this shared object (perhaps) */
	char	*cur_so_name;

	infile_setup();
	cur_so_name = mymalloc(strlen(fname)+1);
	strcpy(cur_so_name,fname);

	/* process the section header string table */
	/* this assignment means we have seen the section but it has no corresponding insect struct */
	cur_infile_ptr->fl_insect_ndx[cur_file_ehdr->e_shstrndx] = (Insect*)(-1);
	scn = my_elf_getscn(cur_file_ptr,cur_file_ehdr->e_shstrndx);
	shp = my_elf_getshdr(scn);
	scn_name = my_elf_strptr(cur_file_ptr,cur_file_ehdr->e_shstrndx,(size_t)(shp->sh_name));
	symname_bits = scn_name_bits = my_elf_getdata(scn,NULL);
	if (scn_name_bits->d_size != 0 && (((char *)(scn_name_bits->d_buf))[0] != '\0'
		|| *((char*)(scn_name_bits->d_buf) + scn_name_bits->d_size - 1) !='\0'))
		lderror(MSG_WARNING,"section %s: malformed string table, initial or final byte",
			scn_name);

	SETEHDR_FLAGS();

	scn = NULL;
	sym_scn = NULL;
	scn_ndx = 0;
	DPRINTF(DBG_FILES,(MSG_DEBUG,"%s: cur_file_name",cur_file_name));
	while((scn = elf_nextscn(cur_file_ptr,scn)) != NULL)
	{
		scn_ndx++;
		shp = my_elf_getshdr(scn);
		scn_name = (char *)scn_name_bits->d_buf + (size_t)(shp->sh_name);

		switch (shp->sh_type) {
		case SHT_STRTAB:
		case SHT_NULL:
		case SHT_REL:
		case SHT_RELA:
		case SHT_HASH:
		case SHT_NOTE:
			break;
		case SHT_SHLIB:
			lderror(MSG_FATAL,
				"static shared library section `%s` appears in a shared object",
					scn_name);
			break;
		case SHT_NOBITS:
		case SHT_PROGBITS:
			if( strcmp(scn_name,".comment") == SAME)
				break;
			if(sflag && ((strcmp(scn_name,".debug") == SAME) ||
				(strcmp(scn_name,".line") == SAME)))
				break;
			tip = insect_setup(scn,shp);
			cur_infile_ptr->fl_insect_ndx[scn_ndx] = tip;
			tip->is_rawbits = NULL;
			(void) list_append(&(cur_infile_ptr->fl_insects), tip);
			break;
		case SHT_SYMTAB:
			if( cur_infile_ptr->fl_insect_ndx[shp->sh_link] == 0 ){
				cur_infile_ptr->fl_insect_ndx[shp->sh_link] = (Insect*)(-1);
				nscn = my_elf_getscn(cur_file_ptr,shp->sh_link);
				nshp = my_elf_getshdr(nscn);
				save_scn_name = scn_name;
				scn_name = (char *)scn_name_bits->d_buf + (size_t)(nshp->sh_name);
                                process_strtab(nscn,nshp);
			}
			sym_scn = scn;
			break;
		case SHT_DYNAMIC:
			dyndata = my_elf_getdata(scn,NULL);
			for( tdyn = (Dyn*)dyndata->d_buf; tdyn->d_tag !=DT_NULL; tdyn++){
				if( tdyn->d_tag == DT_SONAME ){
					if((name_recorded = elf_strptr(cur_file_ptr,
							shp->sh_link,(size_t)tdyn->d_un.d_val))!=NULL){
						if( find_infile(name_recorded) != NULL){
							lderror(MSG_WARNING,
								"attempted multiple inclusion of file: %s",
								name_recorded);
							return;
						}
					}
				}	
			}
			break;
		default:
			if(shp->sh_type < SHT_LOUSER || shp->sh_type > SHT_HIUSER)
				lderror(MSG_WARNING, "unknown section type %d", shp->sh_type);
		}

	}
	if( !name_recorded )
		if(find_infile(cur_file_name) != NULL){
			lderror(MSG_WARNING,"attempted multiple inclusion of file");
			return;
		} else
			(void)list_append(&soneeded_list,cur_so_name);
	else {
		(void)list_append(&soneeded_list,name_recorded);
		cur_infile_ptr->fl_name = cur_file_name;
	}
	(void) list_append(&infile_list, cur_infile_ptr);
	if (sym_scn){
		scn_name = save_scn_name;
		process_symtab(sym_scn);
	}
}
