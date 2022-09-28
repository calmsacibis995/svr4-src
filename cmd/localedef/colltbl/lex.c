/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)localedef:colltbl/lex.c	1.1.3.1"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "colltbl.h"
#include "y.tab.h"

#define TRUE	1
#define FALSE	0
#define SKIPWHITE()	while ((c = getchar()) == ' ' || c == '\t')
#define	isodigit(d)	(d >= '0' && d <= '7')
#define cktoklen(i)	if (i >= BUFSIZ) error(PRERR, "token too long")

extern int	Lineno;
static char	buf[BUFSIZ];
static int	type;
static int	start = TRUE;

yylex()
{
	int	c;
	int	i, j;
	int	clen;
	char    ibuf[6];
again:
	SKIPWHITE();
	/* c now contains the first non-space character */
	if (c == EOF)
		return(c);
	if (start) {
		switch (c) {
		case '#':
			while ((c = getchar()) != '\n')
				if (c == EOF)
					return(EOF);
		case '\n':
			Lineno++;
			goto again;
		default:
			i = 0;
			buf[i++] = c;
			while ((c = getchar()) != EOF && c != ' ' && c != '\t' && c != '\n') {
				cktoklen(i);
				buf[i++] = c;
			}
			ungetc(c, stdin);
			buf[i] = '\0';
			if (strcmp(buf, "codeset") == 0)
				type = CODESET;
			else if (strcmp(buf, "substitute") == 0)
				type = SUBSTITUTE;
			else if (strcmp(buf, "with") == 0)
				type = WITH;
			else if (strcmp(buf, "order") == 0)
				type = ORDER;
			else if (strcmp(buf, "is") == 0) {
				type = IS;
			} else
				type = buf[0];
		}
		if (type != ORDER)
			start = FALSE;
		return(type);
	}
	if (type == CODESET) {
		i = 0;
		while (c != EOF && c != ' ' && c != '\t' && c != '\n') {
			cktoklen(i);
			buf[i++] = c;
			c = getchar();
		}
		ungetc(c, stdin);
		buf[i] = '\0';
		yylval.sval = buf;
		start = TRUE;
		return(ID);
	}
	if (c == '"') {
		clen = 0;
		while ((c = getchar()) != '"') {
			cktoklen(clen);
			if (c == '\n' || c == EOF) {
				buf[clen] = '\0';
				error(NEWLINE, "string", buf);
				Lineno++;
			} else if (c == '\\') {
				switch(c = getchar()) {
				case '"': buf[clen++] = '"'; break;
				case 'n': buf[clen++] = '\n'; break;
				case 't': buf[clen++] = '\t'; break;
				case 'f': buf[clen++] = '\f'; break;
				case 'r': buf[clen++] = '\r'; break;
				case 'b': buf[clen++] = '\b'; break;
				case 'v': buf[clen++] = '\v'; break;
				case 'a': buf[clen++] = '\7'; break;
				case '\\': buf[clen++] = '\\'; break;
				default:
					if (isodigit(c)) {
						j = 0;
						do {
							ibuf[j++] = c;
							c = getchar();
						} while (isodigit(c) && j<3);
						ibuf[j] = '\0';
						buf[clen++] = strtol(ibuf, (char **)NULL, 8);
						ungetc(c, stdin);
					} else
						buf[clen++] = c;
					break;
				}
			} else
				buf[clen++] = c;
		}
		buf[clen] = '\0';
		yylval.sval = strdup(buf);
		start = TRUE;
		return(STRING);
	}

	/* SYMBOL */
	switch (c) {
	case '(':
	case '{':
	case ')':
	case '}':
		return(c);
		break;
	case '\n':
		Lineno++;
		start = TRUE;
		goto again;
	case ';':
		return(SEPARATOR);
	default:
		clen = 0;
		do {
			cktoklen(clen);
			if (c == '\\') {
				if ((c = getchar()) == '\n') {
					Lineno++;
					if (clen != 0) {
						buf[clen] = '\0';
						error(INVALID, "symbol", buf);
					}
					goto again;
				}
				ungetc(c, stdin);
				c = '\\';
			}
			if (c == '\\' || c == '0') {
				ibuf[0] = c;
				if ((c = getchar()) != 'x' && c != 'X' && !isodigit(c)) {
					buf[clen++] = ibuf[0];
					ungetc(c, stdin);
					continue;
				}
				ibuf[0] = '0';
				ibuf[1] = c;
				j = 2;
				if (c == 'x' || c == 'X') {
					while (isxdigit(c=getchar()) && j<5)
						ibuf[j++] = c;
				} else {
					c = getchar();
					while (isodigit(c) && j<5) {
						ibuf[j++] = c;
						c = getchar();
					}
				}
				ungetc(c, stdin);
				ibuf[j] = '\0';
				buf[clen++] = strtol(ibuf, (char **)NULL, 0);
			} else
				buf[clen++] = c;
		} while ((c = getchar()) != EOF && c != ';' && c != ')' && c != '}' 
			&& c != '\n');
		ungetc(c, stdin);

		buf[clen] = '\0';
		if (strcmp(buf, "...") == 0)
			return(ELLIPSES);
		if (clen > 2) {
			error(TOO_LONG, "symbol", buf);
			clen = 2;
		}
		yylval.sval = buf;
		return(SYMBOL);
	}
}
