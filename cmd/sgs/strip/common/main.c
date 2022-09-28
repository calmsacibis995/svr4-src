/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)strip:common/main.c	1.23"
/**************************** CI5/elf-based STRIP *****************************/

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
#include <stdlib.h>
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

#define OPTSTR "blrVx?"
#define FORMAT          "%-16s%-12ld%-6u%-6u%-8o%-10ld%-2s"
#define ROUNDUP(x)      (((x) + 1) & ~1)

#define NOSEG 0
#define IN    1
#define PRIOR 2
#define AFTER 3

#define	FAILURE 1
#define SUCCESS 0

static  int	found_ar_sym_tab = 0;
static	int 	errflag=0;
static	int	xflag = 0;
static	int	lflag = 0;
static	int 	Vflag = 0;
static	int	ar_file = 0;
static	int 	error = 0;

static	Elf     	*elffile, *arf, *elf;
static	Elf_Scn 	*scn, *elf_scn;
static	Elf32_Shdr 	*shdr, *elf_shdr;
static	Elf_Cmd         cmd;
static	Elf32_Ehdr      *ehdr, *elf_ehdr;
static	Elf32_Phdr	*phdr, *phdr1, *elf_phdr;
static	Elf_Data	*data, *elf_data;
static	Elf_Arhdr	*mem_header;

static	unsigned int	ndx;
static 	int 		fd, fdtmp, fdtmp2, fdtmp3, fdartmp;
static	char		*elftmpfile, *artmpfile, *fname, *cur_filenm;
static	char		*buf, *file_buf;
static 	struct stat 	stbuf;
static	mode_t		mode;
static	uid_t		owner;
static	gid_t		group;

static  Elf32_Off	*b_e_seg_table;
static	int		*sec_table;     /* array contains section's new */
                                        /* position in the stripped file*/
static	Elf32_Off       *off_table;     /* array maintains section's offset;
                                           set to retain old offset, else 0 */
static	int             *nobits_table;  /* array maintains NOBITS sections */

static  int             no_of_del_sec = 0;

static int      signum[] = {SIGHUP, SIGINT, SIGQUIT, 0};

static	int	each_file(),
		strip_file(),
		traverse_file(),
		location(),
		build_segment_table(),
		build_file();

static	void	sigexit(),
		strip_exit(),
		copy_file(),
		initialize(),
		quit(),
		copy_non_elf_to_temp_ar(),
		copy_elf_file_to_temp_ar_file();

static char	*get_dir();

extern	void	exit();

extern	int	getopt(),
		unlink(),
		link(),
		close(),
		write(),
		read(),
		chmod();

extern	long	lseek();

/****************************************************************************/

main(argc, argv)
int	argc;
char 	*argv[];
{

	char	*optstr = OPTSTR; /* option string used by getopt() */
        extern  int optind;             /* arg list index */
        int         optchar;
	short	    optcnt = 0;
	int		i;

	for (i = 0; signum[i]; i++)
		if (signal(signum[i], SIG_IGN) != SIG_IGN)
			(void) signal(signum[i], sigexit);

	if (argc == 1) 
	{
                (void) fprintf(stderr,
		"usage: %sstrip [-l -x -r -V -b ] files...\n",SGS);
                exit(FAILURE);
        }

	(void) elf_version(EV_NONE);
        if (elf_version(EV_CURRENT) == EV_NONE) 
	{
                (void) fprintf(stderr,
	        "%sstrip: FATAL: elf_version() failed - libelf.a out of date.\n"
			,SGS);
                exit(FAILURE);
        }

	while ((optchar = getopt(argc, argv, optstr)) != -1) {
		optcnt++;
		switch (optchar) {
		case 'l':	lflag++; xflag++; break;
		case 'x':	xflag++; break;

		case 'r':	(void) fprintf(stderr, "%sstrip: Warning: the -r option has no effect and will be removed in the next release.\n", SGS);
				break;

		case 'V':	Vflag++; 
				optcnt--;
				(void) fprintf(stderr,
					 	"%sstrip: %s %s\n"
						,SGS, CPL_PKG, CPL_REL);
				break;

		case 'b':	(void) fprintf(stderr,"%sstrip: Warning: the -b option has no effect and will be removed in the next release.\n", SGS);
				break;

		case '?':       errflag ++; break;
                default:	break;
                }
        }

	if ( errflag || (optind >= argc))
        {
                if(!( Vflag && (argc==2) && (optcnt==0)))
                {
			(void) fprintf(stderr,
			"usage: %sstrip [-l -x -r -V -b ] files...\n",SGS);
                        exit(FAILURE);
                }
        }


        while (optind < argc)
        {
		fname = argv[optind];
                error = error + (each_file() );
		optind++;
        }

        strip_exit(error);

        /*NOTREACHED*/
}

	

