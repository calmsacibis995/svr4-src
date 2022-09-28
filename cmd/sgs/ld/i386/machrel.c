/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:i386/machrel.c	1.4"
/*
** Module machrel
** perform machine dependent relocations
*/

/****************************************
** Imports
****************************************/

#include	"sgs.h"
#include	"globals.h"

/****************************************
** Local Macros
****************************************/

#define IS_PC_RELATIVE(X) (pc_rel_type[(X)] == 1) 

/*
** build an output relocation entry
** rloc is a pointer to the current position in the output
** relocation section
*/

#define BUILD_RELOC(osect, rtype, ndx, off, rloc)	{\
	Rel	*relbits = (Rel *)((char*) ((osect)->os_outrels->is_rawbits->d_buf)+*(rloc));\
	relbits->r_info = ELF32_R_INFO((ndx),(rtype));\
	relbits->r_offset = (off);\
	*(rloc) += sizeof(Rel); }

/* NOTE: These macros will work reliably only on 32-bit 2's 
 * complement machines.  The type of P in all cases should
 * be unsigned char *
 */

#define	GET4(P)	((long)(((unsigned long)(P)[3] << 24) | \
			((unsigned long)(P)[2] << 16) | \
			((unsigned long)(P)[1] << 8) | \
			(unsigned long)(P)[0]))

#define	PUT4(V, P)	{ (P)[3] = ((V) >> 24); \
			  (P)[2] = ((V) >> 16); \
			  (P)[1] = ((V) >> 8); \
			  (P)[0] = (V); }


/* 386 instruction encodings - used for procedure linkage table entries */

/* opcodes: */

#define INST_JMP		0xe9
#define INST_PUSHL		0x68
#define	SPECIAL_INST		0xff
#define	PUSHL_DISP		0x35
#define	PUSHL_REG_DISP		0xb3
#define	JMP_DISP_IND		0x25
#define	JMP_REG_DISP_IND	0xa3

/****************************************
** Local Variable Definitions
****************************************/

/* static bit arrays used to determine whether a particular
 * relocation type is pc-relative or is a byte-swapped type
 */

static CONST unsigned char pc_rel_type[R_386_NUM] = { 	0,	/* R_386_NONE	*/
							0,	/* R_386_32	*/
							1,	/* R_386_PC32	*/
							0,	/* R_386_GOT32	*/
							1,	/* R_386_PLT32	*/
							0,	/* R_386_COPY	*/
							0,	/* R_386_GLOB_DAT		*/
							0,	/* R_386_JMP_SLOT	*/
							0,	/* R_386_RELATIVE	*/
							0,	/* R_386_GOTOFF	*/
							1 };	/* R_386_GOTPC	*/

/****************************************
** Local Function Declarations
****************************************/

LPROTO(void do_reloc, (Insect *, Word, Addr, Word));
LPROTO(void plt_entry, (Ldsym *, Word));

/****************************************
** Local Function Definitions
****************************************/


/* write a single relocated value to its reference location */

