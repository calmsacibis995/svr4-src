/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)file:file.c	1.17.1.20"
/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#include	<ctype.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdio.h>
#include        <stdlib.h>
#include        <limits.h>
#include        <locale.h>
#include        <archives.h>
#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/mkdev.h>
#include	<sys/stat.h>

/*
**	Types
*/

#define	BYTE	0
#define	SHORT	2
#define	LONG	4
#define	STR	8

/*
**	Opcodes
*/

#define	EQ	0
#define	GT	1
#define	LT	2
#define	STRC	3	/* string compare */
#define	ANY	4
#define AND	5
#define NSET	6	/* True if bit is not set */
#define	SUB	64	/* or'ed in, SUBstitution string, 
			 * for example %ld, %s, %lo 
			 * mask: with bit 6 on, used to locate print formats 
			 */
/*
**	Misc
*/

#define BUFSZ	128
#define	FBSZ	512
#define	reg	register
#define	NBLOCK	20

/* Assembly language comment char */
#ifdef pdp11
#define ASCOMCHAR '/'
#else
#define ASCOMCHAR '#'
#endif
/*
**	Structure of magic file entry
*/

struct	entry	{
	char	e_level;	/* 0 or 1 */
	long	e_off;		/* in bytes */
	char	e_type;		/* BYTE, SHORT, LONG, STR */
	char	e_opcode;	/* EQ, GT, LT, ANY, AND, NSET */
	union	{
		long	num;
		char	*str;
	}	e_value;
	char	*e_str;
};

typedef	struct entry	Entry;

Entry	*mtab;
char	fbuf[FBSZ];
char	*mfile = "/etc/magic";
						/* Fortran */
char	*fort[] = {
	"function","subroutine","common","dimension","block","integer",
	"real","data","double",0};
char	*asc[] = {
	"sys","mov","tst","clr","jmp",0};
						/* C Language */
char	*c[] = {
	"int","char","float","double","short","long","unsigned","register",
	"static","struct","extern", 0};
						/* Assembly Language */
char	*as[] = {
	"globl","byte","even","text","data","bss","comm",0};
char	*strchr();
char	*strcpy();
long	atolo();

char ops[] = {'=','*','&','-','!','~','+','/','%','>','<','^','|',0};

/* start for MB env */
wchar_t wchar;
int     length;
int     IS_ascii;
int     Max;
/* end for MB env */
int     i = 0;
int	fbsz;
int	ifd;
int	tret;
int	hflg = 0;
void	exit();
extern int stat(), lstat();
extern int errno;
extern char *sys_errlist[];

#define	prf(x)	printf("%s:%s", x, strlen(x)>6 ? "\t" : "\t\t");

main(argc, argv)
int  argc;
char **argv;
{
	reg	char	*p;
	reg	int	ch;
	reg	FILE	*fl;
	reg	int	cflg = 0, eflg = 0, fflg = 0;
	auto	char	ap[BUFSZ];
	extern	int	optind;
	extern	char	*optarg;

	(void)setlocale(LC_ALL, "");

	while((ch = getopt(argc, argv, "chf:m:")) != EOF)
	switch(ch) {
	case 'c':
		cflg++;
		break;

	case 'f':
		fflg++;
		if ((fl = fopen(optarg, "r")) == NULL) {
			fprintf(stderr, "cannot open %s\n", optarg);
			goto use;
		}
		break;

	case 'm':
		mfile = optarg;
		break;

	case 'h':
		hflg++;
		break;

	case '?':
		eflg++;
		break;
	}
	if(!cflg && !fflg && (eflg || optind == argc)) {
use:
		fprintf(stderr,
			"usage: file [-c] [-h] [-f ffile] [-m mfile] file...\n");
		exit(2);
	}
	if(mkmtab(mfile, cflg) == -1)
		exit(2);
	if(cflg) {
		prtmtab();
		exit(0);
	}
	for(; fflg || optind < argc; optind += !fflg) {
		reg	int	l;

		if(fflg) {
			if((p = fgets(ap, BUFSZ, fl)) == NULL) {
				fflg = 0;
				optind--;
				continue;
			}
			l = strlen(p);
			if(l > 0)
				p[l - 1] = '\0';
		} else
			p = argv[optind];
		prf(p);				/* print "file_name:<tab>" */

		tret=type(p);
		if(ifd)
			close(ifd);
	}
	if (tret != 0) {
		exit(tret);
	} else {
		exit(0);	/*NOTREACHED*/
	}
}

