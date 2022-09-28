/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nm:common/nm.c	1.26"

#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#define VOID_P void *
#else
#define VOID_P char *
#endif

/* exit return codes */
#define NOARGS	1
#define	BADELF	2

#include "libelf.h"
#include "ccstypes.h"
#include "sgs.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

typedef struct {		/* structure to translate symbol table data */
	int  indx;
	char * name;
	Elf32_Addr value;
	Elf32_Word size;
	int type;
	int bind;
	unsigned char other;
	Elf32_Half shndx;
} SYM;

#define UNDEFINED "U"

#define OPTSTR	"oxhvnurplCVefT?"	/* option string for getopt() */

#define DATESIZE 60

#define TYPE 5
#define BIND 3

static char *key[TYPE][BIND];

static  int	/* flags: ?_flag corresponds to ? option */

	o_flag=0,	/* print value and size in octal */
	x_flag=0,	/* print value and size in hex */
	h_flag=0,	/* suppress printing of headings */
	v_flag=0,	/* sort external symbols by value */
	n_flag=0,	/* sort external symbols by name */
	u_flag=0,	/* print only undefined symbols */
	r_flag=0,	/* prepend object file or archive name to each symbol name */
	p_flag=0,	/* produce terse output */
	l_flag=0,	/* produce long listing of output */
	C_flag=0,	/* print decoded C++ names */
	V_flag=0;	/* print version information */

static int	flags=0;	/* for use with the default behavior of nm */

extern int close();
extern char *elf_demangle();

static char	*prog_name;
static int	errflag=0;
static void usage();
static void each_file();
static void process();
static Elf_Scn *get_scnfd();
static VOID_P get_scndata();
static void get_symtab();
static SYM *readsyms();
static int compare();
static char *lookup();
static void print_ar_files();
static void print_symtab();



/*
 * Parses the command line options and then
 * calls each_file() to process each file.
 */

main(argc, argv)
	int argc;
	char *argv[];
{

	char        *optstr = OPTSTR; /* option string used by getopt() */
	extern  int optind;		/* arg list index */
	int	    optchar;

	/* table of keyletters for use with -p option */

	key[STT_NOTYPE][STB_LOCAL] = "n";
	key[STT_NOTYPE][STB_GLOBAL] = "N";
	key[STT_NOTYPE][STB_WEAK] = "N*";
	key[STT_OBJECT][STB_LOCAL] = "d";
	key[STT_OBJECT][STB_GLOBAL] = "D";
	key[STT_OBJECT][STB_WEAK] = "D*";
	key[STT_FUNC][STB_LOCAL] = "t";
	key[STT_FUNC][STB_GLOBAL] = "T";
	key[STT_FUNC][STB_WEAK] = "T*";
	key[STT_SECTION][STB_LOCAL] = "s";
	key[STT_SECTION][STB_GLOBAL] = "S";
	key[STT_SECTION][STB_WEAK] = "S*";
	key[STT_FILE][STB_LOCAL] = "f";
	key[STT_FILE][STB_GLOBAL] = "F";
	key[STT_FILE][STB_WEAK] = "F*";

	prog_name = argv[0];

	while ( (optchar = getopt(argc, argv, optstr)) != -1) {
		switch (optchar) {
		case 'o':	if (!x_flag) o_flag=1;
				else (void)fprintf(stderr, "%s: -x set, -o ignored\n",
				prog_name);
				flags=1; break;
		case 'x':	if(!o_flag) x_flag=1;
				else (void)fprintf(stderr, "%s: -o set, -x ignored\n",
				prog_name);
				flags=1; break;
		case 'h':	h_flag=1; flags=1; break;
		case 'v':	if(!n_flag) v_flag=1;
				else (void)fprintf(stderr, "%s: -n set, -v ignored\n",
				prog_name);
				flags=1; break;
		case 'n':	if (!v_flag) n_flag=1;
				else (void)fprintf(stderr, "%s: -v set, -n ignored\n",
				prog_name);
				flags=1; break;
		case 'u':	u_flag=1;
				flags=1; break;
		case 'r':	r_flag=1;
				flags=1; break;
		case 'p':	p_flag=1; flags=1; break;
		case 'l':	l_flag=1; break;
		case 'C':	C_flag=1; flags=1; break;
		case 'V':	V_flag=1; flags=1;
				(void) fprintf(stderr, "nm: %s %s\n", PLU_PKG, PLU_REL);break;
		case 'e':	flags=1;
				(void)fprintf(stderr, "%s: Warning: the -e option has no effect and will be removed in the next release\n", prog_name);
				break;
		case 'f':	flags=1;
				(void)fprintf(stderr, "%s: Warning: the -f option has no effect and will be removed in the next release\n", prog_name);
				break;
		case 'T':	flags=1;
				(void)fprintf(stderr, "%s: Warning: the -T option has no effect and will be removed in the next release\n", prog_name);
				break;
		case '?':	errflag +=1;break;
		default:		  break;
		}
	}
	if (flags == 0)
		flags = 1;	/* set the default behavior of nm */

	if ( errflag || (optind >= argc))
	{
		if(! ( V_flag && (argc==2) ) )
		{
			usage();
			exit(NOARGS);
		}
	}

	while (optind < argc) {
		each_file(argv[optind]);
		optind++;
	}
	return (errflag);
}



