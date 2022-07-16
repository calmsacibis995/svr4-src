/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libadm:ckitem.c	1.4.2.1"

#include <stdio.h>
#include <ctype.h>
#include "valtools.h"

extern int	ckquit;

extern void	*calloc(), *realloc();
extern long	strtol();
extern void	free(),
		puterror(),
		puthelp();
extern int	getinput(),
		puttext();
extern char
		*strcpy(),
		*strncpy(),
		*strcat(),
		*strncat(),
		*strchr(),
		*strrchr(),
		*strpbrk(),
		*strdup();
extern int
		strcmp(),
		strncmp(),
		strlen(),
		strspn(),
		strcspn();

void printmenu();

static int	insert();
static char	*deferr, *errmsg, *strtok(), **match();
static char	*defhlp;
static int	getstr(), getnum();
static struct _choice_ *next();

#define PROMPT	"Enter selection"
#define MESG0	"Entry does not match available menu selection. "
#define MESG1	"the number of the menu item you wish to select, or "
#define MESG2	"the token which is associated with the menu item,\
		or a partial string which uniquely identifies the \
		token for the menu item. Enter ?? to reprint the menu."
		
#define TOOMANY	"Too many items selected from menu"
#define NOTUNIQ	"The entered text does not uniquely identify a menu choice."
#define BADNUM	"Bad numeric choice specification"
static char *
setmsg(menup, flag)
CKMENU	*menup;
short	flag;
{
	int	n;
	char	*msg;

	n = 6 + sizeof(MESG2);
	if(flag)
		n += sizeof(MESG0);

	if(menup->attr & CKUNNUM) {
		msg = calloc(n, sizeof(char));
		if(flag)
			(void) strcpy(msg, MESG0);
		else
			msg[0] = '\0';
		(void) strcat(msg, "Enter ");
		(void) strcat(msg, MESG2);
	} else {
		msg = calloc(n+sizeof(MESG1), sizeof(char));
		if(flag)
			(void) strcpy(msg, MESG0);
		else
			msg[0] = '\0';
		(void) strcat(msg, "Enter ");
		(void) strcat(msg, MESG1);
		(void) strcat(msg, MESG2);
	}
	return(msg);
}

CKMENU *
allocmenu(label, attr)
char	*label;
int	attr;
{
	CKMENU *pt;

	if(pt = (CKMENU *) calloc(1, sizeof(CKMENU))) {
		pt->attr = attr;
		pt->label = label;
	}
	return(pt);
}

void
ckitem_err(menup, error)
CKMENU	*menup;
char	*error;
{
	deferr = setmsg(menup, 1);
	puterror(stdout, deferr, error);
	free(deferr);
}

void
ckitem_hlp(menup, help)
CKMENU	*menup;
char	*help;
{
	defhlp = setmsg(menup, 0);
	puthelp(stdout, defhlp, help);
	free(defhlp);
}

ckitem(menup, item, max, defstr, error, help, prompt)
CKMENU	*menup;
char	*item[];
short	max;
char	*defstr, *error, *help, *prompt;
{
	int	n, i;
	char	strval[128];
	char	**list;

	if((menup->nchoices <= 0) && !menup->invis)
		return(4); /* nothing to choose from */

	if(menup->attr & CKONEFLAG) {
		if(((n = menup->nchoices) <= 1) && menup->invis) {
			for(i=0; menup->invis[i]; ++i)
				n++;
		}
		if(n <= 1) {
			if(menup->choice)
				item[0] = menup->choice->token;
			else if(menup->invis)
				item[0] = menup->invis[0];
			item[1] = NULL;
			return(0);
		}
	}

	if(max < 1)
		max = menup->nchoices;

	if(!prompt)
		prompt = PROMPT;
	defhlp = setmsg(menup, 0);
	deferr = setmsg(menup, 1);

reprint:
	printmenu(menup);

start:
	if(n=getstr(strval, defstr, error, help, prompt)) {
		free(defhlp);
		free(deferr);
		return(n);
	}

	if(!strcmp(strval, "??")) {
		goto reprint;
	} if(!strcmp(strval, defstr)) { 
		item[0] = defstr;
		item[1] = NULL;
	} else {
		list = match(menup, strval, max);
		if(!list) {
			puterror(stderr, deferr, (errmsg ? errmsg : error)); 
			goto start;
		}
		for(i=0; (i < max); i++)
			item[i] = list[i];
		free((char *)list);
		item[i] = NULL;
	}
	free(defhlp);
	free(deferr);
	return(0);
}

