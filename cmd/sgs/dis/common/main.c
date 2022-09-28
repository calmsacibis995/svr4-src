/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dis:common/main.c	1.9.1.1"

#include	<stdio.h>
#include	<fcntl.h>

#include	"sgs.h"
#include	"paths.h"
#include	"structs.h"
#include	"ccstypes.h"

#ifdef __STDC__
#include	<stdlib.h>
#endif

#define TOOL	"dis"
#define         LIB     "/lib"
#define OPTSTR "oVLsST?d:D:F:t:l:"
#define OPTIONS "-oVLs? -d name -Dname -F name -t name -l name"

static	int	save_sflag; /* used to save original value of sflag */
			    /* When a file has no symbol table, sflag */
			    /* is set to 0 for only that file; must be reset */
extern	int	getopt(),
		close();
extern  void    exit();
extern  int     strncmp(), strcmp();


struct l_list{
	char	*data;
	struct l_list *next;
};

typedef struct l_list elt;
typedef elt *link;


/** LINKED LIST OF FILE NAMES TO BE DISASSEMBLED - HEAD AND TAIL POINTERS **/

static link 	fhead = NULL, ftail;

/*
 *	main (argc, argv)
 *
 *	This routine calls the function command_line() to process the
 *	command line arguments - options and file names. The file names
 *	(those given via the -l option or via the command line)
 *	are placed in a linked list when the function addl_file() is called.
 *
 */

main(argc, argv)
int	argc;
char	*argv[];
{
	static void     command_line(),
			each_file();

	static char     buffer[BUFSIZ];
	static int	header=0;

	if (argc == 1) {
		(void) fprintf(stderr,
		"%s%s: Usage: %s%s [%s] file(s) \n"
		,SGS,TOOL,SGS,TOOL, OPTIONS);
		exit(1);
	}

	setbuf(stdout, buffer);

	(void)elf_version(EV_NONE);
        if (elf_version(EV_CURRENT) == EV_NONE) 
	{
                (void) fprintf(stderr,"%sdis: libelf.a out of date.\n",SGS);
		exit(1);
        }

	command_line(argc, argv);

	ftail = fhead;		/* Traverse linked list of file names */
	while (ftail != NULL)
	{
		if (!header) 	/* print header only once */
		{
			(void)printf("\t\t****   DISASSEMBLER  ****\n\n");
			header++;
		}
	
		each_file();
		ftail = ftail->next; /* Next file in linked list */
	}

        exit(0);
        /*NOTREACHED*/
}

/*  addl_file()
 *
 *	ADD A FILE NAME INTO THE LINKED LIST OF FILES TO BE DISASSEMBLED.
 *
 */

static void
addl_file (file)
char	*file;
{
	if (fhead == NULL)
	{
		fhead = (link) malloc (sizeof(elt));
		ftail = fhead;
	}
	else
	{
		ftail->next = (link) malloc (sizeof(elt));
		ftail = ftail->next;
	}
	ftail->data = file;
	ftail->next = NULL;
	return;
}


/*
 *	command_line (argc, argv)
 *
 *	This routine processes the command line received by the disassembler.
 */

