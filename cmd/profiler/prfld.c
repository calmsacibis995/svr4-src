/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)profil-3b15:prfld.c	1.11.2.1"
/*
 *	prfld - load profiler with sorted kernel text addresses
 */

#include <stdio.h>
#include <sys/errno.h>
#include <a.out.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <libelf.h>

#ifdef i386
#include <sys/elf_386.h>
#else
#include <sys/elf_M32.h>
#endif

#ifdef i386
Elf32_Addr 	K_TEXT_VA, 	/* Virt Addr of start of kernel text */
		K_DATA_VA; 	/* Virt Addr of start of kernel data */

struct nlist nl[] = {
	{"stext"},		/* symbol table value of kernel text */
	{"sdata"},		/* symbol table value of kernel data */
	{0},
};
#else
#define	K_TEXT_VA 0x40000000	/* Virt Addr of start of kernel text */
#define K_DATA_VA 0x40160000	/* Virt Addr of start of kernel data */
				/* NOTE - There is a dependency on the	*/
				/* mapfile used to build the kernel, here. */
				/* Specifically, the VA of the start of text */
				/* and the length of text segment */
				/* If the VA of kern text changes, this */
				/* value must change */
				/* This is for ELF CCS, only */
#endif

int N_TEXT;
int symcnt;			/* number of symbols */
int maxprf;
int iscoff = 1;			/* presume COFF namelist (1 == COFF, 0 == ELF */

struct syment *stbl;		/* start of COFF symbol table */
Elf32_Sym *e_stbl = NULL;	/* Start of ELF symbol table */
char *namelist = "/stand/unix";	/* namelist file */
extern int errno;
struct stat ubuf,syb;
char *flnm = "/tmp/prf.adrfl";
int fa;

main(argc, argv)
char **argv;
{
	register int *ip, prf;
	register struct syment *sp;
	register Elf32_Sym *esp;
	int ntaddr;
	int *taddr;
	int compar(), *getspace();

	if(argc == 2)
		namelist = argv[1];
	else if(argc != 1)
		error("usage: prfld [/stand/unix]");
	if((prf = open("/dev/prf", O_WRONLY)) < 0)
		error("cannot open /dev/prf");
	taddr=getspace(namelist);
	rdsymtab();
	ip = taddr;
	*ip++ = 0;
	if (iscoff) {
		for(sp = stbl; --symcnt; sp++) {
			if(ip >= &taddr[maxprf])
				error("too many text symbols");
			if( (sp->n_sclass == C_EXT  
			  || sp->n_sclass == C_STAT )
			  && sp->n_scnum == N_TEXT)
				*ip++ = sp->n_value;
		}
	} else {	/* Is ELF */

#ifdef i386
		nlist(namelist, nl);
		K_TEXT_VA = nl[0].n_value;
		if (K_TEXT_VA == 0) error("can not find start of kernel text address.");
		K_DATA_VA = nl[1].n_value;
		if (K_DATA_VA == 0) error("can not find start of kernel data address.");
#endif

		for(esp = e_stbl; symcnt; esp++, --symcnt) {

			if(ip >= &taddr[maxprf]) 
				error("too many text symbols");

			if (esp->st_value < K_TEXT_VA)
				continue;

			if (esp->st_value >= K_DATA_VA)
				continue;

			if ((ELF32_ST_TYPE(esp->st_info)) == STT_FILE)
				continue;

			if ((ELF32_ST_TYPE(esp->st_info)) == STT_SECTION)
				continue;

			*ip++ = esp->st_value;
		}
	}
	ntaddr = ip - taddr;
	qsort(taddr, ntaddr, sizeof (int), compar);
	if(write(prf, taddr, ntaddr*sizeof(int)) != ntaddr*sizeof(int))
		switch(errno) {
		case ENOSPC:
			error("insufficient space in system for addresses");
		case E2BIG:
			error("unaligned data or insufficient addresses");
		case EBUSY:
			error("profiler is enabled");
		case EINVAL:
			error("text addresses not sorted properly");
		default:
			error("cannot load profiler addresses");
		}
	exit(0);
}

compar(x, y)
	register  unsigned  *x, *y;
{
	if(*x > *y)
		return(1);
	else if(*x == *y)
		return(0);
	return(-1);
}

error(s)
char *s;
{
	printf("error: %s\n", s);
	exit(1);
}