/*
 * Print out a usage message in short form when program is invoked
 * with insufficient or no arguements, and in long form when given
 * either a ? or an invalid option.
 */

static void
usage()
{
	(void) fprintf(stderr,
	"Usage: %s [-%s] file(s) ...\n", prog_name, OPTSTR);
	if (errflag)
	{
		(void) fprintf(stderr,
		"\t\t[-o print value and size in octal]\n\
		[-x print value and size in hex]\n\
		[-h suppress printing of headings]\n\
		[-v sort external symbols by value]\n\
		[-n sort external symbols by name]\n\
		[-u print only undefined symbols]\n\
		[-r prepend object or archive filename to symbol name]\n\
		[-p produce terse, easily parsable output]\n\
		[-l produce long listing of output]\n\
		[-C print decoded C++ names]\n\
		[-V print version information]\n\
		[-e print only external or static symbols (ignored)]\n\
		[-f produce full output (ignored)]\n\
		[-T truncate names that over flow column width (ignored)]\n");
	}
}



/*
 * Takes a filename as input.  Test first for a valid version
 * of libelf.a and exit on error.  Process each valid file
 * or archive given as input on the command line.  Check
 * for file type.  If it is an archive, call print_ar_files
 * to process each member of the archive in the same manner
 * as object files on the command line.  The same tests for
 * valid object file type apply to regular archive members.
 * If it is an ELF or COFF object file, process it; otherwise
 * warn that it is an invalid file type and return from
 * processing the file.
 */

static void 
each_file(filename)
char * filename;
{
	Elf        *elf_file;
	int        fd;
	Elf_Kind   file_type;

	struct stat buf;

	Elf_Cmd cmd;
	errno = 0;
	if (stat(filename,&buf) == -1) {
		(void)fprintf(stderr, "%s: ", prog_name);
		perror(filename);
		errflag++;
		return;
	}
	if(elf_version(EV_CURRENT) == EV_NONE)
	{
		(void)fprintf(stderr,"%s: %s: Libelf is out of date\n",
		prog_name, filename);
		exit(BADELF);
	}

	if ((fd = open((filename), O_RDONLY)) == -1)
	{
		(void)fprintf(stderr, "%s: %s: cannot read file\n", prog_name, filename);
		return;
	}
	cmd = ELF_C_READ;
	if ( (elf_file = elf_begin(fd, cmd, (Elf *) 0)) == NULL )
	{
		/*(void)fprintf(stderr,"%s: problem opening file %s\n",
			prog_name, filename);*/
		(void)fprintf(stderr, "%s: %s: %s\n", prog_name, filename, elf_errmsg(-1));
		return;
	}
	file_type = elf_kind(elf_file);
	if(file_type == ELF_K_AR)
	{
		print_ar_files(fd, elf_file, filename);	
	}
	else
	{
		if ( (file_type == ELF_K_ELF) || (file_type == ELF_K_COFF) )
		{
                        if (file_type == ELF_K_COFF)
                                (void)fprintf(stderr, "%s: %s: warning - internal conversion of COFF file to ELF\n",
                                prog_name, filename);

			if(!h_flag)
			{
				if(u_flag)
				{
					if(p_flag)
						(void)printf("\n\n%s:\n\n", filename);
					else
						(void)printf("\n\nUndefined symbols from %s:\n\n", filename);
				}
				else
				{
					if(p_flag)
						(void)printf("\n\n%s:\n", filename);
					else
						(void)printf("\n\nSymbols from %s:\n", filename);
				}
			}
			process(elf_file, filename);
		}
		else
		{
			(void)fprintf(stderr, "%s: %s: invalid file type\n",
				prog_name, filename);
			errflag++;
			elf_end(elf_file);
			(void)close(fd);
		}
	}
	elf_end(elf_file);
	(void)close(fd);
}