type(file)
char	*file;
{
	int	j,nl;
	int	cc,tar;
	char	ch;
	char	buf[BUFSIZ];
	struct	stat	mbuf;
 	union	tblock	*tarbuf;		/* for tar archive file */
	int	(*statf)() = hflg ? lstat : stat;
	int	notdecl;

	ifd = -1;
	if ((*statf)(file, &mbuf) < 0) {
		if (statf == lstat || lstat(file, &mbuf) < 0) {
			printf("cannot open: %s\n", sys_errlist[errno]);
			return(1);
		}
	}
	switch (mbuf.st_mode & S_IFMT) {
	case S_IFCHR:
		printf("character");
		goto spcl;

	case S_IFDIR:
		printf("directory\n");
		return(0);

	case S_IFIFO:
		printf("fifo\n");
		return(0);

 	case S_IFNAM:
			switch (mbuf.st_rdev) {
			case S_INSEM:
       	                 	printf("Xenix semaphore\n");
       	                 	return(0);
			case S_INSHD:
       	                 	printf("Xenix shared memory handle\n");
       	                 	return(0);
			default:
       	              	   	printf("unknown Xenix name special file\n");
       	               	  	return(0);
			}

	case S_IFLNK:
		if ((cc = readlink(file, buf, BUFSIZ)) < 0) {
			printf("readlink error: %s\n", sys_errlist[errno]);
			return(1);
		}
		buf[cc] = '\0';
		printf("symbolic link to %s\n",buf);
		return(0); 

	case S_IFBLK:
		printf("block");
					/* major and minor, see sys/mkdev.h */
spcl:
		printf(" special (%d/%d)\n", 
		  major(mbuf.st_rdev), minor(mbuf.st_rdev));
		return(0);
	}
 	if ((tarbuf = (union tblock *) calloc(sizeof(union tblock) * NBLOCK, sizeof(char))) == (union tblock *) NULL) {
 		printf("tar: cannot allocate physio buffer\n");
 		return(1);
 	}
	ifd = open(file, O_RDONLY);
	if(ifd < 0) {
		printf("cannot open: %s\n", sys_errlist[errno]);
		return(1);
	}
 	/* read tar header */
 	if ( (tar = read(ifd, tarbuf, TBLOCK)) < 0 ) {
		printf("cannot read: %s\n", sys_errlist[errno]);
		return(1);
	}
	if(tar == 0) {
		printf("empty file\n");
		goto out;
	}
 	if ( tar = (strncmp(tarbuf->tbuf.t_magic, "ustar", 5)) == 0) {
 		printf("tar \n");
 		goto out;
 	}
 	if (lseek(ifd, 0, 0) == -1L) {
 		printf("device seek error: %s\n",sys_errlist[errno]);
 		return(1);
 	}
	if ((fbsz = read(ifd, fbuf, FBSZ)) == -1) {
		printf("cannot read: %s\n", sys_errlist[errno]);
		return(1);
	}
	if(sccs()) {	/* look for "1hddddd" where d is a digit */
		printf("sccs \n");
		goto out;
	}
	switch(ckmtab(fbuf,fbsz,0)){ /* ChecK against Magic Table entries */
		case -1:             /* Error */
			exit(2);
		case 0:              /* Not magic */
			break;
		default:             /* Switch is magic index */
			goto out;
	}
	if(ccom() == 0)
		goto notc;
	while(fbuf[i] == '#') {
		j = i;
		while(fbuf[i++] != '\n') {
			if(i - j > 255) {
				printf("data\n"); 
				goto out;
			}
			if(i >= fbsz)
				goto notc;
		}
		if(ccom() == 0)
			goto notc;
	}
check:
	if(lookup(c) == 1) {
		while((ch = fbuf[i++]) != ';' && ch != '{')
			if(i >= fbsz)
				goto notc;
		printf("c program text");
		goto outa;
	}
	nl = 0;
	while(fbuf[i] != '(') {
		if(fbuf[i] <= 0)
			goto notas;
		if(fbuf[i] == ';'){
			i++; 
			goto check; 
		}
		if(fbuf[i++] == '\n')
			if(nl++ > 6)goto notc;
		if(i >= fbsz)goto notc;
	}
	notdecl = 0;
	while(fbuf[i] != ')') {
		if(fbuf[i++] == '\n')
			if(nl++ > 6)
				goto notc;
		if(i >= fbsz)
			goto notc;
		if (notdecl == 0) {
			j = 0;
			while(ops[j] != 0) {
				if (fbuf[i] == ops[j]) {
					notdecl = 1;
					break;
				}
				j++;
			}
		}
	}
	while(fbuf[i] != '{') {
		if(fbuf[i++] == '\n')
			if(nl++ > 6)
				goto notc;
		if(i >= fbsz)
			goto notc;
	}
	if (notdecl == 0) {
		printf("c program text");
		goto outa;
	} else {
		goto notc;
	}
notc:
	i = 0;
	while(fbuf[i] == 'c' || fbuf[i] == '#') {
		while(fbuf[i++] != '\n')
			if(i >= fbsz)
				goto notfort;
	}
	if(lookup(fort) == 1){
		printf("fortran program text");
		goto outa;
	}
notfort:
	i = 0;
	if(ascom() == 0)
		goto notas;
	j = i-1;
	if(fbuf[i] == '.') {
		i++;
		if(lookup(as) == 1){
			printf("assembler program text"); 
			goto outa;
		}
		else if(j != -1 && fbuf[j] == '\n' && isalpha(fbuf[j+2])){
			printf("[nt]roff, tbl, or eqn input text");
			goto outa;
		}
	}
	while(lookup(asc) == 0) {
		if(ascom() == 0)
			goto notas;
		while(fbuf[i] != '\n' && fbuf[i++] != ':')
			if(i >= fbsz)
				goto notas;
		while(fbuf[i] == '\n' || fbuf[i] == ' ' || fbuf[i] == '\t')
			if(i++ >= fbsz)
				goto notas;
		j = i - 1;
		if(fbuf[i] == '.'){
			i++;
			if(lookup(as) == 1) {
				printf("assembler program text"); 
				goto outa; 
			}
			else if(fbuf[j] == '\n' && isalpha(fbuf[j+2])) {
				printf("[nt]roff, tbl, or eqn input text");
				goto outa;
			}
		}
	}
	printf("assembler program text");
	goto outa;
notas:
	/* start modification for multibyte env */	
	IS_ascii = 1;
        if (fbsz < FBSZ)
                Max = fbsz;
        else
                Max = FBSZ - MB_LEN_MAX; /* prevent cut of wchar read */
        /* end modification for multibyte env */
	for(i=0; i < Max; )
		if(fbuf[i]&0200) {
			IS_ascii = 0;
			if (fbuf[0]=='\100' && fbuf[1]=='\357') {
				printf("troff output\n");
				goto out;
			}
		/* start modification for multibyte env */
			if ((length=mbtowc(&wchar, &fbuf[i],MB_LEN_MAX)) <= 0
			    || !wisprint(wchar)){
				printf("data\n"); 
				goto out; 
			}
			i += length;
		}
		else
			i++;
	i = fbsz;
		/* end modification for multibyte env */
	if (mbuf.st_mode&(S_IXUSR|S_IXGRP|S_IXOTH))
		printf("commands text");
	else if(english(fbuf, fbsz))
		printf("English text");
	else if(IS_ascii)
		printf("ascii text");
	else 
		printf("text"); /* for multibyte env */
outa:
	/* 
	 * This code is to make sure that no MB char is cut in half
	 * while still being used.
	 */
	fbsz = (fbsz < FBSZ ? fbsz : fbsz - MB_CUR_MAX + 1);
	while(i < fbsz){
		if (isascii(fbuf[i])){
			i++;
			continue;
		}
		else {
			if ((length=mbtowc(&wchar, &fbuf[i],MB_LEN_MAX)) <= 0
		        	|| !wisprint(wchar)){
			printf(" with garbage\n");
			goto out;
			}
			i = i + length;
		}
	}
	printf("\n");
out:
	return(0);
}


