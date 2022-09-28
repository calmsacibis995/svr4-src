/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)fmli:oeu/genparse.c	1.5"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>		/* EFT abs k16 */
#include "wish.h"
#include "io.h"
#include "retcds.h"
#include "parse.h"
#include "smdef.h"
#include "typetab.h"
#include "mail.h"
#include "terror.h"
#include	"moremacros.h"

long Befkwd;
/* les   static???
static char *strsave();
*/
static char *Nullstr = "";
int
skip(fp, p)
FILE *fp;
struct oeh *p;
{
	return(skiptokwd(fp));
}

long
getnum(fp, type)
FILE *fp;
int type;
{
	char c;
	int i;
	long atol();
	char buf[STR_SIZE];

	if (skipspace(fp) == KEYWORD)
		return(0L);
	for (i = 0, c = getc(fp); isdigit(c); i++, c = getc(fp))
		buf[i] = c;
	buf[i] = '\0';
	ungetc(c, fp);
	if (type == SKIP)
		skiptokwd(fp);
	return(atol(buf));
}

char *
uptokwd(fp)
FILE *fp;
{
	char *p;
	int c;
	int len;
	char buf[STR_SIZE];

	if (skipspace(fp) == KEYWORD)
		return(Nullstr);
	fgets(buf, STR_SIZE, fp);
	p = strsave(buf);
	len = strlen(p);
	p[len - 1] = '\0';
	while ((c = getc(fp)) == ' ' || c == '\t') {
		if (skipspace(fp) == KEYWORD)
			return(NULL);
		fgets(buf, STR_SIZE, fp);
		if ((p = realloc(p, (strlen(buf) + len + 3) * sizeof(char))) == NULL)
			fatal(NOMEM, NULL);
		strcat(p, " ");
		strcat(p, buf);
		len = strlen(p);
		p[len - 1] = '\0';
	}
	ungetc(c, fp);
	return(p);
}

int
nextkwd(fp, buf)
FILE *fp;
char *buf;
{
	int c;
	int i;

	Befkwd = ftell(fp);
	if ((c = getc(fp)) == '\n')
		return(PDONE);
	for (i = 0; c != '\n' && c != '\t' && c != ' ' && c != EOF; c = getc(fp), i++)
		if (i >= STR_SIZE)
			return(PDONE);
		else
			buf[i] = (char)LOWER(c); /* abs: added cast for lint */
	if (c == EOF)
		return(EOF);

	buf[i] = '\0';
	if (strncmp(buf, SEPLINE, 10) == 0)
		return(PDONE);
	ungetc(c, fp);
	/*if (buf[i - 1] != ':' && buf[0] != '>')*/
		/*return(NOTKEY);*/
	return(KEYWORD);
}

/*
** Could easily be made into a macro if preferrable
*/
int
skiptokwd(fp)
FILE *fp;
{
	int c;
	char buf[STR_SIZE];

	for (fgets(buf, STR_SIZE, fp); (c = getc(fp)) == ' ' || c == '\t'; fgets(buf, STR_SIZE, fp));
	if (c == EOF)
		return(EOF);
	ungetc(c, fp);
	return(KEYWORD);
}

int
skipspace(fp)
FILE *fp;
{
	int c;

	while ((c = getc(fp)) != EOF) {
		switch (c) {
		case '\t':
		case ' ':
			continue;
		case '\n':
			if ((c = getc(fp)) == ' ' || c == '\t')
				continue;
			else {
				ungetc(c, fp);
				return(KEYWORD);
			}
		default:
			ungetc(c, fp);
			return(WORD);
		}
	}
	return(EOF);
}
