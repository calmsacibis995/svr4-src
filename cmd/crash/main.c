/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)crash-3b2:main.c	1.18.18.1"

/*
 * This file contains code for the crash functions:  ?, help, redirect,
 * and quit, as well as the command interpreter.
 */

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/fs/s5dir.h>
#include <sys/var.h>
#ifndef i386
#include <sys/psw.h>
#include <sys/pcb.h>
#endif
#include <sys/user.h>
#ifdef i386
#include <sys/lock.h>
#endif
#include <setjmp.h>
#include <locale.h>
#include "crash.h"

#define NARGS 25		/* number of arguments to one function */
#define LINESIZE 256		/* size of function input line */

int mem;			/* file descriptor for dump file */
char *namelist = "/stand/unix";
char *dumpfile = "/dev/mem";
struct user *ubp;			/* pointer to ublock buffer */
FILE *fp;				/* output file pointer */
FILE *rp;				/* redirect file pointer */
int opipe = 0;				/* open pipe flag */
struct var vbuf;			/* var structure buffer */
int Procslot;				/* current process slot */
int Virtmode = 1;			/* virtual or physical mode flag*/
long pipesig;				/* pipe signal catch */

struct syment *File,*Vnode,*Vfs,*V,*Panic,
	*Curproc,*Streams,*Kmeminfo,*Km_pools;	/* namelist symbol pointers */

short N_TEXT;		/* used in symtab.c */
short N_DATA;		/* used in symtab.c */
short N_BSS;		/* used in symtab.c */
int active;	/* Flag set if crash is examining an active system */
jmp_buf	jmp,syn;	/* labels for jump */
void exit();


/* function calls */
extern int getas(),getbufhdr(),getbuffer(),getcallout(),getdblock(),getdis(),
	getgdp(),getinode(),getkfp(),getlcks(),getpage(),getdblk(),
	getmap(),getmess(),getvfsarg(),getnm(),getod(),getpcb(),
	getproc(),getqrun(),getqueue(),getquit(),getrcvd(),getptbl(),getprnode(),
	getrduser(),getsndd(),getsnode(),
	getsrmount(),getstack(),getstat(),getstream(),getstrstat(),
	gettrace(),getsymbol(),gettty(),getuser(),getvar(),getvnode(),getvtop(),
	getfuncs(),getbase(),gethelp(),getsearch(),getdbfree(),getmbfree(),
	getsearch(),getfile(),getdefproc(),getmode(),getredirect(),
	getsize(),getfindslot(),getfindaddr(),getvfssw(),getlinkblk(),
#ifdef i386
	getldt(),getgdt(),getidt(),getpanic(),gettest(),runq(),getplock(),
#endif
	getresrc(),getpty(),gethrt(),getclass(),getasync(),getevmm(),
	getevactive(),get_ufs_inode(),gettsdptbl(),getrtdptbl(),
	getdispq(),gettsproc(),getrtproc(),getkmastat(); 

/* function definition */
struct func {
	char *name;
	char *syntax;
	int (*call)();
	char *description;
};

