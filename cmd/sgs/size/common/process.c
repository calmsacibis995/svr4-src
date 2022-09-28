/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)size:size/common/process.c	1.15"
/* UNIX HEADER */
#include	<stdio.h>

/* SGS HEADERS */
#include	"sgs.h"

/* ELF HEADERS */
#include	"libelf.h"
#include	"ccstypes.h"

/* SIZE HEADERS */
#include	"defs.h"
#include	"process.h"

#ifdef M32
#include <sys/elf_M32.h>
#endif

#ifdef i386
#include <sys/elf_386.h>
#endif

void
process( elf )

Elf * elf;

{

	/* SIZE FUNCTIONS CALLED */
	extern void	error();

	/* EXTERNAL VARIABLES USED */
	extern int	fflag, /* full format for sections */
			Fflag, /* full format for segments */
			nflag; /* include non-loadable segments or sections */
	extern int	numbase; /* hex, octal, or decimal */
	extern char	*fname;
	extern char	*archive;
	extern int	is_archive;
	extern int	oneflag;

	/* LOCAL VARIABLES */
	long	size, /* total size in non-default case for sections */
		first, /* size of first number in default case for sections */
		second, /* size of second number in default case for sections */
		third, /* size of third number in default case for sections */
		totsize; /* total size in default case for sections */
	Elf32_Ehdr	*ehdr;
	Elf32_Phdr	*phdr;
	Elf32_Shdr	*shdr;
	Elf_Scn		*scn;
	unsigned	ndx=0;
	int		numsect=0;
	int		notfirst=0;
	int	i;
	char *name = 0;
	static void	process_phdr();

/***************************************************************************/
/* If there is a program header and the -f flag requesting section infor-  */
/* mation is not set, then process segments with the process_phdr function.*/
/* Otherwise, process sections.  For the default case, the first number    */
/* shall be the size of all sections that are allocatable, nonwritable and */
/* not of type NOBITS; the second number shall be the size of all sections */
/* that are allocatable, writable, and not of type NOBITS; the third number*/
/* is the size of all sections that are writable and not of type NOBITS.   */
/* If -f is set, print the size of each allocatable section, followed by   */
/* the section name in parentheses.                                        */
/* If -n is set, print the size of all sections, followed by the section   */
/* name in parentheses.                                                    */
/***************************************************************************/ 

        if ((ehdr = elf32_getehdr(elf)) == 0)
        {
                error(fname, "invalid file type");
                return;
        }
	if ((phdr = elf32_getphdr(elf)) != 0 && !(fflag)) 
	{
		process_phdr(phdr, ehdr->e_phnum);
		return;
	}

        if (is_archive)
        {
                (void)printf("%s[%s]: ", archive, fname);
        }
        else if (!oneflag && !is_archive)
        {
                (void)printf("%s: ", fname);
        }
	ndx = ehdr->e_shstrndx;
	scn = 0;
	size = 0;
	first = second = third = totsize = 0;
        if (ehdr->e_shnum == 0)
        {
                error(fname, "no section data");
        }
	numsect = ehdr->e_shnum;
	for (i = 0; i < numsect; i++)
	{
		if ((scn = elf_nextscn(elf, scn)) == 0)
		{
			break;
		}
		if ((shdr = elf32_getshdr(scn)) == 0)
		{
			error(fname,"could not get section header");
			break;
		}
		if ((Fflag) && !(fflag))
		{
			error(fname, "no segment data");
			return;
		}
		else if ( (!(shdr->sh_flags &  SHF_ALLOC)) && fflag && !(nflag))
		{
			continue;
		}
		else if ( (!(shdr->sh_flags & SHF_ALLOC)) && !(nflag) )
		{
			continue;
		}
		else if (( shdr->sh_flags & SHF_ALLOC ) && (!( shdr->sh_flags & SHF_WRITE )) && (!( shdr->sh_type == SHT_NOBITS )) && !(fflag) && !(nflag) )
		{
			first += shdr->sh_size;
		}
		else if (( shdr->sh_flags & SHF_ALLOC ) && ( shdr->sh_flags & SHF_WRITE ) && (!( shdr->sh_type == SHT_NOBITS)) && !(fflag) && !(nflag) )
		{
			second += shdr->sh_size;
		}
		else if (( shdr->sh_flags & SHF_WRITE ) && ( shdr->sh_type == SHT_NOBITS ) && !(fflag) && !(nflag) )
		{
			third += shdr->sh_size;
		}
		(name = elf_strptr(elf, ndx, (size_t)shdr->sh_name));
	
		if (fflag || nflag)
		{
			size += shdr->sh_size;
			if (notfirst)
				{
				(void)printf(" + ");
				}
			(void)printf(prusect[numbase], shdr->sh_size);
			(void)printf("(%s)", name);

		}
		notfirst++;

	}
	if ( (fflag || nflag) && (numsect > 0))
	{
		(void)printf(prusum[numbase], size);
	}

	if (!fflag && !nflag)
	{
		totsize = first + second + third;
		if (numbase == DECIMAL)
		{
		     (void)printf("%ld + %ld + %ld = %ld\n", first, second, third, totsize);
		}
		else if (numbase == OCTAL)
		{
		     (void)printf("%lo + %lo + %lo = 0%lo\n", first, second, third, totsize);
		}
		else if (numbase == HEX)
		{
		     (void)printf("%lx + %lx + %lx = 0x%lx\n", first, second, third, totsize);
		}


	}

	if (Fflag)
	{
		if ((phdr = elf32_getphdr(elf)) != 0)
		{
			process_phdr(phdr, ehdr->e_phnum);
			return;
		}
		else
		{
			error(fname, "no segment data");
			return;
		}
	}

}



