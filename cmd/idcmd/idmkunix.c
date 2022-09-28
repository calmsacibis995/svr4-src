/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)idcmd:idmkunix.c	1.3"

/* Config for Installable Drivers and Tunable Parameters */
/*
 *  Version for PC 6300+ style driver installation on
 *  the 386.   
 *
 *  ELF support changes:
 *      bypass cpp, comp, as, by doing cc -c
 *	use direct to compile
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "inst.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* directories */
#define IN	0
#define OUT	1
#define FULL	2
/* awk command */
#define	AWKCMD	"awk '{print substr($1, index($1, \"pack.d\")+7,80)}' ifile > itmp; /bin/mv itmp ifile"
/* error messages */
#define USAGE	"Usage: idmkunix [-i dir] [-o dir] [-r dir] [-c cc -l ld] [-Ddefine] [-Udefine]\n"
#define	EXECF	"Can not exec process '%s'. Errno = %d\n"
#define FORKF	"Can not fork process '%s'. Errno = %d\n"
#define FOPEN	"Can not open '%s' for mode '%s'\n"
#define EXIST	"Directory '%s' does not exist\n"
#define EFILE	"'Driver.o' does not exist in driver package '%s'\n"
#define SUFFIX	"'%s' does not contain the proper suffix for compilation\n"
#define BADCOMP "'%s' will not compile properly\n"
#define LINK	"Can not link-edit unix\n"
#define CROSS	"Must use '-c cc_command' and '-l ld_command' simultaneously\n"
#define INCMP	"Expecting path name of compiler or link-editor: '%s'\n"
#define ELFCHK	"Can't open pipe for ELF check\n"
#define MOVE	"space.o or stubs.o move failed \n"
#define FCNT	"Can not set close-on-exec flag, fildes '%d'\n"

/* CCS components */
char *cpp = "/lib/idcpp";
char *comp = "/lib/idcomp";
char *as = "/bin/idas";
char *ld = "/bin/idld";
char *cc = "/bin/idcc"; /* ELF support */
char *new_acomp = "-Y0,";	/* ELF support */
char *new_optim = "-Y2,";	/* ELF support */
char *new_as = "-Ya,";	/* ELF support */

char cpp_path[256];
char comp_path[256];
char as_path[256];
char ld_path[256];
char cc_path[256]; /* ELF */
char new_acomp_op[256]; /* ELF */
char new_optim_op[256]; /* ELF */
char new_as_op[256]; /* ELF */

char root[80];			/* rootdir passed in on command line */
char include[80];		/* header file location, using new ROOT */

/* input file names */
char	*pfile = "direct";	/* list of configured driver packages */

/* output file names */
char	*ifile = "ifile";	/* ifile for link editor */

/* flags */
int iflag = 0;			/* specified input directory */
int oflag = 0;			/* specified output directory */
int rflag = 0;			/* root directory specified */
int debug = 0;			/* debugging flag */
int cflag = 0;			/* cross-compiler specified */
int lflag = 0;			/* cross-link-editor specified */
int tmpexist = 0;		/* set if temp file names created */
int elfflag = 0;		/* ELF ccs flag */

/* buffers */
char input[512];		/* input directory */
char output[512];		/* output directory */
char current[512];		/* current directory */
char temp[80];			/* temporary buffer */
char cross[512];		/* path name of cross-compiler */
char linked[512];		/* path name of cross-link-editor */
char errbuf[100];		/* hold error messages */
char temp1[100],		/* temporray file names ... */
     temp2[100];		/* used by CCS components */

extern int errno;
extern char *optarg;
char *lastchar();

extern char *getcwd();
extern void exit();

/*
** Allow for upto MAXDEF extra -D/-U on the idmkunix command line.
*/
#define MAXDEF 5
char *predef[MAXDEF];
int pd=0;
extern char *malloc();

