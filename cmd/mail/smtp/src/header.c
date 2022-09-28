/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/header.c	1.5.3.1"
#include <stdio.h>
#include <ctype.h>
#include "xmail.h"
#include "s_string.h"
#include "header.h"
#include "aux.h"
#include "v9regexp.h"

/* imported */
extern char *malloc();
extern char *strcpy();
extern char *strchr();
extern int strncmp();
extern void exit();

/* predeclared */
extern void addheader();
extern void addbody();

/* global to this file */
static regexp *rfprog;
static regexp *fprog;

/* imported */
extern int extrafrom;

/*
 *	Input header from standard input.  Actually two extra lines are also
 *	read, but this isn't a problem.  Save some header lines in the header
 *	array.
 */
extern
getheader(fgetsp, fp)
	char (*fgetsp)();
	FILE *fp;
{
	char buf[4096];
	string *line;
	string *from=NULL;
	string *date=NULL;
	char *cp;
	header *hp;
 
	/*
	 *  forget any old headers
	 */
	for (hp = hdrs; *(hp->name) != '\0'; hp++)
		hp->line = (string *)0;

	/*
	 *  preload the first line for the parsing below
	 */
	if((*fgetsp)(buf, sizeof buf, fp) == NULL)
		return -1;

	/*  If the first line of an rfc822 message
	 *  is a unix from line, use that to `seed' the From: and
	 *  Date: fields.
	 */
	if(strncmp(buf, "From ", 5)==0) {
		if (getunixfrom(buf, &from, &date)==0) {
			for (hp = hdrs; *(hp->name) != '\0'; hp++) {
				if (STRCMP("UnixFrom:", hp) == 0) {
					hp->line = from;
					from = (string *)0;
					continue;
				};
				if (STRCMP("UnixDate:", hp) == 0) {
					hp->line = date;
					date = (string *)0;
					continue;
				};
			}
			return 0;
		}
	}

	/* Now get real 822 headers */
	while (*buf != '\n' && buf != '\0') {
		/* gather a multiple line header field */
		line = s_new();
		do {
			s_append(line, buf);
			if((*fgetsp)(buf, sizeof buf, fp) == NULL)
				return -1;
		} while (*buf==' ' || *buf=='\t');

		/* header lines must contain `:' with no preceding white */
		for(cp=s_to_c(line); *cp; cp++)
			if(isspace(*cp) || *cp==':')
				break;
		if (*cp != ':') {
			addbody(line);
			break;
		}

		/* look for `important' headers */
		for (hp = hdrs; *(hp->name) != '\0'; hp++) {
			if (cistrncmp(s_to_c(line), hp->name, hp->size) == 0) {
				hp->line = line;
				break;
			};
		}
		addheader(line);
	}
	/* at this point, buf contains a line of the body */
	addbody(s_copy(buf));
	return 0;
}

/*
 *	Keep a (circular) list of header lines to output.
 */
typedef struct quux{
	struct quux *next;
	string *line;
} hlist;
static hlist dummy = { &dummy, '\0' };
static hlist *list = &dummy;

/*
 *	Add to list of header lines.
 */
extern void
addheader(line)
string *line;
{
	hlist *thing;

	thing = (hlist *)malloc(sizeof(hlist));
	if (thing == NULL) {
		perror("reading header");
		exit(1);
	}
	thing->line = line;
	thing->next = list->next;
	list->next = thing;
	list = thing;
}

/*
 *	Print list of headers.
 */
extern int
printheaders(fputsp, fp, originalfrom)
	FILE *fp;
	int (*fputsp)();
{
	hlist *thing, *othing;
	int printed = 0;

	for(thing = list->next->next; thing != &dummy; ){
		printed++;
		if(originalfrom && strncmp(s_to_c(thing->line), "From:", 5)==0)
			(*fputsp)(">", fp);
		(*fputsp)(s_to_c(thing->line), fp);
		s_free(thing->line);
		othing = thing;
		thing = thing->next;
		free(othing);
	}
	list = dummy.next = &dummy;	/* reinitialize for future calls */
	return(printed);
}

/*
 *	Keep a (circular) list of body lines to output.
 */