long
atolo(s)			/* determine read format and get e_value.num */
reg	char	*s;
{
	reg	char	*fmt = "%ld";
	auto	long	j = 0L;

	if(*s == '0') {
		s++;
		if(*s == 'x') {
			s++;
			fmt = "%lx";
		} else
			fmt = "%lo";
	}
	sscanf(s, fmt, &j);
	return(j);
}

lookup(tab)
reg	char **tab;
{
	reg	char	r;
	reg	int	k,j,l;

	while(fbuf[i] == ' ' || fbuf[i] == '\t' || fbuf[i] == '\n')
		i++;
	for(j=0; tab[j] != 0; j++) {
		l = 0;
		for(k=i; ((r=tab[j][l++]) == fbuf[k] && r != '\0');k++);
		if(r == '\0')
			if(fbuf[k] == ' ' || fbuf[k] == '\n' || fbuf[k] == '\t'
			    || fbuf[k] == '{' || fbuf[k] == '/') {
				i=k;
				return(1);
			}
	}
	return(0);
}

ccom()
{
	reg	char	cc;

	while((cc = fbuf[i]) == ' ' || cc == '\t' || cc == '\n')
		if(i++ >= fbsz)
			return(0);
	if(fbuf[i] == '/' && fbuf[i+1] == '*') {
		i += 2;
		while(fbuf[i] != '*' || fbuf[i+1] != '/') {
			if(fbuf[i] == '\\')
				i += 2;
			else
				i++;
			if(i >= fbsz)
				return(0);
		}
		if((i += 2) >= fbsz)
			return(0);
	}
	if(fbuf[i] == '\n')
		if(ccom() == 0)
			return(0);
	return(1);
}

