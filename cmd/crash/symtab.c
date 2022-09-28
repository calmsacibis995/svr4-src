/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash-3b2:symtab.c	1.11.7.1"

/*
 * This file contains code for the crash functions: nm, ds, and ts, as well
 * as the initialization routine rdsymtab.
 */

#include <a.out.h>
#include <stdio.h>
#include <string.h>
#include "crash.h"
#ifdef i386
#define MAINSTORE 0
#else 
#include <sys/sbd.h>
#endif
#include <elf.h>
#include <libelf.h>


extern	int	nmlst_tstamp ;		/* namelist timestamp */
extern char *namelist;
extern short N_TEXT,N_DATA,N_BSS;	/* used in symbol search */
struct syment *stbl;			/* symbol table */
int symcnt;				/* symbol count */
char *strtbl;				/* pointer to string table */
extern char *malloc();

int iscoff = 1;				/* presume namelist is COFF */

/* symbol table initialization function */

int
rdsymtab()
{
	FILE *np;
	struct filehdr filehdr;
	struct syment	*sp,
			*ts_symb ;
	struct scnhdr	scnptr ,
			boot_scnhdr ;
	u_int	i ,
		N_BOOTD ;
	char *str;
	long *str2;
	long strtblsize;

	/*
	 * Open the namelist and associate a stream with it. Read the file into a buffer.
	 * Determine if the file is in the correct format via a magic number check.
	 * An invalid format results in a return to main(). Otherwise, dynamically 
	 * allocate enough space for the namelist. 
	 */

	if(!(np = fopen(namelist, "r")))
		fatal("cannot open namelist file\n");
	if(fread((char *)&filehdr, FILHSZ, 1, np) != 1)
		fatal("read error in namelist file\n");
	if(filehdr.f_magic != I386MAGIC) {
		rewind(np);
		if (rdelfsym(np) != 0) {
			fatal("namelist not in a.out format\n");
		}	/* if rdelfsym != 0  . . . */
	}	/* if filehdr.f_magic . . . */
	if ( iscoff ){
		/*
		 * Read the section headers to find the section numbers
		 * for .text, .data, and .bss.  First seek past the file header 
		 * and optional header, then loop through the section headers
		 * searching for the names .text, .data, and .bss.
		 */
		N_TEXT=0;
		N_DATA=0;
		N_BSS=0;
		N_BOOTD=0 ;
		if(fseek(np, (long)(FILHSZ + filehdr.f_opthdr), 0) != 0
		  && fread((char *)&filehdr, FILHSZ, 1, np) != 1) {
			fatal("read error in section headers\n");
		}
	
		for(i=1; i<=filehdr.f_nscns; i++)
		{
			if(fread(&scnptr, SCNHSZ, 1, np) != 1)
				fatal("read error in section headers\n");
	
			if(strcmp(scnptr.s_name,_TEXT) == 0)
				N_TEXT = i ;
			else if(strcmp(scnptr.s_name,_DATA) == 0)
				N_DATA = i ;
			else if(strcmp(scnptr.s_name,_BSS) == 0)
				N_BSS = i ;
			else if(strcmp(scnptr.s_name,"boot") == 0)
			{
				/* save data section for later processing */
				N_BOOTD = 1 ;
				boot_scnhdr = scnptr ;
			}
	
		}
		if(N_TEXT == 0 || N_DATA == 0 || N_BSS == 0) 
			fatal(".text, .data, or .bss was not found in section headers\n");
	
		if(!(stbl=(struct syment *)malloc((unsigned)(filehdr.f_nsyms*SYMESZ))))
			fatal("cannot allocate space for namelist\n");
	
	/* 
		fprintf(fp,"\nfilehdr.f_nsyms=%d ,N_TEXT=%d ,N_DATA=%d ,N_BSS=%d\n",
			filehdr.f_nsyms,N_TEXT,N_DATA,N_BSS);  */
	
		/*
		 * Find the beginning of the namelist and read in the contents of the list.
		 *
		 * Additionally, locate all auxiliary entries in the namelist and ignore.
		 */
	
		fseek(np, filehdr.f_symptr, 0);
		symcnt = 0;
		for(i=0, sp=stbl; i < filehdr.f_nsyms; i++, sp++) {
			symcnt++;
			if(fread(sp, SYMESZ, 1, np) != 1)
				fatal("read error in namelist file\n");
			if(sp->n_numaux) {
				fseek(np,(long)AUXESZ*sp->n_numaux,1);
				i += sp->n_numaux;
			}
		}
	/* 
		fprintf(fp,"symcnt=%d\n",symcnt); */
		/*
		 * Now find the string table (if one exists) and
		 * read it in.
		 */
		if(fseek(np,filehdr.f_symptr + filehdr.f_nsyms * SYMESZ,0) != 0)
			fatal("error in seeking to string table\n");
		
		if(fread((char *)&strtblsize,sizeof(int),1,np) != 1)
			fatal("read error for string table size\n");
		
		if(strtblsize)
		{
			if(!(strtbl = (char *)malloc((unsigned)strtblsize)))
				fatal("cannot allocate space for string table\n");
	
			str2 = (long *)strtbl;
			*str2 = strtblsize;
	
			for(i = 0,str = (char *)((int)strtbl + (int)sizeof(long)); i < strtblsize - sizeof(long); i++, str++)
				if(fread(str, sizeof(char), 1, np) != 1)
					fatal("read error in string table\n");
		}
		else
			str = 0;
	
		/* save timestamp from data space of namelist file */
			
		if(!(ts_symb = symsrch("crash_sync")) || !N_BOOTD)
			nmlst_tstamp = 0 ;
		else
		{
			if(fseek(np,(long)(boot_scnhdr.s_scnptr + (ts_symb -> n_value - boot_scnhdr.s_paddr)),0) != 0)
				fatal("could not seek to namelist timestamp\n") ;
			if(fread((char *)&nmlst_tstamp,sizeof(int),1,np) != 1)
				fatal("could not read namelist timestamp\n") ;
		}
	}	/* if iscoff  */
	fclose(np);
}