static hlist bdummy = { &bdummy, '\0' };
static hlist *blist = &bdummy;

/*
 *	Add to list of body lines.
 */
extern void
addbody(line)
string *line;
{
	hlist *thing;

	thing = (hlist *)malloc(sizeof(hlist));
	if (thing == NULL) {
		perror("reading header");
		exit(1);
	}
	thing->line = line;
	thing->next = blist->next;
	blist->next = thing;
	blist = thing;
}

/*
 *	Print list of body lines
 */
extern void
printbodies(fputsp, fp)
	FILE *fp;
	int (*fputsp)();
{
	hlist *thing, *othing;
	int line = 0;

	for(thing = blist->next->next; thing != &bdummy; ){
		/* dump extraneous new line */
		if(line++!=0 || *s_to_c(thing->line)!='\n')
			(*fputsp)(s_to_c(thing->line), fp);
		s_free(thing->line);
		othing = thing;
		thing = thing->next;
		free(othing);
	}
	blist = bdummy.next = &bdummy;
}

/*
 *  extract sender and date from a unix remote from line.  return -1
 *  if its the wrong format, 0 otherwise.
 */
getunixfrom(line, fpp, dpp)
	char *line;
	string **fpp;
	string **dpp;
{		
	regsubexp subexp[10];

	if (rfprog == NULL)
		rfprog = regcomp(REMFROMRE);
	if (regexec(rfprog, line, subexp, 10)) {
		*fpp = s_new();
		append_match(subexp, *fpp, REMSYSMATCH);
		s_append(*fpp, "!");
		append_match(subexp, *fpp, REMSENDERMATCH);
		*dpp = s_new();
		append_match(subexp, *dpp, REMDATEMATCH);
		return 0;
	}
	if (fprog == NULL)
		fprog = regcomp(FROMRE);
	if (regexec(fprog, line, subexp, 10)) {
		*fpp = s_new();
		append_match(subexp, *fpp, SENDERMATCH);
		*dpp = s_new();
		append_match(subexp, *dpp, DATEMATCH);
		return 0;
	}
	return -1;
}

/*
 *  The addr is either the first whitespace delimited token or
 *  the first thing enclosed in "<" ">".
 *  Sets extrafrom > 0 if a from line with other cruft in it.
 */
extern string *
getaddr(line)
char *line;
{
	register char *lp;
	register int comment = 0;
	register int anticomment = 0;
	register int inquote = 0;
	static string *sender=0;

	if (!sender)
		sender = s_new();
	s_restart(sender);
	lp = line;
	for (; *lp; lp++) {
		if (comment) {
			if (*lp=='(')
				comment++;
			if (*lp==')')
				comment--;
			continue;
		}
		if (anticomment) {
			if (*lp=='>') {
				if (*(lp+1)!='\n')
					extrafrom++;
				break;
			}
		}
		if (inquote) {
			if (*lp=='"')
				inquote = 0;
			s_putc(sender, *lp);
			continue;
		}
		switch (*lp) {
		case '\t':
		case '\n':
			break;
		case ' ':
			if (cistrncmp(lp, " at ", sizeof(" at ")-1)==0) {
				s_putc(sender, '@');
				lp += sizeof(" at ")-2;
			}
			break;
		case '<':
			anticomment = 1;
			if(*s_to_c(sender)!='\0')
				extrafrom++;
			s_restart(sender);
			break;
		case '(':
			extrafrom++;
			comment++;
			break;
		case ',':
			s_restart(sender);
			break;
		case '"':
			inquote = 1;
			/* fall through */
		default:
			s_putc(sender, *lp);
			break;
		}
	}
	s_terminate(sender);
	s_restart(sender);
	return(sender);
}

/*
 *  case independent string compare
 */
extern int
cistrncmp(s1, s2, n)
	char *s1;
	char *s2;
{
	int c1, c2;

	for(; *s1 && n>0; n--, s1++, s2++){
		c1 = isupper(*s1) ? tolower(*s1) : *s1;
		c2 = isupper(*s2) ? tolower(*s2) : *s2;
		if (c1 != c2)
			return -1;
	}
	return(0);
}