static int
each_file()
{

	int	error = 0, err=0;

	cmd = ELF_C_READ;

	found_ar_sym_tab = 0;

	if ( (fd = open(fname, O_RDONLY)) == -1) {
                (void) fprintf(stderr, "%sstrip: %s: cannot open file\n"
					,SGS, fname);
                return(FAILURE);
        }


	if ( (arf = elf_begin(fd, cmd, (Elf *)0) ) == 0)
	{
		(void) fprintf(stderr, 
			"%sstrip: libelf error: %s\n", SGS, elf_errmsg(-1) );
		(void) elf_end(arf);
        	(void) close(fd);   /* done processing this file */
                return(FAILURE);
        }

	if ((elf_kind(arf) == ELF_K_COFF))
	{
		(void) fprintf(stderr, "%sstrip: %s: cannot strip a COFF file\n"
					,SGS, fname);
		(void) elf_end(arf);
                (void) close(fd);   /* done processing this file */
                return(FAILURE);
        } 
	else if ((elf_kind(arf) == ELF_K_AR))
	{
		/* open ar_temp_file */

		artmpfile = tempnam(TMPDIR, "strp2"); 
		if (( fdartmp = open(artmpfile, O_WRONLY | O_APPEND | O_CREAT, (mode_t) 0644)) == NULL)
		{
			(void) fprintf(stderr, 
				"%sstrip: %s: cannot open temporary file\n",
				 SGS, artmpfile);
			(void) elf_end(arf);
                        (void) close(fd);
			exit(FAILURE);	
		}
		ar_file = 1;
		/* write magic string to artmpfile */
		if ((write(fdartmp, ARMAG, SARMAG)) != SARMAG)
		{
			(void) fprintf(stderr,
			"%sstrip: %s: write system failure\n"
			,SGS, artmpfile);
			strip_exit(FAILURE);
		}
	}
	else
	{
		ar_file = 0;
		cur_filenm = fname;
	}


	elftmpfile = tempnam(TMPDIR, "strp1"); /* holds temporary file;   */
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
				    "%sstrip: %s: malformed archive at %ld\n",
				    SGS, fname, elf_getbase(elf) );
				(void) elf_end(elf);
				(void) elf_end(arf);
				(void) close(fd);
				(void) unlink(artmpfile);
				return(FAILURE);
			}
			if ((cur_filenm = (char *)malloc( (strlen(fname) + 3
                         +strlen(mem_header->ar_name))* sizeof(char))) == NULL)
                        {
                                (void) fprintf(stderr,
                                "%sstrip: malloc memory failure\n", SGS);
                                strip_exit(FAILURE);
                        }

                        (void) sprintf(cur_filenm, "%s[%s]"
                                 ,fname, mem_header->ar_name);
		}

		if ( elf_kind(elf) == ELF_K_ELF )
		{
		
			if ( strip_file() == FAILURE ) 
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
					copy_non_elf_to_temp_ar(fname);		
					error++;
				}
			}
			else
			if (ar_file)
			{
				/* copy temp file to temp_ar_file and	    */
				/* change size in header field if necessary */
				copy_elf_file_to_temp_ar_file(fname);
			}
		}
		else  /* decide what to do with non-ELF file */
		{
			if (!ar_file)
			{
				(void)fprintf(stderr,
					"%sstrip: %s: invalid file type\n",
					 SGS, fname);
				(void) close(fd);
				return(FAILURE);
			}
			else
			{
				copy_non_elf_to_temp_ar(fname);
			}
		}

		cmd = elf_next(elf);
		(void) elf_end(elf);

	} /* while elf_begin() */

	err = elf_errno();
	if ( err != 0)
	{
		(void) fprintf(stderr,
			"%sstrip: libelf error: %s\n",SGS, elf_errmsg(err) );
		(void) fprintf(stderr,
			"%sstrip: %s: file not stripped\n", SGS, fname);
		return(FAILURE);
	}

	(void) elf_end(arf);
	if (close(fd) == -1)	/* done processing this file */
	{
		(void)fprintf(stderr,"%sstrip: %s: close system failure\n"
			,SGS,fname);
		strip_exit(FAILURE);
	}


	if (ar_file)
	{
		if (close(fdartmp) == -1) /* done writing to ar_temp_file */ 
		{
			(void)fprintf(stderr,"%sstrip: %s: close system failure\n",
				 SGS, fname);
			strip_exit(FAILURE);
		}
                copy_file(fname, artmpfile); /* copy ar_temp_file to FILE */

		if (found_ar_sym_tab)
		{
			(void) fprintf(stderr,
		"%sstrip: WARNING: %s: symbol table deleted from archive \n"
			,SGS, fname);
			(void) fprintf(stderr,
			"execute  `ar -ts %s` to restore symbol table.\n"
			,fname);
		}
        }
	else
		copy_file(fname,elftmpfile); /* copy temp_file to FILE */

	return(error);

} /* each file */