/* Get the ELF header and, if it exists, call get_symtab()
 * to begin processing of the file; otherwise, return from
 * processing the file with a warning.
 */

static void
process(elf_file, filename)
Elf * elf_file;
char * filename;
{
	Elf32_Ehdr *p_ehdr;

	p_ehdr = elf32_getehdr(elf_file);
	if (p_ehdr == (Elf32_Ehdr *)0)
	{
		/*(void)fprintf(stderr,"%s: %s: problem opening ELF header\n",
			prog_name, filename);*/
		(void)fprintf(stderr, "%s: %s: %s\n", prog_name, filename, elf_errmsg(-1));
		return;
	}

	get_symtab(elf_file, p_ehdr, filename);

}



/* Get section descriptor for the associated string table
 * and verify that the type of the section pointed to is
 * indeed of type STRTAB.  Returns a valid section descriptor
 * or NULL on error.
 */

static Elf_Scn *
get_scnfd(e_file, shstrtab, SCN_TYPE)
Elf         *e_file;
Elf32_Half  shstrtab;
Elf32_Half  SCN_TYPE;
{
	Elf_Scn    *fd_scn;
	Elf32_Shdr *p_scnhdr;


	if( (fd_scn = elf_getscn(e_file, shstrtab) ) == NULL)
	{
		return NULL;
	}

	if( (p_scnhdr = elf32_getshdr(fd_scn) ) == NULL)
	{
		return NULL;
	}

	if( p_scnhdr->sh_type != SCN_TYPE)
	{
		return NULL;
	}

	return(fd_scn);
}


/*
 * Get the section descriptor and set the size of the
 * data returned.  Data is byte-order converted by
 * the access library.  Section size must be calculated
 * on the return from elf_getdata() in order to correctly
 * print out COFF file information.
 */

static VOID_P 
get_scndata(fd_scn, scn_size)
Elf_Scn *fd_scn;
size_t  *scn_size;
{
	Elf_Data *p_data;

	p_data = 0;
	if( (p_data = elf_getdata(fd_scn, p_data)) == 0 ||
		p_data->d_size == 0)
	{
		return NULL;
	}

	*scn_size = p_data->d_size;
	return( p_data->d_buf );
}


/*
 * Print the symbol table.  This function does not print the contents
 * of the symbol table but sets up the parameters and then calls
 * print_symtab to print the symbols.  This function does not assume
 * that there is only one section of type SYMTAB.  Input is an opened
 * ELF file, a pointer to the ELF header, and the filename.
 */