/* function table */
/* entries with NA description fields should be removed in next release */
struct func functab[] = {
	"as","[-e] [-f] [-wfilename] [proc[s]]",
		getas,"address space structures",
	"async","[-wfilename] [-f]",
		getasync,"aio structures",
	"b"," ",getbuffer,"(buffer)",
	"base","[-wfilename] number[s]",
		getbase,"base conversions",
	"buf"," ",getbufhdr,"(bufhdr)",
	"buffer","[-wfilename] [-b|-c|-d|-x|-o|-i] (bufferslot |[-p] st_addr)",
		getbuffer,"buffer data",
	"bufhdr","[-f] [-wfilename] [[-p] tbl_entry[s]]",
		getbufhdr,"buffer headers",
	"c"," ",getcallout,"(callout)",
	"callout","[-wfilename]",
		getcallout,"callout table",
	"class","[-wfilename] [tbl_entry[s]]",
		getclass,"class table",
	"dbfree","[-wfilename]",
		getdbfree,"free data block headers",
	"dblock","[-e] [-wfilename] [[-p] dblk_addr[s]]",
		getdblk,"allocated stream data block headers",
	"defproc","[-wfilename] [-c | slot]",
		getdefproc,"set default process slot",
	"dis","[-wfilename] [-a] -c | st_addr [count]",
		getdis,"disassembler",
	"dispq","[-wfilename] [tbl_entry[s]]",
		getdispq, "dispq table",
	"ds","[-wfilename] virtual_address[es]",
		getsymbol,"data address namelist search",
	"evactive","[-wfilename] [-f] [event_name]",
		getevactive,"active event queue",
	"evmm","[-wfilename]",
		getevmm,"events memory management",
	"f"," ",getfile,"(file)",
	"file","[-e] [-f] [-wfilename] [[-p] address[es]]",
		getfile,"file table",
	"findaddr","[-wfilename] table slot",
		getfindaddr,"find address for given table and slot",
	"findslot","[-wfilename] virtual_address[es]",
		getfindslot,"find table and slot number for given address",
	"fs"," ",getvfssw,"(vfssw)",
	"gdp","[-e] [-f] [-wfilename] [[-p] tbl_entry[s]]",
		getgdp,"gdp structure",
	"help","[-wfilename] function[s]",
		gethelp,"help function",
	"hrt","[-wfilename]",
		gethrt,"high resolution timers",
	"i"," ",getinode,"(inode)",
	"inode","[-e] [-f] [-wfilename] [[-p] tbl_entry[s]]",
		getinode,"inode table",
	"kfp","[-wfilename] [-sprocess] [-r | value]",
		getkfp,"frame pointer for start of stack trace",
	"kmastat","[-wfilename]",
		getkmastat,"kernel memory allocator statistics",
	"l"," ",getlcks,"(lck)",
	"lck","[-e] [-wfilename] [[-p] tbl_entry[s]]",
		getlcks,"record lock tables",
	"linkblk","[-e] [-wfilename] [[-p] linkblk_addr[s]]",
		getlinkblk,"linkblk table",
	"m"," ",getvfsarg,"(vfs)",
	"map","[-wfilename] mapname[s]",
		getmap,"map structures",
	"mbfree","[-wfilename]",
		getmbfree,"free message block headers",
	"mblock","[-e] [-f] [-wfilename] [[-p] mblk_addr[s]]",
		getmess,"allocated stream message block headers",
	"mode","[-wfilename] [v | p]",
		getmode,"address mode",
	"mount"," ",getvfsarg,"(vfs)",
	"nm","[-wfilename] symbol[s]",
		getnm,"name search",
	"od","[-wfilename] [-c|-d|-x|-o|-a|-h] [-l|-t|-b] [-sprocess] [-p] st_addr [count]",
		getod,"dump symbol values",
	"p"," ",getproc,"(proc)",
	"page","[-e] [-wfilename] [[-p] tbl_entry[s]]",
		getpage,"page structure",
	"pcb","[-wfilename] [[-u | -k] [process] | -i [-p] st_addr]",
		getpcb,"process control block",
	"prnode","[-e] [-wfilename] [[-p] tbl_entry[s]]",
		getprnode,"proc node",
	"plock","[-t|-d|-u]",
		getplock,"lock crash process in memory",
	"proc","[-e] [-f] [-wfilename] [([-p] [-a] tbl_entry | #procid)... | -r)]",
		getproc,"process table",
	"q"," ",getquit,"(quit)",
	"ptbl","[-e] [-wfilename] [-sprocess] [[-p] pt_addr [count]]",
		getptbl,"page tables",
	"pty","[-e] [-f] [-wfilename] [-s] [-h] [-l] [([-p] tbl_entry)]",
		getpty,"pty structure",
	"qrun","[-wfilename]",
		getqrun,"list of servicable stream queues",
	"queue","[-e] [-f] [-wfilename] [[-p] queue_addr[s]]",
		getqueue,"allocated stream queues",
	"quit"," ",
		getquit,"exit",
	"rcvd","[-e] [-f] [-wfilename] [[-p] tbl_entry[s]]",
		getrcvd,"receive descriptor",
	"rd"," ",getod,"(od)",
	"rduser","[-e] [-f] [-wfilename] [[-p] tbl_entry[s]]",
		getrduser,"rcvd user table",
	"redirect","[-wfilename] [-c | filename]",
		getredirect,"output redirection",
	"runq"," ", runq, "runq",
	"resource","[-wfilename]", getresrc,"resource list",
	"rtdptbl","[-wfilename] [tbl_entry[s]]",
		getrtdptbl, "real time dispatcher parameter table",
	"rtproc","[-e] [-wfilename] [tbl_entry[s]]",
		getrtproc,"real time process table",
	"s"," ",getstack,"(stack)",
	"search","[-wfilename] [-mmask] [-sprocess] pattern [-p] st_addr length",
		getsearch,"memory search",
	"size","[-x] [-wfilename] structurename[s]",
		getsize,"symbol size",
	"sndd","[-e] [-f] [-wfilename] [[-p] tbl_entry[s]]",
		getsndd,"send descriptor",
	"snode","[-e] [-f] [-wfilename] [[-p] tbl_entry[s]]",
		getsnode,"special node",
	"srmount","[-wfilename] [-p] srmount_addr[s]",
		getsrmount,"server mount list",
	"stack","[-wfilename] [[-u | -k] [process] | -i [-p] st_addr]",
		getstack,"stack dump",
	"stat","[-wfilename]",
		getstat,"dump statistics",
	"stream","[-e] [-f] [-wfilename] [[-p] stream_addr[s]]",
		getstream,"allocated stream table slots",
	"strstat","[-wfilename]",
		getstrstat,"streams statistics",
	"t"," ",gettrace,"(trace)",
	"trace","[-wfilename] [[-r] [process] | -i [-p] st_addr]",
		gettrace,"kernel stack trace",
	"ts","[-wfilename] virtual_address[es]",
		getsymbol,"text address namelist search",
	"tsdptbl","[-wfilename] [tbl_entry[s]]",
		gettsdptbl, "time sharing dispatcher parameter table",
	"tsproc","[-e] [-wfilename] [tbl_entry[s]]",
		gettsproc,"time sharing process table",
	"tty","[-e] [-f] [-wfilename] [-l] [-ttype [[-p] tbl_entry[s]] | [-p] st_addr]",
		gettty,"tty structures",
	"u"," ",getuser,"(user)",
	"user","[-f] [-wfilename] [process]",
		getuser,"uarea",
	"ui"," ",get_ufs_inode,"(uinode)",
	"uinode","[-e] [-f] [-wfilename] [[-p] tbl_entry[s]]",
		get_ufs_inode,"inode table",
	"v"," ",getvar,"(var)",
	"var","[-wfilename]",
		getvar,"system variables",
	"vfs","[-e] [-wfilename] [[-p] address[es]]",
		getvfsarg,"mounted vfs list",
	"vfssw","[-wfilename] [[-p] tbl_entry[s]]",
		getvfssw,"virtual file system switch table",
	"vnode","[-wfilename] [-p] vnode_addr[s]",
		getvnode,"vnode list",
	"vtop","[-wfilename] [-sprocess] st_addr[s]",
		getvtop,"virtual to physical address",
#ifdef i386
	"ldt","[-e] [-wfilename] [process [slot [count]]]",
		getldt,"print local descriptor table",
	"idt","[-e] [-wfilename] [slot [count]]",
		getidt,"print interrupt decriptor table",
	"gdt","[-e] [-wfilename] [slot [count]]",
		getgdt,"print global decriptor table",
	"panic","[-wfilename] [process]", getpanic, "print panic information",
	"test","[mode]", gettest , "enter debug mode",
#endif
	"?","[-wfilename]",
		getfuncs,"print list of available commands",
	"!cmd"," ",NULL,"escape to shell",
	"hdr"," ",getbufhdr,"NA",
	"files"," ",getfile,"NA",
	"fp"," ",getkfp,"NA",
	"r9"," ",getkfp,"NA",
	"mnt"," ",getvfsarg,"NA",
	"dump"," ",getod,"NA",
	"ps"," ",getproc,"NA",
	"k"," ",getstack,"NA",
	"kernel"," ",getstack,"NA",
	"stk"," ",getstack,"NA",
	"ad"," ",gettty,"NA",
	"con"," ",gettty,"NA",
	"term"," ",gettty,"NA",
	"u_area"," ",getuser,"NA",
	"uarea"," ",getuser,"NA",
	"ublock"," ",getuser,"NA",
	"tunable"," ",getvar,"NA",
	"tunables"," ",getvar,"NA",
	"tune"," ",getvar,"NA",
	"calls"," ",getcallout,"NA",
	"call"," ",getcallout,"NA",
	"timeout"," ",getcallout,"NA",
	"time"," ",getcallout,"NA",
	"tout"," ",getcallout,"NA",
	"freelist"," ",getmess,"NA",
	NULL,NULL,NULL,NULL
};

