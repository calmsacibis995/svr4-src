/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mcs:common/main.c	1.20"

#include <stdio.h>
#include <libelf.h>
#include <ar.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory.h>
#include <ccstypes.h>


#ifdef __STDC__
#include	<stdlib.h>
#else
extern char *mktemp();
#endif

#include "paths.h"
#include "sgs.h"

#if defined(__STDC__)
#define VOID void
#else
#define VOID char
#endif


#define FORMAT          "%-16s%-12ld%-6u%-6u%-8o%-10ld%-2s"
#define ROUNDUP(x)      (((x) + 1) & ~1)

#define NOSEG 1
#define IN    2 /* section is IN a segment */
#define PRIOR 3 /* section is PRIOR to a segment */
#define AFTER 4 /* section is AFTER a segment */

#define FAILURE 1
#define SUCCESS 0

#define CODE3 3 /* this code is used to prevent building a new file because
		  	mcs was given only -p and/or -P */ 
#define CODE4 4 /* this code is used to prevent building a new file because
			mcs was given a file that can't be manipulated */

/********************************* STATICS **********************************/

static char *Sect_name = ".comment";

static char *string = NULL;
static unsigned int string_size = 0;
static unsigned int original_size = 0;
static char *temp_string = NULL;
static int temp_string_size = 0;
static int sec_loc, rel_loc; 

static int  	errflag = 0,
		Might_chg = 0,
		aflag = 0,
		cflag = 0,
		Pflag = 0,
		pflag = 0,
		Vflag = 0,
		optcnt = 0,
		optbufsz = 100,
		actmax = 0,
		ar_file = 0,
		error_count = 0,
		NOBITS_Sect = 0,  /* Set if section is of type NOBITS */
		fd, fdartmp, fdtmp3, fdtmp2, fdtmp;

static struct stat 	stbuf;

static char	*elftmpfile, *artmpfile, *fname;
static char     *buf, *file_buf;

static  mode_t  mode;
static	uid_t	owner;
static	gid_t	group;

static unsigned int	Sect_index, rel_scn_index,
			Sect_exists = 0;

static int      *hash_key;      /* an array of hash keys for the file being 
				   compressed */
static int      hash_num;       /* current number of elements in hash_key */
static int      hash_end;       /* last element in hash_key */
static int      *hash_str;      /* an array of strings corresponding to the 
				   array hash_key */

static char     *strings;       /* actual string storage */
static int      next_str;       /* current position in the string array */
static int      str_size;       /* length of the string array */


static	Elf             *elffile, *arf, *elf;
static	Elf_Cmd         cmd;
static	Elf_Scn         *scn, *elf_scn;
static	Elf32_Shdr      *shdr, *elf_shdr;
static	Elf32_Ehdr      *ehdr, *elf_ehdr;
static	Elf32_Phdr      *phdr, *elf_phdr;
static	Elf_Data        *data, *elf_data;
static	Elf_Arhdr       *mem_header;

static Elf32_Off	*b_e_seg_table;

static  int             *sec_table;     /* array contains section's new */
                                        /* position in the file*/
static  Elf32_Off       *off_table;     /* array maintains section's offset;
                                           set to retain old offset, else 0 */
static  int             *nobits_table;  /* array maintains NOBITS sections */


struct action {

                enum what_to_do { ACT_DELETE, ACT_PRINT,
                                  ACT_COMPRESS, ACT_APPEND
                                } a_action;
                char *a_string;
                };

static struct action *Action;

/************************** STATIC FUNCTIONS CALLS ***************************/

static void 	docompress(),
		doappend(),
		doprint(),
		queue(),
		usage(),
		quit(),
		copy_file(),
		copy_non_elf_to_temp_ar(),
		copy_elf_file_to_temp_ar_file(),
		sigexit(),
		mcs_exit();

static int	each_file(),
		build_segment_table(),
		location(),
		traverse_file(),
		build_file(),
		getstr(),
		dohash(),	
		process_file();

static char	*get_dir();

static int      signum[] = {SIGHUP, SIGINT, SIGQUIT, 0};

/************************** EXTERN FUNCTIONS CALLS ***************************/

extern void 	exit();

extern int 	getopt(),
		link(),
		unlink(),
		close(),
		write(),
		read(),
		chmod();

extern long	lseek();


/*****************************************************************************/

main(argc, argv)
int	argc;
char	**argv;
{
	extern int 	optind;
	extern char	*optarg;
	int 		c, i;

	for (i = 0; signum[i]; i++)
                if (signal(signum[i], SIG_IGN) != SIG_IGN)
                        (void) signal(signum[i], sigexit);

	
	if ((Action = (struct action *) malloc((unsigned)optbufsz*sizeof(struct action))) == NULL){
		(void) fprintf(stderr,
		"%smcs: malloc memory allocation failure", SGS);
		exit(FAILURE);
		}
	
	while ((c = getopt(argc, argv, "a:cdn:pPV?")) != EOF)
	{
		switch (c) 
		{
		case 'a':
			optcnt++;
			queue(ACT_APPEND, optarg);
			Might_chg++;
			aflag++;
			break;
		case 'c':
			optcnt++;
			queue(ACT_COMPRESS, NULL);
			Might_chg++;
			cflag++;
			break;
		case 'P':
			Pflag++;
			break;
		case 'p':
			optcnt++;
			queue(ACT_PRINT, NULL);
			pflag++;
			break;
		case 'd':
			optcnt++;
			queue(ACT_DELETE, NULL);
			Might_chg++;
			break;
		case 'n':
			Sect_name = optarg;
			break;
		case 'V':
			Vflag++;
			(void) fprintf(stderr, "%smcs: %s %s\n"
				,SGS, ESG_PKG, ESG_REL);
			break;
		case '?':
			errflag++;
			break;
		default:
			break;
		}
	}


	if (errflag)
	{
		usage();
		exit(FAILURE);
	}

	if (argc == optind && (Might_chg || Pflag || pflag || argc == 1))
		usage();
	else if (!Might_chg && !Pflag && !pflag && !Vflag)
		usage();
		
	elf_version(EV_NONE);
	if (elf_version(EV_CURRENT) == EV_NONE)
	{
		(void) fprintf(stderr,
			"%smcs: elf_version() failed - libelf.a out of date.\n"
			,SGS);
		exit(FAILURE);
	}

	if ( pflag || Pflag || Might_chg) 
	{
		for (; optind < argc; optind++)
		{
			fname = argv[optind];
			error_count = error_count + (each_file(argv[optind]));
		}
	}

	mcs_exit(error_count);
	/*NOTREACHED*/
}

