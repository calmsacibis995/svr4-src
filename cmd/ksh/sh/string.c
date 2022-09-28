/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/string.c	1.2.3.1"
/*
 * string processing routines for Korn shell
 *
 */

#include	"defs.h"
#include	"sym.h"
#ifdef MULTIBYTE
#   include	"national.h"
#endif /* MULTIBYTE */

extern char	*utos();

/*
 * converts integer n into an unsigned decimal string
 */

char *sh_itos(n)
int n;
/*@
	return x satisfying atol(x)==n;
@*/ 
{
	return(utos((ulong)n,10));
}


/*
 * look for the substring <old> in <string> and replace with <new>
 * The new string is put on top of the stack
 */

char *sh_substitute(string,old,new)
const char *string;
const char *old;
char *new;
/*@
	assume string!=NULL && old!=NULL && new!=NULL;
	return x satisfying x==NULL ||
		strlen(x)==(strlen(in string)+strlen(in new)-strlen(in old));
@*/
{
	register const char *sp = string;
	register char *dp;
	register const char *cp;
	const char *savesp = NIL;
	dp = stak_begin();
	if(*sp==0)
		return(NIL);
	if(*(cp=old) == 0)
		goto found;
	do
	{
	/* skip to first character which matches start of old */
		while(*sp && (savesp==sp || *sp != *cp))
		{
#ifdef MULTIBYTE
			/* skip a whole character at a time */
			int c = *sp;
			c = echarset(c);
			c = in_csize(c) + (c>=2);
			while(c-- > 0)
#endif /* MULTIBYTE */
			*dp++ = *sp++;
		}
		if(*sp == 0)
			return(NIL);
		savesp = sp;
	        for(;*cp;cp++)
		{
			if(*cp != *sp++)
				break;
		}
		if(*cp==0)
		/* match found */
			goto found;
		sp = savesp;
		cp = old;
	}
	while(*sp);
	return(NIL);

found:
	/* copy new */
	dp = sh_copy(new,dp);
	/* copy rest of string */
	return(stak_end(sh_copy(sp,dp)));
}

/*
 * put string v onto the heap and return the heap pointer
 */

char *sh_heap(v)
register const char *v;
/*@
	return x satisfying (in v? strcmp(v,x)==0: x==0);
@*/
{
	register char *p;
	if(v)
	{
		sh_copy(v,p=malloc((unsigned)strlen(v)+1));
		return(p);
	}
	else
		return(0);
}


/*
 * TRIM(sp)
 * Remove escape charaters from characters in <sp> and eliminate quoted nulls.
 */

void	sh_trim(sp)
register char *	sp;
/*@
	assume sp!=NULL;
	promise  strlen(in sp) <= in strlen(sp);
@*/
{
	register char *dp;
	register int c;
	if(sp)
	{
		dp = sp;
		while(c= *sp++)
		{
			if(c == ESCAPE)
				c = *sp++;
			if(c)
				*dp++ = c;
		}
		*dp = 0;
	}
}

/*
 * copy string a to string b and return a pointer to the end of the string
 */

char *sh_copy(a,b)
register const char *a;
register char *b;
/*@
	assume a!=NULL && b!= NULL;
	promise strcmp(in a,in b)==0;
	return x satisfying (x-(in b))==strlen(in a);
 @*/
{
	while(*b++ = *a++);
	return(--b);
}

/*
 * G. S. Fowler
 * AT&T Bell Laboratories
 *
 * apply file permission expression expr to perm
 *
 * each expression term must match
 *
 *	[ugo]*[-&+|=]?[rwxst0-7]*
 *
 * terms may be combined using ,
 *
 * if non-null, e points to the first unrecognized char in expr
 */


#ifndef S_IRWXU
#   ifndef S_IREAD
#	define S_IREAD		00400
#	define S_IWRITE		00200
#	define S_IEXEC		00100
#   endif
#   ifndef S_ISUID
#	define S_ISUID		04000
#   endif
#   ifndef S_ISGID
#	define S_ISGID		02000
#   endif
#   ifndef S_ISVTX
#	define S_ISVTX		01000
#   endif
#   ifndef S_IRUSR
#	define S_IRUSR		S_IREAD
#	define S_IWUSR		S_IWRITE
#	define S_IXUSR		S_IEXEC
#	define S_IRGRP		(S_IREAD>>3)
#	define S_IWGRP		(S_IWRITE>>3)
#	define S_IXGRP		(S_IEXEC>>3)
#	define S_IROTH		(S_IREAD>>6)
#	define S_IWOTH		(S_IWRITE>>6)
#	define S_IXOTH		(S_IEXEC>>6)
#   endif

#   define S_IRWXU		(S_IRUSR|S_IWUSR|S_IXUSR)
#   define S_IRWXG		(S_IRGRP|S_IWGRP|S_IXGRP)
#   define S_IRWXO		(S_IROTH|S_IWOTH|S_IXOTH)
#endif


int
strperm(expr, e, perm)
char*		expr;
char**		e;
register int	perm;
/*@
	assume expr!=0;
	assume e==0 || *e!=0;
@*/
{
	register int	c;
	register int	typ;
	register int	who;
	int		num;
	int		op;

	for (;;)
	{
		op = num = who = typ = 0;
		for (;;)
		{
			switch (c = *expr++)
			{
			case 'u':
				who |= S_ISVTX|S_ISUID|S_IRWXU;
				continue;
			case 'g':
				who |= S_ISVTX|S_ISGID|S_IRWXG;
				continue;
			case 'o':
				who |= S_ISVTX|S_IRWXO;
				continue;
			case 'a':
				who = S_ISVTX|S_ISUID|S_ISGID|S_IRWXU|S_IRWXG|S_IRWXO;

				continue;
			default:
				if (c >= '0' && c <= '7') c = '=';
				expr--;
				/*FALLTHROUGH*/
			case '=':
			case '+':
			case '|':
			case '-':
			case '&':
				op = c;
				for (;;)
				{
					switch (c = *expr++)
					{
					case 'r':
						typ |= S_IRUSR|S_IRGRP|S_IROTH;
						continue;
					case 'w':
						typ |= S_IWUSR|S_IWGRP|S_IWOTH;
						continue;
					case 'x':
						typ |= S_IXUSR|S_IXGRP|S_IXOTH;
						continue;
					case 's':
						typ |= S_ISUID|S_ISGID;
						continue;
					case 't':
						typ |= S_ISVTX;
						continue;
					case ',':
					case 0:
						if (who) typ &= who;
						switch (op)
						{
						default:
							perm &= ~who;
							/*FALLTHROUGH*/
						case '+':
						case '|':
							perm |= typ;
							break;
						case '-':
							perm &= ~typ;
							break;
						case '&':
							perm &= typ;
							break;
						}
						if (c) break;
						/*FALLTHROUGH*/
					default:
						if (e) *e = expr - 1;
						return(perm);
					}
					break;
				}
				break;
			}
			break;
		}
	}
}