main(argc, argv)
int argc;
char *argv[];
{
	int m;

	while ((m = getopt(argc, argv, "?#i:o:c:l:r:D:U:")) != EOF)
	{
		switch (m)
		{
		case '#':
			debug++;
			break;
		case 'i':
			strcpy(input, optarg);
			iflag++;
			break;
		case 'o':
			strcpy(output, optarg);
			oflag++;
			break;
		case 'c':
			strcpy(cross, optarg);
			cflag++;
			break;
		case 'l':
			strcpy(linked, optarg);
			lflag++;
			break;
		case 'r':
			strcpy(root, optarg);
			rflag++;
			break;
		case 'D':
		case 'U':
			if (pd < MAXDEF) {
			   predef[pd] = malloc((unsigned) (strlen(optarg)+3));
			   if (m == 'D')
				strcpy(predef[pd], "-D");
			   else
				strcpy(predef[pd], "-U");
			   strcat(predef[pd], optarg);
			   pd++;
			} else
			   fprintf(stderr,"Out of Defines, %s is ignored\n", optarg);
			break;
		case '?':
			fprintf(stderr, USAGE);
			exit(1);
		}
	}

	/* Construct path names for CCS components,
	 * and input and output directories.
	 */
	(void)getcwd(current, sizeof(current));

	if (rflag) {
		strcpy(ld_path, root);
		strcat(ld_path, ld);
	} else
		strcpy(ld_path, ld);

	/*
	 *  Check for ELF
	 */
	elfflag=elfcheck(ELFVERSTR); /* uses ld_path */
	if (debug) {
		if (elfflag)
			fprintf(stderr, "Compilation System type is ELF\n");
		else
			fprintf(stderr, "Compilation System type is COFF\n");
	}

	if( rflag ) {
		if (!elfflag) {
			strcpy(cpp_path, root);
			strcat(cpp_path, cpp);
			strcpy(comp_path, root);
			strcat(comp_path, comp);
			strcpy(as_path, root);
			strcat(as_path, as);
		} else {
			strcpy(cc_path, root);
			strcat(cc_path, cc);

			strcpy(new_as_op, new_as);
			strcat(new_as_op, root);
			strcat(new_as_op, "/bin");

			strcpy(new_optim_op, new_optim);
			strcat(new_optim_op, root);
			strcat(new_optim_op, "/lib");

			strcpy(new_acomp_op, new_acomp);
			strcat(new_acomp_op, root);
			strcat(new_acomp_op, "/lib");
		}
		strcpy(include,"-I");
		strcat(include, root);
		strcat(include, "/usr/include");

		sprintf(temp, "%s%s/%s", root, ROOT, BUILD);
	} else {
		if (elfflag) {
			strcpy(cc_path, cc);

			strcpy(new_as_op, new_as);
			strcat(new_as_op, "/bin");

			strcpy(new_optim_op, new_optim);
			strcat(new_optim_op, "/lib");

			strcpy(new_acomp_op, new_acomp);
			strcat(new_acomp_op, "/lib");
		} else {
			strcpy(cpp_path, cpp);
			strcpy(comp_path, comp);
			strcpy(as_path, as);
		}
		sprintf(include, "%s%s", "-I", "/usr/include");
		sprintf(temp, "%s/%s", ROOT, BUILD);
	}

	getpath(iflag, input, temp);
	getpath(oflag, output, temp);

	if (cflag ^ lflag)
	{
		sprintf(errbuf, CROSS);
		error();
	}

	if (debug) {
		fprintf(stderr, "Input: %s\nOutput: %s\n",
			input, output);
		if (cflag)
			fprintf(stderr, "Cross-compiler=%s\nLink-editor=%s\n",
				cross, linked);
	}

	/* search driver package directories */
	search();

	/* compile files in output directory */
	compout();

	/* link edit object modules */
	linkedit();

	exit(0);
}



/* print error message and exit */

error()
{
	/* check if temporary files exist */
	if (tmpexist) {
		unlink(temp1);
		unlink(temp2);
	}

	fprintf(stderr, "ERROR: %s\n", errbuf);

	exit(1);
}



/* construct full path name */

getpath(flag, buf, def)
int flag;
char *buf, *def;
{
	switch (flag) {
	case 0:		/* use default value */
		strcpy(buf, def);
		break;
	case 1:		/* path name specified */
		if (chdir(buf) != 0) {
			sprintf(errbuf, EXIST, buf);
			error();
		}
		(void)getcwd(buf, 512);  /* sizeof input[], output[] */
		(void)chdir(current);
		break;
	}
}



/* open a file */