static int
each_file(cur_file)
char *cur_file;
{

	char *cur_filenm;
	int code = 0;
	int error = 0, err = 0;
	int update_failure = 0;

	if ( (fd = open(cur_file, O_RDONLY)) == -1)
	{
                (void) fprintf(stderr, "%smcs: %s: Cannot open file\n"
			,SGS, cur_file);
                return(FAILURE); 
        }

	cmd = ELF_C_READ;
        if ( (arf = elf_begin(fd, cmd, (Elf *)0)) == 0)
	{
		(void) fprintf(stderr, 
		"%smcs: libelf error: %s\n",SGS, elf_errmsg(-1) );
                (void) elf_end(arf);
                (void) close(fd);   /* done processing this file */
                return(FAILURE);
	}

        if ((elf_kind(arf) == ELF_K_AR))
	{
                ar_file = 1;
		if (Might_chg)
		{  	/* open ar_temp_file */
                	artmpfile = tempnam(TMPDIR, "mcs2");
                	if (( fdartmp =
			   open(artmpfile, O_WRONLY | O_APPEND | O_CREAT, (mode_t) 0644)) == NULL)
                	{
                        	(void) fprintf(stderr,
                               	 "%smcs: %s: Cannot open temporary file\n",
                               	  SGS, artmpfile);
                        	(void) elf_end(arf);
                        	(void) close(fd);
                        	exit(FAILURE);
                	}
                	/* write magic string to artmpfile */
                	if ((write(fdartmp, ARMAG, SARMAG)) != SARMAG)
			{

				(void) fprintf(stderr,
				"%smcs: %s: write system failure: %s: file not manipulated.\n"
				,SGS, artmpfile, cur_file);
				mcs_exit(FAILURE);
			}
		}
	}
        else
	{
                ar_file = 0;
		cur_filenm = cur_file;
	}

	elftmpfile = tempnam(TMPDIR, "mcs1"); /* holds temporary file;   */
					/* if archive, holds the current  */
					/* member file if it has an ehdr, */
					/* and there were no errors in    */
					/* processing the object file.    */

        while ((elf = elf_begin(fd, cmd, arf)) != 0)
        {
                if (ar_file) /* get header info */
                {
                        if ((mem_header = elf_getarhdr(elf)) == NULL)
                        {
                                (void) fprintf(stderr, 
				"%smcs: %s: malformed archive at %ld\n"
				,SGS, cur_file, elf_getbase(elf) );
				(void) elf_end(elf);
				(void) elf_end(arf);
				(void) close(fd);
				(void) unlink(artmpfile);
				return(FAILURE);
                        }

			if ((cur_filenm = (char *)malloc( (strlen(cur_file) + 3
		  	 +strlen(mem_header->ar_name))* sizeof(char))) == NULL)
			{
				(void) fprintf(stderr,
				"%smcs: malloc memory failure\n", SGS);
				mcs_exit(FAILURE);
			}

			(void) sprintf(cur_filenm, "%s[%s]"
				 ,cur_file, mem_header->ar_name);
                }

		if ((elf_kind(elf) == ELF_K_COFF)){
			(void) fprintf(stderr,
			"%smcs: %s: Warning - internal conversion of COFF file to ELF\n"
			,SGS, cur_filenm);
                        if (elf_update(elf, ELF_C_NULL) == -1) {
                                (void) fprintf(stderr,
                                "%smcs: trouble translating COFF file %s: %s\n"
                                ,SGS,cur_file, elf_errmsg(-1));
				update_failure++;	
                        }
		}
			

		if ( elf_kind(elf) == ELF_K_COFF || elf_kind(elf) == ELF_K_ELF
		     && update_failure == 0 )
                {
			if ( (code = process_file(cur_filenm)) == FAILURE ||
			      code == CODE4)
			{
				if (!ar_file)
				{
					(void) elf_end(arf);
					(void) elf_end(elf);
					(void) close(fd);
					return(FAILURE);	
				}
				else
				{
					copy_non_elf_to_temp_ar(cur_file);
					error++;
				}
					
			}
			else 
			if (ar_file && Might_chg )
			{
				if (code == CODE3)
					 copy_non_elf_to_temp_ar(cur_file); 
				else
					 copy_elf_file_to_temp_ar_file(cur_file); 
			}
                }
                else  /* decide what to do with non-ELF file */
		      /* or a COFF file that failed on a elf_update */
                {
                        if (!ar_file)
                        {
				if (update_failure)
					(void)fprintf(stderr,
			   		"%smcs: ERROR: %s: Cannot manipulate file.\n",SGS, cur_filenm);
				else
					(void)fprintf(stderr, 
			   		"%smcs: %s: invalid file type\n",SGS, cur_filenm);

				(void) close(fd);
				return(FAILURE);
			}
			else
			{
				if (update_failure)
					(void)fprintf(stderr,
					"%smcs: ERROR: %s: Cannot manipulate file.\n",SGS, cur_filenm);

				if (Might_chg)
					copy_non_elf_to_temp_ar(cur_file);
			}
                }
		update_failure = 0;
                cmd = elf_next(elf);
                (void) elf_end(elf);
        }

	err = elf_errno();
	if ( err != 0)
        {
                (void) fprintf(stderr,
                        "%smcs: libelf error: %s\n",SGS, elf_errmsg(err) );
                (void) fprintf(stderr,
                        "%smcs: %s: file not manipulated\n", SGS, cur_file);
                return(FAILURE);
        }


        (void) elf_end(arf);
        (void) close(fd);   /* done processing this file */


	if (ar_file && Might_chg )
        {
                (void) close(fdartmp); /* done writing to ar_temp_file */
                copy_file(fname, artmpfile); /* copy ar_temp_file to FILE */
        }
        else if ( code != CODE3 && Might_chg )
                copy_file(fname,elftmpfile); /* copy temp_file to FILE */

        return(error);

}

static int
process_file(cur_file)
char *cur_file;
{
        int     error = SUCCESS;
	int	x;

        if ( (x = traverse_file(cur_file))  == FAILURE )
        {
		(void) fprintf(stderr, 
		"%smcs: WARNING: %s: Cannot manipulate file.\n"
		,SGS, cur_file);
                error = FAILURE;
        }
        else 
	if (x != CODE4 && x != CODE3 && x != FAILURE )
	{
		if ( build_file() == FAILURE)
        	{
               	 	(void) fprintf(stderr,
			"%smcs: WARNING: %s: Cannot manipulate file.\n"
			,SGS, cur_file);
			error = FAILURE;
        	}
	}

	if ( x == CODE3)
		return(CODE3);
	else
	{
		free(off_table);
		free(sec_table);
		free(nobits_table);
		if ( x == CODE4)
			return(CODE4);
		else
			return(error);
	}
}