char *args[NARGS];		/* argument array */
int argcnt;			/* argument count */
char outfile[100];		/* output file for redirection */
static int tabsize;		/* size of function table */

/* main program with call to functions */
main(argc,argv)
int argc;
char **argv;
{
	struct func *a,*f;
	int c,i,found;
	extern int opterr;
	int arglength;

	(void)setlocale(LC_ALL, "");
	if(setjmp(jmp))
		exit(1);
	fp = stdout;
	strcpy(outfile,"stdout");
	optind = 1;		/* remove in next release */
	opterr = 0;		/* suppress getopt error messages */

	for(tabsize = 0,f = functab; f->name; f++,tabsize++) 
		if(!strcmp(f->description,"NA"))  /* remove in next release */
			break;

	while((c = getopt(argc,argv,"d:n:w:")) !=EOF) {
		switch(c) {
			case 'd' :	dumpfile = optarg;
				 	break;
			case 'n' : 	namelist = optarg;
					break;
			case 'w' : 	strncpy(outfile,optarg,ARGLEN);
					if(!(rp = fopen(outfile,"a")))
						fatal("unable to open %s\n",
							outfile);
					break;
			default  :	fatal("usage: crash [-d dumpfile] [-n namelist] [-w outfile]\n");
		}
	}
	/* backward compatible code */
	if(argv[optind]) {
		dumpfile = argv[optind++];
		if(argv[optind])
			namelist = argv[optind++];
		if(argv[optind])
			fatal("usage: crash [-d dumpfile] [-n namelist] [-w outfile]\n");
	}
	/* remove in next release */
	if(rp)
		fprintf(rp,"dumpfile = %s, namelist = %s, outfile = %s\n",dumpfile,namelist,outfile);
	fprintf(fp,"dumpfile = %s, namelist = %s, outfile = %s\n",dumpfile,namelist,outfile);
	init();

	setjmp(jmp);

	for(;;) {
		getcmd();
		if(argcnt == 0)
			continue;
		if(rp) {
			fp = rp;
			fprintf(fp,"\n> ");
			for(i = 0;i<argcnt;i++)
				fprintf(fp,"%s ",args[i]);
			fprintf(fp,"\n");
		}
		found = 0;
		for(f = functab; f->name; f++) 
			if(!strcmp(f->name,args[0])) {
				found = 1;
				break;
			}
		if(!found) {
			arglength = strlen(args[0]);
			for(f = functab; f->name; f++) {
				if(!strcmp(f->description,"NA"))
					break;     /* remove in next release */
				if(!strncmp(f->name,args[0],arglength)) {
					found++;
					a = f;
				}
			}
			if(found) {
				if(found > 1)
					error("%s is an ambiguous function name\n",args[0]);
				else f = a;
			}	
		}
		if(found) {
			if(!strcmp(f->description,"NA")) /* remove in next release */
				pralias(f);
			if(setjmp(syn)) {
				while(getopt(argcnt,args,"") != EOF);
				if(*f->syntax == ' ') {
					for(a = functab;a->name;a++)
						if((a->call == f->call) &&
						(*a->syntax != ' '))
							error("%s: usage: %s %s\n",f->name,f->name,a->syntax);
				}
				else error("%s: usage: %s %s\n",f->name,f->name,f->syntax);
			}
			else (*(f->call))();
		}
		else prerrmes("unrecognized function name\n");
		fflush(fp);
		resetfp();
	}
}