static void
copy_elf_file_to_temp_ar_file(fname)
char *fname;
{

	char *buf;
        char    mem_header_buf[sizeof( struct ar_hdr ) + 1];

        if ((fdtmp3 = open(elftmpfile, O_RDONLY)) == -1)
        {
                (void) fprintf(stderr,"%sstrip: %s: can't open temporary file\n"
			,SGS, elftmpfile);
                strip_exit(FAILURE);
        }

        (void) stat(elftmpfile, &stbuf); /* for size of file */

        if (( buf = (char*)malloc(ROUNDUP(stbuf.st_size) * sizeof(char)) ) == NULL)
        {
                (void) fprintf(stderr, "%sstrip: malloc failure\n", SGS);
                strip_exit(FAILURE);
        }

        if (read(fdtmp3, buf, (unsigned) stbuf.st_size) != (unsigned) stbuf.st_size)
	{
		(void) fprintf(stderr, "%sstrip: read failure\n", SGS);
               	strip_exit(FAILURE);
	}

	(void) sprintf( mem_header_buf, FORMAT, mem_header->ar_rawname, mem_header->ar_date, (unsigned)mem_header->ar_uid, (unsigned)mem_header->ar_gid, (unsigned)mem_header->ar_mode, stbuf.st_size, ARFMAG );

        if (write(fdartmp, mem_header_buf, sizeof( struct ar_hdr)) != sizeof( struct ar_hdr))
	{
		(void) fprintf(stderr, 
		"%sstrip: %s: write system failure: %s: file not stripped\n"
		,SGS, artmpfile, fname);
		strip_exit(FAILURE);
	}

	if (stbuf.st_size & 0x1)
	{
		buf[stbuf.st_size] = '\n';
        	if (write(fdartmp, buf, (unsigned) ROUNDUP(stbuf.st_size)) == -1)
		{
			(void) fprintf(stderr,
			"%sstrip: %s: write system failure: %s: file not stripped\n"
			,SGS,artmpfile,fname);
			strip_exit(FAILURE);
		}
	}
	else
		if (write(fdartmp, buf, (unsigned) stbuf.st_size) != (unsigned) stbuf.st_size)
		{
			(void) fprintf(stderr,
			"%sstrip: %s: write system failure: %s: file not stripped\n"
			,SGS,artmpfile,fname);
                        strip_exit(FAILURE);
		}

        free(buf);
        (void) close(fdtmp3);
        return;
}