static void
command_line(argc, argv)
int	argc;
char	*argv[];
{
	extern  int	tflag;
	extern	int	oflag;
	extern	int	sflag;
	extern  int     Sflag;
	extern	int	trace;
	extern	int	fflag;
	extern	int	Lflag;
	extern	short	aflag;
	extern  NFUNC   *ffunction;
	extern  unsigned int    strlen();
	extern	char	**namedsec;
	extern	int	*namedtype;
	extern	int	nsecs;

#if I386
        extern int Rflag; /* Reverse 286/386 mode */
#endif

	extern int optind;
	extern char *optarg;

	char *optstr = OPTSTR;
	int optchar, str_1, str_2, str_len, Vflag = 0, errflag = 0;
	char *libs;
	static int lflag = 0;

	if ((ffunction = (NFUNC *) calloc(argc,sizeof(NFUNC))) == NULL) 
	{
                (void)fprintf(  stderr, 
				"memory allocation failure on call to calloc");
		exit(1);
       	}

	if ((namedsec = (char **) calloc(argc,sizeof(char *))) == NULL) 
	{
                (void)fprintf(  stderr, 
				"memory allocation failure on call to calloc");
		exit(1);
	}

	if ((namedtype = (int *) calloc(argc,sizeof(int))) == NULL) 
	{
                (void)fprintf(  stderr, 
				"memory allocation failure on call to calloc");
		exit(1);
	}

	while (( optchar = getopt(argc, argv, optstr)) != -1) 
	{
		switch (optchar) 
		{
				case 'T':
					trace++;
					break;
				case 'o':
					oflag++;
					break;
#if I386
				case 'R':
					Rflag++;
					break;
#endif

				case 's':
					sflag++;
					save_sflag++;
					break;

				case 'S':
					Sflag++; 
					break;
				case 'L':
					Lflag++; 
					break;
				case 'l':
					lflag++;
					str_1 = sizeof(LIBDIR)- 1 + strlen(LIB);
                                        str_2 = strlen(optarg);
                                        /* add 2 for '.a'       */
                                        str_len = str_1 + str_2 +2;
                                        if ((libs = (char *) malloc(str_len+1*sizeof(char))) == NULL) 
					{
                                                (void)fprintf(stderr, 
				 "memory allocation failure on call to malloc");
                                                exit(1);
                                        }

                                        (void)sprintf(libs,"%s%s%s.a"
						,LIBDIR,LIB,optarg);

                                        if (trace > 0)
                                                (void)printf("\nlib is {%s}\n"
								, libs);
					addl_file(libs);
					break;

				case 'd':/*disassemble as a data section flag */
					nsecs++;
					namedsec[nsecs] = optarg;
					namedtype[nsecs] = DATA;
					break;

				case 'D': /* 	print addresses of data 
						rather than offsets */
                                        aflag++;
					nsecs++;
                                        namedsec[nsecs] = optarg;
                                        namedtype[nsecs] = DATA;
                                        break;

				case 't':
					tflag++;
					nsecs++;
					namedsec[nsecs] = optarg;
					namedtype[nsecs] = TEXT;
					break;

				case 'V':	/* version flag */
					Vflag++;
					(void)fprintf(stderr,"%s%s: %s %s\n"
						,SGS,TOOL, SGU_PKG, SGU_REL);
					break;

				case 'F':
					fflag++;
					ffunction[fflag - 1].funcnm = optarg; 
					break;

				case '?': 
					errflag++;
					break;
				default:
					break;
		}	/* end of switch */

	}	/* end of while */
			
	if ( errflag || (optind >= argc))
	{
		if (!( Vflag && (argc==2)) && !lflag )
		{
			(void) fprintf(stderr,
					"%sdis: Usage: %sdis [%s] file\n"
					,SGS,SGS,OPTIONS);
			exit(1);
		}
	}

	if (errflag && lflag)
	{
		(void) fprintf(stderr,
					"%sdis: Usage: %sdis [%s] file\n"
					,SGS,SGS,OPTIONS);
                        exit(1);
	}

	for (; optind < argc; optind++)
	{
		addl_file (argv[optind]);
	}

	return;
}


