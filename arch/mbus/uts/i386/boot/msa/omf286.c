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

#ident	"@(#)mbus:uts/i386/boot/msa/omf286.c	1.3"

#include "../sys/boot.h"
#include "../sys/s2main.h"
#include "../sys/error.h"
#include "../sys/omf2386.h"

extern	char	file_read_msg [];
extern	char	no_tss_msg [];

extern	ushort	ourDS;
extern	char	bdevice_name [MAX_BDEVICE_VALUE];
extern	ushort	prot_mode_boot;
extern	ushort	debug_on_boot;
extern	ushort	mem_alias_sel;
extern	ulong	status;
extern	ulong	actual;

#ifndef lint
#pragma pack(1)
#endif

struct desc_str {
	ushort	limit ;
	ulong	base ;
} ;

struct seg_desc_str {
        ushort seg_limit0_15;
        char seg_base0_7;
        char seg_base8_15;
        char seg_base16_23;
        char attr_1;
        char attr_2;	/* contains limit bits 16-19 */
        char seg_base24_31;
} seg_desc;

struct f286_header {
	char space [3];
	char date_time [16];
	char creator [41];
	struct desc_str gdt_desc;
	struct desc_str idt_desc;
	ushort tss_select;
} f286_hdr;
 
struct tss_286_str {
        ushort backlink;
        char stack_ptrs [12];
        ushort ip;
        ushort flag;
        ushort ax;
        ushort cx;
        ushort dx;
        ushort bx;
        ushort sp;
        ushort bp;
        ushort si;
        ushort di;
        ushort es;
        ushort cs;
        ushort ss;
        ushort ds;
        ushort ldt;
} tss_286;

struct abstxt {
	char phys_addr_b[3];
	unsigned short len;
} abstxt;

#ifndef lint
#pragma pack()
#endif

struct toc {
	long abstxt_offset;
	long debtxt_offset;
	long last_offset;
	long next;
	long reserved;	
} toc;
struct desc_str our_gdt_desc;
struct desc_str our_idt_desc;

struct desc_str target_gdt_desc;
struct desc_str target_idt_desc;

char target_boot_string [256];
char trg_file_name [MAX_PARAM_VALUE];
struct seg_desc_str our_tss_base;
struct tss_286_str our_tss_rec;

ulong phys_addr;
ulong phys_len;
ulong target_tss_base;
ulong k286_start;

do_omf286(tfile_name)
register char tfile_name [MAX_PARAM_VALUE];
{	register long offset;
	register long last_offset;
	register int i;	

	/* save the file name being passed in */ 
	strncpy(&trg_file_name[0], &tfile_name[0], MAX_PARAM_VALUE);	

	/* read in the header and save the necessary fields */ 

	BL_file_read((char *)&f286_hdr, ourDS, (ulong)(sizeof(f286_hdr)), 
			&actual, &status);
	if ((status != E_OK) || (actual != (ulong)(sizeof(f286_hdr)))) 
		error(STAGE2, (ulong)status, (char *)file_read_msg);

	target_gdt_desc.limit = f286_hdr.gdt_desc.limit;
	target_gdt_desc.base = f286_hdr.gdt_desc.base;
	target_idt_desc.limit = f286_hdr.idt_desc.limit;
	target_idt_desc.base = f286_hdr.idt_desc.base;

	/* Read Table of Contents */

	BL_file_read((char *)&toc, ourDS, (ulong) sizeof(toc), 
			&actual, &status);
	if ((status != E_OK) || (actual != (ulong)sizeof toc)) 
		error(STAGE2, (ulong)status, (char *)file_read_msg);

	/* go to start of absolute text */

	scan_to((ulong) ((sizeof(f286_hdr)) + 2) + (ulong) (sizeof(toc)), 
							toc.abstxt_offset);
 
	/* if protected mode and no tss then there is an error */
	if (prot_mode_boot) 
		if (f286_hdr.tss_select == 0)
			error(STAGE2, E_NO_TSS, (char *)no_tss_msg);

	last_offset = (toc.debtxt_offset == 0 ? toc.last_offset:toc.debtxt_offset);
	offset = toc.abstxt_offset;
	i = 0;

	/* find the code to be loaded */
	while (offset < last_offset) {

		char temp_addr[4];
		
		BL_file_read((char *)&abstxt, ourDS, (ulong) sizeof(abstxt), 
			&actual, &status);
		if ((status != E_OK) || (actual != (ulong) sizeof(abstxt))) 
			error(STAGE2, (ulong)status, (char *)file_read_msg);
		
		temp_addr[0] = abstxt.phys_addr_b[0];
		temp_addr[1] = abstxt.phys_addr_b[1];
		temp_addr[2] = abstxt.phys_addr_b[2];
		temp_addr[3] = 0;
		
		phys_addr = *(ulong *) temp_addr;
		phys_len = (ulong) abstxt.len;

		/* the real mode start is the address of the first record */
		/* the first time through it is put in correct */
                /* format, sel : offset */ 

		if (i == 0) {
			k286_start = (phys_addr << 12) & 0xF0000000;
			k286_start += phys_addr & 0xFFFF;
		}
		/* load data into correct place */

		BL_file_read((char *)phys_addr, mem_alias_sel, 
                                      phys_len, &actual, &status);
		if ((status != E_OK) || (actual != phys_len)) 
			error(STAGE2, (ulong)status, (char *)file_read_msg);

		/* set new offset */

		offset += phys_len + sizeof(abstxt);
		i++;
	}
	
	/* these calls do not return */
	
	if (prot_mode_boot)
		do_protected();
	else
		do_real(k286_start);
}

