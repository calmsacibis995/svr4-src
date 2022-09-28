/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:word.c	1.11.4.1"

/*
 * UNIX shell
 */

#include	"defs.h"
#include	"sym.h"
static 	int readb();

/* ========	character handling for command lines	========*/


word()
{
	register unsigned char	c, d;
	struct argnod	*arg = (struct argnod *)locstak();
	register unsigned char	*argp = arg->argval;
	unsigned char	*oldargp;
	int		alpha = 1;

	wdnum = 0;
	wdset = 0;

	while (1)
	{
		while (c = nextc(), space(c))		/* skipc() */
			;

		if (c == COMCHAR)
		{
			while ((c = readc()) != NL && c != EOF);
			peekc = c;
		}
		else
		{
			break;	/* out of comment - white space loop */
		}
	}
	if (!eofmeta(c))
	{
		do
		{
			if (c == LITERAL)
			{
				oldargp = argp;
				while ((c = readc()) && c != LITERAL)
				{ /* quote each character within single quotes */
					unsigned char *pc = readw(c);
					*argp++='\\';
				/* Pick up rest of multibyte character */
					if (c == NL)
						chkpr();
					while(c = *pc++)
						*argp++ = c;
				}
				if(argp == oldargp) { /* null argument - '' */
				/* Word will be represented by quoted null 
				   in macro.c if necessary */
					*argp++ = '"';
					*argp++ = '"';
				}
			}
			else
			{
				*argp++ = c;
				if(c == '\\') {
					*argp++ = readc();
				}
				if (c == '=')
					wdset |= alpha;
				if (!alphanum(c))
					alpha = 0;
				if (qotchar(c))
				{
					d = c;
					while ((*argp++ = (c = nextc())) && c != d)
					{
						if (c == NL)
							chkpr();
						/* don't interpret quoted 
						   characters */
						if (c == '\\') 
							*argp++ = readc();
					}
				}
			}
		} while ((c = nextc(), !eofmeta(c)));
		argp = endstak(argp);
		if (!letter(arg->argval[0]))
			wdset = 0;

		peekn = c | MARK;
		if (arg->argval[1] == 0 && 
		    (d = arg->argval[0], digit(d)) && 
		    (c == '>' || c == '<'))
		{
			word();
			wdnum = d - '0';
		}
		else		/*check for reserved words*/
		{
			if (reserv == FALSE || (wdval = syslook(arg->argval,reserved, no_reserved)) == 0)
			{
				wdarg = arg;
				wdval = 0;
			}
		}
	}
	else if (dipchar(c))
	{
		if ((d = nextc()) == c)
		{
			wdval = c | SYMREP;
			if (c == '<')
			{
				if ((d = nextc()) == '-')
					wdnum |= IOSTRIP;
				else
					peekn = d | MARK;
			}
		}
		else
		{
			peekn = d | MARK;
			wdval = c;
		}
	}
	else
	{
		if ((wdval = c) == EOF)
			wdval = EOFSYM;
		if (iopend && eolchar(c))
		{
			copy(iopend);
			iopend = 0;
		}
	}
	reserv = FALSE;
	return(wdval);
}

unsigned char skipc()
{
	register unsigned char c;

	while (c = nextc(), space(c))
		;
	return(c);
}

unsigned char nextc()
{
	register unsigned char	c, d;

retry:
	if ((d = readc()) == ESCAPE)
	{
		if ((c = readc()) == NL)
		{
			chkpr();
			goto retry;
		}
		peekc = c | MARK;
	}
	return(d);
}

unsigned char *readw(d)
unsigned char d;
{
	static unsigned char c[MULTI_BYTE_MAX + 1];
	int length;
	wchar_t l;
	if(!multibyte || d < 0200) {
		c[0] = d;
		c[1] = '\0';
		return(c);
	}
	peekc = d; /* will not overwrite peekc = '\\' */
	length = mbftowc(c, &l, readc, &peekc);
	if(length < 0)
		length = -length;
	c[length] = '\0';
	return(c);
}

unsigned char readc()
{
	unsigned register char	c;
	register int	len;
	register struct fileblk *f;

	if (peekn)
	{
		c = peekn;
		peekn = 0;
		return(c);
	}
	if (peekc)
	{
		c = peekc;
		peekc = 0;
		return(c);
	}
	f = standin;
retry:
	if (f->fnxt != f->fend)
	{
		if ((c = *f->fnxt++) == 0)
		{
			if (f->feval)
			{
				if (estabf(*f->feval++))
					c = EOF;
				else
					c = SP;
			}
			else
				goto retry;	/* = c = readc(); */
		}
		if (flags & readpr && standin->fstak == 0)
			prc(c);
		if (c == NL)
			f->flin++;
	}
	else if (f->feof || f->fdes < 0)
	{
		c = EOF;
		f->feof++;
	}
	else if ((len = readb()) <= 0)
	{
		if (f->fdes != input || !isatty(input)) {
			close(f->fdes);
			f->fdes = -1;
		}
		f->feof++;
		c = EOF;
	}
	else
	{
		f->fend = (f->fnxt = f->fbuf) + len;
		goto retry;
	}
	return(c);
}

static
readb()
{
	register struct fileblk *f = standin;
	register int	len;

	do
	{
		if (trapnote & SIGSET)
		{
			newline();
			sigchk();
		}
		else if ((trapnote & TRAPSET) && (rwait > 0))
		{
			newline();
			chktrap();
			clearup();
		}
	} while ((len = read(f->fdes, f->fbuf, f->fsiz)) < 0 && trapnote);
	return(len);
}
