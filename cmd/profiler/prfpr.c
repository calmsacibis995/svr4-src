/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)profil-3b15:prfpr.c	1.13.2.1"
/*
 *	prfpr - print profiler log files
 */

#include <stdio.h>
#include <time.h>
#include <a.out.h>
#include <errno.h>
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

int N_TEXT;

int	maxprf;
struct	profile	{
	time_t	p_date;		/* time stamp of record */
	long	*p_ctr;		/* profiler counter values */
} p[2];


struct syment *stbl;		/* start of COFF symbol table */
Elf32_Sym *e_stbl = NULL;	/* start of ELF symbol table */
char *strtab;			/* start of string table */
int symcnt;			/* number of symbols */
int prfmax;			/* actual number of text symbols */
int cutoff = 1;
int pc;
int *t;
int iscoff = 1;			/* presume namelist is COFF */

char *namelist = "/stand/unix";
char *logfile;

long sum, osum;


main(argc, argv)
char **argv;
{
	register int ff, log, i;
	struct syment *sp, *search();
	Elf32_Sym *esp, *elf_search();

	switch(argc) {
		case 4:
			namelist = argv[3];
		case 3:
			cutoff = atoi(argv[2]);
		case 2:
			logfile = argv[1];
			break;
		default:
			error("usage: prfpr file [ cutoff [ namelist ] ]");
	}
	getval(namelist);
	p[0].p_ctr = (long *) malloc(maxprf * sizeof (int));
	p[1].p_ctr = (long *) malloc(maxprf * sizeof (int));
	t = (int *) malloc(maxprf * sizeof (int));
	if((log = open(logfile, O_RDONLY)) < 0)
		error("cannot open data file");
	if(cutoff > 100 || cutoff < 0)
		error("invalid cutoff percentage");
	if(read(log, &prfmax, sizeof prfmax) != sizeof prfmax || prfmax == 0)
		error("bad data file");
	if(read(log, t, prfmax * sizeof (int)) != prfmax * sizeof (int))
		error("cannot read profile addresses");
	osum = sum = ff = 0;
	read(log, &(p[!ff].p_date), sizeof (int));
	read(log, p[!ff].p_ctr, (prfmax + 1) * sizeof (int));
	for(i = 0; i <= prfmax; i++)
		osum += p[!ff].p_ctr[i];

	rdsymtab();
	if (iscoff)
		rdstrtab();
	for(;;) {
		sum = 0;
		if(read(log, &(p[ff].p_date), sizeof (int)) != sizeof (int))
			exit(0);
		if(read(log, p[ff].p_ctr, (prfmax +1) * sizeof (int)) !=
			(prfmax + 1) * sizeof (int))
				exit(0);
		shtime(&p[!ff].p_date);
		shtime(&p[ff].p_date);
		printf("\n");
		for(i = 0; i <= prfmax; i++)
			sum += p[ff].p_ctr[i];
		if(sum == osum)
			printf("no samples\n\n");
		else {
			if (iscoff) {
			for(i = 0; i <= prfmax; i++) {
			pc = 1000 * (p[ff].p_ctr[i] - p[!ff].p_ctr[i]) /
				(sum - osum);
			if(pc > 10 * cutoff)
				if(i == prfmax)
					printf("user     %d.%d\n",
					 pc/10, pc%10);
				else {
					sp = search(t[i], N_TEXT, N_TEXT);
					if(sp == 0)
						printf("unknown  %d.%d\n",
						 pc/10, pc%10);
					else {
						printname(sp);
						printf(" %d.%d\n", pc/10, pc%10);
					}
				}
			}
			} else {	/* Is ELF */
			for(i = 0; i <= prfmax; i++) {
			pc = 1000 * (p[ff].p_ctr[i] - p[!ff].p_ctr[i]) /
				(sum - osum);
			if(pc > 10 * cutoff)
				if(i == prfmax)
					printf("user     %d.%d\n",
					 pc/10, pc%10);
				else {
					esp = elf_search(t[i], N_TEXT, N_TEXT);
					if(esp == NULL)
						printf("unknown  %d.%d\n",
						 pc/10, pc%10);
					else {
						elf_printname(esp);
						printf(" %d.%d\n", pc/10, pc%10);
					}
				}
			}
			}
		}
		ff = !ff;
		osum = sum;
		printf("\n");
	}
}

error(s)
char *s;
{
	printf("error: %s\n", s);
	exit(1);
}

shtime(l)
register time_t *l;
{
	register  struct  tm  *t;

	if(*l == (time_t) 0) {
		printf("initialization\n");
		return;
	}
	t = localtime(l);
	printf("%02.2d/%02.2d/%02.2d %02.2d:%02.2d\n", t->tm_mon + 1,
		t->tm_mday, t->tm_year, t->tm_hour, t->tm_min);
}