static void
do_reloc(isect, rtype, off, value)
	Insect	*isect;
	Word	rtype;
	Addr	off;
	Word	value;
{
	register unsigned char	*memloc;
	register long		uvalue;

	/* find bits in input section */
	memloc = (unsigned char*) isect->is_rawbits->d_buf + off;

	switch (rtype) {
	case R_386_NONE:
		DPRINTF(DBG_RELOC, (MSG_DEBUG, "reloc case R_386_NONE"));
		break;
	case R_386_32:
	case R_386_PC32:
	case R_386_GOT32:
	case R_386_PLT32:
	case R_386_GOTPC:
	case R_386_GOTOFF:
#ifdef	DEBUG
		switch(rtype) {
		case R_386_32:
			DPRINTF(DBG_RELOC, (MSG_DEBUG,
				    "reloc case R_386_32: value %#x, memloc %#x",
				    value, memloc));
			break;
		case R_386_PC32:
			DPRINTF(DBG_RELOC, (MSG_DEBUG,
				    "reloc case R_386_PC32: value %#x, memloc %#x",
				    value, memloc));
			break;
		case R_386_GOT32:
			DPRINTF(DBG_RELOC, (MSG_DEBUG,
				    "reloc case R_386_GOT32: value %#x, memloc %#x",
				    value, memloc));
			break;
		case R_386_PLT32:
			DPRINTF(DBG_RELOC, (MSG_DEBUG,
				    "reloc case R_386_PLT32: value %#x, memloc %#x",
				    value, memloc));
			break;
		case R_386_GOTPC:
			DPRINTF(DBG_RELOC, (MSG_DEBUG,
				    "reloc case R_386_GOTPC: value %#x, memloc %#x",
				    value, memloc));
			break;
		case R_386_GOTOFF:
			DPRINTF(DBG_RELOC, (MSG_DEBUG,
				    "reloc case R_386_GOTOFF: value %#x, memloc %#x",
				    value, memloc));
			break;
		}
#endif
		uvalue = GET4(memloc);
		uvalue += value;
		PUT4(uvalue, memloc);
		break;
	default:
		lderror(MSG_FATAL,
			"unknown relocation type %d in section %s of file %s",
			rtype, isect->is_name, isect->is_file_ptr->fl_name);

	}
}

/* build a single plt entry - code is:
 *	if (building a.out)
 *		JMP	*got_off
 *	else
 *		JMP	*got_off@GOT(%ebx)
 *	PUSHL	&rel_off
 *	JMP	-n(%pc)		# -n is pcrel offset to first plt entry
 *
 *	The got_off@GOT entry gets filled with the address of the PUSHL,
 *	so the first pass through the plt jumps back here, jumping
 *	in turn to the first plt entry, which jumps to the dynamic
 *	linker.	 The dynamic linker then patches the GOT, rerouting
 *	future plt calls to the proper destination.
 */

static void
plt_entry(ldsym, rel_off)
	Ldsym	*ldsym;
	Word	rel_off;
{
	unsigned char *pent;
	Sword	plt_off;
	Word	got_off;
	unsigned char opnd;
	unsigned char *gent;

	got_off = ldsym->ls_PLTGOTndx * GOTENTSZ;
	plt_off = ldsym->ls_PLTndx * PLTENTSZ;
	pent = (unsigned char *)(plt_sect->is_rawbits->d_buf) + plt_off;
	gent = (unsigned char *)(got_sect->is_rawbits->d_buf) + got_off;

	/* fill in the field in the got with the address of the next instruction */
	PUT4(plt_sect->is_newVAddr + plt_off + PLT_INST_SZ, gent);

	if (!Gflag) {
		pent[0] = SPECIAL_INST;
		pent[1] = JMP_DISP_IND;
		pent += 2;
		PUT4((Word)(got_sect->is_newVAddr + got_off), pent);
	} else {
		pent[0] = SPECIAL_INST;
		pent[1] = JMP_REG_DISP_IND;
		pent += 2;
		PUT4((Word)(got_off), pent);
	}
	pent += 4;

	pent[0] = INST_PUSHL;
	pent++;
	PUT4(rel_off, pent);
	pent += 4;

	plt_off = -(plt_off + 16);	/* JMP, PUSHL, JMP take 16 bytes */
	pent[0] = INST_JMP;
	pent++;
	PUT4((long)plt_off, pent);
}

/****************************************
** Global Function Definitions
****************************************/

/* count relocation, got and plt entries for a single
 * relocation section
 * this code parallels the code for reloc_sect, below,
 * which is more heavily commented and actually
 * performs the relocations counted here
 */
