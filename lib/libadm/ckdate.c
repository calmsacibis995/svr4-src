/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libadm:ckdate.c	1.4.3.1"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

extern int	ckquit;

extern long	strtol();
extern void	putprmpt(),
		puterror(),
		puthelp();
extern int	getinput(),
		puttext();

#define MSGSIZ	64
#define PROMPT	"Enter the date"
#define MESG	"Please enter a date"
#define DEFAULT	"%m/%d/%y"

static char	*p_ndigit(),
		*p_date(),
		*p_eday(),
		*p_dlm();

#define MLIM 9
#define STDIG 2
#define LD2 10
#define LD 01
#define UD 31
#define LM 01
#define UM 12
#define LY 70
#define UY 99
#define LCY 1970
#define UCY 9999
#define CCYY 4
#define DELIM1 '/'
#define DELIM2 '-'
#define BLANK ' '
#define TAB '	'

static void
setmsg(msg, fmt)
char	*msg, *fmt;
{
	if((fmt == NULL) || !strcmp(fmt, "%D"))
		fmt = "%m/%d/%y";
	(void) sprintf(msg, "%s. Format is <%s>.", MESG, fmt);
}

static char *
p_ndigit(string, value, n)
char *string;
int *value, n;
{
	char *ptr;
	int accum = 0;

	if(!string) return (0);
	for (ptr = string; *ptr && n>0; n--, ptr++) {
		if(! isdigit(*ptr)) return (NULL);
		accum = (10 * accum) + (*ptr - '0');
		}
	if(n) return(NULL);
	*value = accum;
	return(ptr);
}

static char *
p_date(string, llim, ulim, ndig)
char *string;
int llim, ulim, ndig;
{
	char *ptr;
	int begin = -1;

	if(!(ptr = (char *)p_ndigit(string, &begin, ndig)))
		return (NULL);
	if(begin >= llim && begin <= ulim)
		return(ptr);
	else return(NULL);
}

static char *
p_eday(string, llim, ulim)
char *string;
int llim, ulim;
{
	char *ptr, *copy;
	char daynum[3];
	int begin = -1; 
	int iday = 0;
	int idaymax = 2;

	daynum[0] = '\0';
	if (*string == BLANK) {
		*string++;
		idaymax--;
	}
	copy = string;
	while (isdigit(*copy) && (iday < idaymax)) {
		daynum[iday] = *copy++ ;
		iday++;
	}
	daynum[iday] = '\0';
	if(iday == 1) {
		llim = 1;
		ulim = 9;
	} else if(iday == 2) {
		llim = 10;
		ulim = 31;
	}
	if(iday == 0) 
		return(NULL);

	if(!(ptr = (char *)p_ndigit(string, &begin, iday)))
		return (NULL);

	if(begin >= llim && begin <= ulim)
		return(ptr);
	else return (NULL);
}

/* p_month will parse the string for the month - abbr. form i.e. JAN - DEC */

static char *
p_month(string, mnabr)
char *string, mnabr;
{
	static char *fmonth[] = {
		 "JANUARY", "FEBRUARY", "MARCH", "APRIL", 
		 "MAY", "JUNE", "JULY", "AUGUST", 
		 "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER" 
	};
	static char *amonth[] = {
		 "JAN", "FEB", "MAR", "APR", "MAY", "JUN", 
		 "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" 
	};
	int ichng, icnt;
	char *mnth[12];
	char *copy;
	char mletter[MLIM];
	int mlen;
	int imnth = 0;
	int legit = 0;
	int n = 0;

	if(mnabr == 'a') {
		mlen = 3;
		for (icnt = 0; icnt < 12; icnt++)
			mnth[icnt] = amonth[icnt];
	} else {
		mlen = 9;
		for (icnt = 0; icnt < 12; icnt++)
			mnth[icnt] = fmonth[icnt];
	}

	copy = string;

	while (((islower(*copy)) || (isupper(*copy)))
	     && (imnth < mlen )) {
		mletter[imnth] = toupper(*copy++) ;
		imnth++;
	}
	mletter[imnth] = '\0';
	while (!(legit) && (n < 12)) {
		if(strncmp(mletter, mnth[n], (imnth = strlen(mnth[n]))) == 0)
			legit = 1;	/* found legitimate string */
		n++;
	}
	if(legit) {
		for (ichng = 0; ichng < imnth; ichng++) {
			*string = toupper(*string);
			string++;
		}

		return(string);
		/* I know this causes side effects, but it's less
		  code  than adding in a copy for string and using that */
	} else 
		return(NULL);
}

static char *
p_dlm(string, dchoice)
char *string, dchoice;
{
	char dlm;


	if(! string) return(NULL);
	(void) sscanf(string, "%1c", &dlm);
	if(dchoice == '/')
		return(((dlm == DELIM1) || (dlm == DELIM2)) ? string+1 : NULL);
	else
		return((dlm == dchoice) ? string + 1 : NULL);
}

