/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/doscan.c	2.36.2.1"
/*LINTLIBRARY*/
#include "synonyms.h"
#include <stdio.h>
#include <ctype.h>
#include <varargs.h>
#include <values.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define NCHARS	(1 << BITSPERBYTE)

/* if the _IOWRT flag is set, this must be a call from sscanf */
#define locgetc()	(chcount+=1,(iop->_flag & _IOWRT) ? \
				((*iop->_ptr == '\0') ? EOF : *iop->_ptr++) : \
				getc(iop))
#define locungetc(x)	(chcount-=1, (x == EOF) ? EOF : \
				((iop->_flag & _IOWRT) ? *(--iop->_ptr) : \
				  (++iop->_cnt, *(--iop->_ptr))))

extern int read();

extern unsigned char	_numeric[];

static int 		number();
static int 		readchar();
static unsigned char	*setup();
static int 		string();

static int chcount,flag_eof;

#define	MAXARGS	30	/* max. number of args for fast positional paramters */
static int _mkarglst();

/* stva_list is used to subvert C's restriction that a variable with an
 * array type can not appear on the left hand side of an assignment operator.
 * By putting the array inside a structure, the functionality of assigning to
 * the whole array through a simple assignment is achieved..
 */
typedef struct stva_list {
	va_list	ap;
} stva_list;

int
_doscan(iop, fmt, va_alist)
register FILE *iop;
register unsigned char *fmt;
va_list va_alist;
{
	char tab[NCHARS];
	register int ch;
	int nmatch = 0, len, inchar, stow, size;

	/* variables for postional parameters */
	unsigned char	*sformat = fmt;	/* save the beginning of the format */
	int	fpos = 1;	/* 1 if first postional parameter */
	stva_list	args,	/* used to step through the argument list */
			sargs;	/* used to save the start of the argument list */
	stva_list	arglst[MAXARGS]; /* array giving the appropriate values
					  * for va_arg() to retrieve the
					  * corresponding argument:
					  * arglst[0] is the first argument
					  * arglst[1] is the second argument, etc.
					  */

	/* Check if readable stream */
	if (!(iop->_flag & (_IOREAD | _IORW))) {
		errno = EBADF;
		return(EOF);
	}

	/* Initialize args and sargs to the start of the argument list.
	 * Note that ANSI guarantees that the address of the first member of
	 * a structure will be the same as the address of the structure. */
	args = sargs = *(struct stva_list *)&va_alist;

	chcount=0; flag_eof=0;

	/*******************************************************
	 * Main loop: reads format to determine a pattern,
	 *		and then goes to read input stream
	 *		in attempt to match the pattern.
	 *******************************************************/
	for ( ; ; ) 
	{
		if ( (ch = *fmt++) == '\0')
			return(nmatch); /* end of format */
		if (isspace(ch)) 
		{
		  	if (!flag_eof) 
			{
			   while (isspace(inchar = locgetc()))
	 		        ;
			   if (locungetc(inchar) == EOF)
				flag_eof = 1;
		        }
		  continue;
                }
		if (ch != '%' || (ch = *fmt++) == '%') 
                {
			if ( (inchar = locgetc()) == ch )
				continue;
			if (locungetc(inchar) != EOF)
				return(nmatch); /* failed to match input */
			break;
		}
	charswitch:
		if (ch == '*') 
		{
			stow = 0;
			ch = *fmt++;
		}
		else
			stow = 1;

		for (len = 0; isdigit(ch); ch = *fmt++)
			len = len * 10 + ch - '0';
		if (ch == '$') 
		{
			/* positional parameter handling - the number
			 * specified in len gives the argument to which
			 * the next conversion should be applied.
			 * WARNING: This implementation of positional
			 * parameters assumes that the sizes of all pointer
			 * types are the same. (Code similar to that
			 * in the portable doprnt.c should be used if this
			 * assumption does not hold for a particular
			 * port.) */
			if (fpos) 
			{
				if (_mkarglst(sformat, sargs, arglst) != 0)
					return(EOF);
			}
			if (len <= MAXARGS) 
			{
				args = arglst[len - 1];
			} else {
				args = arglst[MAXARGS - 1];
				for (len -= MAXARGS; len > 0; len--)
					(void)va_arg(args.ap, void *);
			}
			len = 0;
			ch = *fmt++;
			goto charswitch;
		}

		if (len == 0)
			len = MAXINT;
		if ( (size = ch) == 'l' || (size == 'h') || (size == 'L') )
			ch = *fmt++;
		if (ch == '\0' ||
		    ch == '[' && (fmt = setup(fmt, tab)) == NULL)
			return(EOF); /* unexpected end of format */
		if (isupper(ch))  /* no longer documented */
		{
			if (_lib_version == c_issue_4)
				size = 'l';
			ch = _tolower(ch);
		}
		if (ch!= 'n' && !flag_eof)
		  if (ch != 'c' && ch != '[') 
		  {
			while (isspace(inchar = locgetc()))
				;
			if(locungetc(inchar) == EOF )
				break;
		  }
		switch(ch)
		{
		 case 'c':
		 case 's':
		 case '[':
			  size = string(stow,ch,len,tab,iop,&args.ap);
			  break;
                 case 'n':
			  if (size == 'h')
				*va_arg(args.ap, short *) = (short) chcount;
		          else if (size == 'l')
				*va_arg(args.ap, long *) = (long) chcount;
			  else
			  	*va_arg(args.ap, int *) = (int) chcount;
			  continue;
		 case 'i':
                 default:
			 size = number(stow, ch, len, size, iop, &args.ap);
			 break;
                 }
		   if (size) 
		          nmatch += stow;   
                   else return ((flag_eof && !nmatch) ? EOF : nmatch);
                continue;
	}
	return (nmatch != 0 ? nmatch : EOF); /* end of input */
}

