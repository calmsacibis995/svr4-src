/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */

#ident	"@(#)mbus:uts/i386/boot/msa/pload.c	1.3.1.1"

/*
 * This is the MSA Second stage bootstrap loader module.  It executes
 * in 386 protected mode and loads a COFF or ELF format file from a
 * Unix file system and is intended for loading UNIX itself.  It
 * also provides an interface to an OMF file format loader.
 */

#include "../sys/boot.h"
#include "../sys/inode.h"
#include "sys/ino.h"
#include "sys/elf.h"
#include "sys/dir.h"
#include "storclass.h"
#include "syms.h"
#include "sys/bolt.h"
#include "sys/sysmacros.h"
#include "../sys/dib.h"
#include "../sys/s2main.h"
#include "../sys/error.h"
#include "../sys/omf2386.h"
#include "../sys/hdrs.h"
#include "../sys/libfm.h"

#define MAX_SIZE	0xffffffff
#define MAX_SECTS	15		/* Maximum number of loadable sections allowed */
#define	MASK		0xfffffff	/* Address mask */

struct	bsl_mem_used_list	{
	ulong	block_start;
	ulong	block_length;
};

/* static	char	rootdev_key[] = "rootdev";
 * static	char	pipedev_key[] = "pipedev";
 * static	char	swapdev_key[] = "swapdev";
 * static	char	dumpdev_key[] = "dumpdev";
 * static	char	loading_msg[] = "Loading target file ";
 * static	char	target_open_msg[] = "Cannot open target file";
 * static	char	bad_magic_msg[] = "Bad magic number in target file";
 * static	char	file_read_msg[] = "File read error";
 * static	char	mem_overlap_msg[] = "Memory overlap error";
 * static	char	scan_past_msg[] = "Internal error - scan past file";
 */

extern	char	rootdev_key [];
extern	char	pipedev_key [];
extern	char	swapdev_key [];
extern	char	dumpdev_key [];
extern	char	loading_msg [];
extern	char	target_open_msg [];
extern	char	bad_magic_msg [];
extern	char	file_read_msg [];
extern	char	mem_overlap_msg [];
extern	char	scan_past_msg [];

extern	struct	bsl_mem_used_list	bmu_list_entry[10];
extern	struct	dib			dib_data;
extern	ushort	ourDS;
extern	ushort	mem_alias_sel;
extern	ushort	debug_on_boot;
extern	ushort	ramdisk_num;
extern	ulong	status;
extern	ulong	actual;

extern	void	do_omf386();
extern	void	do_omf286();

/*
 * define structure that includes both main header and extra header so
 * entire header information for file may be read at once.
 * Also declare buffer for holding transfers.
 *
 */

unsigned 	short	file_type;

struct coffhdrs coffhdr;
struct boothdr	bhdr;
struct bootsect bsect;
extern Elf32_Ehdr elfhdr;
Elf32_Shdr eshdr;

struct sectinfo {
	ulong offset;
	ulong start;
	ulong size;
	ulong type;
};

struct		bootinfo	binfo;
struct		sectinfo	section[MAX_SECTS];
ulong		kernel_start;
char		dump_buf [1024];	/* dump buffer */

SCNHDR  coffShdr;