static void
get_symtab(elf_file, elf_head_p, filename)
Elf        *elf_file;
Elf32_Ehdr *elf_head_p;
char       *filename;
{

	Elf32_Shdr    *p_shdr;
	Elf_Scn       *scn, *scnfd;
	char          *offset = NULL;
	char          *s_name;
	size_t	      str_size;


/* get section header string table */
	scnfd = get_scnfd(elf_file, elf_head_p->e_shstrndx, SHT_STRTAB);
	if(scnfd == NULL)
	{
		(void)fprintf(stderr, "%s: %s: could not get string table\n",
			prog_name, filename);
		return;
	}
	offset = (char *)get_scndata(scnfd, &str_size);
	if(offset == (char *)0)
	{
		(void)fprintf(stderr, "%s: %s: no data in string table\n",
			prog_name, filename);
		return;
	}

	scn = 0;
	while ( (scn = elf_nextscn(elf_file, scn) ) != 0)
	{
		s_name = NULL;
		p_shdr = NULL;
		if( (p_shdr = elf32_getshdr(scn) ) == 0)
		{
			/*(void)fprintf(stderr, "%s: %s: cannot get section header\n", prog_name, filename);*/
			(void)fprintf(stderr, "%s: %s: %s:\n", prog_name, filename, elf_errmsg(-1));
			return;
		}
		s_name = offset + p_shdr->sh_name;

		if (p_shdr->sh_type == SHT_SYMTAB)
		{
			print_symtab(elf_file, s_name, p_shdr, scn, filename);
		}
	} /* end while */
}


/*
 * Process member files of an archive.  This function provides
 * a loop through an archive equivalent the processing of
 * each_file for individual object files.
 */

static void
print_ar_files(fd, elf_file, filename)
int fd;
Elf *elf_file;
char *filename;
{
	Elf_Arhdr  *p_ar;
	Elf        *arf;
	Elf_Cmd    cmd;
	Elf_Kind   file_type;


	cmd = ELF_C_READ;
	while((arf = elf_begin(fd, cmd, elf_file)) != 0)
	{
		p_ar = elf_getarhdr(arf);
		if(p_ar == NULL)
		{
			/*(void) fprintf(stderr, "%s: %s: cannot read archive header\n",
				prog_name, filename);*/
			(void)fprintf(stderr, "%s: %s: %s\n", prog_name, filename, elf_errmsg(-1));
			return;
		}
		if( (int)strncmp(p_ar->ar_name, "/", 1) == 0)
		{
			cmd = elf_next(arf);
			elf_end(arf);
			continue;
		}

		if(!h_flag)
		{
			if(u_flag)
			{
				if(p_flag)
					(void)printf("\n\n%s[%s]:\n\n",
					filename, p_ar->ar_name);
				else
					(void)printf("\n\nUndefined symbols from %s[%s]:\n\n",
					filename, p_ar->ar_name);
			}
			else
			{
				if(p_flag)
					(void)printf("\n\n%s[%s]:\n",
					filename, p_ar->ar_name);
				else
					(void)printf("\n\nSymbols from %s[%s]:\n",
					filename, p_ar->ar_name);
			}
		}
		file_type = elf_kind(arf);
		if ( (file_type == ELF_K_ELF) || (file_type == ELF_K_COFF) )
		{
			if (file_type == ELF_K_COFF)
				(void)fprintf(stderr, "%s: %s[%s]: warning - internal conversion of COFF file to ELF\n", prog_name, filename, p_ar->ar_name);
			process(arf, p_ar->ar_name);
		}
		else
		{
			(void)fprintf(stderr, "%s: %s: invalid file type\n",
				prog_name, p_ar->ar_name);
			cmd = elf_next(arf);
			elf_end(arf);
			errflag++;
			continue;
		}

		cmd = elf_next(arf);
		elf_end(arf);
	} /* end while */

}


/* Print the symbol table according to the flags that were
 * set, if any.  Input is an opened ELF file, the section name,
 * the section header, the section descriptor, and the filename.
 * First get the symbol table with a call to get_scndata.
 * Then translate the symbol table data in memory by calling
 * readsyms().  This avoids duplication of function calls
 * and improves sorting efficiency.  qsort is used when sorting
 * is requested.
 */
