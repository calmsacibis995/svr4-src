/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)boot:boot/bootlib/elf.c	1.1.2.1"

#include "../sys/boot.h"
#include "../sys/libfm.h"
#include "../sys/inode.h"
#include "../sys/hdrs.h"
#include "sys/sysmacros.h"
#include "sys/ino.h"
#include "sys/alttbl.h"

/*
 * Generic boot filesystem routines.
 */

#include	"sys/elog.h"
#include	"sys/iobuf.h"
#include	"sys/vtoc.h"
#include	"sys/vnode.h"
#include	"sys/fs/bfs.h"
#include	"sys/elf.h"

Elf32_Ehdr elfhdr;

struct coffhdrs	coff;

short headmagic;			/* holds magic number */
int	sec_cnt;
int	b_array_valid;
struct	bootsect  bsect_array[NBSECT];

extern	ulong	actual, status;
extern struct boothdr bhdr;
extern ushort ourDS;
extern int iomove();

#if defined(MB1) || defined(MB2)
#define MINHDRSZ sizeof(headmagic)
#else
#define MINHDRSZ MIN(sizeof (coff), sizeof (elfhdr))
#endif


/*
 *	Get the header of an opened file;  fd is ignored if S5
 *		bhdr is filled in when done
 */
char	common_buf[MINHDRSZ];
int
gethead(bhdr)
struct boothdr *bhdr;
{
	register ushort  residual;

	/* when header is retrieved nullify bsect_array content */
	b_array_valid=FALSE;

	/* read minimal into a common buffer to retrieve headmagic */
	BL_file_read(&common_buf[0], ourDS, MINHDRSZ, &actual, &status);
	iomove ((char *)&common_buf[0], ourDS, (char *)&headmagic, ourDS, sizeof(headmagic));
	debug (printf("Read header magic 0x%lx\n", headmagic));

	sec_cnt=0;
	switch (headmagic) {
	case I386MAGIC:
		/* copy into coff as much as available */
		iomove((char *)&common_buf[0], ourDS, (char *)&coff, ourDS, MINHDRSZ);
		/* read in remaining residual */
		if ( (residual = (sizeof (coff) - MINHDRSZ)) > 0 )
			BL_file_read((char *) &coff + MINHDRSZ, ourDS, residual,
			&actual, &status);
		bhdr->type = COFF;
		bhdr->nsect = (int)coff.mainhdr.f_nscns;
		bhdr->entry = (ulong)coff.secondhdr.entry;
		debug (printf("magic: 0x%lx, tsize: 0x%lx, entry: 0x%lx, text_start: 0x%lx\n", coff.secondhdr.magic, coff.secondhdr.tsize, coff.secondhdr.entry, coff.secondhdr.text_start));
		debug (printf("COFF header, 0x%lx sections\n", bhdr->nsect));
		return(headmagic);
	case ELFMAGIC:
		iomove((char *)&common_buf[0], ourDS, (char *)&elfhdr, ourDS, MINHDRSZ);
		if ((residual = (sizeof (elfhdr) - MINHDRSZ)) > 0 )
			BL_file_read((char *) &elfhdr+MINHDRSZ, ourDS, residual,
			&actual, &status);
		bhdr->type = ELF;
		bhdr->nsect = (int)elfhdr.e_phnum;
		bhdr->entry = (ulong)elfhdr.e_entry;
		debug (printf("ELF header, 0x%lx segments\n", bhdr->nsect));

		return(headmagic);

	default: 
		bhdr->type = NONE;
		return(headmagic);
	}
}
	
struct scnhdr coffsect;
Elf32_Phdr elfphdr;

getsect(bsect)
struct bootsect *bsect;
{
	register int	i;

	/* 
	 * After the file headers, next appear section hdrs, if COFF,
	 * and program headers if ELF.   In order for sequential read
	 * of the sectionns to work, we store them in bsect_array[].
	 * b_array_valid flag is set when bsect_array is filled in,
	 * we skip it there after.
	 * Global var sec_cnt gets incremented every time getsect is
	 * called.
	 */


	switch(headmagic){

	case I386MAGIC:
		if (coff.mainhdr.f_magic != I386MAGIC)
			return(1);
		if ( b_array_valid != TRUE ) {
			sec_cnt = 0;
			for (i=0; i < bhdr.nsect; i++) {
				BL_file_read(&coffsect, ourDS, sizeof(coffsect),
					     &actual, &status);
				if (coffsect.s_flags & STYP_TEXT )
					bsect_array[i].type = TLOAD;
				else if (coffsect.s_flags & STYP_DATA)
					bsect_array[i].type = DLOAD;
				else if ((*(long *)coffsect.s_name == BKINAME) &&
			 		(coffsect.s_size >= 2) ) 
					bsect_array[i].type = BKI;
				else
					bsect_array[i].type = NOLOAD;
		
				bsect_array[i].addr = coffsect.s_vaddr;
				bsect_array[i].size = coffsect.s_size;
				bsect_array[i].offset = coffsect.s_scnptr;
			}
			b_array_valid=TRUE;
		}
		break;

	case ELFMAGIC:
		if (elfhdr.e_ident[EI_MAG0] != 0x7f && elfhdr.e_ident[EI_MAG1] != 'E')
				return(1);
		if ( b_array_valid != TRUE ) {
			/* scan_to goes here */
			scan_to(sizeof(elfhdr), elfhdr.e_phoff);
			sec_cnt=0;
			for (i=0; i < (int)elfhdr.e_phnum; i++) {
				BL_file_read(&elfphdr, ourDS, 
					elfhdr.e_phentsize, &actual, 
					&status);
				switch (elfphdr.p_type) {
					case PT_LOAD:
						switch (elfphdr.p_flags &
						       (PF_R | PF_W | PF_X)) {
							case (PF_R | PF_W | PF_X):
								bsect_array[i].type = DLOAD;
								break;
							case (PF_R | PF_X):
								bsect_array[i].type = TLOAD;
								break;
							default:
								bsect_array[i].type = NOLOAD;
						}	
						break;
					case PT_NOTE:
						bsect_array[i].type = BKI;
						break;
					default:
						bsect_array[i].type = NOLOAD;
				}
				bsect_array[i].addr = elfphdr.p_vaddr;
				bsect_array[i].size = elfphdr.p_filesz;
				bsect_array[i].offset = elfphdr.p_offset;
			}
			b_array_valid=TRUE;
		}
		break;
	default: 
		return(1);

	}
	bsect->type=bsect_array[sec_cnt].type;
	bsect->addr=bsect_array[sec_cnt].addr;
	bsect->size=bsect_array[sec_cnt].size;
	bsect->offset=bsect_array[sec_cnt].offset;
	sec_cnt++;
	return(0);
}