FILE *
open1(file, mode, dir)
char *file, *mode;
int dir;
{
	char path[100];
	FILE *fp;
	char *p;
	int ret;

	switch (dir) {
	case IN:
		sprintf(path, "%s/%s", input, file);
		p = path;
		break;
	case OUT:
		sprintf(path, "%s/%s", output, file);
		p = path;
		break;
	case FULL:
		p = file;
		break;
	}

	if (debug)
		fprintf(stderr, "Open: mode=%s path=%s\n", mode, p);

	if ((fp = fopen(p, mode)) == NULL) {
		sprintf(errbuf, FOPEN, p, mode);
		error();
	}

	if ((ret = fcntl(fileno(fp), F_SETFD, 1)) == -1) {
		sprintf(errbuf, FCNT, fileno(fp));
		error();
	}
	return(fp);
}



/* Go to directories listed in file 'direct'.
 * - compile 'Space.c' if it exists.
 * - check that Driver.o exists.
 *   output is ifile 
 */
search()
{
	FILE *ifp, *pp;
	char buff[100];
	char direct[100];
	int i;
	char *cptr;

	/* These must be loaded, but aren't listed in
	 * the mdevice file.  They always go in.
	 */
	static char *coredirs[] = {
		"/etc/conf/pack.d/kernel" ,
		"/etc/conf/pack.d/pic" ,
		NULL
	};

	ifp = open1(ifile, "w", OUT);		/* ifile */
	chmod(ifile, 0644);

	for( i=0; coredirs[i] != NULL; i++ )
	{
		if( rflag )
		{
			strcpy(direct, root);
			strcat(direct, coredirs[i]);
		}
		else
			strcpy(direct, coredirs[i]);

		wr_ifile(direct, ifp);
	}
	
	chdir (current);
	/* check files in driver packages */
	pp  = open1(pfile, "r", IN);
	while (fgets(buff, 100, pp) != NULL)
	{
		/* extract string (remove white space) */
		sscanf(buff, "%s", direct);

		cptr = lastchar(direct);
		if( *cptr == 'c' )
		{
			compile(direct,1);
			*cptr = 'o';
		}
		fprintf(ifp,"%s\n", direct);
	}
	fclose(pp);
	fclose(ifp);

}

/*
 * Write files in the directory 'name' to the 'ifile' used for link-editing
 * the kernel we're building.  Compile the files that need
 * it. 
 */
wr_ifile(name,ifp)
char *name;
FILE *ifp;
{
	struct stat bstat;
	char buff[100];

	if (debug)
		fprintf(stderr, "search '%s'\n", name);

	/* move to driver package directory */
	if (chdir(name) != 0) {
		sprintf(errbuf, EXIST, name);
		error();
		return;
	}

	/* 
	 * locore.o and start.o must be added for elf linkedit 
	 * They must appear at the very beginning.
	 */
	if (elfflag) {
		if (stat("syms.o", &bstat) == 0 ) 
			fprintf(ifp, "%s/syms.o\n", name);

		if (stat("start.o", &bstat) == 0 ) 
			fprintf(ifp, "%s/start.o\n", name);

		if (stat("locore.o", &bstat) == 0 ) 
			fprintf(ifp, "%s/locore.o\n", name);
	}

	/* compile space.c and add space.o to ifile */
	if (stat("space.c", &bstat) == 0) {
		sprintf(buff, "%s/space.c", name);
		compile(buff, 0);
		fprintf(ifp, "%s/space.o\n", name);
	}

	/* check for Driver.o and add to ifile */
	/* if no Driver.o, check for stubs.c/o */
	if (stat("Driver.o", &bstat) == 0 )
	{
		fprintf(ifp, "%s/Driver.o\n", name);
	}
	else if (stat("stubs.c", &bstat) == 0)
	{
		sprintf(buff, "%s/stubs.c", name);
		compile(buff, 0);
		fprintf(ifp, "%s/stubs.o\n", name);
	}

	/* The next files are only in the kernel directory. */
        /* locore is in vuifile alredy                      */

	/* check for os.o and add to ifile */
	if (stat("os.o", &bstat) == 0 )
	{
		fprintf(ifp, "%s/os.o\n", name);
	}

	/* check for io.o and add to ifile */
	if (stat("io.o", &bstat) == 0 )
	{
		fprintf(ifp, "%s/io.o\n", name);
	}

	/* check for fs.o and add to ifile */
	if (stat("fs.o", &bstat) == 0 )
	{
		fprintf(ifp, "%s/fs.o\n", name);
	}

	/* check for vm.o and add to ifile */
	if (stat("vm.o", &bstat) == 0 )
	{
		fprintf(ifp, "%s/vm.o\n", name);
	}

#if 0	/*
	 *  These kern modules are move to their own
	 *  pack.d directories
	 */
	/* check for gendisp.o and add to ifile */
	if (stat("gendisp.o", &bstat) == 0 )
	{
		fprintf(ifp, "%s/gendisp.o\n", name);
	}
         
	/* check for rt.o and add to ifile */
	if (stat("rt.o", &bstat) == 0 )
	{
		fprintf(ifp, "%s/rt.o\n", name);
	}
	
	/* check for ts.o and add to ifile */
	if (stat("ts.o", &bstat) == 0 )
	{
		fprintf(ifp, "%s/ts.o\n", name);
	}

	/* check for elf.o and add to ifile */
	if (stat("elf.o", &bstat) == 0 )
	{
		fprintf(ifp, "%s/elf.o\n", name);
	}

	/* check for coff.o and add to ifile */
	if (stat("coff.o", &bstat) == 0 )
	{
		fprintf(ifp, "%s/coff.o\n", name);
	}

	/* check for intp.o and add to ifile */
	if (stat("intp.o", &bstat) == 0 )
	{
		fprintf(ifp, "%s/intp.o\n", name);
	}
#endif

 	return;
}