static void
initialize()
{

        Sect_index      = 0;    /* Section index of specified section */
        Sect_exists     = 0;    /* used to decide if a new section
                                   must be created */
        NOBITS_Sect     = 0;
        string_size     = 0; 	/* current size of the section's contents */
        original_size   = 0;	/* original size of the section's contents */
        temp_string_size= 0;
        string          = NULL; /* current section contents */
        temp_string     = NULL;

        sec_loc         = 0;	/* IN, AFTER, or PRIOR to a segment */
        rel_loc         = 0;	/* IN, AFTER, or PRIOR to a segment */
        rel_scn_index   = 0;	/* index of section's relocation section */

        if ((sec_table = (int *) calloc(ehdr->e_shnum, sizeof(int))) == NULL)
        {
                (void) fprintf(stderr,
                        "%smcs: calloc memory allocation failure\n"
                        ,SGS);
                exit(FAILURE);
        }

        if ((off_table = (Elf32_Off *) calloc(ehdr->e_shnum, sizeof(Elf32_Off))) == NULL)
        {
                (void) fprintf(stderr,
                        "%smcs: calloc memory allocation failure\n"
                        ,SGS);
                exit(FAILURE);
        }

	if ((nobits_table = (int *) calloc(ehdr->e_shnum, sizeof(int))) == NULL)        {
                (void) fprintf(stderr,
                        "%smcs: calloc memory allocation failure\n"
                        ,SGS);
                exit(FAILURE);
        }
}

/**********************************************************************/
/* Search through PHT saving the beginning and ending segment offsets */
/**********************************************************************/

static int
build_segment_table(cur_file)
char *cur_file;
{
	unsigned int	i,j;
	Elf32_Phdr      *phdr1;

	if ((b_e_seg_table = (Elf32_Off *) calloc(ehdr->e_phnum*2, sizeof(Elf32_Off))) == NULL)
       	{
		(void) fprintf(stderr,
		"%smcs: calloc memory allocation failure\n" ,SGS);
		mcs_exit(FAILURE);
	}

	phdr1 = phdr;
	for (i=1, j=0; i<=ehdr->e_phnum; i++, phdr1++ )
	{
		b_e_seg_table[j] = phdr1->p_offset;
		b_e_seg_table[j+1] = phdr1->p_offset + phdr1->p_memsz;
		j += 2;
	}

#if DEBUG
	for (i=1, j=0; i<=ehdr->e_phnum; i++, j+=2 )
	(void) printf("DEBUG: b_e_seg_table[%d] is %x\t b_e_seg_table[%d] is %x\n", j,b_e_seg_table[j], j+1, b_e_seg_table[j+1]);
#endif

	scn = 0;
	while ((scn = elf_nextscn(elf, scn)) != 0)
	{
		if ((shdr = elf32_getshdr(scn)) == 0)
		{
			(void) fprintf(stderr,
			"%smcs: %s: no section header table.\n"
			,SGS, cur_file);
			return(FAILURE);
		}

		if (shdr->sh_type == SHT_NOBITS)
		{
			for ( i=1,j=0; i<=(int)ehdr->e_phnum; i++, j+=2)
			{
				if (  shdr->sh_offset + shdr->sh_size >=
				      b_e_seg_table[j] &&
				      shdr->sh_offset + shdr->sh_size <=
				      b_e_seg_table[j+1] )
					b_e_seg_table[j+1] -= shdr->sh_size;
			}
		}
	}
#if DEBUG
	(void) printf("\n");
	for (i=1, j=0; i<=ehdr->e_phnum; i++, j+=2 )
	(void) printf("DEBUG: b_e_seg_table[%d] is %x\t b_e_seg_table[%d] is %x\n", j,b_e_seg_table[j], j+1, b_e_seg_table[j+1]);
#endif

	return(SUCCESS);
}

