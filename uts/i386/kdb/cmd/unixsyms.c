/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:cmd/unixsyms.c	1.3.1.1"

/* In a cross-environment, make sure these headers are for the host system */
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

/* The remaining headers are for the target system. */
#include <libelf.h>
#include "unixsyms.h"

int cmp();

char *namep = namepool;		/* current pointer into namepool */
int symindex;			/* index into symtable */
Elf32_Shdr *esections[20];	/* for ELF section headers */
int verbose;			/* verbose output flag */


main(argc, argv)
int argc;
char **argv;
{
	long patchloc = 0;		/* location to patch output into */
	long sizeloc = 0;		/* location of patch size in file */
	int patchlen;                   /* length of output */
	int slen;
	int ifd = -1;
	int i;
	char *name;
	FILE		*fp;		/* stream for elf file */
	Elf		*elf_file;
	Elf32_Ehdr	*p_ehdr;	/* elf file header */
	unsigned	data_encoding;	/* byte encoding format in file */
	Elf_Scn		*scn;		/* elf section header	*/
	Elf_Scn		*str_scn, *sym_scn;
	Elf_Data	*str_data;	/* info on str_tab	*/
	Elf_Data	*sym_data;	/* info on symtab	*/
	Elf_Data	src_data, dst_data;
	unsigned long	data_word;
	size_t		str_ndx;
	Elf32_Sym	*esym;		/* pointer to ELF symbol	*/
	size_t		sym_size = 0;
	size_t		nsyms = 0 ;	/* number of symbols	*/
	char		*prog = argv[0];
	int		c;
	extern char	*optarg;
	extern int	optind;

	while ((c = getopt(argc, argv, "vi:")) != EOF) {
		switch (c) {
		case 'v':
			verbose = 1;
			break;
		case 'i':
			ifd = open(optarg, O_RDONLY);
			errno = 0;
			break;
		default:
			goto usage;
		}
	}

	if (argc - optind != 1) {
usage:
		fatal(99, "%s: usage: [-v | -i init-cmd-file ] %s ELF-file|COFF-file\n",
			prog, prog);
	}

	if ((fp = fopen(argv[optind], "r+")) == NULL)
		fatal(10, "%s: cannot open %s\n", prog, argv[optind]);

	if (elf_version(EV_CURRENT) == EV_NONE)
		fatal(11, "%s: ELF library is out of date\n", prog);

	if ((elf_file = elf_begin(fileno(fp), ELF_C_READ, (Elf *)0)) == 0) {
		fatal(12, "%s: ELF error in elf_begin: %s\n", prog,
				elf_errmsg(elf_errno()));
	}

	/*
	 *	get ELF header
	 */
	if ((p_ehdr = elf32_getehdr( elf_file )) == 0) {
		fatal(13, "%s: problem with ELF header: %s\n", prog,
				elf_errmsg(elf_errno()));
	}
	data_encoding = p_ehdr->e_ident[EI_DATA];

	/*
	 *	load section table
	 */
	i=0;
	scn = 0;
	while(( scn =  elf_nextscn( elf_file,scn )) != 0 ) {
		esections[ ++i ] = elf32_getshdr( scn );
		if( esections[ i ]->sh_type == SHT_STRTAB){
			str_scn = scn;

			str_data = 0;
			if(( str_data = elf_getdata( str_scn, str_data ))== 0 ||
			    str_data->d_size == 0)
				fatal(10, "%s: could not get string section data.\n");
		}
		else if( esections[ i ]->sh_type == SHT_SYMTAB){
			sym_scn = scn;
			str_ndx = esections[ i ]->sh_link;

			esym = NULL;
			sym_data = 0;
			if ((sym_data = elf_getdata(sym_scn, sym_data )) == 0)
				fatal(10, "%s: no symbol table data.\n");
			sym_size = sym_data->d_size;
			esym = (Elf32_Sym *)sym_data->d_buf;
			nsyms = sym_size / sizeof(Elf32_Sym);
			esym++;	/* first member holds number of symbols	*/
		}
	}



	/* for each symbol in input file... */

	for (i = 1; i < nsyms ; i++, esym++ )
	{


		/* we only care about it if it is external */
		if (ELF32_ST_BIND( esym->st_info) == STB_GLOBAL ) 
		{
			name = elf_strptr( elf_file, str_ndx, (size_t)esym->st_name);

			/* skip symbols that start with '.' */
			if (name[0] == '.')
				continue;

			/* if it's the magic symbol, remember patch location */

			if (strcmp(name, MAGICSYM) == 0) {
				patchloc = esections[esym->st_shndx]->sh_offset +
				    esym->st_value - esections[esym->st_shndx]->sh_addr;
			}

			/* if it's the size symbol, remember the location */

			if (strcmp(name, SIZESYM) == 0){
				sizeloc = esections[esym->st_shndx]->sh_offset +
				    esym->st_value - esections[esym->st_shndx]->sh_addr;
			}

			/* make a symtable entry for the symbol */

			if (symindex == MAXSYMS)
				fatal(11, "too many externs in %s\n", argv[1]);
			symtable[symindex].value = esym->st_value;
			symtable[symindex++].nameoffset = namep - namepool;

			/* copy the symbol's name into the name pool */

			strcpy(namep, name);
			namep += strlen(name) + 1;
		}
	}
	elf_end(elf_file);

	/* sort symtable based on value */

	qsort((char *)symtable, symindex, sizeof(struct symbols), cmp);

	/* write out value/name pairs at patchloc */

	if (!patchloc)
		fatal(1, "no symbol named '%s' found in %s\n", MAGICSYM, argv[optind]);
	if (!sizeloc)
		fatal(1, "no symbol named '%s' found in %s\n", SIZESYM, argv[optind]);

	src_data.d_version = dst_data.d_version = EV_CURRENT;
	src_data.d_buf = dst_data.d_buf = (char *)&data_word;
	src_data.d_size = dst_data.d_size = sizeof(data_word);
	src_data.d_type = ELF_T_WORD;

	if (lseek(fileno(fp), sizeloc, 0) == -1L)
		fatal(3, "%s: can't seek to %s\n", prog, SIZESYM);
	if (read(fileno(fp), &data_word, sizeof(data_word)) != sizeof(data_word))
		fatal(4, "%s: can't read %s\n", prog, SIZESYM);
	if (!elf32_xlatetom(&dst_data, &src_data, data_encoding)) {
		fatal(5, "%s: ELF xlate 1 failed: %s\n", prog,
				elf_errmsg(elf_errno()));
	}
	kdb_symsize = data_word;

	/* Now write out the symbol table we've constructed */

	/* Get to start of symtable, but skip over size and count for now */
	if (fseek(fp, patchloc + 2 * sizeof(long), 0) == -1)
		fatal(3, "%s: can't seek to %s\n", prog, MAGICSYM);

	for (i = 0, patchlen = 0; i < symindex; i++)
	{
		/* Round string length to multiple of 4, so addresses
		   are always long-word aligned. */
		slen = (strlen(symtable[i].nameoffset + namepool) +
				sizeof(long)) & ~(sizeof(long) - 1);
		patchlen += (slen + sizeof(long));
		if (patchlen > kdb_symsize - 4 * sizeof(long)) {
			printf("patch=%d kdb_sym=%d\n",patchlen, kdb_symsize);
			fatal(2, "symbol table too long\n");
		}
		if (verbose) {
			printf("%s: %X\n", symtable[i].nameoffset + namepool,
					   symtable[i].value);
		}
		fwrite(symtable[i].nameoffset + namepool, slen, 1, fp);
		data_word = symtable[i].value;
		if (!elf32_xlatetof(&dst_data, &src_data, data_encoding)) {
			fatal(5, "%s: ELF xlate 2 failed: %s\n", prog,
					elf_errmsg(elf_errno()));
		}
		fwrite((char *)&data_word, sizeof(data_word), 1, fp);
	}

	/* Write out size and count */
	if (fseek(fp, patchloc, 0) == -1)
		fatal(6, "%s: can't seek to %s\n", prog, MAGICSYM);
	data_word = patchlen;
	if (!elf32_xlatetof(&dst_data, &src_data, data_encoding)) {
		fatal(5, "%s: ELF xlate 3 failed: %s\n", prog,
				elf_errmsg(elf_errno()));
	}
	fwrite((char *)&data_word, sizeof(data_word), 1, fp);
	data_word = symindex;
	if (!elf32_xlatetof(&dst_data, &src_data, data_encoding)) {
		fatal(5, "%s: ELF xlate 4 failed: %s\n", prog,
				elf_errmsg(elf_errno()));
	}
	fwrite((char *)&data_word, sizeof(data_word), 1, fp);

	/* Account for header and 2-word null-terminator */
	patchlen += 4 * sizeof(long);

	fprintf(stderr, "%d symbols, table length = %d bytes (decimal)\n",
	    symindex, patchlen);

	if (ifd != -1) {
		int	n, cnt = 0;
		char	buf[1024];

		if (fseek(fp, patchloc + patchlen, 0) == -1)
			fatal(6, "%s: can't seek to end of %s\n", prog, MAGICSYM);
		while ((n = read(ifd, buf, sizeof(buf))) > 0) {
			if ((patchlen += n) >= kdb_symsize)
				fatal(2, "initial command string too long\n");
			fwrite(buf, n, 1, fp);
			cnt += n;
		}
		buf[0] = 0;
		fwrite(buf, 1, 1, fp);
		fprintf(stderr, "initial command string loaded (%d bytes)\n",
				cnt);
		close(ifd);
	}

	fclose(fp);
}

fatal(code, format, arg1, arg2, arg3)
char *format;
char *arg1, *arg2, *arg3;
{
	fprintf(stderr, format, arg1, arg2, arg3);
	exit(code);
}

cmp(a, b)
struct symbols *a, *b;
{
	return(a->value > b->value ? 1 : -1);
}