static void
each_file()
{
	extern 	int		archive,
				symtab,
				debug,
				line,
				Rel_data,
				Rela_data,
				Rel_sec;

	extern	Elf_Cmd         cmd;
	extern	Elf             *elf, *arf;
	extern 	Elf_Arhdr       *mem_header;
	extern	Elf32_Ehdr      *ehdr;
	extern	char		*fname;

	static void	disassembly();
	int 	fd;

	archive = 0;
	symtab  = 0;
	debug   = 0;
	line	= 0;
	Rel_data = 0;
	Rela_data = 0;
	Rel_sec = FAILURE;
	
	fname = ftail->data;  /* Name of file or archive to be disassembled */
	(void)printf("\ndisassembly for %s\n",fname);

	if ( (fd = open(fname, O_RDONLY)) == -1) {
		(void) fprintf(stderr,
				 "%sdis: cannot open file %s\n",SGS, fname);
		return;
	}

	cmd = ELF_C_READ;
	arf = elf_begin(fd, cmd, (Elf *)0);

	if (elf_kind(arf) == ELF_K_AR)
		archive = 1;

	while ((elf = elf_begin(fd, cmd, arf)) != 0) {
				
		if (archive) {
			if ((mem_header = elf_getarhdr(elf) ) == NULL){
				(void) fprintf( stderr,
 				     "%sdis: %s: malformed archive (at %ld)\n",
				     SGS, fname, elf_getbase(elf) );
				return; /* next file on commandline*/
			}
		}

		if ((ehdr = elf32_getehdr(elf)) != 0) {
			if ((elf_kind(elf) == ELF_K_COFF))
			{
				if (archive)
					(void) fprintf(stderr, "%sdis: %s: Warning - internal conversion of COFF file to ELF\n" ,SGS, mem_header->ar_name);
				else
					(void) fprintf(stderr, "%sdis: %s: Warning - internal conversion of COFF file to ELF\n",SGS, fname);	
			}

			if (archive == 1)
				(void)printf("\narchive member\t\t%s\n",
							 mem_header->ar_name);
			disassembly();
		}
		else {
			if (archive){
				if ( strcmp(mem_header->ar_name,"/") != 0 &&
				     strcmp(mem_header->ar_name,"//")!= 0 )
					(void) fprintf(stderr, 
					   "%sdis: %s[%s]: invalid file type\n"
					   ,SGS,fname, mem_header->ar_name);
				}
				else
					(void) fprintf(stderr, 
					       "%sdis: %s: invalid file type\n" 
					       ,SGS, fname);
		}

		cmd = elf_next(elf);
		(void)elf_end(elf);
	}
	(void)elf_end(arf);	
	(void)close(fd);

}

/*
 *	disassembly ()
 *
 *	For each file that is disassembled, disassembly opens the
 *	necessary file pointers, builds the list of section
 *	headers, and if necessary, lists of functions and labels.
 * 	It then calls text_sections or dis_data to disassemble
 *	the sections.
 */