static void
copy_non_elf_to_temp_ar(fname)
char *fname;
{

        char    mem_header_buf[sizeof( struct ar_hdr ) + 1];


	if ( strcmp(mem_header->ar_name,"/") != 0 )
	{

		(void) sprintf( mem_header_buf, FORMAT, mem_header->ar_rawname, mem_header->ar_date, (unsigned)mem_header->ar_uid, (unsigned)mem_header->ar_gid, (unsigned)mem_header->ar_mode, mem_header->ar_size, ARFMAG );

		if (write(fdartmp, mem_header_buf, sizeof( struct ar_hdr)) != sizeof( struct ar_hdr))
		{
			(void) fprintf(stderr, 
			"%sstrip: %s: write system failure: %s: file not stripped\n"
			,SGS,artmpfile,fname);
                        strip_exit(FAILURE);
		}
		if ((file_buf = (char *)malloc( ROUNDUP(mem_header->ar_size))) == NULL)
		{
			(void) fprintf(stderr,"%sstrip: malloc failure\n", SGS);
			strip_exit(FAILURE);
		}

        	if (lseek(fd, elf_getbase(elf), 0) != elf_getbase(elf))
        	{
                	(void) fprintf(stderr,"%sstrip: lseek failure\n", SGS);
                	strip_exit(FAILURE);
        	}

		if (read(fd, file_buf, (unsigned) ROUNDUP(mem_header->ar_size) )
		    != (unsigned) ROUNDUP(mem_header->ar_size) )
		{
			(void) fprintf(stderr, "%sstrip: read failure\n", SGS);
                	strip_exit(FAILURE);
        	}
		if (write(fdartmp, file_buf, (unsigned) ROUNDUP(mem_header->ar_size)) != ROUNDUP(mem_header->ar_size))
		{
			(void) fprintf(stderr, 
			"%sstrip: %s: write system failure: %s: filenot stripped\n"
			,SGS,artmpfile,fname);
			strip_exit(FAILURE);
		}
        	free(file_buf);
	}
	else
		found_ar_sym_tab = 1;
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
                (void) fprintf(stderr,"%sstrip: %s: can't open temporary file\n",
			 SGS, temp_file_name);
               	strip_exit(FAILURE); 
        }

	(void) stat(temp_file_name, &stbuf); /* for size of file */

	if (( buf = (char*)malloc(stbuf.st_size * sizeof(char)) ) == NULL)
	{
		(void) fprintf(stderr,"%sstrip: malloc failure\n", SGS);
		strip_exit(FAILURE);
	}

        if (read(fdtmp2, buf, (unsigned) stbuf.st_size) != (unsigned) stbuf.st_size)
	{
		(void) fprintf(stderr,"%sstrip: %s: read failure\n"
			,SGS,temp_file_name);
		strip_exit(FAILURE);
	}

	temp_fname = get_dir(fname);

      	if ((fd=open(temp_fname, O_WRONLY | O_TRUNC | O_CREAT, (mode_t) 0644)) == -1)
	{
		(void) fprintf(stderr,
				"%sstrip: %s: can't open file for writing\n",
				SGS, temp_fname);
		strip_exit(FAILURE); 
        }

	(void) chmod(temp_fname, mode);
	(void) chown(temp_fname, owner, group);

	if (write(fd, buf, (unsigned) stbuf.st_size) != (unsigned)stbuf.st_size)
	{
		(void) fprintf(stderr, 
			"%sstrip: %s: write system failure: %s: file not stripped\n"
			,SGS,temp_fname,fname);
		(void) unlink(temp_fname);
                strip_exit(FAILURE);
        }
	
	free(buf);
	(void) close(fdtmp2);
	(void) close(fd);
	(void) unlink(temp_file_name); 	/* temp file */
	(void) unlink(fname);		/* original file */
	(void) link(temp_fname, fname);
	(void) unlink(temp_fname);
	return;
}

static int
strip_file()
{
	int 	error = SUCCESS;

	if (traverse_file() == FAILURE)
	{
		(void) fprintf(stderr,"%sstrip: WARNING: %s: Cannot strip file\n",
			SGS, cur_filenm);
		error = FAILURE;
	}
	else if (build_file() == FAILURE)
	{
		(void) fprintf(stderr,"%sstrip: WARNING: %s: Cannot strip file\n",
			SGS, cur_filenm);
		error = FAILURE;
	}

	free(off_table);
	free(sec_table);
	free(nobits_table);
	return(error);
	
}

static void
initialize()
{
	
	no_of_del_sec = 0;

	if ((sec_table = (int *) calloc(ehdr->e_shnum, sizeof(int))) == NULL)
	{
		(void) fprintf(stderr, 
			"%sstrip: memory allocation failure on calloc\n",SGS);
		strip_exit(FAILURE);
	}

	if ((off_table = (Elf32_Off *) calloc(ehdr->e_shnum, sizeof(Elf32_Off))) == NULL)
	{
		(void) fprintf(stderr,
			"%sstrip: memory allocation failure on calloc\n",SGS); 
		strip_exit(FAILURE);
        }

	if ((nobits_table = (int *) calloc(ehdr->e_shnum, sizeof(int))) == NULL)
	{
                (void) fprintf(stderr,
			"%sstrip: memory allocation failure on calloc\n",SGS);
                strip_exit(FAILURE);
        }
}