static int
traverse_file(cur_file)
char 	*cur_file;
{
	Elf_Data	*p_data;
	Elf_Scn         *temp_scn;
        Elf32_Shdr      *temp_shdr;

	int 		act_index,
			x,
			p_size = 0,
			error = 0;

	unsigned int 	ndx, i,
			scn_index = 1;
	char		*rel_name, *temp_name, *p_string, c;

	if ((ehdr = elf32_getehdr(elf)) != 0)
		initialize();
	else
	{
		(void) fprintf(stderr,
		"%smcs: libelf error: %s\n", SGS, elf_errmsg(-1) );
		return(FAILURE);
	}

	if ((phdr = elf32_getphdr(elf)) != NULL)
	{
		if ( build_segment_table(cur_file) == FAILURE)
		return(FAILURE);
	}

	ndx = ehdr->e_shstrndx; /* used to find section's name */
	scn = 0;
	while ((scn = elf_nextscn(elf, scn)) != 0)
	{
                char *name = "";

                if ((shdr = elf32_getshdr(scn)) == 0)
		{
			(void) fprintf(stderr,
				"%smcs: %s: no section header table.\n"
				,SGS, cur_file);
			return(FAILURE);
		}
		else
			name = elf_strptr(elf, ndx, (size_t)shdr->sh_name);


		if (Pflag)
		{
			if (    (phdr!=NULL && 
				(shdr->sh_type != SHT_NOBITS) &&
			    	(location(shdr->sh_offset+shdr->sh_size) == IN))
			     ||
				(phdr==NULL && (strcmp(name,".text")==0 ||
					        strcmp(name,".data")==0 ) )  
			   )
			{

				p_data = 0;
				if ((p_data=elf_rawdata(scn,p_data)) == NULL)
				{
				 	(void) fprintf(stderr,
					"%smcs: libelf error: %s\n"
					,SGS, elf_errmsg(-1));
					return(FAILURE);
				}
				p_string = (char *)p_data->d_buf;
				p_size   = p_data->d_size;
				while (p_size--){
				 	c = *p_string++;
					if (c == '\0')
						(void) printf("\n");
					else
						(void) printf("%c",c);
				}
			}
		}

		if ( shdr->sh_type == SHT_REL )
		{
			if (shdr->sh_info != SHN_UNDEF &&
			(temp_scn = elf_getscn(elf, shdr->sh_info)) != 0)
			{
				if((temp_shdr = elf32_getshdr(temp_scn)) != 0)
				{
					temp_name = elf_strptr(elf, ndx, (size_t)temp_shdr->sh_name);

					if (strcmp(temp_name, Sect_name) == 0 )
					{
					  rel_name = elf_strptr(elf, ndx, (size_t)shdr->sh_name);
					  rel_scn_index = scn_index;
					  if (temp_shdr->sh_type == SHT_NOBITS)
						 rel_loc = location(shdr->sh_offset);
					  else
						 rel_loc = location( shdr->sh_offset + shdr->sh_size);
					}	
				}
			}

		}

		if ( (strcmp(name,Sect_name) == 0) && Sect_index == 0 )
		{
			if (shdr->sh_type == SHT_NOBITS )
			{
				NOBITS_Sect = 1;
				sec_loc = location(shdr->sh_offset);
			}
			else
			    sec_loc = location(shdr->sh_offset + shdr->sh_size);

			if (Sect_index == 0)
			{
				Sect_index = scn_index; /* corresponds to the
					first section that met the conditions */
				Sect_exists = 1;
				data = 0;
				if ((data = elf_rawdata(scn,data)) == NULL)
				{
                                        (void) fprintf(stderr,               
                                        "%smcs: libelf error: %s\n",
                                        SGS, elf_errmsg(-1));
                                        return(FAILURE);
                                }
				string = (char *)data->d_buf;
				string_size = data->d_size;
				original_size = data->d_size;
			}
				
		}

		sec_table[scn_index] = scn_index;
		if (shdr->sh_type == SHT_NOBITS)
			x = location(shdr->sh_offset);
		else
			x = location(shdr->sh_offset + shdr->sh_size);
		if (x==IN || x==PRIOR)
			off_table[scn_index] = shdr->sh_offset;
		if (shdr->sh_type == SHT_NOBITS)
			nobits_table[scn_index] = 1;

		scn_index++;
	}

#if DEBUG
for (i=0; i<=ehdr->e_shnum-1; i++)
        (void) printf("DEBUG: sec_table[%d] = %d   off_table[%d] = %x  nobits_table[%d] = %d\n", i,sec_table[i],i,off_table[i],i,nobits_table[i]);
#endif


	for (act_index=0; act_index < actmax; act_index++)
	{
		switch(Action[act_index].a_action)
		{
		case ACT_PRINT:
					if (!string_size)
						break;
					if ( NOBITS_Sect)
					{
						(void) fprintf(stderr, 
		"%smcs: %s: Cannot print contents of a NOBITS section (%s)\n"
						,SGS, cur_file, Sect_name);
						break;
					}
					
					doprint(cur_file);
					break;
		case ACT_DELETE:
					if ((elf_kind(elf) == ELF_K_COFF))
					{
						error = CODE4;
						(void) fprintf(stderr, 
			"%smcs: %s: Warning: Cannot delete section (%s)\n" 
						,SGS, cur_file, Sect_name);
						(void) fprintf(stderr,
					       "\t\tfrom a COFF object file\n");
						break;
					}
					else
					if (sec_loc == IN)
					{
						error = CODE4;
						(void) fprintf(stderr,
			"%smcs: %s: Warning: Cannot delete section (%s)\n"
						,SGS, cur_file, Sect_name);
						(void) fprintf(stderr,
						"\t\tfrom a segment\n");
						break;
					}
					else
					if (rel_loc == IN)
					{
						error = CODE4;
						(void) fprintf(stderr,
			"%smcs: %s: Warning: Cannot delete section (%s)\n"
						,SGS, cur_file, Sect_name);
						(void) fprintf(stderr,
		"\t\tbecause its relocation section (%s) is in a segment\n"
						,rel_name);
						break;
					}
					else
					if (!string_size)
						break;
					
					string_size = 0;
					string = NULL;
					break;

		case ACT_APPEND:
					if ((elf_kind(elf) == ELF_K_COFF))
                                        {
						error = CODE4;
                                                (void) fprintf(stderr,
			"%smcs: %s: Warning: Cannot append to section (%s)\n"
						,SGS, cur_file, Sect_name);
						(void) fprintf(stderr,
						"\t\tin a COFF object file\n");
                                                break;
                                        }
					else
					if ( NOBITS_Sect )
                                        {
                                                error = CODE4;
                                                (void) fprintf(stderr,
                        "%smcs: %s: Cannot append to a NOBITS section (%s)\n"
                                                ,SGS, cur_file, Sect_name);
                                        	break;
					}
					else 
                                        if (sec_loc == IN)
                                        {
                                                error = CODE4;
                                                (void) fprintf(stderr,
			"%smcs: %s: Warning: Cannot append to section (%s)\n"
						,SGS, cur_file, Sect_name);
                                                (void) fprintf(stderr,
						"\t\tin a segment\n");
                                                break;
                                        }

					doappend(Action[act_index].a_string);
					break;

		case ACT_COMPRESS:
					if ((elf_kind(elf) == ELF_K_COFF))
                                        {
						error = CODE4;
                                                (void) fprintf(stderr,
				"%smcs: %s: Cannot compress section (%s)\n"
						,SGS, cur_file, Sect_name);
						(void) fprintf(stderr,
						"\t\tin a COFF object file\n");
						 break;
                                        }
					else
					if ( NOBITS_Sect) 
					{
						error = CODE4;
						(void) fprintf(stderr,
			"%smcs: % s: Cannot compress a NOBITS section (%s)\n"
						,SGS, cur_file, Sect_name);
						 break;	
					}
					else
                                        if (sec_loc == IN)
                                        {
                                                error = CODE4;
                                                (void) fprintf(stderr,
                        "%smcs: %s: Warning: Cannot compress a section (%s)\n"
                                                ,SGS, cur_file, Sect_name);
                                                (void) fprintf(stderr,
                                                "\t\tin a segment\n");
                                                break;
                                        }

					if (!string_size )
						break;
					temp_string = string;
					temp_string_size = string_size;
					docompress();
					string = strings;
					string_size = next_str;
					break;
		}
	}

	if ( (Pflag || pflag) && (!Might_chg) )
		return(CODE3); /* don't bother creating a new file */
				/* since the file has not changed */

	if ( (Sect_exists == 0 && string_size == 0 && error == 0) ||
	   (!Might_chg) )
		return(CODE3); /* don't bother creating a new file */
			   /* since the string is NULL and the */
			   /* section does not exists */

	
	if (Sect_exists && error == 0)
	{
		if (string_size == 0) /* deleting a section */
		{
			for (i=Sect_index+1; i<ehdr->e_shnum; i++)
				sec_table[i] = sec_table[i] - 1;
			if (rel_scn_index != 0)
			{
				for (i=rel_scn_index+1; i<ehdr->e_shnum; i++)
                                sec_table[i] = sec_table[i] - 1;
				sec_table[rel_scn_index] = 0;
			}
			sec_table[Sect_index] = 0;
		}
	
		if (sec_loc == PRIOR && (string_size > original_size) )
			/* moving section; make it the last section */ 
		{
			sec_table[Sect_index] = ehdr->e_shnum - 1;
			for (i=Sect_index+1; i<ehdr->e_shnum; i++)
       	                 sec_table[i] = sec_table[i] - 1;
		}
	}

#if DEBUG
for (i=0; i<=ehdr->e_shnum-1; i++)
        (void) printf("DEBUG: sec_table[%d] = %d   off_table[%d] = %x  nobits_table[%d] = %d\n", i,sec_table[i],i,off_table[i],i,nobits_table[i]);
#endif
	
	return(error);

}

