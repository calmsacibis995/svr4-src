/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)boot:boot/at386/load.c	1.1.3.1"

#include "../sys/boot.h"
#include "sys/sysmacros.h"
#include "../sys/libfm.h"
#include "../sys/hdrs.h"
#include "../sys/inode.h"

#include "../sys/error.h"
#include "../sys/dib.h"

struct bootattr battrs;
struct bootattr *battrp = &battrs;
struct boothdr	bhdr;
struct dib dib;			/* used as placebo for BL_file_xxx */
ulong actual, status;		/* globals for BL_file_xxx */

extern struct coffhdrs	coff;

extern inode_t in;
extern ushort ourDS;
extern void BL_file_open(), BL_file_read();
extern getchar();
extern off_t disk_file_offset;

#define RESERVED_SIZE	0x3000L
#define DBUFSIZ	1024			/* used in scan_to() */

/* standard error message; the 0L is the 'error' return code */
#define ERROR(msg, arg)	(printf("\nboot: Cannot load %s: %s\n", arg, msg), 0L)

/* occasionally check for keyboard input to abort boot process */
#define CHECK_KBD	if ( enforce && ischar() ) return( 0 ) 

/* global key board check..   used in breadi and bfsread */
kbd_ck_val_t kbd_check;


/*
*
* ABSTRACT:
* 	This procedure scans the input object file upto the offset 
* 	specified by the input parameter.  It dumps any data while scanning.
*
******************************************************************************/
void
scan_to(current_offset, target_offset)
off_t 	current_offset;
off_t	target_offset;
{
	ulong	bytes_to_read;
	ulong	num_blocks;
	char	dump_buf[DBUFSIZ];
	
	if (current_offset == target_offset)
		return;

	if (current_offset > target_offset) {
		status = E_SCAN;
		return;
	}
	else {
		bytes_to_read = target_offset - current_offset ;
		if (bytes_to_read < sizeof(dump_buf)) {
			/* first partial block */
	 		BL_file_read((char *)dump_buf, ourDS, bytes_to_read, 
				&actual, &status);
			if (status != E_OK) 
				return;
			
		}
	    else {
	        /* read whole blocks */
	        num_blocks = bytes_to_read / sizeof(dump_buf) ;
	        while (num_blocks --) {

				/* read file in dump_buf size chunks */
	            
		 		BL_file_read((char *)dump_buf, ourDS, 
				  (long)sizeof dump_buf, &actual, &status);
				if (status != E_OK) 
					return;
			}

	        /* now read partial block if any */

	        bytes_to_read = bytes_to_read % sizeof(dump_buf);
	        BL_file_read(dump_buf, ourDS, bytes_to_read, 
		 				&actual, &status);
			if (status != E_OK) 
				return;
		}
	}
}

/* The stand-alone loader; returns a zero load address in case of error */