static void
disassembly()
{
	Elf_Scn         *scn;
	Elf_Data        *data;
	extern	int	archive,
			line,	
			symtab, 
			debug;

	extern  int	get_rel_section();
	extern	int	Rel_sec;
	extern  unsigned char   *p_data;	
	
	extern	void	build_sections(),
			get_debug_info(),
			get_line_info(),
			build_funcs(),
			label_free(),
			dis_data(),
			dis_text(),
			section_free();

	extern	int	Lflag;
	extern	char	*sname, *fname;
	extern	FUNCLIST	*next_function;
	extern	Elf		*elf;
	extern	Elf32_Shdr	*scnhdr;
	extern	int	sflag;
	extern	int	fflag;
	extern  int	Fflag;

	extern	SCNLIST	*sclist; 

	extern 	Elf_Arhdr       *mem_header;

	static void	search_table();
	static void	dis_funcs();

	SCNLIST		*sectp;

	sflag = save_sflag;

	if (fflag )
		Fflag = 1;

	build_sections(); /* make a linked list of sections */

	if (symtab == 0 && sflag)
	{
		sflag = 0;
		if (archive)
                         (void) fprintf(stderr,
			 "\n%sdis: %s[%s]:" ,SGS,fname, mem_header->ar_name);
		else
			(void) fprintf(stderr,"\n%sdis: %s:" ,SGS, fname);
			
		(void) fprintf(stderr," No symbol table in file: symbolic disassembly cannot be performed.\n");
	}
	
	if ( symtab != 0)
		build_funcs(); /* make linked list of functions for sections*/

	if ( (sflag || Lflag) && debug != 0)
		 get_debug_info();

	if ( line )
		get_line_info();

	if (fflag)
	{
		if (symtab)
			search_table();
		dis_funcs();
		Fflag = 0;
		return;
	}


	
	for ( sectp = sclist; sectp; sectp = sectp->snext )
	{

#if DEBUG
	printf("DEBUG: sectp->shdr->sh_addr is %x: sectp->scnam is %s: sectp_scnum is %d: sectp->stype is %d\n", sectp->shdr->sh_addr, sectp->scnam, sectp->scnum,sectp->stype);
#endif

                if (sectp->stype == TEXT) 
		{
                        sname = sectp->scnam;
                        (void)printf("\nsection\t%s\n", sname );
                        next_function = sectp->funcs;
                        scnhdr = sectp->shdr; 

			scn = elf_getscn(elf, sectp->scnum);
			data = 0;
			if ((data = elf_getdata(scn, data)) == 0 ||
				 data->d_size == 0)
			{
                        	(void) fprintf(stderr, 
				"%sdis: no data in section %s\n", SGS, sname);
				continue;
               		}
                        p_data = (unsigned char *)data->d_buf;
			/* If symbolic disassembly is requested, see if 
			   there's a relocation section associated
			   with the current section */
			if (sflag)
			{
			 	Rel_sec = get_rel_section(sectp->scnum);
			}
                        dis_text(sectp->shdr); 
	
		}
                else
		{
			scn = elf_getscn(elf, sectp->scnum);
                        data = 0;
                        if ((data = elf_getdata(scn, data)) == 0 || data->d_size == 0)
                        {
                                (void) fprintf(stderr,
                                "%sdis: no data in section %s\n", SGS, sname);
                                continue;
                        }
                        p_data = (unsigned char *)data->d_buf;
			dis_data(sectp->shdr);
		}
	}

	section_free();	
	label_free();

	return;
}


/*
 * search the symbol table for the named functions and fill in the
 * information in the ffunction array
 */
static void
search_table()
{
	extern  void	fatal();
	extern	int	fflag;
	extern	NFUNC	*ffunction;
	extern  SCNLIST *sclist;
	extern	Elf	*elf;
	extern	int	symtab;

	int		j;
	char		*name;
	FUNCLIST	*funcp, *fp;
	SCNLIST		*sectp;

	Elf_Scn		*scn;
	Elf32_Shdr	*shdr;
	Elf_Data	*sym_data;
	Elf32_Sym	*p;	
	int		no_of_symbols, counter;		

	for ( j=0; j<fflag; j++)
		ffunction[j].found = 0;

	if ( (scn = elf_getscn(elf,symtab)) != 0)
        {
                if ( (shdr = elf32_getshdr(scn)) != 0)
                {
                   if (shdr->sh_entsize)
                   {
                                sym_data = 0;
				if ( (sym_data = elf_getdata(scn, sym_data)) == NULL)
					fatal("no data in symbol table section");
				no_of_symbols = sym_data->d_size/sizeof(Elf32_Sym);

                                p = (Elf32_Sym *)sym_data->d_buf;
                                p++; /*the first ST entry is skipped*/

                      for (counter = 1; counter<(no_of_symbols); counter++, p++)
		      {
			name = (char *)elf_strptr(elf, shdr->sh_link,(size_t)p->st_name);
			for ( j = 0; j < fflag; j++ )
			{
                        	if (strcmp( name, ffunction[j].funcnm ) == 0)
                                	break;
                	}

			if (j != fflag)
			{
				for(sectp = sclist; sectp; sectp=sectp->snext)
                                	if (sectp->scnum == p->st_shndx)
                                        	break;
                                if (sectp == NULL)
                                        break;
		
				if ( ELF32_ST_TYPE(p->st_info) == STT_FUNC)
				{
					ffunction[j].faddr = p->st_value;
					ffunction[j].fcnindex = counter;
					ffunction[j].found = 1;
					ffunction[j].fscnum = p->st_shndx;
					for (funcp=sectp->funcs; funcp; funcp=funcp->nextfunc)
					{
                                        	if (strcmp((char *)name, funcp->funcnm) == 0)
                                                	break;
                                	}

					if (funcp->nextfunc != NULL)
					{
						fp = funcp->nextfunc;
						while( fp!=NULL && fp->faddr <= funcp->faddr)
							fp=fp->nextfunc;

						if (fp!=NULL)
						ffunction[j].fsize= fp->faddr - funcp->faddr;
						else
						ffunction[j].fsize=
						(sectp->shdr->sh_addr +
						sectp->shdr->sh_size) -
						funcp->faddr;
					}
                                	else
						ffunction[j].fsize= 
						(sectp->shdr->sh_addr + 
						sectp->shdr->sh_size) - 
						funcp->faddr;
				}
			}
		      }
		   }
                  else
                        fatal("the symbol table entry size is 0!");
                }
                else
                        fatal("failed to get the section header");
        }
        else
                fatal("failed to get the symbol table section");

        return;
}