static void
docompress()
{

        int     hash;
        int     i;

        if (hash_key == NULL) {
                hash_key = (int *)malloc((unsigned)sizeof(int)*200);
                hash_end = 200;
                hash_str = (int *)malloc((unsigned)sizeof(int)*200);
                str_size = 10000;
                strings = (char *)malloc((unsigned)str_size);

		if ( hash_key == NULL || hash_str == NULL || strings == NULL)
		{
			(void) fprintf(stderr,
			"%smcs: malloc memory allocation failure\n",SGS);
                        mcs_exit(FAILURE);
		}
        }

        hash_num = 0;
        next_str = 0;

        while (temp_string_size > 0) {
                int     pos;
                pos = getstr();

                temp_string_size -= (next_str - pos);

                hash = dohash(pos + strings);

                for (i=0; i < hash_num; i++) {
			if (hash != hash_key[i])
                                continue;
                        if (strcmp(pos + strings, hash_str[i] + strings) == 0)
                                break;
                }
                if (i != hash_num) {
                        next_str = pos;
                        continue;
                }
                if (hash_num == hash_end) 
		{
                        hash_end *= 2;
                        hash_key = (int *) realloc((char *)hash_key,
                                (unsigned) hash_end * sizeof(int));
                        hash_str = (int *) realloc((char *)hash_str,
                                (unsigned) hash_end * sizeof(int));
			if (hash_key == NULL || hash_str == NULL)
			{
				(void) fprintf(stderr,
				"%smcs: realloc memory allocation failure\n"
				,SGS);
				mcs_exit(FAILURE);
			}
                }
                hash_key[hash_num] = hash;
                hash_str[hash_num++] = pos;
        }
        return;
}


static int
getstr()
{
        register char c;
        int     start;

        start = next_str;

        while ((c = *temp_string++) != '\0'  && (temp_string_size - (next_str - start) ) != 0 ) 
	{
                if (next_str >= str_size)
		{
                        str_size *= 2;
                        if ((strings = (char *)realloc(strings, (unsigned) str_size)) == NULL)
			{
				(void) fprintf(stderr,
				"%smcs: realloc memory allocation failure\n"
				,SGS);
				mcs_exit(FAILURE);
			}
                }
                strings[next_str++] = c;
        }

        if (next_str >= str_size) 
	{
                str_size *= 2;
                if ((strings = (char *)realloc(strings, (unsigned) str_size)) == NULL)
		{
			(void) fprintf(stderr,
			"%smcs: realloc memory allocation failure\n"
			,SGS);
			mcs_exit(FAILURE);
		}
        }

        strings[next_str++] = NULL;
        return(start);
}

#define HALFLONG 16
#define low(x)  (x&((1L<<HALFLONG)-1))
#define high(x) (x>>HALFLONG)

/*
 * hashing has the effect of arranging line in 7-bit bytes and then
 * summing 1-s complement in 16-bit hunks
 */

static int
dohash(str)
char    *str;
{
        long    sum;
        register unsigned       shift;
        register t;
        sum = 1;
        for (shift=0; (t = *str++) != NULL; shift += 7) {
                sum += (long)t << (shift %= HALFLONG);
        }
        sum = low(sum) + high(sum);
        return((short)low(sum) + (short)high(sum));
}


static void
doappend(a_string)
char *a_string;
{
	char	*p;

	if (string_size  == 0)
	{
		if ( (string = (char *) malloc( strlen(a_string) + 2
				 * sizeof(char *) )) == NULL)
		{
			(void) fprintf(stderr,
			"%smcs: malloc memory allocation failure\n"
			,SGS);
			mcs_exit(FAILURE);
		}

		/* ABI: String Tables must begin with a NULL */
		string[0] = '\0';
		(void) memcpy(&string[1], a_string, strlen(a_string)+1 );
		string_size = strlen(a_string) + 2;
	}
	else
	{
		if ((p = (char *) malloc(strlen(a_string)+1+ string_size)) == NULL )
		{
			(void) fprintf(stderr,
			"%smcs: malloc memory allocation failure\n" ,SGS);
			mcs_exit(FAILURE);
		}
		(void) memcpy(p, string, string_size);
		string = p;

		(void) memcpy(&string[string_size], a_string, strlen(a_string) + 1);
		string_size += strlen(a_string) + 1;
	}
	return;
}

static void
doprint(cur_file)
char *cur_file;
{
	size_t	temp_size;
	char	*temp_string;

	(void) fprintf(stdout,"%s:\n", cur_file);

	temp_size = string_size;
	temp_string = string;

	while (temp_size-- ) {
		char c = *temp_string++;
		switch(c) {
                                case '\0':      (void)printf("\n"); break;
                                default:        (void)printf("%c",c);break;
                                }
	}
	(void)printf("\n");

	return;
}

static void
queue(activity, string)
enum what_to_do activity;
char *string;
{
	if(optcnt > optbufsz)
	{
		optbufsz = optbufsz * 2;
		if ((Action = (struct action *) realloc( ( struct action *)Action,(unsigned)optbufsz*sizeof(struct action))) == NULL)
		{
			(void) fprintf(stderr,
			"%smcs: realloc memory allocation failure", SGS);
			mcs_exit(FAILURE);
		}
	}

	Action[actmax].a_action = activity;
	Action[actmax].a_string = string;
	actmax++;

	return;
}

static int
location(offset)
Elf32_Off offset;
{
        int i,j;

        if (phdr == NULL)
        {
                return(NOSEG);
        }
        for ( i=1,j=0; i<=(int)ehdr->e_phnum; i++)
        {
                if (offset >= b_e_seg_table[j] && offset <= b_e_seg_table[j+1])
                {
                        return(IN);
                }
                else if (offset<b_e_seg_table[j])
                {
                        return(PRIOR);
                }
                j += 2;
        }

        return(AFTER);
}

