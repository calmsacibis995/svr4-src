/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/main.c	1.31"
/*
 * ld -- link/editor main program
 */

/****************************************
** Imports
****************************************/

#include	"globals.h"
#include	"macros.h"

/****************************************
** Main Function Declaration
****************************************/

/*
 * The main program
 */
main(argc, argv)
	int argc;
	char** argv;
{
	init_signals();
	libver = EV_CURRENT;
	if (elf_version(EV_CURRENT) == EV_NONE)
		lderror(MSG_ELF,"libelf is out of date");
	 
	process_flags(argc, argv);
	check_flags(argc);
	lib_setup();
	process_files(argc, argv);

#ifdef	DEBUG
	if (debug_bits[DBG_MAIN])
		mapprint("");
#endif	/* DEBUG */
	if (dmode || aflag ) {
		build_specsym(ETEXT_SYM,ETEXT_USYM);
		build_specsym(EDATA_SYM,EDATA_USYM);
		build_specsym(END_SYM,END_USYM);
		build_specsym(DYN_SYM,DYN_USYM);
	}
	if ( outfile_name != NULL)
		if ( find_infile(outfile_name) != NULL)
			lderror(MSG_FATAL, "-o would overwrite %s\n", outfile_name);

       	if (!rflag && sym_find(GOT_USYM, NOHASH) != 0)
		build_specsym(GOT_SYM,GOT_USYM);

	if (Qflag == SET_TRUE)
		make_comment();
	if( ((!dmode && aflag) && (interp_path != NULL)) || (dmode && !Gflag))
		make_interp();
	count_relentries();
	if( dmode )
		make_dyn();
	if(dmode && !Gflag){
		if (znflag) {
			add_undefs_to_dynsymtab();
		}
		make_dynstrtab();
		make_dynsymtab();
	}
	if( Gflag || rflag || !sflag){
		make_strtab();
		make_symtab();
	}
	if (dmode)
	    	make_hash();
/* always after all the make_*'s so that all section names are accounted for */
	make_shstrtab();
	open_out();
	set_off_addr();
	if (mflag)
		ldmap_out();
	update_syms();
	relocate();
	finish_out();
	exit(EXIT_SUCCESS);
	/* NOTREACHED */
}