/***************************************************************************/
/* If there is a program exection header, process segments. In the default */
/* case, the first number is the file size of all nonwritable segments     */
/* of type PT_LOAD; the second number is the file size of all writable     */
/* segments whose type is PT_LOAD; the third number is the memory size     */
/* minus the file size of all writable segments of type PT_LOAD.           */
/* If the -F flag is set, size will print the memory size of each loadable */
/* segment, followed by its permission flags.                              */
/* If -n is set, size will print the memory size of all loadable segments  */
/* and the file size of all non-loadable segments, followed by their       */
/* permission flags.                                                       */
/***************************************************************************/

static void
process_phdr(phdr,num)
Elf32_Phdr	*phdr;
Elf32_Half	num;

{
	int	i;
	int 	notfirst=0;
	Elf32_Phdr	*p;
	long	memsize,
		total,
		First,
		Second,
		Third,
		Totsize;
		extern int	Fflag,
			   	nflag;
		extern int 	numbase; 
		extern char	*fname;
		extern char	*archive;
		extern int	is_archive;
		extern int	oneflag;

	p=phdr;
	memsize=total=0;
	First = Second = Third = Totsize = 0;

        if (is_archive)
        {
                (void)printf("%s[%s]: ", archive, fname);
        }
        else if (!oneflag && !is_archive)
        {
                (void)printf("%s: ", fname);
        }

	for (i = 0; i < (int)num; i++, ++p)
	{
		if ( (!(p->p_flags & PF_W)) && (p->p_type == PT_LOAD) && !(Fflag))
		{
			First += p->p_filesz;
		}
		else if ((p->p_flags & PF_W) && (p->p_type == PT_LOAD) && !(Fflag))
		{
			Second += p->p_filesz;
			Third += p->p_memsz;
		}
		memsize += p->p_memsz;
		if ((p->p_type == PT_LOAD) && nflag)
		{
			if (notfirst)
			{
				(void)printf(" + ");
			}
			(void)printf(prusect[numbase], p->p_memsz);
			total += p->p_memsz;
			notfirst++;
		}
		if ( !(p->p_type == PT_LOAD) && nflag)
		{
			if (notfirst)
			{
				(void)printf(" + ");
			}
			(void)printf(prusect[numbase], p->p_filesz);
			total += p->p_filesz;
			notfirst++;
		}
		if ((p->p_type == PT_LOAD) && Fflag && !nflag)
		{
			if (notfirst)
			{
				(void)printf(" + ");
			}
			(void)printf(prusect[numbase], p->p_memsz);
			notfirst++;
		}
		if ( (Fflag) && !(nflag) && (!(p->p_type == PT_LOAD)) )
		{
			continue;
		}
		if ( Fflag || nflag)
		{
		switch ( p->p_flags )
		{
			case 0: printf("(---)"); break;
			case PF_X: printf("(--x)"); break;
			case PF_W: printf("(-w-)"); break;
			case PF_W+PF_X: printf("(-wx)"); break;
			case PF_R: printf("(r--)"); break;
			case PF_R+PF_X: printf("(r-x)"); break;
			case PF_R+PF_W: printf("(rw-)"); break;
			case PF_R+PF_W+PF_X: printf("(rwx)"); break;
			default:	printf("flags(%#lx)",p->p_flags);
		}
		}
	}
	if (nflag)
	{
		(void)printf(prusum[numbase], total);
	}
	if (Fflag && !nflag)
	{
		(void)printf(prusum[numbase], memsize);
	}
	if (!Fflag && !nflag)
	{
		Totsize = First + Second + (Third - Second);
		if (numbase == DECIMAL)
		{
		     (void)printf("%ld + %ld + %ld = %ld\n", First, Second, Third - Second, Totsize);
		}
		else if (numbase == OCTAL)
		{
		     (void)printf("%lo + %lo + %lo = 0%lo\n", First, Second, Third - Second, Totsize);
		}
		else if (numbase == HEX)
		{
		     (void)printf("%lx + %lx + %lx = 0x%lx\n", First, Second, Third - Second, Totsize);
		}
	}
}