/* find symbol */
struct syment *
findsym(addr)
unsigned long addr;
{
	struct syment *sp;
	struct syment *save;
	unsigned long value;

	value = MAINSTORE;
	save = NULL;

	for(sp = stbl; sp < &stbl[symcnt]; sp++) {
		if(((sp->n_sclass == C_EXT) || (sp->n_sclass == C_STAT)) && 
			((unsigned long)sp->n_value <= addr)
		  && ((unsigned long)sp->n_value > value)) {
			value = (unsigned long)sp->n_value;
			save = sp;
		}
	}
	return(save);
}

/* get arguments for ds and ts functions */
int
getsymbol()
{
	int c;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		do {prsymbol(args[optind++], 0);
		}while(args[optind]);
	}
	else longjmp(syn,0);
}

/* print result of ds and ts functions */
int
prsymbol(string, addr)
	char *string;
	unsigned long addr;
{
	struct syment *spd,*spt,*sp;
	static char temp[SYMNMLEN+1];

	spd = spt = sp = NULL;
	if (!addr) {
		if((addr = strcon(string,'h')) == -1)
			error("\n");
	}

	if(!(spd = findsym((unsigned long)addr)) &&
		!(spt = findsym((unsigned long)addr))) {
			if (string)
				prerrmes("%s does not match\n",string);
			else
				prerrmes("%x does not match\n",addr);
			return;
	}
	if(!spd) 
		sp = spt;
	else if(!spt) 
		sp = spd;
	else {
		if((addr - spt->n_value) < (addr - spd->n_value)) 
			sp = spt;
		else sp = spd;
	}
	if(sp->n_zeroes) {
		strncpy(temp,sp->n_name,SYMNMLEN);
		fprintf(fp,"%-8s",temp);
	}
	else fprintf(fp,"%s",strtbl+sp->n_offset);		
	fprintf(fp," + %x\n",addr - (long)sp->n_value);
}


/* search symbol table */
struct syment *
symsrch(s)
char *s;
{
	struct syment *sp;
	struct syment *found;
	static char tname[SYMNMLEN + 1];
	char *name;

	found = 0;


	for(sp = stbl; sp < &stbl[symcnt]; sp++) {
		if(((sp->n_sclass == C_EXT) || (sp->n_sclass == C_STAT)) &&
			((unsigned long)sp->n_value >= MAINSTORE)) {
			if(sp->n_zeroes)
				{
				strncpy(tname,sp->n_name,SYMNMLEN);
				name = tname;
			} else
				name = strtbl + sp->n_offset;
			if(!strcmp(name,s)) {
				found = sp;
				break;
			}
		}
	}
	return(found);
}

/* get arguments for nm function */
int
getnm()
{
	int c;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) 
		do { prnm(args[optind++]);
		}while(args[optind]);
	else longjmp(syn,0);
}


/* print result of nm function */
int
prnm(string)
char *string;
{
	char *cp;
	struct syment *sp;

	if(!(sp = symsrch(string))) {
		prerrmes("%s does not match in symbol table\n",string);
		return;
	}
	fprintf(fp,"%s   %08.8lx  ",string,sp->n_value);


	if(iscoff){
		if      (sp -> n_scnum == N_TEXT)
			cp = " text";
		else if (sp -> n_scnum == N_DATA)
			cp = " data";
		else if (sp -> n_scnum == N_BSS)
			cp = " bss";
		else if (sp -> n_scnum == N_UNDEF)
			cp = " undefined";
		else if (sp -> n_scnum == N_ABS)
			cp = " absolute";
		else
			cp = " type unknown";

		fprintf(fp,"%s\n", cp);

	} else {	/* Is ELF */

		if	(sp->n_scnum == N_ABS)
			cp = " absolute";
		else if (sp->n_value < 0x02004001)
			cp = " gate segment";
		else if (sp->n_value < 0x40000001)
			cp = " start segment";
		else if (sp->n_value < 0x40160001)
			cp = " text segment";
		else if (sp->n_value < 0x401a0001)
			cp = " data segment";
		else if (sp->n_value < 0x40300000)
			cp = " bss segment";
		else
			cp = " type unknown";
	}

	fprintf(fp,"%s (%s symbol)\n", cp,
		(sp->n_sclass == C_EXT ? "global" : "static/local"));
}