static void
print_symtab(elf_file, s_name, p_shdr, p_sd, filename)
Elf     *elf_file;
char    *s_name;
Elf32_Shdr *p_shdr;
Elf_Scn *p_sd;
char *filename;
{
	Elf32_Sym  *sym;
	SYM        *sym_data;
	size_t     count = 0;
	size_t     sym_size = 0;
	char       *sym_key;


	if (!h_flag && !u_flag)
	{
		/*
		if(l_flag)
			(void)printf("%s:\n", s_name);
		else
		*/
			(void)printf("\n");
		if(!p_flag)
		{
			if(o_flag)
				(void)printf("%-9s%-13s%-13s%-6s%-6s%-6s%-8s%s\n\n",
				"[Index]", " Value", " Size", "Type", "Bind", "Other",
				"Shndx", "Name");
			else if (x_flag)
				(void)printf("%-9s%-11s%-11s%-6s%-6s%-6s%-8s%s\n\n",
				"[Index]", " Value", " Size", "Type", "Bind", "Other",
				"Shndx", "Name");
			else
				(void)printf("%-9s%-11s%-9s%-6s%-6s%-6s%-8s%s\n\n",
				"[Index]", " Value", " Size", "Type", "Bind", "Other",
				"Shndx", "Name");
		}
	}

	/* get symbol table data */
	sym = NULL;
	if ( (sym = (Elf32_Sym *)get_scndata(p_sd, &sym_size) ) == NULL )
	{
		(void)printf("%s: %s - No symbol table data\n", prog_name, filename);
		return;
	}

	count = sym_size/sizeof(Elf32_Sym);

	sym++;	/* first member holds the number of symbols */

	/* translate symbol table data */

	sym_data = readsyms(sym, count, elf_file, p_shdr->sh_link);

	if(sym_data == NULL)
	{
		(void)fprintf(stderr, "%s: %s: problem reading symbol data\n",
			prog_name, filename);
		return;
	}

	/* prepare sorted output if needed */

	if(n_flag || v_flag)
		qsort( (char *)sym_data, count-1, sizeof(*sym_data), compare);

	while (count > 1)
	{

		if (u_flag)
		{
			if( (sym_data->shndx == SHN_UNDEF) &&
				(strlen(sym_data->name)) )
			{
				if(!r_flag)
					(void)printf("    %s\n", sym_data->name);
				else
					(void)printf("    %s:%s\n", filename, sym_data->name);
			}
		} /* end u_flag */
		else if(p_flag)
		{
			if (x_flag)
				(void)printf("0x%.8lx ", sym_data->value);
			else if (o_flag)
				(void)printf("0%.11lo ", sym_data->value);
			else
				(void)printf("%.10lu ", sym_data->value);

			if( (sym_data->shndx == SHN_UNDEF) &&
				(strlen(sym_data->name)) )
				sym_key = UNDEFINED;
			else
				sym_key = lookup(sym_data->type, sym_data->bind);
			if(sym_key != NULL)
			{
				if(!l_flag)
					(void)printf("%c ", sym_key[0]);
				else
					(void)printf("%-3s", sym_key);
			}
			else
			{
				if(!l_flag)
					(void)printf("%-2d", sym_data->type);
				else
					(void)printf("%-3d", sym_data->type);
			}
			if (!r_flag)
				(void)printf("%s\n", sym_data->name);
			else
				(void)printf("%s:%s\n", filename, sym_data->name);
			
		} /* end p_flag */

		else
		{

			(void)printf("[%d]\t|", sym_data->indx); 
			if(o_flag)
				(void)printf("0%.11lo|0%.11lo|",
					sym_data->value,
					sym_data->size);
			else if(x_flag)
				(void)printf("0x%.8lx|0x%.8lx|",
					sym_data->value,
					sym_data->size);
			else
				(void)printf("%10lu|%8ld|",
					sym_data->value,
					sym_data->size);

			switch (sym_data->type)
			{
				case STT_NOTYPE:	(void)printf("%-5s", "NOTY");break;
				case STT_OBJECT:	(void)printf("%-5s", "OBJT");break;
				case STT_FUNC:		(void)printf("%-5s", "FUNC");break;
				case STT_SECTION:	(void)printf("%-5s", "SECT");break;
				case STT_FILE:		(void)printf("%-5s", "FILE");break;
				default:
					if(o_flag)
						(void)printf("%#-5o", sym_data->type);
					else if (x_flag)
						(void)printf("%#-5x", sym_data->type);
					else
						(void)printf("%-5d", sym_data->type);
			}
			(void)printf("|");
			switch (sym_data->bind)
			{
				case STB_LOCAL:	(void)printf("%-5s", "LOCL");break;
				case STB_GLOBAL:(void)printf("%-5s", "GLOB");break;
				case STB_WEAK:	(void)printf("%-5s", "WEAK");break;
				default:
					(void)printf("%-5d",sym_data->bind);
					if(o_flag)
						(void)printf("%#-5o", sym_data->bind);
					else if (x_flag)
						(void)printf("%#-5x", sym_data->bind);
					else
						(void)printf("%-5d", sym_data->bind);
			}
			(void)printf("|");
			if(o_flag)
				(void)printf("%#-5o", sym_data->other);
			else if (x_flag)
				(void)printf("%#-5x", sym_data->other);
			else
				(void)printf("%-5d", sym_data->other);
			(void)printf("|");

			switch (sym_data->shndx) {
				case SHN_UNDEF: (void)printf("%-7s", "UNDEF"); break;
				case SHN_ABS:	(void)printf("%-7s", "ABS"); break;
				case SHN_COMMON: (void)printf("%-7s", "COMMON"); break;
				default:
					if(o_flag)
						(void)printf("%-7d",
							sym_data->shndx);
					else if (x_flag)
						(void)printf("%-7d",
							sym_data->shndx);
					else
						(void)printf("%-7d",
							sym_data->shndx);
		 	}
			(void)printf("|");
			if(!r_flag)
				(void)printf("%s\n", sym_data->name);
			else
				(void)printf("%s:%s\n", filename, sym_data->name);
		} /* end else */

		sym_data++;
		count--;
	}
}