static int
getnum(strval, max, begin, end)
char	*strval;
int	*begin, *end;
int	max;
{
	int n;
	char *pt;

	*begin = *end = 0;
	pt = strval;
	for(;;) {
		if(*pt == '$') {
			n = max;
			pt++;
		} else {
			n = strtol(pt, &pt, 10);
			if((n <= 0) || (n > max))
				return(1);
		}
		while(isspace(*pt))
			pt++;

		if(!*begin && (*pt == '-')) {
			*begin = n;
			pt++;
			while(isspace(*pt))
				pt++;
			continue;
		} else if(*pt) {
			return(1); /* wasn't a number, or an invalid one */
		} else if(*begin) {
			*end = n;
			break;
		} else {
			*begin = n;
			break;
		}
	}
	if(!*end)
		*end = *begin;
	return((*begin <= *end) ? 0 : 1);
}

static char **
match(menup, strval, max)
CKMENU *menup;
char *strval;
int max;
{
	struct _choice_ *chp;
	char **choice;
	int begin, end;
	char *pt, *found;
	int i, len, nchoice;
		
	nchoice = 0;
	choice = (char **) calloc((unsigned)max, sizeof(char *));

	do {
		if(pt = strpbrk(strval, " \t,")) {
			do {
				*pt++ = '\0';
			} while(strchr(" \t,", *pt));
		}

		if(nchoice >= max) {
			errmsg = TOOMANY;
			return((char **)0);
		}
		if(!(menup->attr & CKUNNUM) && isdigit(*strval)) {
			if(getnum(strval, menup->nchoices, &begin, &end)) {
				errmsg = BADNUM;
				return((char **)0);
			}
			chp = menup->choice;
			for(i=1; chp; i++) {
				if((i >= begin) && (i <= end)) {
					if(nchoice >= max) {
						errmsg = TOOMANY;
						return((char **)0);
					}
					choice[nchoice++] = chp->token;
				}
				chp = chp->next;
			}
			continue;
		}

		found = NULL;
		chp = menup->choice;
		for(i=0; chp; i++) {
			len = strlen(strval);
			if(!strncmp(chp->token, strval, len)) {
				if(chp->token[len] == '\0') {
					found = chp->token;
					break;
				} else if(found) {
					errmsg = NOTUNIQ;
					return((char **)0); /* not unique */
				}
				found = chp->token;
			}
			chp = chp->next;
		}

		if(menup->invis) {
			for(i=0; menup->invis[i]; ++i) {
				len = strlen(strval);
				if(!strncmp(menup->invis[i], strval, len)) {
					if(chp->token[len] == '\0') {
						found = menup->invis[i];
						break;
					} else if(found) {
						errmsg = NOTUNIQ;
						return((char **)0);
					}
					found = menup->invis[i];
				}
			}
		}
		if(found) {
			choice[nchoice++] = found;
			continue;
		}
		errmsg = NULL;
		return((char **)0);
	} while((strval = pt) && *pt);
	return(choice);
}

int
setitem(menup, choice)
CKMENU *menup;
char *choice;
{
	struct _choice_ *chp;
	int n;
	char *pt;

	if(choice == NULL) {
		/* request to clear memory usage */
		chp = menup->choice;
		while(chp) {
			(void) free(chp->token); /* free token and text */
			(void) free(chp);
			chp = chp->next;
			menup->longest = menup->nchoices = 0;
		}
	}
		
	chp = (struct _choice_ *)calloc(1, sizeof(struct _choice_));
	if(chp == NULL)
		return(1);

	pt = strdup(choice);
	if(pt == NULL)
		return(1);
	if(!*pt || isspace(*pt))
		return(2);

	chp->token = strtok(pt, " \t\n");
	chp->text = strtok(NULL, "");
	while(isspace(*chp->text))
		chp->text++;
	n = strlen(chp->token);
	if(n > menup->longest)
		menup->longest = (short) n;

	if(insert(chp, menup))
		menup->nchoices++;
	else
		free(chp); /* duplicate entry */
	return(0);
}

int
setinvis(menup, choice)
CKMENU	*menup;
char	*choice;
{
	int	index;

	index = 0;
	if(choice == NULL) {
		if(menup->invis == NULL)
			return(0);
		while(menup->invis[index])
			free(menup->invis[index]);
		free(menup->invis);
		return(0);
	}
		
	if(menup->invis == NULL)
		menup->invis = (char **) calloc(2, sizeof(char *));
	else {
		while(menup->invis[index])
			index++; /* count invisible choices */
		menup->invis = (char **) realloc((void *) menup->invis, 
			(index+2)* sizeof(char *));
	}
	if(!menup->invis)
		return(-1);
	menup->invis[index] = strdup(choice);
	return(0);
}
		
