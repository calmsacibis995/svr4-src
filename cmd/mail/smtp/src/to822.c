/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/to822.c	1.3.3.1"
#include <stdio.h>
#include <ctype.h>
#include "v9regexp.h"
#include "xmail.h"
#include "s_string.h"
#include "header.h"
#include "aux.h"
#include "addrformat.h"

/* imports */
header hdrs[] = {
	HEADER("Date:"),
	HEADER("From:"),
	HEADER("To:"),
	HEADER("")
};
#define datehdr hdrs[0]
#define fromhdr hdrs[1]
#define tohdr hdrs[2]

/* imported */
extern void getheader();
extern int printheaders();
extern void printbodies();
extern char *fgets();
extern string *getaddr();
extern int cistrncmp();
extern void exit();

/* predeclared */
static char *convertdate();
extern char *convertaddr();
static int indomain();

/* exported */
int extrafrom = 0;

extern int
to822(fputsp, in, out, sender, domain, rcvr)
	int (*fputsp)();
	FILE *in, *out;		/* file to output to */
	char *sender;		/* name of sender (already in 822 format) */
	char *domain;
	char *rcvr;
{
	string *from=s_new();
	string *date=s_new();
	string *remfrom=s_new();
	string *faddr;
	static regexp *pp=NULL;
	int originalfrom=0;
	regsubexp subexp[10];
	char xbuf[4096];
	int n;
	extern int debug;

	/* get UNIX from line */
	if(fgets(xbuf, sizeof xbuf, in)==NULL) {
		fprintf(stderr, "to822: empty file.\n");
		return -1;
	}

	/* first line had better be a from */
	if (pp==NULL)
		pp = regcomp(REMFROMRE);
	if (regexec(pp, xbuf, subexp, 10)) {
		append_match(subexp, from, REMSENDERMATCH);
		append_match(subexp, date, REMDATEMATCH);
		append_match(subexp, remfrom, REMSYSMATCH);
	} else {
		fprintf(stderr, "to822: From_ line missing or malformed.\n");
		return -1;
	}

	/* get any pre-existing RFC822 header lines */
	getheader(fgets, in);

	/*
	 *  Output new message.  If there is an original From: line with
	 *  an address in the requested domain, leave it alone.  Otherwise
	 *  generate a From: line and turn any existing one into
	 *  Original-From:
	 */
	if (fromhdr.line != NULL) {
		faddr = getaddr(HCONTENT(fromhdr));
		if(domain!=NULL && !indomain(s_to_c(faddr), domain)){
			sprintf(xbuf, "%s %s\n", fromhdr.name, sender);
			(*fputsp)(xbuf, out);
			originalfrom = 1;
		}
	} else {
		sprintf(xbuf, "%s %s\n", fromhdr.name, sender);
		(*fputsp)(xbuf, out);
	}
	if (datehdr.line == NULL){
		sprintf(xbuf, "%s %s\n", datehdr.name, convertdate((s_to_c(date))));
		(*fputsp)(xbuf, out);
	}
	if (tohdr.line == NULL && rcvr != NULL){
		sprintf(xbuf, "%s %s\n", tohdr.name, rcvr);
		(*fputsp)(xbuf, out);
	}
		
	printheaders(fputsp, out, originalfrom);
	(*fputsp)("\n", out);
	printbodies(fputsp, out);
	n = 0;
	while (fgets(xbuf, sizeof(xbuf), in)!=NULL) {
		(*fputsp)(xbuf, out);
		n += strlen(xbuf)+1;
		if (n>1024) {
			n -= 1024;
			setalarm(5 * 60, "sending mail data");
			if (debug)
				putc('.', stderr);
		}
	}
	s_free(from);
	s_free(date);

	return 0;
}

/* juggle date fields
	Unix ctime: Mon Oct 19 19:47:25 EDT 1987
	822 standard: [Mon, ] 19 Oct 87 19:47:25 EDT
		(sometimes the , is omitted; sometimes (Mon) at end)
 */
static char *
convertdate(date)
char *date;
{
	static string *ndate=NULL;
	char *field[6];
	char *sp;
	int i;

	sp = date;
	/* parse the date into fields */
	for (i = 0; i < 6; i++) {		
		while (isspace(*sp) || *sp == ',' || *sp == '-')
			*sp++ = '\0';
		field[i] = sp;
		while (!isspace(*sp) && *sp != ',' && *sp != '-' && *sp != '\0')
			sp++;
	}
	*sp = '\0';
	if (strlen(field[5])==4)
		field[5] += 2;

	/* shuffle the fields into internet format */
	if (ndate==NULL)
		ndate = s_new();
	s_restart(ndate);
	s_append(ndate, field[0]);
	s_append(ndate, ", ");
	s_append(ndate, field[2]);
	s_append(ndate, " ");
	s_append(ndate, field[1]);
	s_append(ndate, " ");
	s_append(ndate, field[5]);
	s_append(ndate, " ");
	s_append(ndate, field[3]);
	s_append(ndate, " ");
	s_append(ndate, field[4]);
	return s_to_c(ndate);
}

/*
 *  return true if addr belongs to domain
 */
static int
indomain(addr, domain)
	char *addr;
	char *domain;
{
	int n, m;
	int punct;

	n = strlen(domain);
	m = strlen(addr);
	if(m<n)
		return 0;
	punct = *(addr+m-n);
	if(punct!='@' && punct!='.')
		return 0;
	return(cistrncmp(addr+m-n+1, domain+1, n-1)==0);
}
