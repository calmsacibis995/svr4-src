/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dump:common/fcns.c	1.12"

#include <stdio.h>
#include "libelf.h"
#include "dumpmap.h"
#include "dump.h"

#include <sys/elf_M32.h>
#include <sys/elf_386.h>

/*
 * Print the symbols in the archive symbol table.
 * The function requires a file descriptor returned by
 * a call to open(), a pointer to an archive file opened with elf_begin(),
 * a pointer to the archive header of the associated archive symbol table,
 * and the name of the archive file.
 * Seek to the start of the symbol table and read it into memory.
 * Assume that the number of entries recorded in the beginning
 * of the archive symbol table is correct but check for truncated
 * symbol table.
 */

int
ar_sym_read(fd, elf_file, ar_p, filename )
int       fd;
Elf	  *elf_file;
Elf_Arhdr *ar_p;
char	  *filename;

{
	extern	long	sgetl( );
	extern int p_flag;
	extern  char *  prog_name;

	long    here;
	long	symsize;
	long	num_syms;
	char    *offsets;
	long    n;
	char	num_buf[sizeof(long)];
	Elf_Arhdr    arbuf;
	typedef unsigned char    word[4];
	word *ar_sym;

	(void)printf("%s:\n", filename);

	if(!p_flag)
	{
		(void)printf("     **** ARCHIVE SYMBOL TABLE ****\n");
		(void)printf("%-8s%s\n\n", "Offset", "Name" );
	}

	if ((symsize = ar_p->ar_size) == 0)
	{
		(void)fprintf( stderr, "%s: %s: cannot read symbol table header\n",
			prog_name, filename);
		return( FAILURE );
	}
	if ((ar_sym = (word *) malloc( symsize * sizeof(char) )) == NULL)
	{
		(void)fprintf(stderr, "%s: %s: could not malloc space\n", prog_name, filename);
		return;
	}

	here = elf_getbase(elf_file);
	if( (lseek(fd, here, 0) ) != here)
	{
		(void)fprintf(stderr, "%s: %s: could not lseek\n", prog_name, filename);
		return;
	}

	if( (read(fd, ar_sym, symsize * sizeof(char) )) == -1)
	{
		(void)fprintf(stderr, "%s: %s: could not read\n", prog_name, filename);
		return;
	}

	num_syms = sgetl( ar_sym );
	ar_sym++; 
	offsets = (char *)ar_sym;
	offsets += (num_syms)*sizeof(long);

	 for( ; num_syms; num_syms--, ar_sym++)
	{
		(void)printf( "%-8ld", sgetl( ar_sym ) );
		if( (n = strlen(offsets) ) == NULL)
		{
				(void)fprintf( stderr, "%s: %s: premature EOF\n",
					prog_name, filename);
				return( FAILURE );

		}
		(void)printf("%s\n",offsets);
		offsets += n + 1;
	}
	return( SUCCESS );

}

/*
 * Symbols in the archive symbol table are in machine independent
 * representation.  This function translates each symbol.
 */

#include <values.h>

long
sgetl(buffer)
register char *buffer;
{
	register long w = 0;
	register int i = BITSPERBYTE * sizeof(long);

	while ((i -= BITSPERBYTE) >= 0)
		w |= (long) ((unsigned char) *buffer++) << i;
	return (w);
}


/*
 * Print the program execution header.  Input is an
 * opened ELF or COFF object file, the number of structure
 * instances in the header as recorded in the ELF header, and the filename.
 */

void
dump_exec_header(elf_file, nseg, filename)
Elf      *elf_file;
unsigned nseg;
char     *filename;
{
	Elf32_Phdr  	*p_phdr;
	int		counter;
	extern int      v_flag, p_flag;
	extern char     *prog_name;

	if(!p_flag)
	{
		(void)printf(" ***** PROGRAM EXECUTION HEADER *****\n");
		(void)printf("Type        Offset      Vaddr       Paddr\n");
		(void)printf("Filesz      Memsz       Flags       Align\n\n");
	}

	if ( (p_phdr = elf32_getphdr(elf_file)) == 0)
	{
		/*(void)fprintf(stderr,"%s: %s: problem reading program exec header\n",
			prog_name, filename);*/
		return;
	}

	for (counter=0; counter<nseg; counter++)
	{
		if(p_phdr == 0)
		{
			(void)fprintf(stderr, "%s: %s: premature EOF on program exec header\n",
				prog_name, filename);
			return;
		}

		if(!v_flag)
		{
			(void)printf("%-12d%-12#x%-12#x%-12#x\n%-12#x%-12#x%-12lu%-12#x\n\n",
				(int)p_phdr->p_type,
				p_phdr->p_offset,
				p_phdr->p_vaddr,
				p_phdr->p_paddr,
				(unsigned long)p_phdr->p_filesz,
				(unsigned long)p_phdr->p_memsz,
				(unsigned long)p_phdr->p_flags,
				(unsigned long)p_phdr->p_align);
		}
		else
		{
			switch (p_phdr->p_type)
			{
				case PT_NULL: (void)printf("%-12s", "NULL"); break;
				case PT_LOAD: (void)printf("%-12s", "LOAD"); break;
				case PT_DYNAMIC: (void)printf("%-12s", "DYN"); break;
				case PT_INTERP: (void)printf("%-12s", "INTERP"); break;
				case PT_NOTE: (void)printf("%-12s", "NOTE"); break;
				case PT_PHDR: (void)printf("%-12s", "PHDR"); break;
				case PT_SHLIB: (void)printf("%-12s", "SHLIB"); break;
				default:
					(void)printf("%-12d", (int)p_phdr->p_type);break;
			}
			(void)printf("%-12#x%-12#x%-12#x\n%-12#x%-12#x",
				p_phdr->p_offset,
				p_phdr->p_vaddr,
				p_phdr->p_paddr,
				(unsigned long)p_phdr->p_filesz,
				(unsigned long)p_phdr->p_memsz);

			switch (p_phdr->p_flags)
			{
				case 0: printf("%-12s", "---"); break;
				case PF_X: (void)printf("%-12s", "--x"); break;
				case PF_W: (void)printf("%-12s", "-w-"); break;
				case PF_W+PF_X: (void)printf("%-12s", "-wx"); break;
				case PF_R: (void)printf("%-12s", "r--"); break;
				case PF_R+PF_X: (void)printf("%-12s", "r-x"); break;
				case PF_R+PF_W: (void)printf("%-12s", "rw-"); break;
				case PF_R+PF_W+PF_X: (void)printf("%-12s", "rwx"); break;
				default:
					(void)printf("%-12d", p_phdr->p_flags);break;
			}
			(void)printf("%-12#x\n\n", (unsigned long)p_phdr->p_align);
		}
		p_phdr++;
	}
}