int
ckdate_err(fmt, error)
char	*fmt, *error;
{
	char	defmesg[MSGSIZ];

	if(fmtcheck(fmt) == 1)
		return(4);
	setmsg(defmesg, fmt);
	puterror(stdout, defmesg, error);
	return(0);
}

int
ckdate_hlp(fmt, help)
char	*fmt, *help;
{
	char	defmesg[MSGSIZ];

	if(fmtcheck(fmt) == 1)
		return(4);
	setmsg(defmesg, fmt);
	puthelp(stdout, defmesg, help);
	return(0);
}

/*
*	A little state machine that checks out the format to
*	make sure it is acceptable.
*		return value 1: NG
*		return value 0: OK
*/
int
fmtcheck(fmt)
char	*fmt;
{
	int	percent = 0;

	while(*fmt) {
		switch(*fmt++) {
		 	case '%': /* previous state must be start or letter */
				if(percent == 0)
					percent = 1;
				else
					return(1);
				break;
			case 'd': /* previous state must be "%"*/
			case 'e':
			case 'm':
			case 'y':
			case 'Y':
			case 'D':
			case 'h':
			case 'b':
			case 'B':
				if(percent == 1)
					percent = 0;
				else
					return(1);
				break;
			case TAB: /* previous state must be start or letter */
			case BLANK:
			case DELIM1:
			case DELIM2:
				if(percent == 1)
					return(1);
				break;
			default:
				return(1);
		}
	}
	return(percent);
}

int
ckdate_val(fmt, input)
char *fmt, *input;
{
	char ltrl, dfl;
	int valid = 1; 	/* time of day string is valid for format */

	if((fmt != NULL) && (fmtcheck(fmt) == 1))
		return(4);

	if(fmt == NULL)
		fmt = DEFAULT;
	ltrl = dfl = '\0';
	while(*fmt && valid) {
		if((*fmt) == '%') {
			fmt++;
			switch (*fmt) {
			  case 'd': 
				input = p_date(input, LD, UD, STDIG);
				if(!input)
					valid = 0;
				break;

			  case 'e': 
				input = p_eday(input, LD2, UD);
				if(!input)
					valid = 0;
				break;

			  case 'm': 
				input = p_date(input, LM, UM, STDIG);
				if(!input)
					valid = 0;
				break;

			  case 'y': 
				input = p_date(input, LY, UY, STDIG);
				if(!input)
					valid = 0;
				break;

			  case 'Y': 
				input = p_date(input, LCY, UCY, CCYY);
				if(!input)
					valid = 0;
				break;

			  case 'D': 
				input = p_date(input, LM, UM, STDIG);
				if(!input) {
					valid = 0;
					break;
				}
				input = p_dlm(input, DELIM1);
				if(!input) {
					valid = 0;
					break;
				}
				input = p_date(input, LD, UD, STDIG);
				if(!input) {
					valid = 0;
					break;
				}
				input = p_dlm(input, DELIM1);
				if(!input) {
					valid = 0;
					break;
				}
				input = p_date(input, LY, UY, STDIG);
				if(!input)
					valid = 0;
				break;

			  case 'h': 
			  case 'b': 
				input = p_month(input, 'a');
				if(!input)
					valid = 0;
				break;

			  case 'B': 
				input = p_month(input, 'f');
				if(!input)
					valid = 0;
				break;

			  default: 
				(void) sscanf(input, "%1c", &ltrl);
				input++;
			}
		} else {
			dfl = '\0';
			(void) sscanf(input, "%1c", &dfl);
			input++;
		}
		fmt++;
	}	 /* end of while fmt and valid */

	if(!(*fmt) && (*input)) {
		if(*input) 
			valid = 0;
	}
	return((valid == 0)); 
}

ckdate(date, fmt, defstr, error, help, prompt)
char *date;
char *fmt;
char *prompt;
char *defstr, *error, *help;
{
	char	defmesg[MSGSIZ];
	char	input[128];
	char	*ept, end[128];

	ept = end;
	*ept = '\0';

	if((fmt != NULL) && (fmtcheck(fmt) == 1))
		return(4);

	setmsg(defmesg, fmt);
	(void) sprintf(ept, "[?,q]");
	
	if(!prompt)
		prompt = PROMPT;

start:
	putprmpt(stderr, prompt, NULL, defstr);
	if(getinput(input))
		return(1);

	if(!strlen(input)) {
		if(defstr) {
			(void) strcpy(date, defstr);
			return(0);
		}
		puterror(stderr, defmesg, error);
		goto start;
	} else if(!strcmp(input, "?")) {
		puthelp(stderr, defmesg, help);
		goto start;
	} else if(ckquit && !strcmp(input, "q")) {
		return(3);
	} else if(ckdate_val(fmt, input)) {
		puterror(stderr, defmesg, error);
		goto start;
	}
	(void) strcpy(date, input);
	return(0);
}