static int
insert(chp, menup)
struct _choice_ *chp;
CKMENU *menup;
{
	struct _choice_ *last, *base;
	int n;

	base = menup->choice;
	last = (struct _choice_ *)0;

	if(!(menup->attr & CKALPHA)) {
		while(base) {
			if(strcmp(base->token, chp->token) == 0)
				return 0;
			last = base;
			base = base->next;
		}
		if(last)
			last->next = chp;
		else
			menup->choice = chp;
		return 1;
	}

	while(base) {
		if((n = strcmp(base->token, chp->token)) == 0)
			return 0;
		if(n > 0) {
			/* should come before this one */
			break;
		}
		last = base;
		base = base->next;
	}
	if(last) {
		chp->next = last->next;
		last->next = chp;
	} else {
		chp->next = menup->choice;
		menup->choice = chp;
	}
	return 1;
} 

void
printmenu(menup)
CKMENU *menup;
{
	register int i;
	struct _choice_ *chp;
	char *pt;
	char format[16];
	int c;

	(void) fputc('\n', stderr);
	if(menup->label) {
		(void) puttext(stderr, menup->label, 0, 0);
		(void) fputc('\n', stderr);
	}
	(void) sprintf(format, "%%-%ds", menup->longest+5);

	(void) next((struct _choice_ *) 0);
	chp = ((menup->attr & CKALPHA) ? next(menup->choice) : menup->choice);
	for(i=1; chp; ++i) {
		if(!(menup->attr & CKUNNUM))
			(void) fprintf(stderr, "%3d  ", i);
		(void) fprintf(stderr, format, chp->token);
		if(chp->text) {
			/* there is text associated with the token */
			pt = chp->text;
			while(*pt) {
				(void) fputc(*pt, stderr);
				if(*pt++ == '\n') {
					if(!(menup->attr & CKUNNUM))
						(void) fprintf(stderr, "%5s", "");
					(void) fprintf(stderr, format, "");
					while(isspace(*pt))
						++pt;
				}
			}
		}
		(void) fputc('\n', stderr);
		chp = ((menup->attr & CKALPHA) ? 
			next(menup->choice) : chp->next);
		if(chp && ((i % 10) == 0)) {
			/* page the choices */
			(void) fprintf(stderr, "\n... %d more menu choices to follow;",
				 menup->nchoices - i);
			(void) fprintf(stderr, "\n<RETURN> for more choices, <CTRL-D> to stop display:");
			while(((c=getc(stdin)) != EOF) && (c != '\n'))
				; /* ignore other chars */
			(void) fputc('\n', stderr);
			if(c == EOF)
				break; /* stop printing menu */
		}
	}
}

static int
getstr(strval, defstr, error, help, prompt)
char	*strval;
char	*defstr, *error, *help, *prompt;
{
	char input[128];
	char *ept, end[128];

	*(ept=end) = '\0';
	if(defstr) {
		(void) sprintf(ept, "(default: %s) ", defstr);
		ept += strlen(ept);
	}
	(void) strcat(ept, "[?,??,q]");

start:
	(void) fputc('\n', stderr);
	puttext(stderr, prompt, 0, 0);
	(void) fprintf(stderr, " %s: ", end);

	if(getinput(input))
		return(1);

	if(strlen(input) == 0) {
		if(defstr) {
			(void) strcpy(strval, defstr);
			return(0);
		}
		puterror(stderr, deferr, (errmsg ? errmsg : error));
		goto start;
	} else if(!strcmp(input, "?")) {
		puthelp(stderr, defhlp, help);
		goto start;
	} else if(ckquit && !strcmp(input, "q")) {
		/*(void) strcpy(strval, input);*/
		return(3);
	}
	(void) strcpy(strval, input);
	return(0);
}

static struct _choice_ *
next(chp)
struct _choice_ *chp;
{
	static char *last;
	static char *first;
	struct _choice_ *found;

	if(!chp) {
		last = NULL;
		return(NULL);
	}

	found = (struct _choice_ *)0;
	for(first=NULL; chp; chp=chp->next) {
		if(last && strcmp(last, chp->token) >= 0)
			continue; /* lower than the last one we found */

		if(!first || strcmp(first, chp->token) > 0) {
			first = chp->token;
			found = chp;
		}
	}
	last = first;
	return(found);
}

static char *
strtok(string, sepset)
char	*string, *sepset;
{
	register char	*p, *q, *r;
	static char	*savept;

	/*first or subsequent call*/
	p = (string == NULL)? savept: string;

	if(p == 0)		/* return if no tokens remaining */
		return(NULL);

	q = p + strspn(p, sepset);	/* skip leading separators */

	if(*q == '\0')		/* return if no tokens remaining */
		return(NULL);

	if((r = strpbrk(q, sepset)) == NULL)	/* move past token */
		savept = 0;	/* indicate this is last token */
	else {
		*r = '\0';
		savept = ++r;
	}
	return(q);
}