do_protected()
{
	/* save gdt, idt */
	gdt_move(&our_gdt_desc, 0);
	idt_move(&our_idt_desc, 0);
	
	/* if gdt, idt entry is empty copy from fw table */
        /* to target table */
	copy_desc(our_gdt_desc, target_gdt_desc);
	copy_desc(our_idt_desc, target_idt_desc);
	
	/* set tss registers, load new gdt and idt, jump to start */
        /* protected mode address */
	set_tss();
	gdt_move(&target_gdt_desc, 1);
	idt_move(&target_idt_desc, 1);
	jump_tss(0, (f286_hdr.tss_select & MASK_INDEX)); /* offset, selector */
}

do_real(kernel_start)
register ulong kernel_start;
{
	startunix(kernel_start, debug_on_boot);

}

copy_desc(fw_desc, target_desc)
register struct desc_str fw_desc;
register struct desc_str target_desc; 
{	unsigned int i;
	
	/* we do not want to access past the end of the target descriptor */ 
        /* table or the end of the fw descriptor table */

	for (i = 0; ((i < target_desc.limit/8) && (i < fw_desc.limit/8)); i++) {

	/* we only want to copy if the target descriptor table entry is */ 
        /* empty and if the fw descriptor table entry is not empty */

		if ((check_entry(target_desc.base + (i * 8), mem_alias_sel))) 
			if (!(check_entry(fw_desc.base + (i * 8), 
                                                           mem_alias_sel))) 

				iomove(fw_desc.base + (i * 8), mem_alias_sel, 
                                   target_desc.base + (i * 8), mem_alias_sel, 
                                        sizeof(seg_desc));
	}
} 

char temp_base [4];
set_tss()
{	register char fname_len, dname_len, index;

	/* find physical location of tss */

	target_tss_base = target_gdt_desc.base + 
                                       (f286_hdr.tss_select & MASK_INDEX); 

	/* make a local copy of the descriptor (our_tss_base) and */
	/* rearrange those base fields to proper format */

	iomove(target_tss_base, mem_alias_sel, &our_tss_base, ourDS, 
                                                          sizeof(seg_desc));
	temp_base[0] = our_tss_base.seg_base0_7;
	temp_base[1] = our_tss_base.seg_base8_15;
	temp_base[2] = our_tss_base.seg_base16_23;
	temp_base[3] = our_tss_base.seg_base24_31;
	target_tss_base = *(ulong *) temp_base;
	
	/* use the formatted base to make a local copy of the tss segment */
        /* and registers (our_tss_rec) */

	iomove(target_tss_base, mem_alias_sel, &our_tss_rec, ourDS, 
							sizeof(tss_286));
	/* make the device name, file name string to be booted */
	/* the format is device name length:device:filename length filename */
	/* device name length includes the :'s */
	
	index = 0;

	dname_len = strlen(bdevice_name);
	fname_len = strlen(trg_file_name);	
	target_boot_string[index++] = dname_len +2;	
	target_boot_string[index++] = ':';	
	strncpy(&target_boot_string[index], bdevice_name, dname_len);	
	index += dname_len;
	target_boot_string[index++] = ':';	
	target_boot_string[index++] = fname_len;	
	strncpy(&target_boot_string[index], trg_file_name, fname_len);	

	/* fill in the local copy of the tss registers */
	/* si contains the selector to the boot name string */
	/* di contains the offset - this works because the string is located */
	/* in the first 64K of memory (this is guaranteed by the position */
  	/* of this file when linked in the makefile) */	

	our_tss_rec.si = ourDS;
	our_tss_rec.di = (ushort)target_boot_string;
	our_tss_rec.cx = 0x1234;
	if (debug_on_boot)
		our_tss_rec.dx = 0x1235;
	else
		our_tss_rec.dx = 0x1234;	

	/* move the filled in copy to the physical location of target tss */ 

	iomove(&our_tss_rec, ourDS, target_tss_base, mem_alias_sel, 
                                                          sizeof(tss_286));
}