unsigned long
bload( path, enforce, loadaddr )
char	*path;
int	enforce;
paddr_t	loadaddr;
{
	paddr_t	kstart, endseg;
	long	size;
	int	offset, availseg;
	int	foundload = FALSE, 
		foundbki = FALSE;
	int	c;			/* count of valid segments */
	int	i;
	int	nsect;
	struct	bootsect bsect;
	short	bkiinfo = -1;

	/* open the file */
	BL_file_open (path, &dib, &status);
	if (status != E_OK)
		return (ERROR("file not opened", path));
	debug(printf("LOAD %s opened, battrp->size %ld, in.size: %ld \n", path, battrp->size, in.i_size));
	/* get header information from file */
	/* bhdr is initialized as well as coffhdrs and elfhdr as approp */
	gethead(&bhdr);    
	debug(printf("LOAD boothdr, type: %ld, nsect: %ld, entry: 0x%lx\n", bhdr.type, bhdr.nsect,bhdr.entry));
	if (bhdr.type == NONE) 
		return (ERROR("not an 80386 ELF or COFF binary", path));

	CHECK_KBD;

	/*  set up binfo for loading kernel */
	if (enforce) {
		kbd_check = ENFORCE;
		loadaddr = binfo.memavail[availseg = 0].base;
		endseg = binfo.memavail[0].extent + loadaddr; 

		/* reserve space for bootstrap argument passing */
		binfo.memused[0].base = 0L;
		binfo.memused[0].extent = RESERVED_SIZE;
		binfo.memused[0].flags = 0;
		binfo.memusedcnt = 1;
	} else 
		kbd_check = IGNORE;


	if (loadaddr < RESERVED_SIZE)
		loadaddr = RESERVED_SIZE; /* prevent scribble */

	kstart = loadaddr;

	/* load the program in sections/segments */
	/* if enforce, check for BKI version;  abort if BKI is wrong or absent */

	for (bsect.size = 0, nsect = 0; nsect < bhdr.nsect; ) {
		if (bsect.size == 0) {
			/* getsect returns with next sec/segment */
			if (getsect (&bsect) == 0) {
			scan_to(disk_file_offset, bsect.offset);
			nsect++;
			} else
				return (ERROR("cannot read segment/sections", path));
		}

		switch (bsect.type) {
		    case BKI:
			foundbki = TRUE;
			/* regardless of ELF/COFF, load bkiinfo */
			if (enforce) {
				BL_file_read (&bkiinfo, ourDS, 2, &actual, &status);
				if ((actual != 2) | (status != E_OK))
					return (ERROR("cannot read BKI section", path));
				debug(printf("BKI found version %ld\n", bkiinfo);getchar());
				if (bkiinfo < BKIVERSION) 
					return (ERROR("BKI too old", path));
				if (bkiinfo > BKIVERSION)
					return (ERROR("BKI too new", path));
			}
			bsect.size = 0;
			break;
		   case TLOAD:
		   case DLOAD:
			foundload = TRUE;
			/* calculate size of segment we have room to load */
			if (enforce) {
				/*  check for room on curnt memavail */
				while ((size = (endseg - loadaddr)) <=0 ) {
					if (++availseg >= binfo.memavailcnt)
						return (ERROR("required memory for kernel is not present", path));
					loadaddr = binfo.memavail[availseg].base;
					endseg = loadaddr + binfo.memavail[availseg].extent;
				}
				if (size > bsect.size)
					size = bsect.size;
			} else
				size = bsect.size;

			/* load text or data, as much as we have room for */
			/* loadaddr is already phys, bread will phyaddr again */
				
			BL_file_read(loadaddr-physaddr(0), ourDS, size, &actual, &status);
			if (actual != size)
				return (0); /* key pressed ! */

			debug(printf("loaded segment/section at %lx, extent %lx from x%lx\n", loadaddr, size, bsect.offset)); 
			/* check if start address in this seg */
			if (INTERVAL(bsect.addr, size, bhdr.entry)) 
				kstart = bhdr.entry - bsect.addr + loadaddr;

			/* reflect usage */
			if (enforce) {
				binfo.memused[binfo.memusedcnt].base = loadaddr;
				binfo.memused[binfo.memusedcnt].extent =
					ctob(btoc(size));
				binfo.memused[binfo.memusedcnt++].flags =
					binfo.memavail[availseg].flags | 
				((bsect.type == TLOAD) ? B_MEM_KTEXT : B_MEM_KDATA);
			}

			loadaddr = ctob(btoc(loadaddr + size));

			if ((bsect.size -= size) > 0) {
				bsect.addr += size;
				bsect.offset += size;
			}
			break;

		   default:
			bsect.size = 0;
			break;
		}
	}

	if ( !foundload )
		return (ERROR("missing text or data segment", path));

	if ( enforce && !foundbki )
		return (ERROR("missing BKI segment", path));

	CHECK_KBD;

	/* return start physical address for the binary */

	return(kstart);
}