/*
**	Read symbol table of ELF namelist
*/

rdelfsym(fp)
FILE *fp;
{
	register int i;
	register Elf32_Sym *sy;
	register struct syment *sp;
	struct syment *ts_symb;
	Elf *elfd;
	Elf_Scn	*scn;
	Elf32_Shdr *eshdr;
	Elf32_Sym *symtab;
	Elf_Data *data;
	Elf_Data *strdata;
	int fd;
	int nsyms;

        if (elf_version (EV_CURRENT) == EV_NONE) {
		fatal("ELF Access Library out of date\n");
	}

	fd = fileno(fp);

	if ((lseek(fd, 0L, 0)) == -1L) {
		fatal("Unable to rewind namelist file\n");
	}

        if ((elfd = elf_begin (fd, ELF_C_READ, NULL)) == NULL) {
		fatal("Unable to elf begin\n");
	}

	if ((elf_kind(elfd)) != ELF_K_ELF) {
		elf_end(elfd);
		return (-1);
	}

	scn = NULL;
	while ((scn = elf_nextscn(elfd, scn)) != NULL) {

		if ((eshdr = elf32_getshdr(scn)) == NULL) {
			elf_end(elfd);
			fatal("cannot read section header\n");
		}

		if (eshdr->sh_type == SHT_SYMTAB) {
			break;		/* Can only do 1 symbol table */
		}
	}

		/* Should have scn and eshdr for symtab */

	data = NULL;
	if (((data = elf_getdata(scn, data)) == NULL) ||
		(data->d_size == 0) || (!data->d_buf)) {
			elf_end(elfd);
			fatal("can not read symbol table\n");
	}

	symtab = (Elf32_Sym *)data->d_buf;

	nsyms = data->d_size / sizeof(Elf32_Sym);

	/*
	**	get string table
	*/

	if ((scn = elf_getscn(elfd, eshdr->sh_link)) == NULL) {
		elf_end(elfd);
		fatal("ELF strtab read error\n");
	}

	strdata = NULL;
	if (((strdata = elf_getdata(scn, strdata)) == NULL) ||
		(strdata->d_size == 0) || (!strdata->d_buf)) {
			elf_end(elfd);
			fatal("string table read failure\n");
	}

	if ((strtbl = malloc(strdata->d_size)) == NULL)
		fatal("cannot allocate space for string table\n");

	(void)memcpy(strtbl, strdata->d_buf, strdata->d_size);

	if((stbl=(struct syment *)malloc((unsigned)(nsyms*sizeof(SYMENT)))) == NULL)
		fatal("cannot allocate space for namelist\n");

	/*
	**	convert ELF symbol table info to COFF
	**	since rest of pgm uses COFF
	*/

	symcnt = 0;
	sp = stbl;
	sy = symtab;
	for (i = 0; i < nsyms; i++, sy++) {

		if ((ELF32_ST_TYPE(sy->st_info)) == STT_FILE)
			continue;

		if ((ELF32_ST_TYPE(sy->st_info)) == STT_SECTION)
			continue;

		sp->n_zeroes = 0L;
		sp->n_offset = sy->st_name;
		sp->n_value = sy->st_value;
		sp->n_scnum = sy->st_shndx;
		sp->n_type = ELF32_ST_TYPE(sy->st_info);
		sp->n_sclass =  ELF32_ST_BIND(sy->st_info);
		sp->n_numaux = 0;

		if (sp->n_scnum == SHN_ABS)
			sp->n_scnum = N_ABS;

		if (sp->n_sclass == STB_GLOBAL)
			sp->n_sclass = C_EXT;
		else
			sp->n_sclass = C_STAT;

		sp++;
		symcnt++;
	}

	/* Get time stamp */

	if(!(ts_symb = symsrch("crash_sync")))
                nmlst_tstamp = 0 ;
        else {

		if ((scn = elf_getscn(elfd, ts_symb->n_scnum)) == NULL) {
			elf_end(elfd);
			fatal("ELF timestamp scn read error\n");
		}

		if ((eshdr = elf32_getshdr(scn)) == NULL) {
			elf_end(elfd);
			fatal("cannot read timestamp section header\n");
		}

		if ((lseek(fd,
			(long)(ts_symb->n_value - eshdr->sh_addr + eshdr->sh_offset),
				0)) == -1L)
                        fatal("could not seek to namelist timestamp\n") ;

                if ((read(fd, (char *)&nmlst_tstamp, sizeof(nmlst_tstamp))) !=
				sizeof(nmlst_tstamp))
                        fatal("could not read namelist timestamp\n");
        }

	iscoff = 0;

	elf_end(elfd);

	return(0);
}