/*******************************************************************************/
/* Search through PHT saving the beginning and ending segment offsets in table */
/*******************************************************************************/

static int
build_segment_table()
{
	unsigned int    i,j;

	if ((b_e_seg_table = 
		(Elf32_Off *) calloc(ehdr->e_phnum*2, sizeof(Elf32_Off))) == NULL)
	{
		(void) fprintf(stderr,
			"%sstrip: memory allocation failure on calloc\n",SGS);
		strip_exit(FAILURE);
	}

	phdr1 = phdr;
	for (i=1, j=0; i<=ehdr->e_phnum; i++, phdr1++ ) 
	{
		b_e_seg_table[j] = phdr1->p_offset;
		b_e_seg_table[j+1] = phdr1->p_offset + phdr1->p_memsz;
		j += 2;
	}

#if DEBUG
	(void) printf("\n");
	for (i=1, j=0; i<=ehdr->e_phnum; i++, j+=2 )
	(void) printf("DEBUG: b_e_seg_table[%d] is %x\t b_e_seg_table[%d] is %x\n"
	,j,b_e_seg_table[j], j+1, b_e_seg_table[j+1]);
#endif
	scn = 0;
	while ((scn = elf_nextscn(elf, scn)) != 0)
	{
		if ((shdr = elf32_getshdr(scn)) == 0)
		{
			(void) fprintf(stderr,
				"%sstrip: %s: no section header table\n"
				,SGS, cur_filenm);
			return(FAILURE);
		}

		if (shdr->sh_type == SHT_NOBITS)
		{
			for ( i=1,j=0; i<=(int)ehdr->e_phnum; i++, j+=2)
			{
			  if ( shdr->sh_offset + shdr->sh_size >= b_e_seg_table[j] && 
			       shdr->sh_offset + shdr->sh_size <= b_e_seg_table[j+1] )
					b_e_seg_table[j+1] -= shdr->sh_size;
			} 
		}
	}

#if DEBUG
	(void) printf("\n");
	for (i=1, j=0; i<=ehdr->e_phnum; i++, j+=2 )
	(void) printf("DEBUG: b_e_seg_table[%d] is %x\t b_e_seg_table[%d] is %x\n"
	,j,b_e_seg_table[j], j+1, b_e_seg_table[j+1]);
#endif

	return(SUCCESS);
}