static int
build_file()
{

	int             scn_no, x;
        unsigned int    no_of_symbols = 0;
        Elf32_Sym       *p, *q;
        unsigned int    c = 0;
        Elf32_Off       new_offset = 0, r;
	Elf32_Word	new_sh_name= 0;  /* to hold the offset for the new 
					 section's name */


        if ((fdtmp = open(elftmpfile, O_RDWR | O_TRUNC | O_CREAT, (mode_t) 0644)) == -1){
                (void) fprintf(stderr,
			"%smcs: %s: can't open temporary file\n"
			,SGS, elftmpfile);
                return(FAILURE);
        }

        if ((elffile = elf_begin(fdtmp, ELF_C_WRITE, (Elf *) 0)) == NULL)
        {
                (void) fprintf(stderr,
                        "%smcs: %s: trouble reading file\n"
                        ,SGS, elftmpfile);
                (void) close(fdtmp);
                return(FAILURE);
        }


        if ((elf_ehdr = elf32_newehdr(elffile)) == NULL)
	{
		(void) fprintf(stderr,"%smcs: libelf error: %s\n",
		SGS, elf_errmsg(-1));
		return(FAILURE);
	}	
        (void) memcpy(elf_ehdr, ehdr, sizeof(Elf32_Ehdr) );


	if ( phdr != NULL)
        {
                elf_flagelf(elffile, ELF_C_SET, ELF_F_LAYOUT); 

                if ((elf_phdr=elf32_newphdr(elffile, ehdr->e_phnum)) == NULL)
		{
			(void) fprintf(stderr,"%smcs: libelf error: %s\n",
			SGS, elf_errmsg(-1));
			return(FAILURE);
		}
                (void) memcpy(elf_phdr, phdr, ehdr->e_phentsize * ehdr->e_phnum);

                x = location(elf_ehdr->e_phoff);
                if ( x == AFTER)
                        new_offset = (Elf32_Off) ehdr->e_ehsize;
        }


	scn = 0;
        scn_no = 1;
        while ((scn = elf_nextscn(elf, scn)) != 0)
        {
                /*  If section should be copied to new file NOW */
                if ( sec_table[scn_no] != 0  &&  sec_table[scn_no] <= scn_no)
                {
                        if ((shdr = elf32_getshdr(scn)) == NULL)
			{
			   (void) fprintf(stderr,"%smcs: libelf error: %s\n",
				SGS, elf_errmsg(-1));
				return(FAILURE);
			}	

                        if ((elf_scn = elf_newscn(elffile)) == NULL)
			{
			  (void) fprintf(stderr,"%smcs: libelf error: %s\n",
				SGS, elf_errmsg(-1));
	                        return(FAILURE);
			}
                        if ((elf_shdr= elf32_getshdr(elf_scn)) == NULL)
			{
			  (void) fprintf(stderr,"%smcs: libelf error: %s\n",
                                SGS, elf_errmsg(-1));
                                return(FAILURE);
			}
                        (void) memcpy(elf_shdr, shdr, sizeof(Elf32_Shdr));

				/* update link and info fields */
                        if ( shdr->sh_type == SHT_HASH || 
			     shdr->sh_type == SHT_SYMTAB)
			{
				if (sec_table[shdr->sh_link] < 0)
					elf_shdr->sh_link = 0;
				else
					elf_shdr->sh_link = (Elf32_Word)sec_table[shdr->sh_link];
			}

			else if ( shdr->sh_type == SHT_REL )
			{
				if (sec_table[shdr->sh_link] <0)
                                        elf_shdr->sh_link = 0;
				else
				elf_shdr->sh_link = (Elf32_Word) sec_table[shdr->sh_link];
				if (sec_table[shdr->sh_info] < 0)
					elf_shdr->sh_info = 0;
				else
					elf_shdr->sh_info = (Elf32_Word) sec_table[shdr->sh_info];
			}
		
                        data = 0;
                        if ((data = elf_getdata(scn, data)) == NULL)
			{
			    (void) fprintf(stderr,"%smcs: libelf error: %s\n"
                                ,SGS, elf_errmsg(-1));
                                return(FAILURE);
			}
                        if ((elf_data = elf_newdata(elf_scn) ) == NULL)
			{
			    (void) fprintf(stderr,"%smcs: libelf error: %s\n"
                                ,SGS, elf_errmsg(-1));
                                return(FAILURE);
			}

			/*IF SECTION MOVED OR REMOVED, UPDATE SYMBOL TABLE */
			if ( Sect_index != scn_no &&
				shdr->sh_type == SHT_SYMTAB && 
				shdr->sh_entsize > 0  && 
				((Sect_exists && string_size == 0 ) 
				|| (sec_loc == PRIOR && (string_size > 
							original_size) )) )
                        {
                                no_of_symbols = shdr->sh_size/shdr->sh_entsize;
                                p = (Elf32_Sym *)data->d_buf;
                                q = p;

                                for ( c = 0; c < no_of_symbols; c++, p++)
                                {
                                  if (p->st_shndx <= ehdr->e_shnum &&
                                      p->st_shndx  > 0 )
                                  {
                                        if (sec_table[p->st_shndx] != 0 )
					   p->st_shndx = sec_table[p->st_shndx];
					else{
                                                p->st_name = 0;
                                                p->st_value = 0;
                                                p->st_size = 0;
                                                p->st_info = 0;
                                                p->st_other = 0;
                                                p->st_shndx = 0;
                                        }
                                  }
                                }

                                data->d_buf = (VOID *)q;
                        }

			*elf_data = *data;
	
			if ( Sect_index == scn_no )
			{
				elf_shdr->sh_size = string_size;
				elf_data->d_size = string_size;

				if (string_size > original_size)
				{ 
                			if (( elf_data->d_buf = (char*)malloc( string_size * sizeof(char)) ) == NULL)
						{
                        				(void) fprintf(stderr, "%smcs: malloc memory allocation failure\n", SGS);
							mcs_exit(FAILURE);
                				}
				}
				elf_data->d_buf = string;
			}
			else	/* add new section name to shstrtab? */
			if( !Sect_exists && string_size &&                      
                                scn_no == ehdr->e_shstrndx &&
                                elf_shdr->sh_type == SHT_STRTAB &&
                                ((x=location(elf_shdr->sh_offset
							+elf_shdr->sh_size))
                                 != IN || x != PRIOR) )
			{
				if ((elf_data->d_buf = (char*)malloc( (elf_shdr->sh_size + strlen(Sect_name) + 1) )) == NULL)
                                {
                                        (void) fprintf(stderr, "%smcs: malloc memory allocation failure\n", SGS);
                                        mcs_exit(FAILURE);
                                } /*put original data plus new data in section*/
				(void) memcpy(elf_data->d_buf, data->d_buf, data->d_size);
				(void) memcpy(&((char *)elf_data->d_buf)[data->d_size], Sect_name, strlen(Sect_name) + 1);
				new_sh_name = elf_shdr->sh_size;
                                elf_shdr->sh_size += strlen(Sect_name) +1;
				elf_data->d_size += strlen(Sect_name) +1;

			}

                        if (phdr != NULL) /* compute offsets */
                        {

                         if (off_table[scn_no] == 0) /*compute section offset*/
                         {
                                r = new_offset % elf_shdr->sh_addralign;
                                if (r)
                                   new_offset += elf_shdr->sh_addralign - r;
                                elf_shdr->sh_offset = new_offset;
                                elf_data->d_off     = 0;

                         if (nobits_table[scn_no] == 0)
                                new_offset += elf_shdr->sh_size;
                         }
                         else
                                new_offset = off_table[scn_no];

                        }

                }
                scn_no++;
        }

	/* IF NEW SECTION */

	if( !Sect_exists && string_size)
        {
                if((elf_scn = elf_newscn(elffile)) == NULL)
		{
			(void) fprintf(stderr,"%smcs: libelf error: %s\n",
			SGS, elf_errmsg(-1));
			return(FAILURE);
		}

                if((elf_shdr= elf32_getshdr(elf_scn)) == NULL)
		{
			(void) fprintf(stderr,"%smcs: libelf error: %s\n",
                        SGS, elf_errmsg(-1));
                        return(FAILURE);
		}

		elf_shdr->sh_name = new_sh_name;
		elf_shdr->sh_type = SHT_PROGBITS;
		elf_shdr->sh_flags = 0;
                elf_shdr->sh_addr = 0;
		if (phdr != NULL)
			elf_shdr->sh_offset = new_offset;
		else
			elf_shdr->sh_offset = 0;
                elf_shdr->sh_size = string_size;
                elf_shdr->sh_link = 0;
                elf_shdr->sh_info = 0;
		elf_shdr->sh_addralign = 1;
                elf_shdr->sh_entsize = 0;
		
		if ((elf_data=elf_newdata(elf_scn)) == NULL)
		{
			(void) fprintf(stderr,"%smcs: libelf error: %s\n",
                        SGS, elf_errmsg(-1));
                        return(FAILURE);
		}
		elf_data->d_size = string_size;
		if (( elf_data->d_buf = (char*)malloc( string_size * sizeof(char)) ) == NULL)
        	{
                	(void) fprintf(stderr, "%smcs: malloc memory allocation failure\n", SGS);
                	mcs_exit(FAILURE);
        	}
		(void) memcpy(&((char *)elf_data->d_buf)[0], string, string_size);	
		elf_data->d_align = 1;
		new_offset += string_size;
        }

	/* IF MOVING SECTION TO END */
	else	
	if ( phdr!=NULL && sec_loc == PRIOR && string_size > original_size )
	{

		if ((scn = elf_getscn(elf, Sect_index)) == NULL)
		{
			(void) fprintf(stderr,"%smcs: libelf error: %s\n",
                        SGS, elf_errmsg(-1));
                        return(FAILURE);
		}
		if ((shdr = elf32_getshdr(scn)) == NULL)
		{
			(void) fprintf(stderr,"%smcs: libelf error: %s\n",
                        SGS, elf_errmsg(-1));
                        return(FAILURE);
		}
		if ((elf_scn = elf_newscn(elffile)) == NULL)
		{
			(void) fprintf(stderr,"%smcs: libelf error: %s\n",
                        SGS, elf_errmsg(-1));
                        return(FAILURE);
		}
		if ((elf_shdr= elf32_getshdr(elf_scn)) == NULL)
		{
                        (void) fprintf(stderr,"%smcs: libelf error: %s\n",
                        SGS, elf_errmsg(-1));
                        return(FAILURE);
                }
		(void) memcpy(elf_shdr, shdr, sizeof(Elf32_Shdr));
		elf_shdr->sh_offset = new_offset;  /* UPDATE fields */
		elf_shdr->sh_size = string_size;
		elf_shdr->sh_link = (Elf32_Word) sec_table[shdr->sh_link];
		elf_shdr->sh_info = (Elf32_Word) sec_table[shdr->sh_info];

		data = 0;
		if ((data = elf_getdata(scn, data)) == NULL)
		{
                        (void) fprintf(stderr,"%smcs: libelf error: %s\n",
                        SGS, elf_errmsg(-1));
                        return(FAILURE);
                }
		if (( elf_data=elf_newdata(elf_scn)) == NULL)
		{
                        (void) fprintf(stderr,"%smcs: libelf error: %s\n",
                        SGS, elf_errmsg(-1));
                        return(FAILURE);
                }
		(void) memcpy(elf_data, data, sizeof(Elf_Data));
                elf_data->d_size = string_size; /* UPDATE fields */
                if (( elf_data->d_buf = (char*)malloc( string_size * sizeof(char)) ) == NULL)
                {
                        (void) fprintf(stderr, 
			"%smcs: malloc memory allocation failure\n", SGS);
                        mcs_exit(FAILURE);
                }
                (void) memcpy(&((char *)elf_data->d_buf)[0], string, string_size);
	
		new_offset += string_size;
	}
		
	/* In the event that the position of the sting table has changed, */
        /* as a result of deleted sections, update the ehdr->e_shstrndx.  */

	if ( elf_ehdr->e_shstrndx > 0 && elf_ehdr->e_shnum > 0 && 
	     sec_table[elf_ehdr->e_shstrndx] < (int)  elf_ehdr->e_shnum ) 
         	elf_ehdr->e_shstrndx = 
			(Elf32_Half) sec_table[elf_ehdr->e_shstrndx];

        if (phdr!=NULL)
        {
			/* UPDATE location of program header table */
                if (location(elf_ehdr->e_phoff) == AFTER)
                {
                        r = new_offset % 4;
                        if (r)
                                new_offset += 4 - r;

                        elf_ehdr->e_phoff = new_offset;
                        new_offset += elf_ehdr->e_phnum*elf_ehdr->e_phentsize;

                }
			/* UPDATE location of section header table */
                if  ( (location(elf_ehdr->e_shoff) == AFTER) ||
			( (location(elf_ehdr->e_shoff) == PRIOR) &&
			  (!Sect_exists && string_size) ) 
		    )
                {
                        r = new_offset % 4;
                        if (r)
                                new_offset += 4 - r;

                        elf_ehdr->e_shoff = new_offset;
                }
		free(b_e_seg_table);
        }

	if (elf_update(elffile, ELF_C_WRITE) < 0)
        {
		(void) fprintf(stderr,"%smcs: libelf error: %s\n",
		SGS, elf_errmsg(-1));
        	return(FAILURE);
        }

        (void) elf_end(elffile);
        (void) close(fdtmp);

        return (SUCCESS);
}