ascom()
{
	while(fbuf[i] == ASCOMCHAR) {
		i++;
		while(fbuf[i++] != '\n')
			if(i >= fbsz)
				return(0);
		while(fbuf[i] == '\n')
			if(i++ >= fbsz)
				return(0);
	}
	return(1);
}

sccs() 
{				/* look for "1hddddd" where d is a digit */
	reg int j;

	if(fbuf[0] == 1 && fbuf[1] == 'h') {
		for(j=2; j<=6; j++) {
			if(isdigit(fbuf[j]))  
				continue;
			else  
				return(0);
		}
	} else {
		return(0);
	}
	return(1);
}

english (bp, n)
char *bp;
int  n;
{
#	define NASC 128		/* number of ascii char ?? */
	reg	int	j, vow, freq, rare;
	reg	int	badpun = 0, punct = 0;
	auto	int	ct[NASC];

	if (n<50)
		return(0); /* no point in statistics on squibs */
	for(j=0; j<NASC; j++)
		ct[j]=0;
	for(j=0; j<n; j++)
	{
		if (bp[j]<NASC)
			ct[bp[j]|040]++;
		switch (bp[j])
		{
		case '.': 
		case ',': 
		case ')': 
		case '%':
		case ';': 
		case ':': 
		case '?':
			punct++;
			if(j < n-1 && bp[j+1] != ' ' && bp[j+1] != '\n')
				badpun++;
		}
	}
	if (badpun*5 > punct)
		return(0);
	vow = ct['a'] + ct['e'] + ct['i'] + ct['o'] + ct['u'];
	freq = ct['e'] + ct['t'] + ct['a'] + ct['i'] + ct['o'] + ct['n'];
	rare = ct['v'] + ct['j'] + ct['k'] + ct['q'] + ct['x'] + ct['z'];
	if(2*ct[';'] > ct['e'])
		return(0);
	if((ct['>']+ct['<']+ct['/'])>ct['e'])
		return(0);	/* shell file test */
	return (vow*5 >= n-ct[' '] && freq >= 10*rare);
}