void
count_sect(isp, rsp, osp)
	Insect	*isp, *rsp;
	Os_desc	*osp;
{
	register Rel	*reloc, *rend;
	register Ldsym  *sym;
	Boolean		noload = FALSE;
	Word  		rstndx, rtype, rsize; 

	/* is input section loadable? */
	if (!(isp->is_shdr->sh_flags & SHF_ALLOC))
		noload = TRUE;
				
	rsize = rsp->is_shdr->sh_size;
	reloc = (Rel *)rsp->is_rawbits->d_buf;

	/* read relocation records - there may be 2 types of relocation
	 * entries: one with an addend field and one without
	 * A relocation section will be all of one or the other kind.
	 * This code is only for one without an addend
	 */
				
	rend = (Rel*)((char *)reloc + rsize);
	for ( ; reloc < rend; ++reloc) {
		rtype = ELF32_R_TYPE(reloc->r_info);
		rstndx = ELF32_R_SYM(reloc->r_info);

		sym = rsp->is_file_ptr->fl_oldindex[rstndx];

		/* in the static case,
		 * just check for got entries
		 * we handled the ld -r case above 
		 */
		if (!dmode) {
			if (rtype == R_386_GOT32)
				if (!sym->ls_GOTndx)
					sym->ls_GOTndx = countGOT++;
			continue;
		}

		/* dynamic mode */
		if (ELF32_ST_BIND(sym->ls_syment->st_info) == STB_LOCAL) {
			if (!IS_PC_RELATIVE(rtype)) {
				if (rtype == R_386_GOT32) {
					if (!sym->ls_GOTndx) {
						sym->ls_GOTndx = countGOT++;
						if (Gflag)
							grels += sizeof(Rel);
					}
				} else if (Gflag && !noload &&
					rtype != R_386_GOTOFF)
					osp->os_szoutrel += sizeof(Rel);
			}
			continue;
		}


		/* if here, we have a global or weak symbol */
		
		if (rtype == R_386_GOT32) {
			if (!sym->ls_GOTndx) {
				sym->ls_GOTndx = countGOT++;
				if (Gflag || (sym->ls_deftag < REF_RELOBJ) 
				|| (sym->ls_syment->st_shndx == SHN_UNDEF))
					grels += sizeof(Rel);
			}
			continue;
		}
		if (rtype == R_386_PLT32) {
			if ((Gflag && !Bflag_symbolic) ||
				(sym->ls_deftag < REF_RELOBJ) ||
				(sym->ls_syment->st_shndx == SHN_UNDEF)) {
				if (!sym->ls_PLTndx) {
					sym->ls_PLTndx = countPLT++;
					sym->ls_PLTGOTndx = countGOT++;
					prels += sizeof(Rel);
				}
			}
			continue;
		}
		if (rtype == R_386_GOTOFF || rtype == R_386_GOTPC)
			continue;
		if ((sym->ls_deftag >= REF_RELOBJ) && 
			sym->ls_syment->st_shndx != SHN_UNDEF) { 
			if (Gflag && !Bflag_symbolic && !noload) 
				osp->os_szoutrel += sizeof(Rel); 
			else if (!IS_PC_RELATIVE(rtype)) {
				if (Gflag && !noload)
					osp->os_szoutrel += sizeof(Rel);
			}
			continue;
		}
		/* symbol defined in a .so
		 * or undefined 
		 */
		if ((sym->ls_deftag == REF_DEFN) || 
			(sym->ls_syment->st_shndx == SHN_UNDEF)) {
			if (Gflag || bflag || 
				(sym->ls_syment->st_shndx == SHN_UNDEF)
				|| (ELF32_ST_TYPE(sym->ls_syment->st_info) 
				== STT_NOTYPE)) {
				osp->os_szoutrel += sizeof(Rel);
			} else {

			/* special relocation processing for dynamic a.outs
			 * referencing symbols defined in a .so w/o pic or got
			 * reloc types; if sym is a function, create plt entry
			 * for it; if data, then allocate space for this symbol in 
			 * common in this object and relocate reference to
			 * the new space; if sym  was initiallized, create
			 * relocation entry specifying that data must be copied
			 * at run-time
			 */

				if (ELF32_ST_TYPE(sym->ls_syment->st_info) 
					== STT_FUNC) {
					if (!sym->ls_PLTndx) {
						sym->ls_PLTndx = countPLT++;
						sym->ls_PLTGOTndx = countGOT++;
						prels += sizeof(Rel);
					}
				}
				else if ((ELF32_ST_TYPE(sym->ls_syment->st_info) == STT_OBJECT) &&
				(sym->ls_syment->st_shndx != SHN_ABS)) {
					sym->ls_syment->st_shndx = SHN_COMMON;
					bss_align = sym->ls_syment->st_value = WORD_ALIGN;
					if (sym->ls_scnptr->is_shdr->sh_type 
						!= SHT_NOBITS)
						if (!sym->ls_COPYalloc) {
							sym->ls_COPYalloc = TRUE;
							copyrels += sizeof(Rel);
						}
				}
			}
		} /* end not defined */
	} /* end read reloc entries */
}