/* Create a process and execute command.
 * Command must be a.out and not shell.
 * Return 0 on success, -1 on failure.
 */

proc(cmd)
char *cmd[];
{
	static int id, w;
	static int stat;

	if (debug) {
		char **p;

		fprintf(stderr, "proc: ");
		for (p = cmd; *p != NULL; p++)
			fprintf(stderr, "%s ", *p);
		fprintf(stderr, "\n");
	}

	if ((id = fork()) == 0) {
		execv(cmd[0], cmd);
		sprintf(errbuf, EXECF, *cmd, errno);
		error();
	} else if (id  == -1) {
		sprintf(errbuf, FORKF, *cmd, errno);
		error();
	} else
		w = wait(&stat);
	if (w == id && stat == 0)
		return(0);

	return(-1);
}


/*  
 *  ELF support put in, .o file is assumed
 */
compile(file, mvflag)		/* mvflag  gives a hint for special cases (coredirs[]) */
char *file;
int mvflag;
{
	static char buf[200];
	static char movbuf[200];
	static char ofile[200];
	char *p,*q;
	char *mv="/bin/mv";
	char *mvtarget;
	int rc;
	static int initialize = 1;

	/* cross-compiler */
	static char *crosscc[] = {
		"/bin/sh",
		"-c",
		buf,
		NULL,
	};

	/* preprocessor */
	static char *preprocess[] = {
		cpp_path,
		NULL,
		temp1,
		"-DiAPX386",	/* Always on.  CSDS/80386 dependent */
#ifdef MB1 
		"-DMB1",
#else
#ifdef  MB2 
		"-DMB2",
#else
		"-DAT386",
#endif /* MB2 */
#endif /* MB1 */
#ifdef MBUS
		"-DMBUS",
#endif /* MBUS */
#ifdef VPIX
		"-DVPIX",
#endif /* VPIX */
#ifdef MERGE386
		"-DMERGE386",
#endif /* MERGE386 */
#ifdef BLTCONS
		"-DBLTCONS",
#endif /* BLTCONS */
#ifdef EVC
		"-DEVC",
#endif /* EVC */
#ifdef AT380
		"-DAT380",
#endif /* AT380 */
		"-DSYSV",	/* Always On */
		"-D_KERNEL",	/* Always On */
		"-DINKERNEL",	/* Always On */
#ifdef WEITEK
		"-DWEITEK",
#endif /* WEITEK */
		NULL,  /*  there are 5 nulls because MAXDEF is 5; must match */
		NULL,
		NULL,
		NULL,
		NULL,
		buf,
		include,
		NULL,
		NULL
	};
	
	/* comp - instead of front and back */
	static char *comp[] = {
		comp_path,
		"-i", temp1,
		"-o", temp2, "-f", NULL, "-ds", "-dl", NULL
	};
	
	/* assembler */
	static char *assembler[] = {
		as_path, "-dl", "-o", NULL, temp2, NULL
	};

	/* for ELF build, just the cc command line array */
	static char *elfcc[] = {
		cc_path,
		NULL,
		"-DiAPX386",	/* Always on.  CSDS/80386 dependent */
#ifdef MB1 
		"-DMB1",
#else
#ifdef  MB2 
		"-DMB2",
#else
		"-DAT386",
#endif /* MB2 */
#endif /* MB1 */
#ifdef VPIX
		"-DVPIX",
#endif /* VPIX */
#ifdef MERGE386
		"-DMERGE386",
#endif /* MERGE386 */
#ifdef BLTCONS
		"-DBLTCONS",
#endif /* BLTCONS */
#ifdef EVC
		"-DEVC",
#endif /* EVC */
#ifdef AT380
		"-DAT380",
#endif /* AT380 */
		"-DSYSV",	/* Always On */
		"-D_KERNEL",	/* Always On */
		"-DINKERNEL",	/* Always On */
#ifdef WEITEK
		"-DWEITEK",
#endif /* WEITEK */
		NULL,  /*  there are 5 nulls because MAXDEF is 5; must match */
		NULL,
		NULL,
		NULL,
		NULL,
		"-c",
		new_acomp_op,
		new_as_op,
		buf,
		include,
		NULL,
		NULL
	};
	char	**gcomp; /* generic pointer: COFF-preprocess, ELF-cc */
/* to by pass compilation
	return;
*/
#if 0
		/* 
	 	 * this should be deleted since -c is no longer valid
	 	 * for idmkunix.c 
	 	 */
		 /* determine if in cross-environment */
		if (cflag) {
			sprintf(buf, "%s -Ml -c -I%s %s", cross, output, file);

			if (proc(crosscc)) {
				sprintf(errbuf, BADCOMP, file);
				error();
			}
			return;
		}
#endif
	p = lastchar(file);
	if (p == file)		/* file is empty */
		return;
	if (p[-1] != '.') {
		sprintf(errbuf, SUFFIX, file);
		error();
	}

	/* preprocessor */
	sprintf(buf, "-I%s", output);

	if (elfflag) {
		gcomp=elfcc;
		strcpy(ofile, file);
		q=lastchar(ofile);
		*q='o';
		mvtarget=q-6;  /* point at space or stubs */
	}else {
		gcomp=preprocess;
		/* create temp file names and set flag */
		tmpnam(temp1);
		tmpnam(temp2);
		tmpexist = 1;
	}
	gcomp[1] = file;
	if (initialize) {
		for (rc=0; gcomp[rc] != NULL; rc++) 
			;
	   	pd--;
	   	for (; pd >= 0; pd--, rc++)
			gcomp[rc] = predef[pd];
	   	/* Shift all the NULLS away */
	   	pd = rc;
	   	for (; gcomp[rc] == NULL; rc++)
			;
	   	for ( ; gcomp[rc] != NULL; rc++, pd++)
			gcomp[pd] = gcomp[rc];
	   	initialize = 0;
	}
	if (!elfflag) 
		gcomp[pd] = (*p == 's') ? "-P" : NULL;

	rc=proc(gcomp);

	if (rc) {
		if (debug) {
			if (elfflag)
				fprintf(stderr, "cc: rc=%d\n", rc);
			else
				fprintf(stderr, "preprocessor: rc=%d\n", rc);
		}
	} else { 
		if (!elfflag) {
		     switch (*p) {
			case 'c':
				comp[6] = file;
				if (rc = proc(comp)) {
					if (debug)
						fprintf(stderr, "comp: rc=%d\n", rc);
					break;
				}

			case 's':
				/* Assemble */
				/* .s files use preprocessor output in temp1 */
				assembler[4] = (*p == 's') ? temp1 : temp2;

				/* output file should have .o suffix */
				*p = 'o';
				assembler[3] = file;

				if (rc = proc(assembler)) {
					if (debug)
						fprintf(stderr, "assembler: rc=%d\n", rc);
					break;
				}
				break;

			default:
				sprintf(errbuf, SUFFIX, file);
				error();
				break;
			} 
		} else { 	/* elfflag */
			/* check if  mov is appropriate */
			if (mvflag && (strncmp(mvtarget, "space", 5)==0 ||
			    strncmp(mvtarget, "stubs", 5)==0)) {
				if (debug) {
					fprintf(stderr,"/bin/ls of cf.d: \n");
					strcpy(movbuf, "/bin/ls s*.o");
					if (system(movbuf) != 0)
						sprintf(errbuf, "LS of s*.o failed\n");
				}
				sprintf(movbuf, "%s %s %s", mv, mvtarget, ofile);
				if (system(movbuf) != 0) {
					sprintf(errbuf, MOVE);
				}
			} else 
				if (debug)
					fprintf(stderr, 
					"mv not needed for this compile\n");
		}
	}

	if (!elfflag) {
		/* erase temporary files */
		unlink(temp1);
		unlink(temp2);
		tmpexist=0;
	}

	if (rc) {
		sprintf(errbuf, BADCOMP, file);
		error();
	}
}