/* Return appropriate keyletter(s) for -p option.
 * Returns an index into the key[][] table or NULL if
 * the value of the keyletter is unknown.
 */
static char *
lookup(a, b)
int a;
int b;
{
	return( ( (a<TYPE) && (b<BIND) ) ? key[a][b] : NULL );
}


/* Translate symbol table data particularly for sorting.
 * Input is the symbol table data structure, number of symbols,
 * opened ELF file, and the string table link offset.
 */
static SYM *
readsyms(data, num, elf, link)
Elf32_Sym  *data;
size_t      num;
Elf         *elf;
Elf32_Word  link;
{
	SYM *s, *buf;
	int i;

	if( (buf = (SYM *)calloc(num, sizeof(SYM)) ) == NULL)
	{
		(void)fprintf(stderr, "%s: cannot calloc space\n", prog_name);
		return NULL;
	}

	s = buf;	/* save pointer to head of array */

	for(i=1; i<num; i++,data++,buf++)
	{
		buf->indx = i;
		if(C_flag)
			buf->name = elf_demangle( (char *)elf_strptr(elf, link, data->st_name) );
		else
			buf->name = (char *)elf_strptr(elf, link, data->st_name);
		buf->value = data->st_value;
		buf->size = data->st_size;
		buf->type = ELF32_ST_TYPE(data->st_info);
		buf->bind = ELF32_ST_BIND(data->st_info);
		buf->other = data->st_other;
		buf->shndx = data->st_shndx;
	}	/* end for loop */

	return(s);
}


/* compare either by name or by value for sorting.
 * This is the comparison function called by qsort to
 * sort the symbols either by name or value when requested.
 */
static int
compare(a, b)
SYM *a, *b;
{
	if(n_flag)
		return((int)strcmp(a->name, b->name));
	else
	{
		if(a->value > b->value)
			return 1;
		else
			return( (a->value==b->value) -1 );
	}
}