/* returns argcnt, and args contains all arguments */
int
getcmd()
{
	char *p;
	int i;
	static char line[LINESIZE+1];
	FILE *ofp;
	
	ofp = fp;
	printf("> ");
	fflush(stdout);
	if(fgets(line,LINESIZE,stdin) == NULL)
		exit(0);
	line[LINESIZE] = '\n';
	p = line;
	while(*p == ' ' || *p == '\t') {
		p++;
	}
	if(*p == '!') {
		system(p+1);
		argcnt = 0;
	}
	else {
		for(i = 0; i < NARGS; i++) {
			if(*p == '\n') {
				*p = '\0';
				break;
			}
			while(*p == ' ' || *p == '\t')
				p++;
			args[i] = p;
			if(strlen(args[i]) == 1)
				break;
			if(*p == '!') {
				p = args[i];
				if(strlen(++args[i]) == 1)
					error("no shell command after '!'\n");
				pipesig = (long)signal(SIGPIPE,SIG_IGN);
				if((fp = popen(++p,"w")) == NULL) {
					fp = ofp;
					error("cannot open pipe\n");
				}
				if(rp != NULL)
					error("cannot use pipe with redirected output\n");
				opipe = 1;
				break;
			}
			if(*p == '(')
				while((*p != ')') && (*p != '\n'))
					p++;
			while(*p != ' ' && *p != '\n')
				p++;
			if(*p == ' ' || *p == '\t')
				*p++ = '\0';
		}
		args[i] = NULL;
		argcnt = i;
	}
}


