/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)exstr:exstr.c	1.3.3.1"

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>

/* external functions */

extern	char 	*malloc();
extern	int	getopt();
extern	void	exit();
extern	int	atoi();
extern	int	_filbuf();
extern	char	*optarg;
extern	int	optind, opterr;	

/* static functions */

static	void	extract();
static	void	replace();
static	void	yankstr();
static	void	badformat();
static	void	prstr();
static	int	getachar();
static  void	usage();

/* static variables */

static	int	eflg;		/* find strings in source file(s) */
static	int	dflg;           /* use replaced string a second argument */
static  int	rflg;		/* replace strings by function calls */
static  int	errflg;         /* syntax error on command line */
static  char	*Fname;		/* name of source file */ 	
static  int	Lineno;		/* line number in source file */
static	int	Posno;		/* character position within line */
static  int	flag;           /* sets when newline is encountered */
static  jmp_buf	to_eof;


main(argc, argv)
int	argc;
char	*argv[];
{
	int	ch;		

	while ((ch = getopt(argc, argv, "erd")) != -1)
		switch(ch) {
		    case 'e':
			if (rflg)
				errflg++;
			else
				eflg++;
			continue;
		    case 'r':
			if (eflg)
				errflg++;
			else
				rflg++;
			continue;
		    case 'd':
			if (eflg)
				errflg++;
			else
				dflg++;
			continue;
		    default:
			errflg++;
		}
	if (optind ==  argc || errflg) 
		usage();
	if (!rflg)
		for(;optind<argc;optind++)
			extract(argv[optind]);
	else  {
		if (optind+1 != argc)
			usage();
		replace(argv[optind]);
	}
	exit(0);
}

static	void
extract(name)
char	*name;
{
	if (freopen(name,"r",stdin) == NULL) {
		(void)fprintf(stderr,"exstr: ERROR: couldn't open file '%s'\n",name);
		exit(1);
	}
	Fname = name;
	flag = 1;
	Lineno = 0;

	if (setjmp(to_eof) != 0)
		return;

	for (;;) {
		char ch;

		ch = getachar();

		switch (ch) {
		case '#':
			if (Posno != 0)
				continue;
			do {
				ch = getachar();
			} while (isspace(ch));
			if (ch == 'd')
				continue;
			while (getachar() != '\n');
			break;
		case '"':
			yankstr();
			break;
		case '\'':
			while ((ch = getachar()) != '\'')
				if (ch == '\\')
					ch = getachar();
			break;

		case '/':
			ch = getachar();
			if (ch == '*') {
				int level = 0;
				while(level != 2) {
					ch = getachar();
					if (ch == '*')
						level = 1;
					else if (level == 1 && ch == '/')
						level++;
					else
						level = 0;
				}
			}
			break;
		case '\\':
			ch = getachar();
			break;
		}
	}
}

static	void
yankstr()
{
	char cc;
	char dbuf[BUFSIZ];
	register char *dp = dbuf;
	int saved_posno;
	int saved_lineno;

	saved_posno = Posno;
	saved_lineno = Lineno;
	while ((cc = getachar()) != '"') {
		if(cc == '\\') {
			*dp++ = cc;
			cc = getachar();
		}
		if (cc == '\n') {
			dp--;
			continue;
		}
		*dp++ = cc;
	}
	*dp = 0;
	prstr(dbuf,saved_lineno,saved_posno);
}

static	void
prstr(cp, lineno, posno)
register char *cp;
{
	if (eflg)
		(void)fprintf(stdout, "%s:%d:%d:::%s\n", Fname, lineno, posno, cp);
	else
		(void)fprintf(stdout, "%s:%s\n", Fname, cp);

}

static	void
usage()
{
	(void)fprintf(stderr, "usage: exstr [-e] files\n");
	(void)fprintf(stderr, "or   : exstr -r [-d] file\n");
	exit(1);
}

static	int
getachar()
{
	int cc;

	cc = getchar();
	if (flag) {
		Lineno++;
		Posno = 0;
		flag = 0;
	} else
		Posno++;
	if (cc == EOF)
		longjmp(to_eof, 1);
	if (cc == '\n')
		flag = 1;
	return(cc);
}


