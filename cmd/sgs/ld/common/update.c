/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/update.c	1.26"

/*
 * Virtual address, offset and displacement calculations
 */

/************************************************************
 * Includes
 ***********************************************************/

#include	"globals.h"

/************************************************************
 * Global function definitions
 ***********************************************************/

void
set_off_addr()
{

	Addr		cur_vaddr;
	Phdr		*phdr;
	Listnode	*np1, *np2, *np3;
	Sg_desc		*sgp;
	Sg_desc		*dynseg = NULL;
	Sg_desc		*shlibseg = NULL;
	Os_desc		*osp;
	Insect		*isp;
	Addr		true_end;

	if ((dmode && Gflag) || (!dmode && !aflag))
		cur_vaddr = 0;
	else
		cur_vaddr = firstseg_origin;

	for(LIST_TRAVERSE(&seg_list, np1, sgp)){

		Off	osoffset;		/* offset of previous output section */
		Off	offPHDR;		/* offset of first loadable seg */

		phdr = &(sgp->sg_phdr);

		if ( (dmode && !Gflag) && phdr->p_type == PT_PHDR) {
			osoffset = phdr->p_offset = outfile_ehdr->e_phoff;
			phdr->p_vaddr = cur_vaddr + phdr->p_offset;
			phdr->p_filesz = phdr->p_memsz = sizePHDR;
			offPHDR = phdr->p_offset;
			continue;
		}

		if (phdr->p_type == PT_INTERP) {
			sgp->sg_phdr.p_vaddr = 0;
			sgp->sg_phdr.p_memsz = 0;
			if (interp_sect != NULL) {
				sgp->sg_phdr.p_offset = interp_sect->is_outsect_ptr->
					os_shdr->sh_offset;
				sgp->sg_phdr.p_filesz = interp_sect->is_outsect_ptr->
					os_shdr->sh_size;
			}
			continue;
		}


		/* We will fill in the dynamic stuff later. */
		if ((phdr->p_type == PT_DYNAMIC) && dmode) {
			dynseg = sgp;
			continue;
		}

		if (phdr->p_align == 0) 
			if (phdr->p_type == PT_SHLIB){
				shlibseg = sgp;
				continue;
			} else if( phdr->p_type != PT_PHDR)
				phdr->p_align = CHUNK_SIZE;

		if( sgp->sg_osectlist.head == NULL){
			continue;
		}

		/* get segment offset from offset of first outsection in segment */

		if( dmode && !Gflag && (phdr->p_type == PT_LOAD) && offPHDR){
			/* if first loadable segment, adjust by size of program headers */
			phdr->p_offset = offPHDR;
			offPHDR = (Off)0;
			phdr->p_filesz = phdr->p_memsz = sizePHDR;
		} else {
			phdr->p_offset = ((Os_desc*)(sgp->sg_osectlist.head->data))->os_shdr->sh_offset;
			phdr->p_filesz = phdr->p_memsz = 0;
		}

		if(sgp->sg_flags & (1 << SGA_VADDR)){
			if(cur_vaddr != 0 && cur_vaddr != firstseg_origin &&
				cur_vaddr > phdr->p_vaddr)
				lderror(MSG_WARNING,"user defined segment addresses overlap; previous segment overlaps segment named: %s, current address = %#x, segment address = %#x",
					sgp->sg_name,
					(unsigned long)cur_vaddr,
					(unsigned long)phdr->p_vaddr);
			cur_vaddr = phdr->p_vaddr;
		} else {
			phdr->p_vaddr = cur_vaddr = ROUND(cur_vaddr, phdr->p_align) + 
					(phdr->p_offset%phdr->p_align);
		}
		
		
		/* save address of first executable segment for default use in build_ophdr() */
		if( (phdr->p_flags | PF_X) && !firstexec_seg)
			firstexec_seg = phdr->p_vaddr;

		osoffset = phdr->p_offset;

		for(LIST_TRAVERSE(&(sgp->sg_osectlist),np2,osp)){

			Addr 		bssvaddr;
			Elf_Data	*data;

			if ((dmode || aflag) && (osp->os_shdr->sh_flags & SHF_ALLOC)) {
				osp->os_shdr->sh_addr = cur_vaddr = cur_vaddr + 
							osp->os_shdr->sh_offset - osoffset;
			}
			DPRINTF(DBG_UPDATE,(MSG_DEBUG,"vaddr: osection %s, 0x%x",osp->os_name,osp->os_shdr->sh_addr));
			osoffset = osp->os_shdr->sh_offset;

			data = NULL;

			bssvaddr = cur_vaddr;
			for(LIST_TRAVERSE(&(osp->os_insects),np3,isp)){
				bssvaddr = ROUND(bssvaddr, isp->is_shdr->sh_addralign);

				data = isp->is_outdata;
				isp->is_newFOffset = data->d_off + osoffset;
				if ((dmode || aflag) && 
					(osp->os_shdr->sh_flags & SHF_ALLOC)) {
					if (isp->is_shdr->sh_type == SHT_NOBITS) {
						isp->is_newVAddr = bssvaddr;
						bssvaddr += isp->is_shdr->sh_size;
					} else
						isp->is_newVAddr = cur_vaddr + data->d_off;
				}
			DPRINTF(DBG_UPDATE,(MSG_DEBUG," isection %s, vaddr 0x%x, offset 0x%x",isp->is_name,isp->is_newVAddr,isp->is_newFOffset));
				isp->is_displ = data->d_off;

			} /* input sections for this output section complete */

			true_end = ROUND(osoffset,osp->os_shdr->sh_addralign) + 
					osp->os_shdr->sh_size;

			if(osp->os_shdr->sh_type != SHT_NOBITS)
				phdr->p_filesz = true_end - phdr->p_offset;

			phdr->p_memsz = true_end - phdr->p_offset;
			if (sgp->sg_sizesym != NULL)
				sgp->sg_sizesym->ls_syment->st_value = phdr->p_memsz;

		} /* output sections for this segment complete */

		cur_vaddr += true_end - osoffset;	/*adjust for last section size */
		if((sgp->sg_length != 0) && (sgp->sg_length < phdr->p_memsz) )
			lderror(MSG_FATAL,"segment %s calculated size larger than user-defined size",
					sgp->sg_name);

		if (phdr->p_type == PT_NOTE) {
			sgp->sg_phdr.p_vaddr = 0;
			sgp->sg_phdr.p_paddr = 0;
			sgp->sg_phdr.p_align = 0;
			sgp->sg_phdr.p_memsz = 0;
		}
	} /* segment traversal complete */

	if ((dynseg != NULL) && (dynamic_sect != NULL)) {
		dynseg->sg_phdr.p_vaddr = dynamic_sect->is_newVAddr;
		dynseg->sg_phdr.p_offset = dynamic_sect->is_newFOffset;
		dynseg->sg_phdr.p_filesz = dynamic_sect->is_rawbits->d_size;
		dynseg->sg_phdr.p_flags = PF_R | PF_W | PF_X;
	}

	if (shlibseg->sg_osectlist.head != NULL) {
		shlibseg->sg_phdr.p_offset = ((Os_desc *)shlibseg->sg_osectlist.head->data)->os_shdr->sh_offset;
		shlibseg->sg_phdr.p_filesz = ((Os_desc *)shlibseg->sg_osectlist.head->data)->os_shdr->sh_size;
	}
		
}
