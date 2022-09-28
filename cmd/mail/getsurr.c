/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:getsurr.c	1.3.3.1"
/*
    NAME
	getsurr - read the next field of the surrogate file

    SYNOPSIS
	getsurr(FILE *fp, string *buf, int firstfield)

    DESCRIPTION
	Read the next field from the surrogate file and store it into
	buf. These fields are surrounded by single quotes. Within the
	single quotes, \' will permit nesting.

	If firstfield is set, then this is the first field of a set
	of fields. An EOF is permitted here, as is the # comment
	character, plus the special

		Defaults:

	line which is used to reset the default settings of the
	transport return codes, via setsurg_rc().

	The resulting field is stored into buf.

    RETURN VALUES
	-1	Unnatural EOF or other error
	 0	Natural EOF
	 1	Regular return
*/
#include "mail.h"

static int getfield();

int getsurr (fp, buf, firstfield)
FILE *fp;
string *buf;
int  firstfield;
{
	static char	pn[] = "getsurr";
	int	ch;

	s_terminate(buf);

	while (1) {
		/* Skip leading whitespace and blank lines */
		while (((ch = fgetc(fp)) != EOF) && isspace(ch))
			;

		switch (ch) {
		case EOF:
			/* OK to hit EOF here */
			return (firstfield == TRUE ? 0 : -1);

		case '\'':
			return getfield(fp, buf);

		case 'd':
		case 'D':
			if (firstfield == TRUE) {
			    char lbuf[256];
			    string *s;

			    /* If default return code states redefined, handle here */
			    if (fgets(lbuf, sizeof(lbuf), fp) == (char *)NULL) {
				return (-1);
			    }

			    if (casncmp (lbuf, "efaults:", 8) != 0) {
				return (-1);
			    }

			    Tout(pn,"---------- Next '%s' entry ----------\n", mailsurr);

			    s = s_copy(lbuf+8);
			    if (setsurg_rc(s, DEFAULT, (int*)0) == (char *)NULL) {
			        s_free(s);
				return (-1);
			    }
			    s_free(s);
			    continue;
			}
			return (-1);

		case '#':
			if (firstfield == TRUE) {
				/* If we find a '#' before anything else on */
				/* the line, assume it's a comment indicator */
				/* and flush through the newline */
				while (((ch = fgetc(fp)) != '\n') && (ch != EOF))
					;
				if (ch == EOF) {
					return(0);
				}
				continue;
			}
			/* FALLTHROUGH */

		default:
			/* Trouble in River City... */
			return (-1);
		}
	}
}

static int getfield(fp, buf)
FILE *fp;
string *buf;
{
	int	ch;

	while ((ch = fgetc(fp)) != '\'') {
		switch (ch) {
		case EOF:
			/* Bad to hit EOF here */
			s_terminate(buf);
			return (-1);

		case '\n':
			/* Eat unescaped newline plus following whitespace */
			while (((ch = fgetc(fp)) != EOF) && isspace(ch))
				;

			if (ch == EOF) {
				s_terminate(buf);
				return (-1);
			}
			(void) ungetc(ch, fp);
			break;

		case '\\':
			/* Next char escaped. Take it regardless */
			ch = fgetc(fp);
			if (ch == EOF) {
				s_terminate(buf);
				return (-1);
			}
			/* FALLTHROUGH */

		default:
			s_putc(buf, ch);
			break;
		}
	}

	s_terminate(buf);
	return (1);
}