static int
traverse_file()
{
	
	int		SYM = 0;
	int		SYM_LINK = 0;
	int 		REL = 0;	
	int		DYN = 0;
	int		x,y;

	unsigned int	i;

	int 		scn_no;
	Elf_Scn         *temp_scn;
	Elf32_Shdr      *temp_shdr;


	if ((ehdr = elf32_getehdr(elf)) != 0 )
		initialize();
	else
		return(FAILURE);

	if ( (phdr = elf32_getphdr(elf)) != NULL )
	{
		if ( build_segment_table() == FAILURE)
			return(FAILURE);
	}

	/*******************************************************************/
	/* Loop through the existing sections. If the current section can  */
	/* be savely deleted from the file, then mark its corresponding    */
	/* entry in the array with a -1. If the current section should not be */
	/* removed from the file, then mark its corresponding entry in the */
	/* array with its new section number. 				   */ 
	/*******************************************************************/

	ndx = ehdr->e_shstrndx;
        scn = 0;
        scn_no = 1;
	
	while ((scn = elf_nextscn(elf, scn)) != 0) 
	{
	
		char *name = "";
		char *temp_name = "";

		if ((shdr = elf32_getshdr(scn)) != 0)
			name = elf_strptr(elf, ndx, (size_t)shdr->sh_name);
		else
		{
			(void) fprintf(stderr,
                        	"%sstrip: %s: no section header table\n"
                        	,SGS, cur_filenm);
			return(FAILURE);
		}
			
		if ( shdr->sh_type == SHT_REL )
		{
			REL = 1;

			/****************************************************/
			/* If SHT_REL section applies to either a .debug    */
			/* or a .line section (only a .line section if	    */
			/* the -l flag was set), then delete the relocation */
			/* section if both the section and its relocation   */
			/* section are not contained within a segment.      */
			/****************************************************/

			x = location(shdr->sh_offset + shdr->sh_size);

			if (shdr->sh_info != SHN_UNDEF && 
			     (temp_scn = elf_getscn(elf, shdr->sh_info)) != 0)
                        {
				if ((temp_shdr = elf32_getshdr(temp_scn)) != 0)
				{
	                        	temp_name = elf_strptr(elf, ndx, (size_t)temp_shdr->sh_name);
					if (temp_shdr->sh_type == SHT_NOBITS)
					   y=location(temp_shdr->sh_offset);
					else
					   y=location(temp_shdr->sh_offset+
						 temp_shdr->sh_size);	

					if ( ((strcmp(temp_name, ".debug") == 0
					       && !lflag) 
					     || strcmp(temp_name,".line") == 0 )
					   &&
					     ( y != IN )
					   &&
					     ( x != IN ) )
					{
						sec_table[scn_no] = -1;
						no_of_del_sec++;
					}
					else
					{
					   sec_table[scn_no] = scn_no - 
							       no_of_del_sec;
					   if ( (x == IN) || (x == PRIOR) )
						off_table[scn_no] = 
							shdr->sh_offset;
					   if ( (strcmp(temp_name,".debug") == 
						0 && !lflag) || 
						strcmp(temp_name, ".line") == 0 &&
						y != IN )
					       	   sec_table[shdr->sh_info] = 
							shdr->sh_info - no_of_del_sec;
					}
				}
				else
				{
                                	(void) fprintf(stderr, 
					"%sstrip: %s: no section header table\n"
					,SGS, cur_filenm);
                        		return(FAILURE);
				}
			}
			else
			{
                        	sec_table[scn_no] = scn_no - no_of_del_sec;
				if (x==IN || x == PRIOR)
					off_table[scn_no] = shdr->sh_offset;
			}
		}

		else if ( ((strcmp(name,".debug") == 0  && !lflag)
				|| strcmp(name,".line") == 0) &&
				sec_table[scn_no] == 0 &&
				( location(shdr->sh_offset+shdr->sh_size)!=IN) )
			{
				sec_table[scn_no] = -1;
				no_of_del_sec++;
			}

		else if ( sec_table[scn_no] != -1 )
		{
			sec_table[scn_no] = scn_no - no_of_del_sec;
			if (shdr->sh_type == SHT_NOBITS)
				x = location(shdr->sh_offset);
			else
				x = location(shdr->sh_offset+shdr->sh_size);
			if (x==IN || x==PRIOR)
				off_table[scn_no] = shdr->sh_offset;

			if (shdr->sh_type == SHT_SYMTAB && x!=IN)
			{
				SYM=scn_no; /*eliminate ST later if possible*/
				
				/* determine if string table can go too! */

				if (shdr->sh_link != SHN_UNDEF &&
                		(temp_scn = elf_getscn(elf, shdr->sh_link))!= 0)
				{
                        	  if ((temp_shdr = elf32_getshdr(temp_scn))!= 0)
				  {
			   		if ( (location(temp_shdr->sh_offset
					      +temp_shdr->sh_size)!=IN) &&
					      ehdr->e_shstrndx != shdr->sh_link)

                                        	SYM_LINK = shdr->sh_link;
						/* eliminate later */
                        	  }
				  else
				  {
					(void) fprintf(stderr, 
					   "%sstrip: %s: no section header table\n"
					   ,SGS, cur_filenm);
                        		return(FAILURE);
				  }
                		}
			}

			else if (shdr->sh_type == SHT_DYNSYM)
				DYN=1; /* have a dynamic symbol table 
				        * can remove global symbol table */
			else if (shdr->sh_type == SHT_NOBITS)
				nobits_table[scn_no] = 1;
		}

		scn_no++;

	} /* while elf_nextscn() */


	if ( SYM && !xflag && (!REL || DYN) )
	{
        	sec_table[SYM] = -1;
		off_table[SYM] = 0;
                no_of_del_sec++;

		for (i=SYM+1; i<ehdr->e_shnum && sec_table[i] != -1; i++)
			sec_table[i] = sec_table[i] - 1;
		
		if (SYM_LINK)
		{
			sec_table[SYM_LINK] = -1;
			off_table[SYM_LINK] = 0;
			no_of_del_sec++;
		  	for (i=SYM_LINK+1; i<ehdr->e_shnum && sec_table[i] != -1; i++)
				sec_table[i] = sec_table[i] - 1;

		}

	}

#if DEBUG
	(void) printf("\n");
	for (i=0; i<=ehdr->e_shnum-1; i++)
	(void) printf("DEBUG: sec_table[%d] = %d   off_table[%d] = %x  nobits_table[%d] = %d\n", i,sec_table[i],i,off_table[i],i,nobits_table[i]);
#endif

	return(SUCCESS);
}