rdsymtab()
{
	struct	filehdr	filehdr;
	struct  scnhdr  scnptr;
	FILE	*fp;
	struct	syment *sp;
	int	i;

	int fd;
	Elf *elfd;
	Elf_Scn *scn;
	Elf32_Ehdr *ehdr;
	Elf32_Shdr *eshdr;

	if((fp = fopen(namelist, "r")) == NULL)
		error("cannot open namelist file");
	if(fread(&filehdr, FILHSZ, 1, fp) != 1)
		error("read error in namelist file");

#ifdef i386
	if(filehdr.f_magic != I386MAGIC) {
#else
	if(filehdr.f_magic != FBOMAGIC) {
#endif

		iscoff = 0;
		fclose(fp);
	}

	N_TEXT=0;

	if (iscoff) {
	/*
	 * Read the section headers to find the section numbers
	 * for .text, .data, and .bss.  First seek past the file header 
	 * and optional header, then loop through the section headers
	 * searching for the names .text, .data, and .bss.
	 */
	if(fseek(fp, FILHSZ + filehdr.f_opthdr, 0) != 0)
		error("error in seeking to section headers");

	for(i=1; i<=(int)filehdr.f_nscns; i++) {
		if(fread(&scnptr, SCNHSZ, 1, fp) != 1)
			error("read error in section headers");

		switch(scnmatch(scnptr.s_name,1,_TEXT)) {
			case 1:		/* .text */
					N_TEXT = i; break;
			default:
					break;
		}
	}
	if(N_TEXT == 0) 
		error(".text was not found in section headers");

	if((stbl=(struct syment *)sbrk(filehdr.f_nsyms*sizeof(SYMENT))) == (struct syment *)-1)
		error("cannot allocate space for namelist");
	fseek(fp, filehdr.f_symptr, 0);
	symcnt = 0;
	for(i=0, sp=stbl; i < filehdr.f_nsyms; i++, sp++) {
		symcnt++;
		if(fread(sp, SYMESZ, 1, fp) != 1)
			error("read error in namelist file");
		if(sp->n_numaux) {
			fseek(fp, AUXESZ*sp->n_numaux, 1);
			i += sp->n_numaux;
		}
	}
	brk(sp);
	fclose(fp);

	} else {	/* ELF format */

		if ((elf_version(EV_CURRENT)) == EV_NONE)
			error("Elf Access library out of date");

		if ((fd = open(namelist, 0)) == -1)
			error("cannot open namelist file");

		if ((elfd = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
			error("can't elf_begin");

		if ((ehdr = elf32_getehdr(elfd)) == NULL) {
			elf_end(elfd);
			close(fd);
			error("namelist not in a.out format");
		}

		scn = NULL;
		while ((scn = elf_nextscn(elfd, scn)) != NULL) {

			if ((eshdr = elf32_getshdr(scn)) == NULL) {
				elf_end(elfd);
				close(fd);
				error("Error reading Shdr for symbol table");
			}

			if (eshdr->sh_type == SHT_SYMTAB) {
				if (alloc_symtab(scn)) {
					elf_end(elfd);
					close(fd);
					error("unable to allocate symbol tab\n");
				}

				break;		/* only handle 1 symbol table */
			}
		}

		elf_end(elfd);
		close(fd);
	}
}

static
alloc_symtab(scn)
Elf_Scn *scn;
{
	Elf_Data *data = NULL;
	static size_t size = 0;

	if (((data = elf_getdata(scn, data)) == NULL) ||
		(data->d_size == 0) ||
		(data->d_buf == NULL)) {
			return (-1);
	}


	size = data->d_size;

	if ((e_stbl = (Elf32_Sym *)malloc(size)) == NULL) {
		return (-1);
	}

	(void)memcpy((char *)e_stbl, (char *)data->d_buf, size);

	symcnt = size / sizeof(Elf32_Sym);
	return (0);
}

/* 
 * scnmatch() is only called by rdsymtab() to match a section
 * name in the section headers to find .text, .data, and .bss.
 * A number is returned indicating which name matched, or a zero
 * is returned if none matched.  This routine was copied out of
 * exec.c in the kernel code.
 */
scnmatch(target, count, sources)
char *target;
int count;
char *sources;
{
	register char *p, *q, **list;

	list = &sources;
	while (count-- > 0) {
		p = target;
		q = *list++;
		while(*p == *q++) {
			if (*p++ == '\0')
			      return(list - &sources);
		}
	}
	return(0);
}

#define MAXPRF 0
struct nlist setup[] = {
		{"maxprf"},
		{0},
	};

 int *
getspace(nmlist)
	char	*nmlist;
	{
	int *space;
	int f;
	mode_t m;

	stat (nmlist,&syb);
	if ((stat(flnm,&ubuf) == -1) ||
		 (ubuf.st_mtime <= syb.st_ctime))
			goto creafl;
	fa = open(flnm,O_RDWR);
	if (read(fa,&maxprf,sizeof (int)) != sizeof (int)){
		close(fa);
		unlink(flnm);
		goto creafl;
	}
	if ((space = (int *)malloc(maxprf * sizeof(int))) == NULL) {
		perror("Cannot malloc space for symbol table");
		exit(errno);
	}
	close(fa);
	return(space);
 creafl:
		m = umask((mode_t)0);
		fa = open(flnm, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		umask(m);
		chown(flnm, (uid_t)0, getegid());
		
		nlist(nmlist,setup);
	if (setup[MAXPRF].n_value != 0){
		if((f = open("/dev/kmem", O_RDONLY)) == -1) {
			perror("Cannot open /dev/kmem to read maxprf");
			exit(errno);
		}
		lseek(f, (long)setup[MAXPRF].n_value, 0);
		if (read(f,&maxprf,sizeof (int)) != sizeof (int)) {
			perror("Cannot read maxprf");
			exit(errno);
		}
		if (write(fa, &maxprf, sizeof maxprf) == -1) {
			perror("Cannot write value of maxprf to prf.adrfl");
			exit(errno);
		}
		close(fa);
		close(f);
		if ((space = (int *)malloc(maxprf * sizeof(int))) == NULL) {
			perror("Cannot malloc space for symbol table");
			exit(errno);
		}
	} else {
		fprintf(stderr, "No value for maxprf found in %s\n", nmlist);
		exit(2);
	}
	return(space);
}
