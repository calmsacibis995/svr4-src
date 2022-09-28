/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)strings:strings.c	1.3.1.1"
/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft 	*/
/*	Corporation and should be treated as Confidential.	*/


/*
 *	@(#) strings.c 1.3 88/05/09 strings:strings.c
 */
/* Copyright (c) 1979 Regents of the University of California */
#include <stdio.h>
#include "x.out.h"
#include <ctype.h>
#include <libelf.h>

#ifdef i386
#include <sys/elf_386.h>
#else
#include <sys/elf_M32.h>
#endif

# define NOTOUT		0
# define AOUT		1
# define BOUT		2
# define XOUT		3
# define ELF		4

long	ftell();
#define dirt(c) (!isascii(c) || !isprint(c))
/*
 * Strings - extract strings from an object file for whatever
 *
 * Bill Joy UCB 
 * April 22, 1978
 *
 * The algorithm is to look for sequences of "non-junk" characters
 * The variable "minlen" is the minimum length string printed.
 * This helps get rid of garbage.
 * Default minimum string length is 4 characters.
 *
 *	MODIFICATION HISTORY:
 *	M000	05 Dec 82	andyp
 *	- Made handle x.out, b.out formats; previously only handled a.out.
 *	  Moos are only approximate, there was a fair amount of reorganizing.
 */

/*struct	exec header;*/						/*M000*/
union uexec {								/*M000*/
	struct xexec	u_xhdr;	/* x.out */				/*M000*/
	struct aexec	u_ahdr;	/* a.out */				/*M000*/
	struct bexec	u_bhdr;	/* b.out */				/*M000*/
} header;								/*M000*/
struct xexec	*xhdrp	= &(header.u_xhdr);				/*M000*/
struct aexec	*ahdrp	= &(header.u_ahdr);				/*M000*/
struct bexec	*bhdrp	= &(header.u_bhdr);				/*M000*/

char	*infile = "Standard input";
int	oflg;
int	asdata;
long	offset;
int	minlength = 4;

main(argc, argv)
	int argc;
	char *argv[];
{
	int  hsize, htype;						/*M000*/
	int fd;
	Elf *elf;
	Elf32_Ehdr *ehdr;
	Elf_Scn *scn;
	Elf32_Shdr *shdr;
	char *scn_name;

	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		register int i;
		if (argv[0][1] == 0)
			asdata++;
		else
			for (i = 1; argv[0][i] != 0; i++)
				switch (argv[0][i]) {

				case 'o':
					oflg++;
					break;

				case 'a':
					asdata++;
					break;

				default:
					if (!isdigit(argv[0][i])) {
						fprintf(stderr, "Usage: strings [ - | -a ] [ -o ] [ -# ] [ file ... ]\n");
						exit(1);
					}
					minlength = argv[0][i] - '0';
					for (i++; isdigit(argv[0][i]); i++)
						minlength = minlength * 10 + argv[0][i] - '0';
					i--;
					break;
				}
		argc--, argv++;
	}
	do {
		if (argc > 0) {
			if (freopen(argv[0], "r", stdin) == NULL) {
				perror(argv[0]);
				exit(1);
			}
			infile = argv[0];
			argc--, argv++;
		}
		/* M000 begin */
		if (asdata) 
			htype =  NOTOUT;
		else {
			hsize = fread ((char *) &header, sizeof (char),
					sizeof (header), stdin);
			htype = ismagic (hsize, &header, stdin);
		}
		switch (htype) {
			case AOUT:
				fseek (stdin, (long) ADATAPOS (ahdrp), 0);
				find ((long) ahdrp->xa_data);
				continue;
			case BOUT:
				fseek (stdin, (long) BDATAPOS (bhdrp), 0);
				find ((long) bhdrp->xb_data);
				continue;
			case XOUT:
				fseek (stdin, (long) XDATAPOS (xhdrp), 0);
				find ((long) xhdrp->x_data);
				continue;
			case ELF:
		/* Will take care of COFF M32 and i386 also */
		/* As well as ELF M32, i386 and Sparc */

				fd = fileno(stdin);
				lseek(fd, 0L, 0);
				elf = elf_begin(fd, ELF_C_READ, NULL);
				ehdr = elf32_getehdr(elf);
				scn = 0;
				while ((scn = elf_nextscn(elf, scn)) != 0)
				{
			 		if ((shdr = elf32_getshdr(scn)) != 0)
			       			scn_name = elf_strptr(elf, ehdr->e_shstrndx, (size_t)shdr->sh_name);
					/* There is more than one */
					/* .data section */

					if ((strcmp(scn_name, ".rodata") == 0) ||
						(strcmp(scn_name, ".rodata1") == 0) ||
						(strcmp(scn_name, ".data") == 0) ||
						(strcmp(scn_name, ".data1") == 0))
					{
						fseek(stdin, (long) shdr->sh_offset, 0);
						find((long) shdr->sh_size);
					}
		 		}
				continue;
			case NOTOUT:
			default:
				fseek(stdin, (long) 0, 0);
				find((long) 100000000L);
				continue;
		}
		/* M000 end */
	} while (argc > 0);
}

