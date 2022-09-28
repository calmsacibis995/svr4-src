/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/globals.c	1.16"
/*
 * Global variables
 */

/****************************************
** Imports
****************************************/

#include	<stdio.h>
#include	"paths.h"
#include	"sgs.h"
#include	"globals.h"

/****************************************
** Variables
****************************************/

/*
 * Below are the global variables used by the linker.  Their meanings are
 * defined in globals.h
 */

#ifdef	DEBUG
Boolean debug_bits[DBG_MAX]; 		/* Debugging flag bits array */
#endif	/* DEBUG */

Boolean		aflag, bflag, dmode=TRUE, mflag, rflag, sflag, tflag, zdflag,
		znflag, ztflag, Gflag, Bflag_symbolic, Bflag_dynamic;
Setstate	Qflag = NOT_SET;
Word		bss_align;
Word		copyrels;
Word		countGOT = GOT_XNumber;
Word		countPLT = PLT_XNumber;
Word		count_dynglobs;
Word		count_dynstrsize;
Word		count_namelen;
Word		count_osect;
Word		count_outglobs;
Word		count_outlocs;
Word		count_rela;
Word		count_strsize;
Ehdr		*cur_file_ehdr;
int		cur_file_fd;
char		*cur_file_name = "command line";
Elf		*cur_file_ptr;
Infile		*cur_infile_ptr;
Word		dynbkts;
char		*dynoutfile_name;
Word		ehdr_flags;
char		*entry_point;
Addr		firstexec_seg;
Addr		firstseg_origin = FIRSTSEG_ORIGIN;
Word		grels;
List		infile_list;
char		*interp_path;
char		*ld_run_path;
char		*libdir = NULL;
Word		libver;
char 		*llibdir = NULL;
char 		*libpath = LIBPATH;
Word		orels;
Ehdr		*outfile_ehdr;
Elf		*outfile_elf;
int		outfile_fd;
char		*outfile_name = A_OUT;
Phdr		*outfile_phdr;
Word		prels;
List		seg_list;
Word		sizePHDR;
List		soneeded_list = {NULL, NULL};
List		symbucket[NBKTS];
Elf_Data	*symname_bits;
Boolean		textrel;