static int
build_file()
{
	int             scn_no, x;	
	unsigned int    no_of_symbols = 0;
	Elf32_Sym       *p, *q;
	unsigned int    c = 0;
	Elf32_Off	new_offset = 0, r;
	char *name = "";
	int ndx = ehdr->e_shstrndx;



	if ((fdtmp = open(elftmpfile, O_RDWR | O_TRUNC | O_CREAT, (mode_t) 0644)) == -1){
                (void) fprintf(stderr,"%sstrip: %s: can't open temporary file\n"
			,SGS, elftmpfile);
                strip_exit(FAILURE);
        }

        if ((elffile = elf_begin(fdtmp, ELF_C_WRITE, (Elf *) 0)) == NULL)
        {
		(void) fprintf(stderr,"%sstrip: libelf error: %s\n"
			,SGS, elf_errmsg(-1));
                (void) close(fdtmp);
                return(FAILURE);
        }

        if ((elf_ehdr = elf32_newehdr(elffile)) == NULL)
	{
                (void) fprintf(stderr,"%sstrip: libelf error: %s\n",
                SGS, elf_errmsg(-1));
                return(FAILURE);
        }

        (void) memcpy(elf_ehdr, ehdr, sizeof(Elf32_Ehdr) );

	if ( phdr != NULL)
	{
		elf_flagelf(elffile, ELF_C_SET, ELF_F_LAYOUT);
			
		if ((elf_phdr = elf32_newphdr(elffile, ehdr->e_phnum)) == NULL)
		{
                	(void) fprintf(stderr,"%sstrip: libelf error: %s\n",
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
	

		/*  If section should be copied to 'stripped' file */
		if ( sec_table[scn_no] != -1 )
		{
			if ((shdr = elf32_getshdr(scn)) == NULL)
			{
				(void) fprintf(stderr,"%sstrip: libelf error: %s\n",
				SGS, elf_errmsg(-1));
				return(FAILURE);
			}
		
			name = elf_strptr(elf, ndx, (size_t)shdr->sh_name);

			if ((elf_scn = elf_newscn(elffile)) == NULL)
			{
				(void) fprintf(stderr,"%sstrip: libelf error: %s\n",
				SGS, elf_errmsg(-1));
				return(FAILURE);
			}
			
                        if ((elf_shdr= elf32_getshdr(elf_scn)) == NULL)
			{
				(void) fprintf(stderr,"%sstrip: libelf error: %s\n",
				SGS, elf_errmsg(-1));
				return(FAILURE);
			}
			(void) memcpy(elf_shdr, shdr, sizeof(Elf32_Shdr));

			if ( shdr->sh_type == SHT_HASH || shdr->sh_type == SHT_SYMTAB)
			{
				if (sec_table[shdr->sh_link] < 0) {
					elf_shdr->sh_link = 0;
					(void) fprintf(stderr,
					    "%sstrip: Warning: %s: %s linked to a section that has been removed \n"
					    ,SGS, fname,name);
				}
				else elf_shdr->sh_link = (Elf32_Word) sec_table[shdr->sh_link];
			} else if ( shdr->sh_type == SHT_REL )
			{
				if (sec_table[shdr->sh_link] < 0) {
					elf_shdr->sh_link = 0;
					(void) fprintf(stderr,
					    "%sstrip: Warning: %s: %s linked to a section that has been removed \n"
					    ,SGS, fname,name);
				}
				else elf_shdr->sh_link = (Elf32_Word) sec_table[shdr->sh_link];
				if (sec_table[shdr->sh_info] < 0) {
					elf_shdr->sh_info = 0;
					(void) fprintf(stderr,
					    "%sstrip: Warning: %s: %s info field set to a section that has been removed \n"
					    ,SGS, fname,name);
				}
				else elf_shdr->sh_info = (Elf32_Word) sec_table[shdr->sh_info];
			}
			

			data = 0;
                        if ((data = elf_getdata(scn, data)) == NULL)
			{
				(void) fprintf(stderr,"%sstrip: libelf error: %s\n",
				SGS, elf_errmsg(-1));
				return(FAILURE);
			}
			
                        if ((elf_data = elf_newdata(elf_scn)) == NULL)
			{
				(void) fprintf(stderr,"%sstrip: libelf error: %s\n",
				SGS, elf_errmsg(-1));
				return(FAILURE);
			}

			/**************************************************/
			/* If the number of deleted sections is > 0, then */
			/* we must loop through the SYMBOL TABLE section  */
			/* updating the st_shndx field or nulling out the */
			/* entry if it pertains to a deleted section.     */
			/**************************************************/
	
			if (no_of_del_sec > 0 && shdr->sh_type == SHT_SYMTAB &&
				shdr->sh_entsize > 0   )
			{
				no_of_symbols = shdr->sh_size/shdr->sh_entsize;
				p = (Elf32_Sym *)data->d_buf;
				q = p;

				for ( c = 0; c < no_of_symbols; c++, p++) 
				{
				  if (p->st_shndx <= ehdr->e_shnum &&
				      p->st_shndx  > 0 )
				  {
				   	if (sec_table[p->st_shndx] != -1 ) 
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

			if (phdr != NULL && elf_shdr->sh_type != SHT_NULL) 
			/* compute offsets */
			{
			 	if (off_table[scn_no] == 0) 
				/*compute section offset*/
			 	{
					if (elf_shdr->sh_addralign > 1)
					/*0 or 1 means no alignment constraints */
					{ /* compute remainder */
					  r = new_offset % elf_shdr->sh_addralign;
					  if (r)
				   	  new_offset +=elf_shdr->sh_addralign - r;
					}
					elf_shdr->sh_offset = new_offset;
					elf_data->d_off     = 0;
		
			 		if (nobits_table[scn_no] == 0)
					new_offset += elf_shdr->sh_size;
			 	}
			 	else
				{
					if(nobits_table[scn_no] == 1)
						new_offset = off_table[scn_no];
					else
						new_offset = off_table[scn_no] + elf_shdr->sh_size;
				}

			}

		} /* while there are sections to copy */

		scn_no++;
	} /* while elf_nextscn() */

	/* In the event that the position of the sting table has changed, */
	/* as a result of deleted sections, update the ehdr->e_shstrndx.  */

	if ( elf_ehdr->e_shstrndx > 0 && elf_ehdr->e_shnum > 0 &&
	     sec_table[elf_ehdr->e_shstrndx] <  (int) elf_ehdr->e_shnum )
         		elf_ehdr->e_shstrndx = 
				(Elf32_Half) sec_table[elf_ehdr->e_shstrndx];

	if (phdr!=NULL)
	{
		if (location(elf_ehdr->e_phoff) == AFTER)
		{
			r = new_offset % 4;
			if (r)
				new_offset += 4 - r;
			
       			elf_ehdr->e_phoff = new_offset;
               		new_offset += elf_ehdr->e_phnum*elf_ehdr->e_phentsize;
	
		}

		if (location(elf_ehdr->e_shoff) == AFTER)
		{
			r = new_offset % 4;
                        if (r)
                                new_offset += 4 - r;

			elf_ehdr->e_shoff = new_offset;
		}

	}

	if (elf_update(elffile, ELF_C_WRITE)<0)
	{
		(void) fprintf(stderr,"%sstrip: libelf error: %s\n"
			,SGS, elf_errmsg(-1));
		return(FAILURE); 
	}
		
	(void) elf_end(elffile);	
	(void) close(fdtmp);

	return (SUCCESS);
}
/************************************************************************/
/* Determines whether a section falls PRIOR to, IN or AFTER a segment   */
/************************************************************************/

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

static void
quit()
{
	(void) unlink(artmpfile);
	(void) unlink(elftmpfile);
}

static void
strip_exit(val)
{
       	quit(); 
        exit(val);
        return;
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

static char *
get_dir(pathname)
char *pathname;
{

        char *directory;
        char *p;
        unsigned int position = 0;

        if ((p=strrchr(pathname, '/')) == NULL)
        { /* file in current working directory */

                return( mktemp("./str3XXXXXX"));
        }
        else
        {
                p++;
                position = strlen(pathname) - strlen(p);
                if ((directory = (char *)malloc(position+10+1)) == NULL)
                {
                        (void) fprintf(stderr,
                        "%sstrip: malloc memory failure\n", SGS);
                        strip_exit(FAILURE);
                }
                (void) strncpy(directory, pathname, position);
                directory[position] = '\0';
		(void)strcat(directory,"str3XXXXXX");
		return(mktemp(directory));
        }

}