/* get arguments for ? function */
int
getfuncs()
{
	int c;

	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	prfuncs();
}


/* print all function names in columns */

#define NCOL	5

int
prfuncs()
{
	int i,j,len;
	struct func *ff;
	char tempbuf[20];

	len = (tabsize + NCOL - 1) / NCOL;
	for(i = 0; i < len; i++) {
		ff = functab + i;
		for(j = 0; j < NCOL; j++) {
			if(*ff->description != '(')
				fprintf(fp,"%-15s",ff->name);
			else {
				tempbuf[0] = 0;
				strcat(tempbuf,ff->name);
				strcat(tempbuf," ");
				strcat(tempbuf,ff->description);
				fprintf(fp,"%-15s",tempbuf);
			}
			ff += len;
			if((ff - functab) >= tabsize)
				break;
		}
		fprintf(fp,"\n");
	}
	fprintf(fp,"\n");
}

/* get arguments for help function */
int
gethelp()
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
		do {
			prhelp(args[optind++]);
		}while(args[optind]);
	}
	else prhelp("help");
}

/* print function information */
int
prhelp(string)
char *string;
{
	int found = 0;
	struct func *ff,*a,*aa;

	for(ff=functab;ff->name;ff++) {
		if(!strcmp(ff->name,string)){
			found = 1;
			break;
		}
	}
	if(!found)
		error("%s does not match in function list\n",string);
	if(!strcmp(ff->description,"NA"))  /* remove in next release */
		pralias(ff);
	if(*ff->description == '(') {
		for(a = functab;a->name != NULL;a++)
			if((a->call == ff->call) && (*a->description != '('))
					break;
		fprintf(fp,"%s %s\n",ff->name,a->syntax);
		if(findstring(a->syntax,"tbl_entry"))
			fprintf(fp,"\ttbl_entry = slot number | address | symbol | expression | range\n");
		if(findstring(a->syntax,"st_addr"))
			fprintf(fp,"\tst_addr = address | symbol | expression\n");
		fprintf(fp,"%s\n",a->description);
	}
	else {
		fprintf(fp,"%s %s\n",ff->name,ff->syntax);
		if(findstring(ff->syntax,"tbl_entry"))
			fprintf(fp,"\ttbl_entry = slot number | address | symbol | expression | range\n");
		if(findstring(ff->syntax,"st_addr"))
			fprintf(fp,"\tst_addr = address | symbol | expression\n");
		fprintf(fp,"%s\n",ff->description);
	}
	fprintf(fp,"alias: ");
	for(aa = functab;aa->name != NULL;aa++)
		if((aa->call == ff->call) && (strcmp(aa->name,ff->name)) &&
			strcmp(aa->description,"NA"))
				fprintf(fp,"%s ",aa->name);
	fprintf(fp,"\n");
	fprintf(fp,"\tacceptable aliases are uniquely identifiable initial substrings\n");
}

