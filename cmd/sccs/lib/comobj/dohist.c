/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/dohist.c	6.9"
# include	"../../hdr/defines.h"
# include	"../../hdr/had.h"
#include	<string.h>

extern char *Mrs;
extern int Domrs;

static char	Cstr[RESPSIZE];
static char	Mstr[RESPSIZE];
char	*savecmt();	/* function returning character ptr */
static char	*getresp();

void
dohist(file)
char *file;
{
	char line[BUFSIZ];
	int doprmt;
	register char *p;
	FILE	*in;
	FILE	*fdfopen();
	extern char *Comments, *getresp();
	void	mrfixup();
	int	isatty(), xopen();

	in = xfopen(file,0);
	while ((p = fgets(line,sizeof(line),in)) != NULL)
		if (line[0] == CTLCHAR && line[1] == EUSERNAM)
			break;
	if (p != NULL) {
		while ((p = fgets(line,sizeof(line),in)) != NULL)
			if (line[3] == VALFLAG && line[1] == FLAG && line[0] == CTLCHAR)
				break;
			else if (line[1] == BUSERTXT && line[0] == CTLCHAR)
				break;
		if (p != NULL && line[1] == FLAG) {
			Domrs++;
		}
	}
	(void) fclose(in);
	doprmt = 0;
	if (isatty(0) == 1)
		doprmt++;
	if (Domrs && !Mrs) {
		if (doprmt)
			printf("MRs? ");
		Mrs = getresp(" ",Mstr);
	}
	if (Domrs)
		mrfixup();
	if (!Comments) {
		if (doprmt)
			printf("comments? ");
		sprintf(line,"\n");
		Comments = getresp(line,Cstr);
	}
}


static char *
getresp(repstr,result)
char *repstr;
char *result;
{
	char line[BUFSIZ], *strcat();
	register int done, sz;
	register char *p;
	extern char	had_standinp;
	int	fatal();

	result[0] = 0;
	done = 0;
	/*
	save old fatal flag values and change to
	values inside ()
	*/
	FSAVE(FTLEXIT | FTLMSG | FTLCLN);
	if ((had_standinp && (!HADY || (Domrs && !HADM)))) {
		Ffile = 0;
		fatal("standard input specified w/o -y and/or -m keyletter (de16)");
	}
	/*
	restore the old flag values and process if above
	conditions were not met
	*/
	FRSTR();
	sz = sizeof(line) - size(repstr);
	while (!done && fgets(line,sz,stdin) != NULL) {
		p = strend(line);
		if (*--p == '\n') {
			if (*--p == '\\') {
				copy(repstr,p);
			}
			else {
				*++p = 0;
				++done;
			}
		}
		else
			fatal("line too long (co18)");
		if ((int) (size(line) + size(result)) > RESPSIZE)
			fatal("response too long (co19)");
		strcat(result,line);
	}
	return(result);
}


static char	*Qarg[NVARGS];
char	**Varg = Qarg;

valmrs(pkt,pgm)
struct packet *pkt;
char *pgm;
{
	extern char *Sflags[];
	register int i;
	int st;
	register char *p;
	char	*auxf();
	int	wait(), execvp(), close(), fatal();
	void	exit();

	Varg[0] = pgm;
	Varg[1] = auxf(pkt->p_file,'g');
	if (p = Sflags[TYPEFLAG - 'a'])
		Varg[2] = p;
	else
		Varg[2] = Null;
	if ((i = fork()) < 0) {
		fatal("cannot fork; try again (co20)");
	}
	else if (i == 0) {
		for (i = 4; i < 15; i++)
			(void) close(i);
		execvp(pgm,Varg);
		exit(1);
	}
	else {
		wait(&st);
		return(st);
	}
	/*NOTREACHED*/
}

# define	LENMR	60

void
mrfixup()
{
	register char **argv, *p, c;
	char *ap, *stalloc();
	int len;

	argv = &Varg[VSTART];
	p = Mrs;
	NONBLANK(p);
	for (ap = p; *p; p++) {
		if (*p == ' ' || *p == '\t') {
			if (argv >= &Varg[(NVARGS - 1)])
				fatal("too many MRs (co21)");
			c = *p;
			*p = 0;
			if ((len = size(ap)) > LENMR)
				fatal("MR number too long (co24)");
			*argv = stalloc(len);
			copy(ap,*argv);
			*p = c;
			argv++;
			NONBLANK(p);
			ap = p;
		}
	}
	--p;
	if (*p != ' ' && *p != '\t') {
		if ((len = size(ap)) > LENMR)
			fatal("MR number too long (co24)");
		copy(ap,*argv++ = stalloc(len));
	}
	*argv = 0;
}


# define STBUFSZ	500

char *
stalloc(n)
register int n;
{
	static char stbuf[STBUFSZ];
	static int stind = 0;
	register char *p;

	p = &stbuf[stind];
	if (&p[n] >= &stbuf[STBUFSZ-1])
		fatal("out of space (co22)");
	stind += n;
	return(p);
}


char *
savecmt(p)
register char *p;
{
	register char	*p1, *p2;
	int	ssize, nlcnt;
	char *fmalloc();

	nlcnt = 0;
	for (p1 = p; *p1; p1++)
		if (*p1 == '\n')
			nlcnt++;
/*
 *	ssize is length of line plus mush plus number of newlines
 *	times number of control characters per newline.
*/
	ssize = (strlen(p) + 4 + (nlcnt * 3)) & (~1);
	p1 = fmalloc((unsigned) ssize);
	p2 = p1;
	while (1) {
		while(*p && *p != '\n')
			*p1++ = *p++;
		if (*p == '\0') {
			*p1 = '\0';
			return(p2);
		}
		else {
			p++;
			*p1++ = '\n';
			*p1++ = CTLCHAR;
			*p1++ = COMMENTS;
			*p1++ = ' ';
		}
	}
	/*NOTREACHED*/
}