find(cnt)
	long cnt;
{
	static char buf[BUFSIZ];
	register int c, cc;

	cc = 0;
	for (c = !EOF; (cnt > 0) && (c != EOF); cnt--) {
		c = getc(stdin);
		if (dirt(c)) {
			if (cc >= minlength) {
				if (oflg)
					printf("%7ld ", ftell(stdin) - cc - 1);
				buf[cc] = '\0';
				puts(buf);
			}
			cc = 0;
		} else {
			if (cc < (BUFSIZ - 2))
				buf[cc] = c;
			++cc;
		}
	}
}

/* M000 begin */
ismagic(hsize, hdr, fp)
	int hsize;
	union uexec *hdr;
	FILE *fp;
{
	switch ((int) (hdr->u_bhdr.xb_magic)) {
		case A_MAGIC1:
		case A_MAGIC2:
		case A_MAGIC3:
		case A_MAGIC4:
			if (hsize < sizeof (struct bexec))
				return (NOTOUT);
			else
				return (BOUT);
		default:
			break;
	}
	switch (hdr->u_xhdr.x_magic) {
		case X_MAGIC:
			if (hsize < sizeof (struct xexec))
				return (NOTOUT);
			else
				return (XOUT);
		default:
			break;
	}
	switch (hdr->u_ahdr.xa_magic) {
		case A_MAGIC1:
		case A_MAGIC2:
		case A_MAGIC3:
		case A_MAGIC4:
			if (hsize < sizeof (struct aexec))
				return (NOTOUT);
			else
				return (AOUT);
		default:
			break;
	}
	return (tryelf(fp));
}
/* M000 end */


tryelf(fp)
FILE *fp;
{
	int fd;
	Elf *elf;
	Elf32_Ehdr *ehdr;

	fd = fileno(fp);

	if ((elf_version(EV_CURRENT)) == EV_NONE) {
		fprintf(stderr, "%s\n", elf_errmsg(-1));
		return(NOTOUT);
	}

	lseek(fd, 0L, 0);

	if ((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL) {
		fprintf(stderr, "%s\n", elf_errmsg(-1));
		return(NOTOUT);
	}

	if ((elf_kind(elf)) == ELF_K_NONE) {
		elf_end(elf);
		return(NOTOUT);
	}

	if ((ehdr = elf32_getehdr(elf)) == NULL) {
		fprintf(stderr, "%s\n", elf_errmsg(-1));
		elf_end(elf);
		return(NOTOUT);
	}

	if ((ehdr->e_type == ET_CORE) || (ehdr->e_type == ET_NONE)) {
		elf_end(elf);
		return(NOTOUT);
	}

	elf_end(elf);

	return(ELF);

}
