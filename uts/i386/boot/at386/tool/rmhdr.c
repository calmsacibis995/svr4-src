/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)boot:boot/at386/tool/rmhdr.c	1.1.2.1"

/* fix up the boot code so it's absolute binary for loading from disk */
/* magic number is checked for ELF or COFF boot code */

#include <a.out.h>
#include <fcntl.h>
#include "sys/elf.h"

#define ELFMAGIC 0x457f

char i[1024] ;
extern int errno ;

main (argc, argv)
int argc ;
char **argv ;
{
	struct filehdr test ;
	Elf32_Ehdr	elfhdr;
	Elf32_Phdr	elfphdr;
	int file, ofile, numchars ;
	long x ;

	if ( (file = open (argv[1], O_RDONLY, 0)) < 0 )  {
		printf ("Open of %s failed, returned %d\n", argv[1], errno) ;
		exit (0) ;
	}
	if ( (ofile = open (argv[2], O_CREAT | O_RDWR, 0777)) < 0 ) {
		printf ("Open of %s failed, returned %d\n", argv[2], errno) ;
		exit (0) ;
	}

	read (file, &test, sizeof(struct filehdr));

	switch (test.f_magic) {
	case I386MAGIC:
		x = (test.f_nscns * sizeof(struct scnhdr)) + 
		     sizeof(struct aouthdr);
		break;

	case ELFMAGIC:
		lseek (file, 0, 0); 
		read (file, &elfhdr, sizeof(elfhdr));
		read (file, &elfphdr, sizeof(elfphdr));
		x = elfphdr.p_offset - sizeof(elfhdr) - sizeof(elfphdr);
		break;

	default:
		printf ("Neither COFF or ELF file supplied\n");
		break;
	}

	lseek (file, x, 1);
	while ((numchars = read (file, i, 1024)) != 0)	
	{
		write (ofile, i, numchars) ;
	}
}