/* void fillin_gotplt
** machine dependent part of relocate() that
** initializes the first got entry
** and fills in the reserved slot of the plt
*/
void
fillin_gotplt()
{

	/* initialize first got entry with address of dynamic section */
	if (dmode && got_sect) {
		Ldsym *sym;
		if((sym = sym_find("_DYNAMIC", NOHASH)) == NULL)
			lderror(MSG_SYSTEM,"internal error:  `_DYNAMIC` not found in ld symbol table");
		else if (!sym->ls_GOTdone) 
			PUT4(sym->ls_syment->st_value,
				((unsigned char *)got_sect->is_rawbits->d_buf));
	}


        /* fill in the reserved slot in the procedure linkage table
	 * the first entry is:
	 *  if (building a.out) {
	 *	PUSHL	got[1]		# the address of the link map entry
	 *	JMP	*got[2]		# the address of rtbinder
	 *  } else {
	 *	PUSHL	got[1]@GOT(%ebx)	# the address of the link map entry
	 *	JMP	*got[2]@GOT(%ebx)	# the address of rtbinder
	 *  }
	 */
	if (dmode && plt_sect) {
		unsigned char *pent;
		unsigned long plt_addr; /* distance between the plt entry and the GOT */

                pent = (unsigned char *)plt_sect->is_rawbits->d_buf + PLT_XRTLD * PLTENTSZ;
		if (!Gflag) {
			pent[0] = SPECIAL_INST;
			pent[1] = PUSHL_DISP;
			pent += 2;
			PUT4((Word)(got_sect->is_newVAddr + GOT_XLINKMAP * GOTENTSZ), pent);
			pent += 4;
			pent[0] = SPECIAL_INST;
			pent[1] = JMP_DISP_IND;
			pent += 2;
			PUT4((Word)(got_sect->is_newVAddr + GOT_XRTLD * GOTENTSZ), pent);
		} else {
			pent[0] = SPECIAL_INST;
			pent[1] = PUSHL_REG_DISP;
			pent += 2;
			PUT4((Word)(GOT_XLINKMAP * GOTENTSZ), pent);
			pent += 4;
			pent[0] = SPECIAL_INST;
			pent[1] = JMP_REG_DISP_IND;
			pent += 2;
			PUT4((Word)(GOT_XRTLD * GOTENTSZ), pent);
		}
	}

}



/* void reloc_sect()
** process a single relocation section
** There are 2 types of relocation: globals/non-section locals, section locals
** for globals and non-section locals, the reference location or relocation
** entry addend field contains a signed constant.
** The relocated value is the value (vaddr) of the
** symbol referenced plus that constant.  If the relocation
** type is pc-relative, the address of the reference point
** is subtracted from the value.
** For section locals, the reference point contains a signed constant
** plus the offset of the symbol definition into the section
** in which it is defined. The relocated value is the constant
** plus offset plus virtual address of the section containing
** the symbol definition. Again, if the relocation
** type is pc-relative, the address of the reference point
** is subtracted from the value.
*/