/***************************************************************
 * Functions to read the input stream in an attempt to match incoming
 * data to the current pattern from the main loop of _doscan().
 ***************************************************************/
static int
number(stow, type, len, size, iop, listp)
int stow, type, len, size;
register FILE *iop;
va_list *listp;
{
	char numbuf[64], inchar, lookahead;
	register char *np = numbuf;
	register int c, base;
	int digitseen = 0, dotseen = 0, expseen = 0, floater = 0, negflg = 0;
	long lcval = 0;
	switch(type) 
	{
	case 'e':
	case 'f':
	case 'g':
		floater++;
		/* FALLTHROUGH */
	case 'd':
	case 'u':
        case 'i':
		base = 10;
		break;
	case 'o':
		base = 8;
		break;
	case 'x':
	case 'p':
		base = 16;
		break;
	default:
		return(0); /* unrecognized conversion character */
	}
	switch(c = locgetc()) 
	{
	case '-':
		negflg++;
		/* FALLTHROUGH */
	case '+':
		if (--len <= 0)
		   break;
		if ( (c = locgetc()) != '0')
		   break;
		/* FALLTHROUGH */
        case '0':
                if ( (type != 'i' && type != 'x') || (len <= 1) )  
		   break;
	        if ( ((inchar = locgetc()) == 'x') || (inchar == 'X') ) 
	        {
		   lookahead = readchar(iop);
		   if ( isxdigit(lookahead) )
		   {
		       base =16;

		       if ( len <= 2)
		       {
			  locungetc(lookahead);
			  len -= 1;            /* Take into account the 'x'*/
                       }
		       else 
		       {
		          c = lookahead;
			  len -= 2;           /* Take into account '0x'*/
		       }
                   }
	           else
	           {
	               locungetc(lookahead);
	               locungetc(inchar);
                   }
		}
	        else
	        {
		    locungetc(inchar);
		    if (type == 'i')
	            	base = 8;
                }
	}
	for (; --len  >= 0 ; *np++ = (char)c, c = locgetc()) 
	{
		if (np > numbuf + 62)           
		{
		    errno = ERANGE;
		    return(0);
                }
		if (isdigit(c) || base == 16 && isxdigit(c)) 
		{
			int digit = c - (isdigit(c) ? '0' :  
			    isupper(c) ? 'A' - 10 : 'a' - 10);
			if (digit >= base)
				break;
			if (stow && !floater)
				lcval = base * lcval + digit;
			digitseen++;


			continue;
		}
		if (!floater)
			break;
		if (c == _numeric[0] && !dotseen++)
			continue;
		if ( (c == 'e' || c == 'E') && digitseen && !expseen++) 
                {
			if (--len < 0)
				break;
			inchar = readchar(iop);
			if (isdigit(inchar))
			{
				*np++ = (char)c;
				c = inchar;
				continue;
			}
			else if (inchar == '+' || inchar == '-' ||
				(isspace(inchar) && (_lib_version == c_issue_4)))
			{
				if ((len-2) < 0)
				{
					locungetc(inchar);
					break;
				}
				--len;
				lookahead = readchar(iop);
				if (isdigit(lookahead))
				{
					*np++ = (char)c;
					*np++ = (char)inchar;
					c = lookahead;
					continue;
				}
				else
				{
					locungetc(lookahead);
					locungetc(inchar);
				}
			}
			else
			{
				locungetc(inchar);
			}
		}
		break;
	}


	if (stow && digitseen)
		if(floater) 
		{
			register double dval;
	
			*np = '\0';
			dval = atof(numbuf);
			if (negflg)
				dval = -dval;
			if (size == 'l' || size =='L')
				*va_arg(*listp, double *) = dval;
			else
				*va_arg(*listp, float *) = (float)dval;
		}
		else
		{
			/* suppress possible overflow on 2's-comp negation */
			if (negflg && lcval != HIBITL)
				lcval = -lcval;
			if (size == 'l')
				*va_arg(*listp, long *) = lcval;
			else if (size == 'h')
				*va_arg(*listp, short *) = (short)lcval;
			else
				*va_arg(*listp, int *) = (int)lcval;
		}
	if (locungetc(c) == EOF)
	    flag_eof=1;
	return (digitseen); /* successful match if non-zero */
}