static void
replace(name)
char	*name;
{
	char	linebuf[BUFSIZ];	/* buffer to read lines from source file */
	char	*cp;
	int	curlineno;		/* line number in strings file */
	int	curposno;		/* character position in string file */
	int	savelineno = 0;
	int	curmsgno;		/* message number in strings file */
	int	wrong_msg;		/* invalid message number */
	int	cont_str = 0;		/* string continues in the next line */ 	
	char	*repstr;
	char	repbuf[BUFSIZ], *repbufp;
	char	curline[BUFSIZ];
	char	outbuf[BUFSIZ];
	char	*inp;			/* keeps track of character position within input file */
	char	*outp;			/* keeps track of character position within output buffer */
	char	*msgfile;
	FILE	*fi;			/* input source file pointer */

	inp = linebuf;
	outp = outbuf;
	linebuf[0] = '\0';
	/* open input C source file */
	if ((fi = fopen(name,"r")) == (FILE *)NULL) {
		(void)fprintf(stderr,"exstr: ERROR: couldn't open file '%s'\n",name);
		exit(1);
	}
	Fname = name;

	(void)fprintf(stdout, "extern char *gettxt();\n");

	/* process file containing the list of strings */
	while (fgets(repbuf,sizeof repbuf, stdin) != (char *)NULL) {

		wrong_msg = 0;

		/* save a copy of the current line */
		(void)strcpy(curline, repbuf);

		/* take apart the input string */
		repbufp = strchr(repbuf,':');
		if (repbufp == (char *)NULL)
			badformat(curline);
		*repbufp++ = '\0';
		/* verify that string belongs to the input C source file */
		if (strcmp(repbuf, name) != NULL)
			continue;
		repstr = strchr(repbufp,':');
		if (repstr == (char *)NULL)
			badformat(curline);
		*repstr++ = '\0';
		curlineno = atoi(repbufp);
		if (curlineno < savelineno) {
			(void)fprintf(stderr, "exstr: ERROR: stdin: line out of order\n");
			(void)fprintf(stderr, "%s", curline);
			exit(1);
		}
		savelineno = curlineno;
		repbufp = repstr;
		repstr = strchr(repbufp,':');
		if (repstr == (char *)NULL)
			badformat(curline);
		repstr[strlen(repstr) - 1 ] = '\0';
		*repstr++ = '\0';
		curposno = atoi(repbufp);
		repbufp = repstr;
		repstr = strchr(repbufp,':');
		if (repstr == (char *)NULL)
			badformat(curline);
		*repstr++ = '\0';
		msgfile = repbufp;
		if ( strlen(msgfile) > (uint)14 || *msgfile == '\0' ){
			(void)fprintf(stderr, 
			   "exstr: ERROR: stdin: invalid message file name '%s'\n",
						msgfile);
			(void)fprintf(stderr, "%s", curline);
			exit(1);
		}
		repbufp = repstr;
		repstr = strchr(repbufp,':');
		if (repstr == (char *)NULL)
			badformat(curline);
		*repstr++ = '\0';
		cp = repbufp;
		while(*cp)
			if(!isdigit(*cp++)) {
				wrong_msg++;
				break;
			}
		if ( *repbufp == '\0' || wrong_msg ) {
			(void)fprintf(stderr,
			  "exstr: ERROR: stdin: invalid message number '%s'\n",
				repbufp);
			(void)fprintf(stderr, "%s", curline);
			exit(1);
			}
		curmsgno = atoi(repbufp);

		/* move up to this line */
		while (Lineno != curlineno ) {
			if (outp != outbuf) {
				while(*inp != '\0')
					*outp++ = *inp++;
				*outp = '\0';
				(void)fputs(outbuf,stdout);
			} else if (*linebuf != '\0')
				(void)fputs(linebuf,stdout);
			outp = outbuf;
			inp = linebuf;
			if (fgets(linebuf, sizeof linebuf, fi) == (char *)NULL) {
				(void)fprintf(stderr, "read error\n");
				exit(1);
			}
			Lineno++;
			Posno = 0;
		}
		if (Posno > curposno) {
			(void)fprintf(stderr, "Bad input record line number %d\n",Lineno);
			exit(1);
		}
		while (Posno != curposno) {
			*outp++ = *inp++;
			Posno++;
		}
		if (*inp != '"') {
			fprintf(stderr, "exstr: ERROR: cannot replace string '%s' in line (%d) of file (%s)\n", repstr, Lineno, Fname);
			exit(1);
		}
		/* check if string continues in next line */
		cp = inp + 1;
		while (cp[strlen(cp)-2] == '\\' && cp[strlen(cp)-1] == '\n') { 
			while (*cp != '\0') {
				if (*cp == '\\') {
					cp += 2;
					continue;
				} else if (*cp++ == '"')
					break;
			}
			if (*cp != '\0')
				break;
			if ((cp = fgets(linebuf,sizeof linebuf, fi)) == (char *)NULL) {
				fprintf(stderr, "exstr: ERROR: read error in file (%s)\n", Fname);
				exit(1);
			}
			cont_str++;
			Lineno++;
		}
		if (cont_str) {
			cp = linebuf;
			while (*cp != '\0') {
				if (*cp == '\\') {
					if (*++cp != '\0')
						cp++;
					continue;
				} else if (*cp++ == '"')
					break;
			}
			if (*cp == '\0') {
				fprintf(stderr, "exstr: ERROR: cannot replace string '%s' in line (%d) of file (%s)\n", repstr, Lineno, Fname);
				exit(1);
			}
			inp = cp;
			Posno = cp - linebuf;
		}
		if (dflg)
			outp += sprintf(outp,"gettxt(\"%s:%d\", \"%s\")",msgfile, curmsgno, repstr);
		else
			outp += sprintf(outp,"gettxt(\"%s:%d\", \"\")",msgfile,curmsgno);
		if (!cont_str) {
			inp += strlen(repstr)+2;
			Posno += strlen(repstr)+2;
		}
		else
			cont_str = 0;
	}
	if (outp != outbuf) {
		while(*inp != '\0')
			*outp++ = *inp++;
		*outp = '\0';
		(void)fputs(outbuf,stdout);
	}
	while(fgets(linebuf,sizeof linebuf, fi) != (char *)NULL)
		(void)fputs(linebuf, stdout);
	
}

static	void
badformat(line)
char  	*line;
{
	(void)fprintf(stderr, "exstr: ERROR: stdin: Badly formatted replacement string\n%s", line);
	exit(1);
}


