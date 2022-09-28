/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ldd:ldd.c	1.2"

/* List Dynamic Dependencies command 
 * 
 * Print list of shared objects loaded by this process.
 *
 * usage is: ldd [-d|-r] file
 *
 * ldd opens file and verifies the information in the elf header.
 * If file is not a dynamioc a.out, we simply exit.
 * If it is a dynamic object, we set up some environment variables
 * and exec the file.  The dynamic linker will print out some
 * diagnostic information, according to the environment variables set.
 *
 * If neither -d nor -r is specified, we set only LD_TRACE_LOADED_OBJECTS.
 * The dynamic linker will print the pathnames of all dynamic objects it
 * loads, and then exit.
 *
 * If -d or -r is specified, we also set LD_WARN; the dynamic linker will
 * perform its normal relocations and issue warning messages for unresolved
 * references. It will then exit.
 *
 * If -d is specified, we set LD_BIND_NOW=0, so that the dynamic linker
 * will not perform PLT type relocations.
 * If -r is specified, we set LD_BIND_NOW=1, so that the dynamic linker
 * will perform all relocations.
 */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>
#include <libelf.h>
#include "machdep.h"


main(argc, argv)
int argc;
char *argv[];
{
	static char *usage = "[-r|-d] filename";
	static char *trace = "LD_TRACE_LOADED_OBJECTS=1";
	static char *dwarn = "LD_WARN=1";
	static char *bnow = "LD_BIND_NOW=1";
	static char *blazy = "LD_BIND_NOW=";
	Elf *elf;
	ELF_EHDR *ehdr;
	ELF_PHDR *phdr;
	char *cname = argv[0];
	char *fname ;
	int fd, c, i, filecount = 0 ;
	int dflag = 0, rflag = 0, errflag = 0, dynamic = 0, status = 0;
	unsigned int idcount;
	extern int opterr, optind;
	extern char *optarg;

	/* verify command line syntax and process arguments */
	opterr = 0; /* disable getopt error message */
	while (optind < argc) {
		c = getopt(argc, argv, "dr");
		switch(c) {
		case 'd' : 
			dflag = 1;
			if (rflag || filecount) 
				errflag++;
			break;
		case 'r' :
			rflag = 1;
			if (dflag || filecount)
				errflag++;
			break;
		case '?' :
			errflag++;
			break;
		default :
			if (filecount) 
				errflag++;
			else {
				fname = argv[optind++];
				filecount++;
			}
			break;
		}
		if (errflag)
			break;
	}
	if (errflag || !filecount) {
		fprintf(stderr,"usage: %s %s\n", cname, usage);
		exit(1);
	}

	/* check that the file is executable */

	if ((access(fname, 1)) != 0) {
		fprintf(stderr,"%s: can't execute: %s\n", cname, fname);
		exit(1);
	}

	/* coordinate libelf's version information */
	if (elf_version(EV_CURRENT) == EV_NONE) {
		fprintf(stderr,"%s: internal object file version error %s\n", cname, fname);
		exit(1);
	}

	/* open file and get elf header */
	if ((fd = open(fname, O_RDONLY)) == -1) {
		fprintf(stderr,"%s: can't open: %s\n", cname, fname);
		exit(1);
	}

	if ((elf = elf_begin(fd, ELF_C_READ,(Elf *)0)) == (Elf *)0) {
		fprintf(stderr,"%s: bad file type: %s\n", cname, fname);
		exit(1);
	}

	if (elf_kind(elf) != ELF_K_ELF) {
		fprintf(stderr,"%s: %s is not an ELF file\n", cname, fname);
		exit(1);
	}


	/* verify information in file header */

	if ((ehdr = elf_getehdr(elf)) == (ELF_EHDR *)0) {
		fprintf(stderr,"%s: can't read ELF header in: %s\n", cname, fname);
		exit(1);
	}

	/* check class and encoding */
	if (ehdr->e_ident[EI_CLASS] != M_CLASS ||
		ehdr->e_ident[EI_DATA] != M_DATA) {
		fprintf(stderr,"%s: %s has wrong class or data encoding",cname,fname);
		return 0;
	}
	/* check type */
	if (ehdr->e_type != ET_EXEC) {
		fprintf(stderr,"%s: bad magic number in: %s\n", cname, fname);
		exit(1);
	}
	if (ehdr->e_machine != M_TYPE) {
		fprintf(stderr,"%s: wrong machine type in: %s\n", cname, fname);
		exit(1);
	}

	/* read program header and check for dynamic section - if missing,
	 * just exit.
	 */
	if ((phdr = elf_getphdr(elf)) == (ELF_PHDR *)0) {
		fprintf(stderr,"%s: can't read program header in: %s\n", cname, fname);
		exit(1);
	}

	for (i = 0; i < (int)ehdr->e_phnum; i++) {
		if (phdr->p_type == PT_DYNAMIC) {
			dynamic = 1;
			break;
		}
		phdr = (ELF_PHDR *)((unsigned long)phdr + ehdr->e_phentsize);
	}
	if (!dynamic)
		exit(0);

	/* close file and free library data */
	elf_end(elf);

	if (close(fd) == -1) {
		fprintf(stderr,"%s: can't close %s\n", cname, fname);
		exit(1);
	}

	/* set environment and exec file */
	if (putenv(trace) != 0) { /* trace loaded objects */
		fprintf(stderr,"%s: can't add to environment\n", cname);
		exit(1);
	}

	if (dflag || rflag) { /* warn if relocation errors */
		if (putenv(dwarn) != 0) {
			fprintf(stderr,"%s: can't add to environment\n", cname);
			exit(1);
		}
		if (rflag) { /* perform all relocations */
			if (putenv(bnow) != 0) {
				fprintf(stderr,"%s: can't add to environment\n", cname);
				exit(1);
			}

		}
		else { /* perform only data relocations */
			if (putenv(blazy) != 0) {
				fprintf(stderr,"%s: can't add to environment\n", cname);
				exit(1);
			}
		}
	}

	if ((i = fork()) == -1) {
			fprintf(stderr,"%s: can't fork\n",cname);
			exit(1);
	}

	if (i) {  /* parent */
		(void)wait(&status);
		exit(status);
	}
	else {  /* child */
		if ((execl(fname, fname, (char *)0)) == -1) {
			fprintf(stderr,"%s: exec of: %s failed\n",cname,fname);
			exit(1);
		}
	}
}