/******************************************************************************
*
* TITLE: pload
*
* CALLING SEQUENCE:
*  	pload(path);
*
* INTERFACE VARIABLES:
* 	path
* 		is a POINTER to a null terminated string containing the pathname
* 		of the target file to be booted
*
* CALLING PROCEDURES:
* 	pboot
*
* CALLS:
*	BL_file_open, BL_file_read
*	error
*	loadexe
*
* ABSTRACT:
* 	This procedure opens the target file, verifies the magic number in the
* 	header, and transfers control to the COFF loader.
*
******************************************************************************/
pload(path)
register char *path;
{	register ulong	cnt;
			 
	BL_file_open(path, &dib_data, &status);
	if (status != E_OK) {
		cnt = strlen(target_open_msg);
		iomove(target_open_msg, ourDS, &dump_buf[0], ourDS, cnt);
		iomove((char *)path, ourDS, &dump_buf[cnt], ourDS,
			MAX_PARAM_VALUE);
		error(STAGE2, (ulong)status, (char *)dump_buf);
	}

	/*
	 * Get header info from file and check magic number, etc.
	 */

	file_type = gethead(&bhdr);

	if ((bhdr.type == NONE) && ((file_type & RMX_MASK) != BOOT_LOADABLE_286) &&
			((file_type & RMX_MASK) != BOOT_LOADABLE_386))
		error(STAGE2, E_BAD_MAGIC_NUM, bad_magic_msg);
	else {
		/* print loading message */
		cnt = strlen(loading_msg);
		iomove(loading_msg, ourDS, dump_buf, ourDS, cnt);
		iomove((char *)path, ourDS, &dump_buf[cnt], ourDS,
			MAX_PARAM_VALUE);
		error(STAGE2, E_OK, (char *)dump_buf);
	}

	if ((file_type & RMX_MASK) == BOOT_LOADABLE_286)
		do_omf286((char *) path);
	else
		if ((file_type & RMX_MASK) == BOOT_LOADABLE_386)
			do_omf386((char *) path);

	/* initialize/update the bootinfo structure */

	for (cnt = 0; cnt < sizeof(binfo); cnt++)
		((char *)&binfo)[cnt] = 0;

	/* The actual top of physical memory is available indirectly from
	 * the bmu_list_entry structure passed from the first stage. The
	 * first entry indicates where the first stage is located and
	 * by design that is at the top of physical memory.
	 */

	binfo.memavail[0].base = 0;
	binfo.memavail[0].extent = bmu_list_entry[0].block_start +
				   bmu_list_entry[0].block_length;
	binfo.memavail[0].flags = 0;
	binfo.memavailcnt = 1;

	/* Copy the program name to be booted into bargv[0] */

	strncpy(binfo.bargv[0], path, B_STRSIZ);
	binfo.bargc++;

	/*
	 * perform the actual load
	 */

	loadexe();
}