static void
copy_elf_file_to_temp_ar_file(cur_file)
char *cur_file;
{

        char *buf;
        char    mem_header_buf[sizeof( struct ar_hdr ) + 1];

        if ((fdtmp3 = open(elftmpfile, O_RDONLY)) == -1)
        {
                (void) fprintf(stderr,"%smcs: %s: can't open temporary file\n"
			,SGS, elftmpfile);
                mcs_exit(FAILURE);
        }

        (void) stat(elftmpfile, &stbuf); /* for size of file */

        if (( buf = (char*)malloc(ROUNDUP(stbuf.st_size) * sizeof(char)) ) == NULL)
        {
                (void) fprintf(stderr, "%smcs: malloc memory allocation failure\n", SGS);
                mcs_exit(FAILURE);
        }

        if (read(fdtmp3, buf, (unsigned)stbuf.st_size) != (unsigned)stbuf.st_size )
	{
		(void) fprintf(stderr, 
		"%smcs: %s: read system failure: %s: file not manipulated.\n"
		,SGS, elftmpfile,cur_file);
		mcs_exit(FAILURE);
	}

        (void) sprintf( mem_header_buf, FORMAT, mem_header->ar_rawname, mem_header->ar_date, (unsigned)mem_header->ar_uid, (unsigned)mem_header->ar_gid, (unsigned)mem_header->ar_mode, stbuf.st_size, ARFMAG );

        if (write(fdartmp, mem_header_buf, (unsigned)sizeof( struct ar_hdr))
	   != (unsigned)sizeof( struct ar_hdr) )
	{
		(void) fprintf(stderr, 
		"%smcs: %s: write system failure: %s: file not manipulated.\n"
                ,SGS, elftmpfile, cur_file);
                mcs_exit(FAILURE);
        }

        if (stbuf.st_size & 0x1)
        {
                buf[stbuf.st_size] = '\n';
                if (write(fdartmp, buf, (unsigned)ROUNDUP(stbuf.st_size) )
		 != (unsigned)ROUNDUP(stbuf.st_size) )
		{
		   (void) fprintf(stderr, 
		"%smcs: %s: write system failure: %s: file not manipulated.\n"
			,SGS, elftmpfile, cur_file);
			mcs_exit(FAILURE);
		}
        }
        else
		if (write(fdartmp, buf, (unsigned)stbuf.st_size) 
		    != (unsigned)stbuf.st_size)
		{
		   (void) fprintf(stderr, 
		   "%smcs: %s: write system failure: %s: file not manipulated\n"
                        ,SGS, elftmpfile,cur_file);
                   mcs_exit(FAILURE);
		}

        free(buf);
        (void) close(fdtmp3);
        return;
}