/* find tbl_entry or st_addr in syntax string */
int
findstring(syntax,substring)
char *syntax;
char *substring;
{
	char string[81];
	char *token;

	strcpy(string,syntax);
	token = strtok(string,"[] ");
	while(token) {
		if(!strcmp(token,substring))
			return(1);
		token = strtok(NULL,"[] ");
	}
	return(0);
}

/* this function and all obsolete aliases should be removed in next release */
/* print valid function names for obsolete aliases */
int
pralias(ff)
struct func *ff;
{
	struct func *a;

	fprintf(fp,"Valid calls to this function are:  ");
	for(a = functab;a->name;a++)
		if((a->call == ff->call) && (strcmp(a->name,ff->name)) &&
			(strcmp(a->description,"NA")))
				fprintf(fp,"%s ",a->name);
	error("\nThe alias %s is not supported on this processor\n",
		ff->name);
}


/* terminate crash session */
int
getquit()
{
	if(rp)
		fclose(rp);
	exit(0);
}

/* get arguments for redirect function */
int
getredirect()
{
	int c;
	int close = 0;

	optind = 1;
	while((c = getopt(argcnt,args,"w:c")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 'c' :	close = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) 
		prredirect(args[optind],close);
	else prredirect(NULL,close);
}

/* print results of redirect function */
int
prredirect(string,close)
char *string;
int close;
{
	if(close)
		if(rp) {
			fclose(rp);
			rp = NULL;
			strcpy(outfile,"stdout");
			fp = stdout;
		}
	if(string) {
		if(rp) {
			fclose(rp);
			rp = NULL;
		}
		if(!(rp = fopen(string,"a")))
			error("unable to open %s\n",string);
		fp = rp;
		strncpy(outfile,string,ARGLEN);
	}
	fprintf(fp,"outfile = %s\n",outfile);
	if(rp)
		fprintf(stdout,"outfile = %s\n",outfile);
}

/* plock command - lock process in core */
int
getplock()
{
	int c, op = PROCLOCK;

	optind = 1;
	while((c = getopt(argcnt,args,"tdu")) !=EOF) {
		switch(c) {
			case 't' :	op = TXTLOCK;
					break;
			case 'd' :	op = DATLOCK;
					break;
			case 'u' :	op = UNLOCK;
					break;
			default  :	longjmp(syn,0);
		}
	}
	if (args[optind])
		longjmp(syn,0);
	if (plock(op) == -1)
		error("plock failed - can't %slock process\n",
			op == UNLOCK ? "un" : "");
}
