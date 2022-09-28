/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/from822.c	1.4.3.1"
#include <stdio.h>
#include <ctype.h>
#include "xmail.h"
#include "s_string.h"
#include "header.h"
#include "aux.h"

/*
 *	Convert the rfc822 message on standard input into `UNIX' format
 *	and write it onto the passed FILE. 
 */

/* header tags */
header hdrs[] = {
	HEADER("Date:"),
	HEADER("From:"),
	HEADER("Sender:"),
	HEADER("UnixFrom:"),
	HEADER("UnixDate:"),
	HEADER("Reply-To:"),
	HEADER("")
};
#define datehdr hdrs[0]
#define fromhdr hdrs[1]
#define senderhdr hdrs[2]
#define unixfromhdr hdrs[3]
#define unixdatehdr hdrs[4]
#define replyhdr hdrs[5]

/* imported */
extern int getheader();
extern string *getaddr();
extern int printheaders();
extern void printbodies();
extern char *lowercase();
extern char *convertaddr();


/* predeclared */
static char *getfrom();
static char *getdate();

/* exported */
int extrafrom = 0;

/*
 *  network name gets tacked onto the return addresses if addnet is set or if
 *  rfc822 routing is specified.
 */
from822(net, fgetsp, fi, fo, defsender)
	char *net;
	char *(*fgetsp)();
	FILE *fi;		/* input file */
	FILE *fo;		/* output file */
	char *defsender;
{
	char *from=NULL;
	char *date;
	char buf[4096];
	extern int fputs();

	getheader(fgetsp, fi);

	/*  Get sender's address.  If anything else is on the from line,
	 *  keep it under another name.  If no `Sender:' line is found, use
	 *  `From:' line, else `UnixFrom:' line (given by `From ' in input)
	 */
	if (unixfromhdr.line != NULL) {
		from = s_to_c(unixfromhdr.line);
	} else if (replyhdr.line != NULL) {
		from = getfrom(HCONTENT(replyhdr), net);
	} else if (senderhdr.line != NULL) {
		from = getfrom(HCONTENT(senderhdr), net);
	} else if (fromhdr.line != NULL) {
		from = getfrom(HCONTENT(fromhdr), net);
	} else if (defsender != NULL) {
		from = getfrom(defsender, net);
	} else {
		from = "unknown";
	}
	
	/*  Get date line, or make one up */
	if (datehdr.line != NULL)
		date = getdate(HCONTENT(datehdr));
	else if(unixdatehdr.line != NULL)
		date = s_to_c(unixdatehdr.line);
	else
		date = getdate((char *)0);

	/* output UNIX header */
	if (from != NULL && from != '\0' && date != '\0')
		print_remote_header(fo, from, date, "");

	/* output the rest */
	if(printheaders(fputs, fo, 0))
		fputs("\n", fo);
	printbodies(fputs, fo);
	while ((*fgetsp)(buf, sizeof(buf), fi)!=NULL)
		fputs(buf, fo);
}

/*
 *  Return true if the two lines are the same modulo <>, whitespace,
 *  and newline.
 */
sameaddr(line, addr)
	char *line;
	char *addr;
{
	if (line==NULL)
		return(1);
	while (*line==' ' || *line=='\t' || *line=='<')
		line++;
	while (*addr==' ' || *addr=='\t' || *addr=='<')
		addr++;
	while(*addr && *addr!='>' && *addr!=' ' && *addr!='\t'){
		if(*addr != *line)
			break;
		addr++;
		line++;
	}
	while (*line=='>' || *line==' ' || *line=='\t')
		line++;
	while (*addr=='>' || *addr==' ' || *addr=='\t')
		addr++;
	while(*line)
		if(*addr++ != *line++)
			return(0);
	return(1);
}

/*
 *  The sender is either the next first whitespace delimited token or
 *  the first thing enclosed in "<" ">".
 *  Sets extrafrom > 0 if a from line with other cruft in it.
 *  Returns pointer to static area containing address converted to bang format.
 */
static char *
getfrom(line, net)
char *line;
char *net;
{
	register char *lp;
	string *sender;

	sender = getaddr(line);
	lp = convertaddr(s_to_c(sender));
	if(net!=NULL){
		s_reset(sender);
		s_append(sender, net);
		s_append(sender, "!");
		s_append(sender, lp);
		lp = s_to_c(sender);
	}
	return lp;
}

/*
 *  Get a date line.  Convert to `UNIX' format.
 *	use the current time instead if misparse, or if day omitted
 *	822 standard: [Mon, ] 19 Oct 87 19:47:25 EDT
 *		(sometimes the , is omitted; sometimes (Mon) at end)
 *	Unix: Mon Oct 19 19:47:25 EDT 1987
 *
 */
static char *
getdate(line)
char *line;
{
	register char *sp;
	int i;
	long t, time();
	char *p, *nl, *ctime();
	char *field[6];
	static string *date=0;
	static string *olddate=0;

	date = s_reset(date);
	olddate = s_reset(olddate);
	if (line) {
		s_append(olddate, line);
		sp = s_to_c(olddate);
		for (i = 0; *sp && i < 6; i++) {
			while (isspace(*sp) || *sp == ',' || *sp == '-')
				*sp++ = '\0';
			field[i] = sp;
			while (!isspace(*sp) && *sp != ',' && *sp != '-'
							&& *sp != '\0')
				sp++;
		}
		*sp = '\0';
		if (i==6  && isalpha(*field[0])) {
			s_append(date, field[0]);
			s_putc(date, ' ');
			s_append(date, field[2]);
			s_putc(date, ' ');
			if(strlen(field[1])==1)
				s_putc(date, ' ');
			s_append(date, field[1]);
			s_putc(date, ' ');
			s_append(date, field[4]);
			s_putc(date, ' ');
			for (p = field[5]; *p; p++)
				if(islower(*p))
					*p = toupper(*p);
			s_append(date, field[5]);
			s_putc(date, ' ');
			if (field[3][2]=='\0')
				s_append(date, "19");
			s_append(date, field[3]);
		}
	}
	s_terminate(date);
	if (*s_to_c(date) == '\0') {
		(void) time(&t);
		sp = ctime(&t);
		if ((nl = strchr(sp, '\n')) != 0)
			*nl = 0;
		s_append(date, sp);
	}
	return s_to_c(date);
}

/* Return 1 if addr has only one bang in it */
int
onehop(addr)
char *addr;
{
	register char *p;
	register int cnt=0;

	for (p=addr; *p; )
		if (*p++ == '!')
			if (cnt++>0)
				return 0;
	return 1;
}