static void
copy_non_elf_to_temp_ar(cur_file)
char *cur_file;
{

        char    mem_header_buf[sizeof( struct ar_hdr ) + 1];


        if ( strcmp(mem_header->ar_name,"/") != 0 )
        {

                (void) sprintf( mem_header_buf, FORMAT, mem_header->ar_rawname, mem_header->ar_date, (unsigned)mem_header->ar_uid, (unsigned)mem_header->ar_gid, (unsigned)mem_header->ar_mode, mem_header->ar_size, ARFMAG );

                if (write(fdartmp, mem_header_buf, sizeof( struct ar_hdr))
		    != sizeof( struct ar_hdr) )
		{
		  	(void) fprintf(stderr,
		      "%smcs: write system failure: %s: file not manipulated.\n"
			,SGS,cur_file);
                        mcs_exit(FAILURE);
		}
                if ((file_buf = (char *)malloc( ROUNDUP(mem_header->ar_size))) == NULL)
                {
                        (void) fprintf(stderr, 
			"%smcs: malloc memory allocation failure\n", SGS);
                        mcs_exit(FAILURE);
                }

                if (lseek(fd, elf_getbase(elf), 0) != elf_getbase(elf))
                {
                        (void) fprintf(stderr, 
		      	"%smcs: lseek system failure: %s: file not manipulated\n"
			, SGS, cur_file);
                        mcs_exit(FAILURE);
                }

                if (read(fd, file_buf, (unsigned) ROUNDUP(mem_header->ar_size) ) 
			!= (unsigned) ROUNDUP(mem_header->ar_size))
                {
                        (void) fprintf(stderr, 
			"%smcs: read system failure: %s: file not manipulated\n"
			,SGS, cur_file);
                        mcs_exit(FAILURE);
                }
                if (write(fdartmp, file_buf, (unsigned)ROUNDUP(mem_header->ar_size) ) != (unsigned)ROUNDUP(mem_header->ar_size))
		{
			(void) fprintf(stderr,
			"%smcs: write system failure: %s: file not manipulated\n"
			, SGS,cur_file);
                        mcs_exit(FAILURE);
                }

                free(file_buf);
        }
	else
	if (Might_chg)
	{
                (void) fprintf(stderr,
		"%smcs: WARNING: %s: symbol table deleted from archive \n"
		,SGS, fname);
                (void) fprintf(stderr,
               	"execute  `ar -ts %s` to restore symbol table.\n"
		,fname);
	}
}

static void
copy_file(fname, temp_file_name)
char *fname;
char *temp_file_name;
{
	register int i;
	char *temp_fname;

	for (i = 0; signum[i]; i++) /* started writing, cannot interrupt */
                (void) signal(signum[i], SIG_IGN);

        (void) stat(fname, &stbuf); /* for mode, ownership of original file */
        mode = stbuf.st_mode;
	owner = stbuf.st_uid;
	group = stbuf.st_gid;

        if ((fdtmp2 = open(temp_file_name, O_RDONLY)) == -1)
        {
		(void) fprintf(stderr, 
			"%smcs: %s: can't open temporary file\n"
			,SGS, temp_file_name);
                mcs_exit(FAILURE);
        }

        (void) stat(temp_file_name, &stbuf); /* for size of file */

        if (( buf = (char*)malloc(stbuf.st_size * sizeof(char)) ) == NULL)
        {
                (void) fprintf(stderr, 
		"%smcs: malloc memory allocation failure\n", SGS);
                mcs_exit(FAILURE);
        }

        if ( read(fdtmp2, buf, (unsigned)stbuf.st_size) != (unsigned)stbuf.st_size)
	{
		(void) fprintf(stderr,
			"%smcs: %s: read system failure\n"
			,SGS, temp_file_name);
		mcs_exit(FAILURE);
	}

	temp_fname = get_dir(fname);

        if ((fd=open(temp_fname, O_WRONLY | O_TRUNC | O_CREAT, (mode_t) 0644)) == -1)        {
                (void) fprintf(stderr,"%smcs: %s: can't open file for writing\n"
			,SGS, temp_fname);
                mcs_exit(FAILURE);
        }

        (void) chmod(temp_fname, mode);
	(void) chown(temp_fname, owner, group);

        if (( write(fd, buf, (unsigned)stbuf.st_size)) != (unsigned)stbuf.st_size)
	{
		(void) fprintf(stderr,
		"%smcs: %s: write system failure: %s: file not manipulated\n"
		,SGS, temp_fname,fname);
		(void) unlink(temp_fname);
		mcs_exit(FAILURE);
	}

        free(buf);
        (void) close(fdtmp2);
        (void) close(fd);
        (void) unlink(temp_file_name); 	/* temp file */
	(void) unlink(fname); 		/* original file */
	(void) link(temp_fname, fname);
	(void) unlink(temp_fname);
        return;
}

static void
quit()
{
        (void) unlink(artmpfile);
        (void) unlink(elftmpfile);
}

/*ARGSUSED0*/
static void
sigexit(i)
        int i;
{
	(void) unlink(artmpfile );
	(void) unlink(elftmpfile );
        exit(100);
}

static void
usage()
{
        (void) fprintf(stderr,
        "usage: %smcs -Vcdp -a 'string' [-n 'name'] files...\n", SGS);
        mcs_exit(FAILURE);
        return;
}

static void
mcs_exit(val)
{
	quit();
        exit(val);
        return;
}

static char *
get_dir(pathname)
char *pathname;
{

	char *directory;
	char *p;
	unsigned int position = 0;

	if ((p=strrchr(pathname, '/')) == NULL)
	{ /* file in current working directory */
		
		return( mktemp("./mcs3XXXXXX"));
	}
	else
	{
		p++;
		position = strlen(pathname) - strlen(p);
		if ((directory = (char *)malloc(position+10+1)) == NULL)
		{
			(void) fprintf(stderr,
			"%smcs: malloc memory failure\n", SGS);
			mcs_exit(FAILURE);
		}
		(void) strncpy(directory, pathname, position);
		directory[position] = '\0';
		(void)strcat(directory,"mcs3XXXXXX");

		return( mktemp(directory));
	}

}