rdsymtab()
{
	struct filehdr filehdr;
	struct scnhdr scnptr;
	FILE *fp;
	struct syment *sp;
	int i;

	int fd;
	Elf *elfd;
	Elf_Scn *scn;
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

	if (iscoff) {
	/*
	 * Read the section headers to find the section number
	 * for .text. First seek past the file header 
	 * and optional header, then loop through the section headers
	 * searching for the names .text.
	 */
	N_TEXT=0;
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

	if((stbl=(struct syment *)sbrk(filehdr.f_nsyms*sizeof(struct syment))) == (struct syment *)-1)
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
			error("can't elf_ebgin");

		if ((elf_kind(elfd)) != ELF_K_ELF) {
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

			if (eshdr->sh_type != SHT_SYMTAB)
				continue;

			if (alloc_symtab(scn)) {
				elf_end(elfd);
				close(fd);
				error("unable to allocate symbol tab\n");
			}

			if ((scn = elf_getscn(elfd, eshdr->sh_link)) == NULL) {
				elf_end(elfd);
				close(fd);
				error("Error reading String table scn header");
			}

			if ((eshdr = elf32_getshdr(scn)) == NULL) {
				elf_end(elfd);
				close(fd);
				error("Error reading strtab Shdr");
			}

			if (eshdr->sh_type != SHT_STRTAB) {
				elf_end(elfd);
				close(fd);
				error("Error: Incorrect Strtab");
			}

			if (alloc_strtab(scn)) {
				elf_end(elfd);
				close(fd);
				error("unable to allocate string table\n");
			}

			break;		/* only handle 1 symbol table */
		}

		elf_end(elfd);
		close(fd);
	}
}

static
alloc_symtab(scn)
Elf_Scn *scn;
{
	Elf_Data *data;
	static size_t size = 0;

	if (((data = elf_getdata(scn, NULL)) == NULL) ||
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

static
alloc_strtab(scn)
Elf_Scn *scn;
{
	Elf_Data *data;

	if (((data = elf_getdata(scn, NULL)) == NULL) ||
		(data->d_size == 0) ||
		(data->d_buf == NULL)) {
			return (-1);
	}

	if ((strtab = (char *)malloc(data->d_size)) == NULL) {
		return (-1);
	}

	(void)memcpy(strtab, data->d_buf, data->d_size);

	return (0);
}

struct syment *
search(addr, sect1, sect2)
{
	register struct syment *sp;
	register struct syment *save;
	unsigned value;

	value = 0;
	save = 0;
	for(sp = stbl; sp < &stbl[symcnt]; sp++) {
		if( ( sp->n_sclass == C_EXT || sp->n_sclass == C_STAT )
		  && (sp->n_scnum == sect1
		  || sp->n_scnum == sect2) && sp->n_value <= addr
		  && sp->n_value > value) {
			value = sp->n_value;
			save = sp;
		}
	}
	return(save);
}

Elf32_Sym *
elf_search(addr, sect1, sect2)
unsigned addr;
int sect1, sect2;
{
	register Elf32_Sym *esp;
	register Elf32_Sym *esave;
	register unsigned value;

	value = 0;
	esave = NULL;

	for(esp = e_stbl; esp < &e_stbl[symcnt]; esp++) {
		if ((ELF32_ST_TYPE(esp->st_info)) == STT_FILE)
			continue;

		if ((ELF32_ST_TYPE(esp->st_info)) == STT_SECTION)
			continue;

		if ((esp->st_value <= addr) && (esp->st_value > value)) {
			value = esp->st_value;
			esave = esp;
		}
	}

	return(esave);
}


/* 
 * scnmatch() is only called by rdsymtab() to match a section
 * name in the section headers to find .text.
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

rdstrtab()	/* called only if namelist is COFF */
{
	static long length;
	struct filehdr filehdr;
	FILE *fp;

	if((fp = fopen(namelist, "r")) == NULL)
		error("cannot open namelist file");
	if(fread(&filehdr, FILHSZ, 1, fp) != 1)
		error("read error in namelist file");

	fseek(fp, filehdr.f_symptr + filehdr.f_nsyms * SYMESZ, 0);
	if (fread((char *)&length, sizeof(long), 1, fp) != 1
	   || (strtab = (char *)malloc((unsigned)length)) == NULL
	   || fread(strtab + sizeof(long), sizeof(char),
	      length - sizeof(long), fp) != length-sizeof(long)
	   || strtab[length - 1] != '\0' ) {
		fprintf(stderr, "error in obtaining string table");
	}
}

printname(ent)
struct syment *ent;
{
	if (ent->n_zeroes == 0L)
		printf("%s", strtab + ent->n_offset);
	else
		printf("%-8.8s", ent->n_name);
}

elf_printname(ent)
Elf32_Sym *ent;
{

#ifdef i386
	printf("%-13s	", (strtab + ent->st_name));
#else
	printf("%s", (strtab + ent->st_name));
#endif

}
 
getval(nmlist)
	char *nmlist;
	{
	struct stat ubuf,syb;
	int f;
	char *flnm = "/tmp/prf.adrfl";

	stat (nmlist,&syb);
	if ((stat(flnm,&ubuf) == -1) ||
		 (ubuf.st_mtime <= syb.st_ctime)) {
		perror("Bad address file, execute  prfld");
		exit(2);
	}
	if((f = open(flnm, O_RDONLY)) == -1) {
		perror("Cannot open /tmp/prf.adrfl");
		exit(errno);
	}
	if (read (f,&maxprf, sizeof (int)) == -1) {
		perror("Cannot read /tmp/prf.adrfl");
		exit(errno);
	}
	close(f);
	}
