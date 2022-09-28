/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/relocate.c	1.7"
/*
** Module relocate
** set-up for relocations
*/

/****************************************
** Imports
****************************************/

#include	"sgs.h"
#include	"globals.h"
#include	"macros.h"

/****************************************
** Global Definitions
****************************************/

Insect *first_rel_sect = (Insect *)0;

/****************************************
** Local Function Declarations
****************************************/

LPROTO(void make_rel_sect, (Os_desc*));

/****************************************
** Local Function Definitions
****************************************/

/* create output relocation section */
static void
make_rel_sect(osp)
	Os_desc	*osp;
{
	register Insect	*rsect;

	rsect = NEWZERO(Insect);
	rsect->is_shdr = NEWZERO(Shdr);
	rsect->is_shdr->sh_type = SHT_REL;
	if (dmode && (osp->os_shdr->sh_flags & SHF_ALLOC)) {
		rsect->is_shdr->sh_flags = SHF_ALLOC;
		if (first_rel_sect == (Insect *)0)
			first_rel_sect = rsect;
	}
	rsect->is_shdr->sh_size = osp->os_szoutrel;
	rsect->is_shdr->sh_addralign = WORD_ALIGN;
	rsect->is_shdr->sh_entsize = my_elf_fsize(ELF_T_REL,1,libver);
	rsect->is_name = (char*)mymalloc(strlen(".rel") +
				strlen(osp->os_name) + 1);
	strcpy(rsect->is_name,".rel");
	strcat(rsect->is_name,osp->os_name);
	rsect->is_outsect_ptr = osp;

	rsect->is_rawbits = NEWZERO(Elf_Data);
	rsect->is_rawbits->d_buf = (char *)mymalloc(osp->os_szoutrel);
	rsect->is_rawbits->d_type = ELF_T_REL;
	rsect->is_rawbits->d_size = osp->os_szoutrel;
	rsect->is_rawbits->d_align = WORD_ALIGN;
	rsect->is_rawbits->d_version = libver;

	osp->os_outrels = rsect;
	place_section(rsect);
}

/****************************************
** Global Function Definitions
****************************************/

/* count the number of output relocation entries and global
 * offset table or procedure linkage table entries
 * this function searches the segment and outsect lists
 * and passes each input reloc section to count_sect
 * it allocates space for any output relocations needed
 */
void
count_relentries()
{
	Listnode		*np1, *np2, *np3, *np4;
	Sg_desc			*sgp;
	register Os_desc	*osp;
	register Insect		*isp;
	register Insect		*rsp;

	countGOT = GOT_XNumber;
	countPLT = PLT_XNumber;
	grels = prels = copyrels = 0;

	for(LIST_TRAVERSE(&seg_list,np1,sgp)){

		for(LIST_TRAVERSE(&(sgp->sg_osectlist),np2,osp)){

			osp->os_szinrel = osp->os_szoutrel = 0;

			for(LIST_TRAVERSE(&(osp->os_insects),np3,isp)){

				for(LIST_TRAVERSE(&(isp->is_rela_list),np4,rsp)){
					osp->os_szinrel += rsp->is_shdr->sh_size;
					/* ld -r, same number of output 
					 * and input relocs
					 */
					if (rflag) { 
						osp->os_szoutrel += 
							rsp->is_shdr->sh_size;
						if (!aflag)
							continue;
					}
					count_sect(isp, rsp, osp);
					
				} /* end reloc list traversal */
			} /* end insect list traversal */
			
			count_rela += osp->os_szoutrel;

			/* create output relocation section */
			if (osp->os_szoutrel) {
				make_rel_sect(osp);
				/* check for relocations against non-writable allocatable sections */
				if (!textrel && 
					((osp->os_shdr->sh_flags &
					(SHF_ALLOC|SHF_WRITE)) == SHF_ALLOC))
						textrel = TRUE;
			}

		} /* end outsect list traversal */

	} /* end seg_list traversal */

	/* make got section */
	count_rela += grels;
	if (countGOT != GOT_XNumber ||
		sym_find(GOT_SYM, NOHASH) != 0 ||
		sym_find(GOT_USYM, NOHASH) != 0) {
		make_got(grels);
		if (first_rel_sect == (Insect *)0)
			first_rel_sect = got_sect->is_outsect_ptr->os_outrels;
	}

	/* the following line must always be executed -- no early returns from this function */
	if (aflag || dmode)
		make_bss();
	/* allocate copy rels to bss sections */
	if (copyrels != 0) {
		count_rela += copyrels;
		/* create output reloc section */
		bss_sect->is_outsect_ptr->os_szoutrel = copyrels;
		make_rel_sect(bss_sect->is_outsect_ptr);
	}

	/* Make the plt section, this is last to ensure that the .rel.plt
	 * section is after all the other .rel sections
	 */
	if (countPLT != PLT_XNumber) {
		make_plt(prels);
		if (first_rel_sect == (Insect *)0)
			first_rel_sect = plt_sect->is_outsect_ptr->os_outrels;
	}
}


