/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/pass2.c	1.18"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <libelf.h>
#include <fcntl.h>
#include "gendefs.h"
#include "symbols.h"
#include "section.h"
#include <ccstypes.h>

/*
 *
 *	"pass2.c" is a file that contains the main routine for the
 *	final pass of the assembler.
 *
 *	name	internal	mode	use
 *
 *	ifile	filenames[0]		name used in error messages
 *	ofile	filenames[1]	write	assembly output
 *	t2	filenames[2]	read*	relocation entries
 *	
 *

/*
 *
 *	The following are declarations for the major global variables
 *	used in the second pass of the assembler.  The variables and
 *	their meainings are as follows:
 *
 *
 *
 *	fdrel	This is the file descriptor where preliminary relocation
 *		entries are to be written.  As it happens, this will
 *		always refer to the temporary file "t2" during execution
 *		of this program.
 *
 *
 *
 *	filenames[0]
 *		This is a pointer to the string which is the input file
 *		name ("ifile", as described at the beginning of this
 *		file).  This will be used if necessary to report the
 *		file name in an error message (see "errors.c").
 *
 *
 *
 *
 *	seccnt	A count of the number of sections. The first
 *		section is numbered 1.
 *
 *	sectab	The array of section headers that will be written 
 *		after the file header in the output file.  Indexed
 *		from 2 through seccnt (section 1 is reserved for the
 *		string table).
 *
 */

extern short passnbr;

extern int seccnt;

extern char *filenames[];

extern short anyerrs;


extern void	onintr(),
		flags(),
		delexit(),
		deltemps(),
		traverse(),
		errmsg(),
		reloc(),
		write_elf_header(),
		write_symbol_table(),
		codgen(),
		alloc_lcomm(),
		codout();
extern unsigned long	
		addstr();
extern int	close();

extern symbol *lookup();


extern char *strtab;
extern long currindex;


unsigned int relent;

short uflag = NO;

#if M4ON
extern short rflag;
#endif


FILE 	*fdrel;
static int 	fdout;

Elf 	*elffile;
Elf_Scn *elf_string_section;


#if DEBUG
static short testas = TESTVAL;

extern FILE *perfile;	/* performance data file descriptor */
/*
 *	Performance data variables
 */
extern long	ttime;
extern struct	tbuffer {
	long	proc_user_time;
	long	proc_system_time;
	long	child_user_time;
	long	child_system_time;
} ptimes;
extern	long	times();
#endif

/*
 *
 *	"aspass2" is the main function for the second pass of the assembler.
 *	It performs the following sequence of steps:
 *
 *
 *	     1. The following file descriptors are opened:
 *
 *		fdrel	This is opened for writing on the temporary file
 *			t2.  The preliminary relocation information for
 *			the text section will be written to this file.
 *
 *		fdout	The final object file.
 *
 *	     2.	The following occurs for each section S in sectab[]:
 *
 *		A section descriptor is generated for each section
 *		which contains section header information.
 *
 *		The function "codout" (from codout.c) is called for
 *		each user defined section which will call the action
 *		routines to complete any pass1 processing and generate
 * 		the data for each setion.
 *
 *	     3. The file descriptor "fdout" is initialized and is
 *		set to point to the beginning of the object file to
 *		be created.  The function "write_elf_header" is called to
 *		create the object file header.
 *
 *	     4. The file descriptor "fdrel" is opened to read from t5,
 *		and "reloc" is called to process the text section
 *		and data section relocation entries and write them
 *		to the object file.
 *
 *	     5. If the assembler has not been called for testing
 *		the temporary file are removed by calling "deltemps".
 *
 */