/******************************************************************************
*
* TITLE: loadexe
*
* CALLING SEQUENCE:
* 	loadexe();
*
* INTERFACE VARIABLES:
* 	None
*
* CALLING PROCEDURES:
* 	pload
*
* CALLS:
*	BL_file_read, BL_file_close, BL_seek
*	bs_disconnect
*	error
* 	scan_to
*	protect
*	iomove
*	startunix
*
* ABSTRACT:
* 	This procedure loads a COFF or ELF image from the boot source (disk, tape,
* 	or bootserver), loads any RAM disk image from tape, sets up the
* 	necessary data structure (bootinfo)  in RAM, and tranfers control
* 	to the kernel start routine.
*
******************************************************************************/
loadexe()
{	register ulong	shdr_offset;
	register ulong	tot_size;
	register ulong	mem_top;
	register ulong	firstbyte;
	register uint	i, no_of_sections;
	register ushort	no_of_scans;
	register ushort	done;
	register ushort	cnt;
	register ushort	temp;
 	register Elf32_Half	shdr_entsize;

	kernel_start = ( bhdr.entry << 12) & 0xF0000000;
	kernel_start += bhdr.entry & 0xFFFF;

	if (bhdr.type == COFF) {
		shdr_offset = sizeof(coffhdr);
		shdr_entsize = SCNHSZ;
	} else {
		shdr_offset = sizeof(elfhdr);
		shdr_entsize = elfhdr.e_phentsize;
	}

	no_of_scans = bhdr.nsect;

	i = 0;
	while (no_of_scans--) {
		getsect(&bsect, file_type);

		if ((bsect.type == TLOAD) || (bsect.type == DLOAD)) {
			section[i].start = bsect.addr & MASK;
			section[i].size = bsect.size;
			section[i].offset = bsect.offset;
			section[i].type = bsect.type;
			i++;
		}
		shdr_offset += (ulong) shdr_entsize;
	}
	no_of_sections = i;

	tot_size = shdr_offset;

	/* protect the location where text, data, and bss will go */

	for (i=0; i<no_of_sections; i++) {
		if (!(protect(section[i].start, section[i].size))) {
			status = E_MEM_OVERLAP;
			error(STAGE2, (ulong)status, (char *)mem_overlap_msg);
		}
	}

	/*
	 * Define memory segment reserved for the booted kernel.
	 */
	binfo.memusedcnt = 0;
	binfo.memused[binfo.memusedcnt].base = 0;
	binfo.memused[binfo.memusedcnt].extent = RESERVED_SIZE;
	binfo.memused[binfo.memusedcnt++].flags = 0;
	for (i=0; i<no_of_sections; i++) {

		/*  scan upto offset where we want to load from */

		if (section[i].offset > tot_size) {
			scan_to(tot_size, section[i].offset);
			tot_size = section[i].offset;
		}
		/* load the section */
		BL_file_read((char *)section[i].start, mem_alias_sel, section[i].size,
					&actual, &status);
		if (status == E_EOF) {
			status = E_OK;
		} else {
			if ((actual != section[i].size) || (status !=E_OK))
				error(STAGE2, (ulong)status, (char *)file_read_msg);
		}
		/*
		 * Define memory segments used by the booted kernel.
		 */
		binfo.memused[binfo.memusedcnt].base = section[i].start;
		binfo.memused[binfo.memusedcnt].extent = ctob(btoc(section[i].size));
		if (section[i].type == TLOAD)
			binfo.memused[binfo.memusedcnt++].flags |= B_MEM_KTEXT;
		else {
			binfo.memused[binfo.memusedcnt++].flags |= B_MEM_KDATA;
			mem_top = section[i].start + section[i].size;
		}
		tot_size += section[i].size;
	}

	/* all done - close the file */
	BL_file_close (&status);

	/* special for Bootserver booting */

	if (dib_data.hdr.device_type == BS_DIB)
		bs_disconnect(&status);

	/*
	 * compute firstbyte for the kernel - normalize to
	 * even page boundary.
	 */

	firstbyte = mem_top;
	if (firstbyte & 0xfff)
		firstbyte = (firstbyte | 0xFFF) + 1; 	/* align 4K */

	if (debug_on_boot)
		binfo.bootflags |= BF_DEBUG;
	else
		binfo.bootflags &= ~BF_DEBUG;

	binfo.bootflags |= BF_MB2SA;

#ifndef MB1
	/* check if any kernel params are specified in the BPS */
	ck_k_param((char *)rootdev_key, strlen(rootdev_key));
	ck_k_param((char *)pipedev_key, strlen(pipedev_key));
	ck_k_param((char *)swapdev_key, strlen(swapdev_key));
	ck_k_param((char *)dumpdev_key, strlen(dumpdev_key));
#endif

	/* I002 - calculate checksum */

	binfo.checksum = 0;
	for (cnt = 0; cnt < (sizeof(binfo)-sizeof(int)); cnt++)
		binfo.checksum += ((char *)&binfo)[cnt];

	/* copy the bootinfo structure to where the kernel is going to look */
	iomove(&binfo, ourDS, (char *)BOOTINFO_LOC, mem_alias_sel,
			sizeof(binfo));

	/* Now, the last leg of the relay race */
	startunix(kernel_start, binfo.bootflags & BF_DEBUG);
}

/******************************************************************************
*
* TITLE: scan_to
*
* CALLING SEQUENCE:
* 	scan_to(current_offset, target_offset);
*
* INTERFACE VARIABLES:
* 	curr_offset
* 		is of type LONG and is the current offset in the object file.
* 	target_offset
* 		is of type LONG and is the target offset in the object file
*		to scan.
*
* CALLING PROCEDURES:
* 	loadexe
*
* CALLS:
* 	BL_file_read
*	error
*
* ABSTRACT:
* 	This procedure scans the input object file upto the offset
* 	specified by the input parameter.  It dumps any data while scanning.
*
******************************************************************************/
scan_to(current_offset, target_offset)
register ulong 	current_offset;
register ulong	target_offset;
{
	register ulong	bytes_to_read;
	register ulong	num_blocks;

	if (current_offset == target_offset)
		return;

	if (current_offset > target_offset) {
		status = E_SCAN;
		error(STAGE2, (ulong)status, (char *)scan_past_msg);
	} else {
		bytes_to_read = target_offset - current_offset;
		if (bytes_to_read < sizeof(dump_buf)) {
			/* first partial block */
	 		BL_file_read((char *)dump_buf, ourDS, bytes_to_read,
				&actual, &status);
			if (status != E_OK)
				error(STAGE2, (ulong)status,
					(char *)file_read_msg);
		} else {
	        /* read whole blocks */
	        num_blocks = bytes_to_read / sizeof(dump_buf);
	        while (num_blocks --) {
				/* read file in dump_buf size chunks */
		 		BL_file_read((char *)dump_buf, ourDS,
				  (long)sizeof dump_buf, &actual, &status);
				if (status != E_OK)
					error(STAGE2, (ulong)status,
						(char *)file_read_msg);
			}

	        /* now read partial block if any */

	        bytes_to_read = bytes_to_read % sizeof(dump_buf);
	        BL_file_read(dump_buf, ourDS, bytes_to_read,
		 				&actual, &status);
			if (status != E_OK)
				error(STAGE2, (ulong)status,
					(char *)file_read_msg);
		}
	}
}