/* compile files in output directory */

compout()
{
	if (chdir(output) != 0) {
		sprintf(errbuf, EXIST, output);
		error();
	}

	compile("fsconf.c", 1);
	compile("conf.c", 1);
	compile("vector.c", 1);
}



/* link edit object modules and create 'unix' */

linkedit()
{
#if 0  /* cflag is not supported */
	static char buf[200];
	static char *crossld[] = {
		"/bin/sh",
		"-c",
		buf,
		NULL
	};
#endif
	static char *link[] = {
		ld_path,
		"-o", "unix", "-e", "_start", "vuifile",
		"conf.o", "fsconf.o",
		"vector.o", "ifile", NULL
	};
	char elfbuf[256];
	static char awkbuf[200];
	char **p;

#if 0	/* dead code, cflag no longer supported */
	sprintf(buf, "%s %s %s",
		linked,
		"-o unix -e _start vuifile conf.o fsconf.o",
		 "vector.o ifile -la");
#endif 
	if (elfflag) {
		sprintf (elfbuf, 
			"%s -dn -o %s/unix -e _start -M%s/kernmap `cat %s/%s` \
			 %s/conf.o %s/fsconf.o %s/vector.o",
			ld_path, "../cf.d", "../cf.d", "../cf.d", ifile,
			"../cf.d", "../cf.d", "../cf.d");
		if (debug)
			fprintf(stderr,"ELF link edit command line is %s\n", 
			elfbuf);
		strcpy(awkbuf, AWKCMD);
		if (system(awkbuf) != 0) {
			sprintf(errbuf, "ifile shrink by AWK failed\n");
			error();
		}

		if (chdir("../pack.d") != 0) {
			sprintf(errbuf, "can not chdir to pack.d\n");
			error();
		}
		if (system(elfbuf) != 0){
			chdir(current);
			sprintf(errbuf, LINK);
			error();
		}
		chdir(current);
			
		
	} else {
#if 0
		p = cflag ? crossld : link;
#endif
		p=link;
		if (proc(p)) {
			sprintf(errbuf, LINK);
			error();
		}
	}
}


/* return a pointer to the last character in a string */

char *
lastchar(str)
char *str;
{
	char *p;

	for (p = str; *p != NULL; p++);
	return (p == str ? p : p - 1);
}

/* check the version string returned by ld */
int
elfcheck (v_string)
char *v_string;
{
	FILE	*ptr;
	char	cmd[128];
	char	buf[128];

	buf[0]='\0';
	sprintf(cmd, "%s -V 2>&1 | /bin/grep %s ", ld_path, v_string);
	if (debug)
		fprintf(stderr, "Version check done on %s\n", cmd);
	ptr = popen(cmd, "r");
	if (ptr == NULL) {
		sprintf(errbuf, ELFCHK);
		error();
	}
	fgets(buf, 100, ptr);
	pclose(ptr);
	return((strlen(buf) != 0) ? TRUE : FALSE);
}