void
aspass2()
{
	register int i;
	register struct scninfo *sect;
	register symbol *ptr;
	Elf32_Shdr *strtab_hdr;
	Elf_Data *data;
	extern void elferror();

	passnbr = 2;

#if DEBUG
	ttime = times(&ptimes);		/* Performance data collected here */
#endif


	/*
	 * Allocate any remaining .lcomm symbols in .bss
	 */
	traverse(alloc_lcomm);


	if ((fdout = open(filenames[1], O_RDWR | O_TRUNC | O_CREAT, (mode_t) 0644)) == -1){
		errmsg("", "Cannot Open Output File");
		deltemps();
		exit(127);
	}

	if (elf_version(EV_CURRENT) == EV_NONE)
		elferror("Libelf.a out of date");
	if ((elffile = elf_begin(fdout, ELF_C_WRITE, (Elf *) 0)) == NULL)
		elferror("libelf error: elf_begin:");

	write_elf_header(elffile);

	if ((elf_string_section = elf_newscn(elffile)) == NULL)
		elferror("libelf error: elf_newscn-string section:");
	if ((strtab_hdr = elf32_getshdr(elf_string_section)) == NULL)
		elferror("libelf error: elf_getshdr:");
	GET_SECTION(strtab_hdr, ".strtab", SHT_STRTAB, NULL, NULL);

	if((fdrel = fopen(filenames[2],"w"))==NULL)
		aerror("Cannot Open Temporary (rel) File");


	for (i= 2, sect = &sectab[2] ; i <= seccnt; i++,sect++) {
		ptr = lookup(sect->name, N_INSTALL);
		ptr->value = 0;  /* zero out section symbol flags */

		/* get new section descriptor and set section header info */
		if ((sect->s_scn = elf_newscn(elffile)) == NULL)
			elferror("libelf error: elf_newscn:");
		if ((sect->s_she = elf32_getshdr(sect->s_scn)) == NULL)
			elferror("libelf error: elf_getshdr:");
		GET_SECTION(sect->s_she, sect->name, sect->type, sect->flags,NULL);
		sect->s_she->sh_link = NULL;
		sect->s_she->sh_info = NULL;

		relent = 0;
		codout(i);
		

		if (sect->type == SHT_NOBITS) {
			if ((data = elf_newdata(sect->s_scn)) == NULL)
				elferror("libelf error: elf_newdata:");
			ENTER_DATA(NULL, ELF_T_BYTE,sect->size, sect->addralign);
		      }
		/* number of relocation entries for sect */
		sect->s_nreloc = relent;
	}
	/*
	 * raw code for all sections now complete
	 */

	(void) fflush(fdrel);
	if (ferror(fdrel))
		aerror("trouble writing; probably out of temp-file space");
	(void) fclose(fdrel);	/* flush the buffer */

	write_symbol_table();

	if ((fdrel = fopen(filenames[2],"r")) == NULL)
		aerror("Cannot Open Temporary (rel) File");
	reloc();
	(void) fclose(fdrel);	/* relocation info is complete and appended */

	if ((data = elf_newdata(elf_string_section)) == NULL)
		elferror("libelf error: elf_newdata-string section:");

	ENTER_DATA(strtab, ELF_T_BYTE, currindex, 1)

#if DEBUG
/*
 *	Performance data collected and written out here
 */

	ttime = times(&ptimes) - ttime;
	if ((perfile = fopen("as.info", "r")) != NULL ) {
		(void) fclose(perfile);
		if ((perfile = fopen("as.info", "a")) != NULL ) {
			(void) fprintf(perfile,
			   "as2\t%07ld\t%07ld\t%07ld\t%07ld\t%07ld\tpass 2\n",
			    ttime, ptimes);
			(void) fclose(perfile);
		}
	}
#endif

	if (!anyerrs) {

		if (elf_update(elffile, ELF_C_WRITE)< 0)
			elferror("libelf error: elf_update:");
		(void) elf_end(elffile);

		if (close(fdout) == -1)
			aerror("trouble closing elf file");

#if DEBUG
		if (testas != TESTVAL + 1)
#endif
			deltemps();
#if STATS
{
	extern char *firstbrk;
	extern char *sbrk();
	extern char *brk();
	extern unsigned long
		tablesize,
		numids,
		numins,
		numlnksins,
		numretr,
		numlnksretr,
		numreallocs,
		inputsz;
	extern symbol *cfile;
	long growth;
	FILE *fdstat;
	char *msgfile;

	growth = (long) (sbrk(0) - firstbrk);
	if (cfile)
		msgfile = filenames[0];
	else
		msgfile = cfile->name;
	if ((fdstat = fopen(STATPATH,"a")) == NULL)
		(void) fprintf(stderr,"Cannot open statistics file %s to append\n",
			STATPATH);
	else {
		(void) fprintf(fdstat,
		"%lu\t%lu\t%lu\t%ld\t%lu\t%lu\t%lu\t%lu\t%lu\t%s\n",
			inputsz,tablesize,numids,growth,numretr,numlnksretr,
			numins,numlnksins,numreallocs,msgfile);
		(void) fclose(fdstat);
	}
}
#endif
		return;
	}
	else
		delexit();
	/*NOTREACHED*/
}