void
reloc_sect(isect, rsect, osect)
	Insect	*isect, *rsect;
	Os_desc	*osect;
{
	register Word	value; 
	Word		roff, rstndx, rsize, refaddr;
	Word		R1addr, ovalue;
	unsigned char	rtype, R1type;
	register Rel	*reloc, *rend;
	register Ldsym	*rldsym;
	Boolean		noload = FALSE;

	DPRINTF(DBG_RELOC, (MSG_DEBUG,
			    "relocating section %s from file %s",
			    isect->is_name, isect->is_file_ptr->fl_name));

	/* is input section loadable ? */
	if (!(isect->is_shdr->sh_flags & SHF_ALLOC))
		noload = TRUE;

	rsize = rsect->is_shdr->sh_size;
	reloc = (Rel *)rsect->is_rawbits->d_buf;

	/* read relocation records - there may be 2 types of relocation
	 * entries: one with an addend field and one without
	 * A relocation section will be all of one or the other kind.
	 * This code is only for one without an addend
	 */

	rend = (Rel *)((char *)reloc + rsize);
	for ( ; reloc < rend; ++reloc) {
		rtype = ELF32_R_TYPE(reloc->r_info);
		roff = reloc->r_offset;
		rstndx = ELF32_R_SYM(reloc->r_info);
		DPRINTF(DBG_RELOC, (MSG_DEBUG,
				    "reloc: type %d, offset %d, rstndx %d",
				    rtype, roff, rstndx));

		/* get the ld symbol table version of the relocation symbol,
		 * and its new updated value 
		 */
		rldsym = rsect->is_file_ptr->fl_oldindex[rstndx];
		if (ELF32_ST_BIND(rldsym->ls_syment->st_info) == STB_LOCAL &&
			rldsym->ls_syment->st_shndx == SHN_UNDEF &&
			rldsym->ls_syment->st_name != 0) {
			lderror(MSG_FATAL,
				"relocation against undefined local symbol %s from file %s",
				rldsym->ls_name, rldsym->ls_flptr->fl_name);
		}
		value = rldsym->ls_syment->st_value;

		/* calculate virtual address of reference point;
		 * equals offset into section + vaddr of section
		 * for loadable sections, or offset plus section
		 * displacement for nonloadable sections
		 */
		if (!noload && (aflag || dmode))
			refaddr = roff + isect->is_newVAddr;
		else
			refaddr = roff + isect->is_displ;

		/* static case - preserving relocations  (ld -r or ld -a -r) */
		if (rflag) {
			if( (ELF32_ST_BIND(rldsym->ls_syment->st_info) == STB_LOCAL)
			&& (ELF32_ST_TYPE(rldsym->ls_syment->st_info) == STT_SECTION) ) {
			/* relocation off section symbol - update
			 * reference location to account for displacement
			 * of input section containing symbol definition
			 */
				if (!aflag)
					do_reloc(isect, rtype, roff, rldsym->ls_scnptr->is_displ);
				BUILD_RELOC(osect, rtype, rldsym->ls_scnptr->is_outsect_ptr->os_ndxsectsym, refaddr, &orels);
			}
			else 
				BUILD_RELOC(osect, rtype, rldsym->ls_outndx, refaddr, &orels);
			if (!aflag)
				continue;
		}

		/* building an absolute file or shared object */
		if (ELF32_ST_BIND(rldsym->ls_syment->st_info) == STB_LOCAL) {
			if (ELF32_ST_TYPE(rldsym->ls_syment->st_info) == STT_SECTION) {
				/* for section symbols, value is vaddr of
				 * the section for loadable sections,
				 * displacement of the input section
				 * within its output section for non-loadable
				 */
				if (rldsym->ls_scnptr->is_shdr->sh_flags & SHF_ALLOC)
					value = rldsym->ls_scnptr->is_newVAddr;
				else
					value = rldsym->ls_scnptr->is_displ;
			}
			else if (rtype == R_386_GOTPC)
				value = (Word) (got_sect->is_newVAddr) - refaddr;
			else if (rtype == R_386_GOT32) {
				/* global offset table relocation -
				 * relocate reference to got entry
				 * and initialize got entry if not
				 * already done; if a shared object,
				 * create an output reloc entry for got
				 * since address we calculate is relative
				 * to 0
				 */
				ovalue = value;
				value = (Word) (rldsym->ls_GOTndx * GOTENTSZ);
				if (!rldsym->ls_GOTdone) {
					rldsym->ls_GOTdone = TRUE;
					R1addr = (Word)((char *)(got_sect->
						is_rawbits->d_buf) +
						(rldsym->ls_GOTndx * GOTENTSZ));
					PUT4(ovalue, (char *)R1addr);
					if (Gflag) {
						BUILD_RELOC(got_sect->
							is_outsect_ptr,
							R_386_RELATIVE,
							STN_UNDEF,
							value +
							((Word) got_sect->
							is_newVAddr), &grels);
					}
				}
			}
			if (rtype == R_386_GOTOFF) {
				/* static offset from the GOT */
				ovalue = value;
				value -= (Word) (got_sect->is_newVAddr);
			}
			if (IS_PC_RELATIVE(rtype))
				value -= refaddr;
			else if (Gflag && !noload) { 
				if (rtype != R_386_GOT32 && rtype != R_386_GOTOFF) {
					R1type = R_386_RELATIVE;
					BUILD_RELOC(osect, R1type, STN_UNDEF,
					refaddr, &orels);
				}
			}
			do_reloc(isect, rtype, roff, value);
			continue;
		} 


		/* if here, we have a global or weak symbol */

		if (rtype == R_386_GOT32) {
			/* relocate reference to got entry for this
			 * symbol; if got entry has not already
			 * been initialized, do so and create
			 * an output relocation record for it if necessary
			 */
			ovalue = value;
			value = (Word) (rldsym->ls_GOTndx * GOTENTSZ);
			if (!rldsym->ls_GOTdone) {
				rldsym->ls_GOTdone = TRUE;
				R1addr = (Word)((char *)(got_sect->is_rawbits->
					d_buf) + (rldsym->ls_GOTndx * GOTENTSZ));

				/* if building a .so (w/o Bsymbolic) or
				 * the symbol is undefined or defined in a
				 * .so, we initialize the got entry with 0
				 * and create a reloc entry; otherwise
				 * we place the symbol value in the got
				 * entry - if building a .so w/ Bsymbolic
				 * we still need a reloc entry, since the
				 * value calculated is 0-based
				 */
				if ((rldsym->ls_deftag < REF_RELOBJ) ||
					(rldsym->ls_syment->st_shndx 
						== SHN_UNDEF) ||
					(Gflag && !Bflag_symbolic)) {
						PUT4(0, (char *)R1addr);
						BUILD_RELOC(got_sect->is_outsect_ptr, R_386_GLOB_DAT, rldsym->ls_outndx, value + ((Word) got_sect->is_newVAddr), &grels);
					}
				else {
					PUT4(ovalue, (char *)R1addr);
					if (Gflag && Bflag_symbolic) {
						BUILD_RELOC(got_sect->
							is_outsect_ptr,
							R_386_RELATIVE,
							STN_UNDEF,
							value + ((Word) got_sect->is_newVAddr), &grels);
					}
				}
			}
			do_reloc(isect, rtype, roff, value);
			continue;
		} 
		if (rtype == R_386_GOTPC) {
			value = (Word) (got_sect->is_newVAddr) - refaddr;
			do_reloc(isect, rtype, roff, value);
			continue;
		}
		if (rtype == R_386_PLT32) {
			/* procedure linkage table reloc -
			 * if we don't have a definition or are
			 * building a shared object (w/o Bsymbolic)
			 * we create a plt entry, and relocate this
			 * reference to that entry - else we
			 * relocate reference directly to symbol
			 */
			if ((Gflag && !Bflag_symbolic) ||
				(rldsym->ls_deftag < REF_RELOBJ) ||
				(rldsym->ls_syment->st_shndx == SHN_UNDEF)) {
				value = (Word)(plt_sect->is_newVAddr) +
					(rldsym->ls_PLTndx * PLTENTSZ);
				if (!rldsym->ls_PLTdone) {
					rldsym->ls_PLTdone = TRUE;
					plt_entry(rldsym, prels);
					BUILD_RELOC(plt_sect->is_outsect_ptr,
						R_386_JMP_SLOT,
						rldsym->ls_outndx,
						(Word)(got_sect->is_newVAddr) +
						rldsym->ls_PLTGOTndx * GOTENTSZ,
						&prels);
				}
			}
			value -= refaddr; /* plt references are pc-relative */
			do_reloc(isect, rtype, roff, value);
			continue;
		} 

		if ((rldsym->ls_deftag >= REF_RELOBJ) && 
			(rldsym->ls_syment->st_shndx != SHN_UNDEF)) {
			/* symbol defined in a dot-o - 
			 * non-GOT, non-PLT relocation 
			 * if building a .so (w/o Bsymbolic) just create 
			 * a reloc entry since we don't know if definition 
			 * we have is one that will be used at run-time; 
			 * else do relocation if building a .so w/ Bsymbolic 
			 * and the reloc type is not pc-relative, 
			 * create a reloc entry, since vaddr is 0-based
			 */
			if (Gflag && !Bflag_symbolic && !noload) {
				BUILD_RELOC(osect, rtype, rldsym->ls_outndx, refaddr, &orels);
			} else {
				if (IS_PC_RELATIVE(rtype))
					value -= refaddr;
				else if (Gflag && Bflag_symbolic && !noload) {
					R1type = R_386_RELATIVE;
					BUILD_RELOC(osect, R1type, STN_UNDEF, refaddr, &orels);
				}
				do_reloc(isect, rtype, roff, value);
			}
			continue;
		} 
		if ((rldsym->ls_deftag == REF_DEFN) || 
			(rldsym->ls_syment->st_shndx == SHN_UNDEF)) {
			/* undefined symbol or defined in a shared object 
			 * if symbol undefined or building a shared object 
			 * or bflag is set
			 * just create an output reloc entry; if building
			 * an a.out and bflag is not set:
			 *  1) if symbol is a function, create a plt entry
			 *        for it and relocate reference to that entry
			 *  2) if symbol is an object we have already
			 *	  allocated space for it in bss; we relocate
			 *	  the reference to that space; if original
			 *	  symbol was initialized data we also create
			 *	  a special reloc entry that tells the 
			 *	  dynamic linker to copy the data at run-time;
			 */	  

			if (Gflag || bflag || 
				(rldsym->ls_deftag != REF_DEFN) ||
				(ELF32_ST_TYPE(rldsym->ls_syment->st_info) == STT_NOTYPE)) {
				BUILD_RELOC(osect, rtype, rldsym->ls_outndx, refaddr, &orels);
			}
			else {
				if (ELF32_ST_TYPE(rldsym->ls_syment->st_info)
					== STT_FUNC) {
					/* function */
					value = (Word)(plt_sect->is_newVAddr) +
						(rldsym->ls_PLTndx * PLTENTSZ);
					if (!rldsym->ls_PLTdone) {
						rldsym->ls_PLTdone = TRUE;
						plt_entry(rldsym, prels);
						BUILD_RELOC(plt_sect->
							is_outsect_ptr,
							R_386_JMP_SLOT,
							rldsym->ls_outndx,
							(Word)(got_sect->
							is_newVAddr) +
							rldsym->ls_PLTGOTndx *
							GOTENTSZ, &prels);
					}
				}
				else if ((ELF32_ST_TYPE(rldsym->ls_syment->st_info) == STT_OBJECT) &&
				(rldsym->ls_syment->st_shndx != SHN_ABS)) {
					if (rldsym->ls_scnptr->is_shdr->
						sh_type != SHT_NOBITS) {
						if (!rldsym->ls_COPYdone) {
							rldsym->ls_COPYdone = TRUE;
							BUILD_RELOC(bss_sect->is_outsect_ptr, R_386_COPY, rldsym->ls_outndx, value, &copyrels);
						}
					}
				}
				/* are other types errors ? */
				if (IS_PC_RELATIVE(rtype))
					value -= refaddr;
				do_reloc(isect, rtype, roff, value);
			}
		}
	} /* end of while reloc loop */
	return;
}
