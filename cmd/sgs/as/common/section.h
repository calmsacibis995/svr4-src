/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/section.h	1.7"

#include <libelf.h>
#include "expand.h"
#include "codeout.h"

/*
 * section header information
*/
#define _TEXT ".text"
#define _DATA ".data"
#define _DATA1 "data1"
#define _BSS  ".bss"
#define _INIT ".init"
#define _RODATA ".rodata"
#define NOT_USER 0x0100

/*
 * output section information
 */

struct  scninfo {
	/* header information */
        char    *name; 
        long    type;
        long    flags;
	long	size;
        long    link;
        long    info;
        long    addralign;
	/* other information */
	unsigned int	s_nreloc;	/* number of relocation entries  */
	Elf32_Shdr	*s_she;
	Elf_Scn		*s_scn;
	SDIBOX		*Sdihead;
	SDIBOX		*Sditail;
	CODEBOX		*Codehead;
	CODEBOX		*Codetail;
	
};

extern struct scninfo *sectab;


#define GET_SECTION(sect_hdr, name, type, flags, entsize) \
	sect_hdr->sh_name = addstr(name);\
	sect_hdr->sh_type = type;\
	sect_hdr->sh_flags = flags;\
	sect_hdr->sh_addr = NULL;\
	sect_hdr->sh_entsize = entsize;
	
	