/******************************************************************************
*
* TITLE: protect
*
* CALLING SEQUENCE:
* 	safe = protect(current_offset, target_offset);
*
* INTERFACE VARIABLES:
*	safe
*		is of type SHORT and is TRUE is the block of memory specified
*		does NOT overlap with current entries in the bootstrap
*		loader used space list.
* 	physical_address
* 		is the physical address of the block of contiguous memory to
* 		be protected from overwriting. An entry for the physical
* 		memory is placed in the bootstrap loader used space list.
* 	length
* 		is the size of the memory block to be protected from
*		overwriting.
*
* CALLING PROCEDURES:
* 	loadexe
*
* CALLS:
* 	None
*
* ABSTRACT:
* 	This procedure checks if the block of contiguous memory specified
* 	by the physical address and length parameters is present in the
* 	bootstrap used space list.  If not present, it enters the block
* 	in the used space list and returns TRUE (1), else, it returns
* 	FALSE (0).
*
* 	Loaders use this list to avoid loading data into previously used
* 	memory, hence the block is protected (from a cooperating loader).
*
******************************************************************************/
protect (physical_address, length)
register ulong	physical_address;
register ulong	length;
{	register ushort	protected;
	register ushort	index;

	protected = TRUE;
	index = 0;

	/* Search current entries in used space list to find overlaps
	 */
	while ((protected) && (bmu_list_entry[index].block_length !=0) &&
		(index < 10)) {
		if (((physical_address + length) <=
				bmu_list_entry[index].block_start) ||
		     (physical_address >= (bmu_list_entry[index].block_start +
					  bmu_list_entry[index].block_length)))
			  /* no overlap yet */
			  index++;
		 else
			  protected = FALSE;
	}

	/* If memory is not overlapping with an existing entry, add it to
	 * the end of the list.
	 */
	if (protected && (index < 10)) {
		bmu_list_entry[index].block_start = physical_address;
		bmu_list_entry[index].block_length = length;
		index++;
		bmu_list_entry[index].block_start = 0;
		bmu_list_entry[index].block_length = 0;
	} else
		/* actually ran out of room in the used list */

		protected = FALSE;
	return(protected);
}

#ifndef MB1
/******************************************************************************
*
* TITLE: ck_k_param
*
* CALLING SEQUENCE:
* 	ck_k_param;
*
* INTERFACE VARIABLES:
* 	pname
* 		is a pointer to the parameter name
* 	len
* 		is the length of the parameter name
*
* CALLING PROCEDURES:
* 	loadexe
*
* CALLS:
* 	BP_get
*
* ABSTRACT:
* 	This procedure checks if the parameter specified
* 	exists in the BPS.  If the parameter exists, then it is
* 	passed to the booted kernel through the bootinfo structure.
*
******************************************************************************/
ck_k_param(pname, len)
register char	*pname;
register ulong	len;
{
	status = BP_get(pname, (char *)dump_buf, B_STRSIZ);
	if (status == E_OK) {
		strncpy(&binfo.bargv[binfo.bargc][0], pname, len);
		binfo.bargv[binfo.bargc][len] = '=';
		strncpy(&binfo.bargv[binfo.bargc][len + 1], dump_buf,
			strlen(dump_buf));
		binfo.bargc++;
	}
}
#endif /* ! MB1 */