/* Get a character. If not using sscanf and at the buffer's end 
 * then do a direct read(). Characters read via readchar()
 * can be  pushed back on the input stream by locungetc()
 * since there is padding allocated at the end of the stream buffer. */
static int
readchar(iop)
FILE	*iop;
{
	char	inchar;

	if ((iop->_flag & _IOWRT) || (iop->_cnt != 0))
		inchar = locgetc();
	else
	{
		if ( read(fileno(iop),&inchar,1) != 1)
			return(EOF);
		chcount += 1;
	}
	return(inchar);
}

static int
string(stow, type, len, tab, iop, listp)
register int stow, type, len;
register char *tab;
register FILE *iop;
va_list *listp;
{
	register int ch;
	register char *ptr;
	char *start;

	start = ptr = stow ? va_arg(*listp, char *) : NULL;
	if (type == 'c' && len == MAXINT)
		len = 1;
	while ( (ch = locgetc()) != EOF &&
	    !(type == 's' && isspace(ch) || type == '[' && tab[ch])) 
        {
		if (stow) 
			*ptr = (char)ch;
		ptr++;
		if (--len <= 0)
			break;
	}
	if (ch == EOF ) 
	{
	       flag_eof = 1;
	       chcount-=1;
        }
        else if (len > 0 && locungetc(ch) == EOF)
		flag_eof = 1;
	if (ptr == start)
		return(0); /* no match */
	if (stow && type != 'c')
		*ptr = '\0';
	return (1); /* successful match */
}

static unsigned char *
setup(fmt, tab)
register unsigned char *fmt;
register char *tab;
{
	register int b, c, d, t = 0;

	if (*fmt == '^') 
	{
		t++;
		fmt++;
	}
	(void) memset(tab, !t, NCHARS);
	if ( (c = *fmt) == ']' || c == '-')  /* first char is special */
	{
		tab[c] = (char)t;
		fmt++;
	}
	while ( (c = *fmt++) != ']') 
	{
		if (c == '\0')
			return(NULL); /* unexpected end of format */
		if (c == '-' && (d = *fmt) != ']' && (b = fmt[-2]) < d) 
		{
			(void) memset(&tab[b], t, d - b + 1);
			fmt++;
		} 
		else
			tab[c] = (char)t;
	}
	return (fmt);
}


/* This function initializes arglst, to contain the appropriate 
 * va_list values for the first MAXARGS arguments.
 * WARNING: this code assumes that the sizes of all pointer types
 * are the same. (Code similar to that in the portable doprnt.c
 * should be used if this assumption is not true for a
 * particular port.) */
static int
_mkarglst(fmt, args, arglst)
char	*fmt;
stva_list args;
stva_list arglst[];
{
	int maxnum, n, curargno, flags;

	maxnum = -1;
	curargno = 0;
	while ((fmt = strchr(fmt, '%')) != 0)
	{
		fmt++;	/* skip % */
		if (*fmt == '*' || *fmt == '%')
			continue;
		if (fmt[n = strspn(fmt, "01234567890")] == '$')
		{
			curargno = atoi(fmt) - 1;	/* convert to zero base */
			fmt += n + 1;
		}
		if (maxnum < curargno)
			maxnum = curargno;
		curargno++;	/* default to next in list */

		fmt += strspn(fmt, "# +-.0123456789hL$");
		if (*fmt == '[') {
			fmt++; /* has to be at least on item in scan list */
			if ((fmt = strchr(fmt, ']')) == NULL)
				return(-1); /* bad format */
		}
	}
	if (maxnum > MAXARGS)
		maxnum = MAXARGS;
	for (n = 0 ; n <= maxnum; n++)
	{
		arglst[n] = args;
		(void)va_arg(args.ap, void *);
	}
	return(0);
}