/* void relocate()
** Finds every input relocation section for each output
** section and invokes reloc_sect to relocate that section.
*/

void
relocate()
{
	Listnode	*lptr1, *lptr2, *lptr3, *lptr4;
	Sg_desc		*seg;
	register Os_desc *osect;
	register Insect *isect, *rsect;
	unsigned	ndx;

	/* index of output symbol table section - but only if one exists */
	if (dmode && !Gflag)
		ndx = my_elf_ndxscn(dynsymtab_sect->is_outsect_ptr->os_scn);
	else if (Gflag || rflag || !sflag)
		ndx = my_elf_ndxscn(symtab_sect->is_outsect_ptr->os_scn);

	/* calculate offset of plt relocations within output relocation
	 * sections - used for creating plt entries
	 * set link and info fields in got and plt section headers
	 */
	if (dmode) {
		if (plt_sect && plt_sect->is_outsect_ptr->os_szoutrel) {
			plt_sect->is_outsect_ptr->os_outrels->
				is_outsect_ptr->os_shdr->sh_link = ndx;
			plt_sect->is_outsect_ptr->os_outrels->
				is_outsect_ptr->os_shdr->sh_info 
				= my_elf_ndxscn(plt_sect->is_outsect_ptr->os_scn);
		}
		if (got_sect && got_sect->is_outsect_ptr->os_szoutrel) {
			got_sect->is_outsect_ptr->os_outrels->
				is_outsect_ptr->os_shdr->sh_link = ndx;
			got_sect->is_outsect_ptr->os_outrels->
				is_outsect_ptr->os_shdr->sh_info = 
				my_elf_ndxscn(got_sect->is_outsect_ptr->os_scn);
		}
		if (copyrels){
			bss_sect->is_outsect_ptr->os_outrels->
                                is_outsect_ptr->os_shdr->sh_link = ndx;
			bss_sect->is_outsect_ptr->os_outrels->
                                is_outsect_ptr->os_shdr->sh_info =
                                my_elf_ndxscn(bss_sect->is_outsect_ptr->os_scn);		}
	}

	/* initialize counters */
	prels = grels = copyrels = 0;

	/* read through segment list for output sections */
	for (LIST_TRAVERSE(&seg_list, lptr1, seg)) {

		/* process each output section in this segment */
		for (LIST_TRAVERSE(&(seg->sg_osectlist), lptr2, osect)) {
			if (osect->os_szinrel == 0)
				continue;
			orels = 0;
			if (osect->os_szoutrel) {
				if( !dmode && sflag && !rflag)
					lderror(MSG_SYSTEM,"internal error, output relocations for output section %s with sflag set",
						osect->os_name);
				osect->os_outrels->is_outsect_ptr->
					os_shdr->sh_link = ndx;
				osect->os_outrels->is_outsect_ptr->
					os_shdr->sh_info =my_elf_ndxscn(osect->os_scn);
			}
			/* if -z text option was given, and we have
			 * output relocations for non-writable, allocatable
			 * sections, we issue a diagnostic and exit
			 */
			if (dmode && ztflag) {
				if (osect->os_szoutrel && 
					((osect->os_shdr->sh_flags &
					(SHF_ALLOC|SHF_WRITE)) == SHF_ALLOC))
					lderror(MSG_FATAL, "relocations remain against allocatable but non-writable section: %s",osect->os_name);
			}
			/* process each input section in this output
			 * section
			 */
			for (LIST_TRAVERSE(&(osect->os_insects),lptr3,isect)) {

				/* process each relocation section */
				for (LIST_TRAVERSE(&(isect->is_rela_list), lptr4, rsect))
					reloc_sect(isect, rsect, osect);
			}
		}
	}
	fillin_gotplt();
}