/*
 * disassemble the functions in the ffunction array
 */

static void
dis_funcs()
{
	extern	void		dis_text();
	extern	NFUNC		*ffunction;
	extern	int		fflag, sflag;
	extern	char		*fname;
	extern 	Elf		*elf;
	extern	Elf32_Ehdr	*ehdr;
	extern	char		*sname;
	extern	FUNCLIST	*next_function;
	extern  Elf_Arhdr       *mem_header;
	extern  unsigned        char    *p_data;
	extern	int		archive;
	
	Elf_Scn		*scn;
	Elf32_Shdr	*shdr;
	Elf_Data	*data;
	int		i;
	FUNCLIST	func;
	Elf32_Addr	save_sh_addr;
	Elf32_Word	save_sh_size;

	for (i = 0; i <fflag; i++)
  	{
		if (!ffunction[i].found)
		{
			if (archive)
		 		(void) fprintf(stderr, 
					"%s%s: %s[%s]: function %s not found\n",
					SGS, TOOL, fname, mem_header->ar_name, 
					ffunction[i].funcnm);
			else
		 		(void) fprintf(stderr, 
					"%s%s: %s: function %s not found\n",
					SGS, TOOL, fname, ffunction[i].funcnm);
			continue;
		}

		/* Get the section header, and section data */
		scn = elf_getscn(elf, ffunction[i].fscnum);
		shdr = elf32_getshdr(scn);
		/* Let's pretend that the section contains only	*/
		/* the data for the current function. Thus must	*/
		/* save the original values 			*/
		save_sh_addr = shdr->sh_addr;
		save_sh_size = shdr->sh_size;

		sname = elf_strptr(elf,ehdr->e_shstrndx, (size_t)shdr->sh_name);
		(void) printf("\nsection\t%s\n", sname);

		data = 0;
		if ((data = elf_getdata(scn, data)) == 0 || data->d_size == 0)
		{
			(void) fprintf(stderr,
			"%sdis: no data in section %s\n", SGS, sname);
			return;
		}

		/* seek to the start of the function, and change the section
		 * header to fake out dis_text(); make it think the section
		 * has only one function.
		 */
		p_data = (unsigned char *)data->d_buf;

		p_data += (ffunction[i].faddr - shdr->sh_addr);
		shdr->sh_addr = ffunction[i].faddr;
		shdr->sh_size = ffunction[i].fsize;
		
		func.funcnm = ffunction[i].funcnm;
		func.faddr = ffunction[i].faddr;
		func.fcnindex = ffunction[i].fcnindex;
		func.nextfunc = NULL;
		next_function = &func;

		if (sflag)
			(void) get_rel_section(ffunction[i].fscnum);

		dis_text(shdr);
		shdr->sh_addr = save_sh_addr;
		shdr->sh_size = save_sh_size;
	}	
	return;
}
