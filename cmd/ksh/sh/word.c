/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/word.c	1.5.3.1"

#include	"defs.h"
#include	"sym.h"
#include	"builtins.h"
#ifdef	NEWTEST
#   include	"test.h"
#endif	/* NEWTEST */



static struct argnod *letflg = 0;
static void setupalias();
static int here_copy();
static int here_tmp();
static int qtrim();

/* This module defines the following routines */
char	*match_paren();
int	sh_lex();

/* This module references these external routines */
extern char	*sh_tilde();

/* ========	character handling for command lines	========*/

/*
 * Get the next word and put it on the top of the stak
 * Determine the type of word and set sh.wdnum and sh.wdset accordingly
 * Returns the token type
 */

sh_lex()
{
	register int c;
	register int d;
	register char *argp;
	register char *tildp;
	char chk_keywd;
	int 	alpha = 0;
	sh.wdnum=0;
	sh.wdval = 0;
	/* condition needed to check for keywords, name=value */
	chk_keywd = (sh.reserv!=0 && !(sh.wdset&IN_CASE)) || (sh.wdset&KEYFLG);
	sh.wdset &= ~KEYFLG;
	if(letflg)
	{
		/* return argument inside ((...)) */
		sh.wdarg = letflg;
		letflg = 0;
		return(0);
	}
	sh.wdarg = (struct argnod*)stak_begin();
	sh.wdarg->argnxt.ap = 0;
	argp = sh.wdarg->argval;
	tildp = NULL;
	while(1)
	{
		while((c=io_nextc(), isblank(c)));
		if(c==COMCHAR)
		{
			while((c=io_readc()) != NL && c != ENDOF);
			io_unreadc(c);
		}
		else	 /* out of comment - white space loop */
			break;
	}
	if(c=='~')
		tildp = argp;
	if(!ismeta(c))
	{
		do
		{
			if(c==LITERAL)
			{
				argp = match_paren(argp,c,c);
				alpha = -1;
			}
			else
			{
				if(argp==sh.wdarg->argval&&chk_keywd&&isalpha(c))
				{
					alpha++;
				}
				if(argp >= (char*)sh.brkend)
					sh_addmem(BRKINCR);
				*argp++=(c);
				if(c == ESCAPE)
					*argp++ = io_readc();
				if(alpha>0)
				{
					if(c == '[')
					{
						argp = match_paren(argp,'[',']');
					}
					else if(c=='=')
					{
						sh.wdset |= KEYFLG;
						tildp = argp;
						alpha = 0;
					}
					else if(!isalnum(c))
						alpha = 0;
				}
				if(qotchar(c))
				{
					argp = match_paren(argp,c,c);
				}
			}
			d = c;
			c = io_nextc();
			if(d==DOLLAR && c==LBRACE)
			{
				*argp++ = c;
				argp = match_paren(argp, LBRACE, RBRACE);
				c = io_nextc();
			}
			else if(c==LPAREN && patchar(d))
			{
				if(d==DOLLAR)
					sh.nested_sub++;
				*argp++ = c;
				argp = match_paren(argp, LPAREN, RPAREN);
				c = io_nextc();
			}
			else if(tildp!=NULL &&  (c == '/'  || c==':' || ismeta(c)))
			{
				/* check for tilde expansion */
				register char *dir;
				*argp = 0;
				sh.staktop = argp;
				dir=sh_tilde(tildp);
				/* This check needed if sh_tilde() uses malloc() */
#ifndef INT16
				if(sh.stakbot != (STKPTR)sh.wdarg)
				{
					tildp += ((char*)sh.staktop-argp);
					argp = sh.staktop;
					sh.wdarg = (struct argnod*)sh.stakbot;
				}
#endif /*INT16 */
				if(dir)
				{
					argp=tildp;
					argp = sh_copy(dir,argp);
				}
				else
					tildp = NULL;
			}
			/* tilde substitution after : in variable assignment */
			/* left in as unadvertised compatibility feature */
			if(c==':' && (sh.wdset&KEYFLG))
				tildp = argp+1;
		}
		while(!ismeta(c));
		argp=stak_end(argp);
		io_unreadc(c);
#ifdef	NEWTEST
		if(sh.wdset&IN_TEST)
		{
			tildp = ((struct argnod*)argp)->argval;
			if(sh.wdset&TEST_OP1)
			{
				if(tildp[0]=='-' && tildp[2]==0 &&
					strchr(test_unops,tildp[1]))
				{
					sh.wdnum = tildp[1];
					sh.wdval = TESTUNOP;
				}
				else if(tildp[0]=='!' && tildp[1]==0)
				{
					sh.wdval = '!';
				}
				else
					sh.wdval = 0;
				sh.wdset &= ~TEST_OP1;
				return(sh.wdval);
			}
			c = sh_lookup(tildp, test_optable);
			switch(c)
			{
			case TEST_END:
				return(sh.wdval=ETSTSYM);

			default:
				if(sh.wdset&TEST_OP2)
				{
					sh.wdset &= ~TEST_OP2;
					sh.wdnum = c;
					return(sh.wdval=TESTBINOP);	
				}

			case TEST_OR: case TEST_AND:
			case 0:
				return(sh.wdval = 0);
			}
		}
#endif	/*NEWTEST */
		if(((struct argnod*)argp)->argval[1]==0 &&
			(d=((struct argnod*)argp)->argval[0],isdigit(d)) &&
			(c=='>' || c=='<'))
		{
			sh_lex();
			sh.wdnum |= (d-'0');
		}
		else
		{
			/*check for reserved words and aliases */
			sh.wdval = (sh.reserv!=0?sh_lookup(((struct argnod*)argp)->argval,tab_reserved):0);
			/* for unity database software, allow select to be aliased */
			if((sh.reserv!=0 && (sh.wdval==0||sh.wdval==SELSYM)) || (sh.wdset&CAN_ALIAS))
			{
				/* check for aliases */
				struct namnod* np;
				argp = ((struct argnod*)argp)->argval;
				if((sh.wdset&(IN_CASE|KEYFLG))==0 &&
					(np=nam_search(argp,sh.alias_tree,N_NOSCOPE))
					&& !nam_istype(np,M_FLAG)
					&& (argp=nam_strval(np)))
				{
					setupalias(argp,np);
					st.peekn = 0;
					nam_ontype(np,M_FLAG);
					sh.wdset |= KEYFLG;
					return(sh_lex());
				}
			}
		}
	}
	else if(dipchar(c))
	{
		sh.wdval = c;
		d = io_nextc();
		if(d==c)
		{
			sh.wdval = c|SYMREP;
			if(c=='<')
			{
				if((d=io_nextc())=='-')
					sh.wdnum |= IOSTRIP;
				else
					io_unreadc(d);
			}
			/* arithmetic evaluation ((expr)) */
			else if(c == LPAREN && sh.reserv != 0)
			{
				*argp++ =(DQUOTE);
				argp = match_paren(argp, LPAREN, RPAREN);
				*(argp-1)=(DQUOTE);
				c = io_nextc();
				if(c != ')')
				{
					/*
					 * process as nested () command
					 * for backward compatibility
					 */
					*argp++ = ')';
					*argp++ = c;
					stak_end(argp);
					qtrim(argp = sh.wdarg->argval);
					setupalias(argp,(struct namnod*)0);
					sh.wdval = st.peekn = '(';
				}
				else
				{
					stak_end(argp);
					letflg = sh.wdarg;
					sh.wdarg = (struct argnod*)stak_begin();
					stak_end(sh_copy(e_let,sh.wdarg->argval));
					sh.wdval = 0;
				}
			}
		}
		else if(c=='|')
		{
			if(d=='&')
				sh.wdval = COOPSYM;
			else
				io_unreadc(d);
		}
#ifdef DEVFD
		else if(d==LPAREN && iochar(c))
			sh.wdval = (c=='>'?OPROC:IPROC);
#endif	/* DEVFD */
		else if(c==';' && d=='&')
			sh.wdval = ECASYM;
		else
			io_unreadc(d);
	}
	else
	{
		if((sh.wdval=c)==ENDOF)
		{
			sh.wdval=EOFSYM;
			if(st.standin->ftype==F_ISALIAS)
				io_pop(1);
		}
		if(st.iopend && eolchar(c))
		{
			if(sh.owdval || is_option(NOEXEC))
				c = getlineno(1);
			if(here_copy(st.iopend)<=0 && sh.owdval)
			{
				sh.owdval = ('<'|SYMREP);
				sh.wdval = EOFSYM;
				sh.olineno = c;
				sh_syntax();
			}
			st.iopend=0;
		}
	}
	sh.reserv=0;
	return(sh.wdval);
}

static void setupalias(string,np)
char *string;
struct namnod *np;
{
	register struct fileblk *f;
	register int line;
	f = new_of(struct fileblk,0);
	line = st.standin->flin;
	io_push(f);
	io_sopen(string);
	f->flin = line-1;
	f->ftype = F_ISALIAS;
	f->feval = (char**)np;
	f->flast = st.peekn;
}

/*
 * read until matching <close>
 */

char *match_paren(argp,open,close)
register char *argp;
register int open;
{
	register int c;
	register int count = 1;
	register int quoted = 0;
	int was_dollar=0;
	int line = st.standin->flin;
	if(open==LITERAL)
		*argp++ = '"';
	while(count)
	{
		/* check for unmatched <open> */
		if(quoted || open==LITERAL)
			c = io_readc();
		else
			c = io_nextc();
		if(c==0)
		{
			/* eof before matching quote */
			/* This keeps old shell scripts running */
			if(filenum(st.standin)!=F_STRING || is_option(NOEXEC))
			{
				sh.olineno = line;
				sh.owdval = open;
				sh.wdval = EOFSYM;
				sh_syntax();
			}	
			io_unreadc(0);
			c = close;
		}
		if(c == NL)
		{
			if(open=='[')
			{
				io_unreadc(c);
				break;
			}
			sh_prompt(0);
		}
		else if(c == close)
		{
			if(!quoted)
				count--;
		}
		else if(c == open && !quoted)
			count++;
		if(open==LITERAL && (escchar(c) || c=='"'))
			*argp++ = ESCAPE;
		if(argp >= (char*)sh.brkend)
			sh_addmem(BRKINCR);
		*argp++ = c;
		if(open==LITERAL)
			continue;
		if(!quoted)
		{
			if(open==LBRACE && (c=='<'||c=='>') )
			{
				/* reserved for future use */
				sh.wdval = c;
				sh_syntax();
			}
			/* check for nested '', "", and `` within $() */
			if(open!=close) 
			{
				if(c==LITERAL)
					argp--;
				else if(!qotchar(c))
					goto skip;
				argp = match_paren(argp,c,c);
			}
			/* check for $() within (), '', "", and `` */
			else if(was_dollar && c==LPAREN)
			{
				if(open==c)
					count--;
				argp = match_paren(argp,LPAREN,RPAREN);
			}
		skip:
			was_dollar = (c==DOLLAR);
		}
		if(c == ESCAPE)
			quoted = 1 - quoted;
		else
			quoted = 0;
	}
	if(open==LITERAL)
		argp[-1] = '"';
	return(argp);
}

/*
 * read in here-document from script
 * small non-quoted here-documents are stored as strings 
 * quoted here documents, and here-documents without special chars are
 * treated like file redirection
 */

static int here_copy(ioparg)
struct ionod	*ioparg;
{
	register int	c;
	register char	*bufp;
	register struct ionod *iop;
	register char	*dp;
	int		fd = -1;
	int		match;
	int		savec = 0;
	int		special = 0;
	int		nosubst;
	char		obuff[IOBSIZE+1];
	if(iop=ioparg)
	{
		int stripflg = iop->iofile&IOSTRIP;
		register int nlflg;
		here_copy(iop->iolst);
		iop->iodelim=iop->ioname;
		/* check for and strip quoted characters in ends */
                nosubst = qtrim(iop->iodelim);
		if(stripflg)
			while(*iop->iodelim=='\t')
				iop->iodelim++;
		dp = iop->iodelim;
		match = 0;
		nlflg = stripflg;
		bufp = obuff;
		sh_prompt(0);	
		do
		{
			if(nosubst || savec==ESCAPE)
				c = io_readc();
			else
				c = io_nextc();
			if((savec = c)<=0)
				c = '\n';
			else
				special |= escchar(c);
			if(c=='\n')
			{
				if(match>0 && iop->iodelim[match]==0)
				{
					savec =1;
					break;
				}
				sh_prompt(0);	
				if(match>0)
					goto trymatch;
				nlflg = stripflg;
				match = 0;
				goto copy;
			}
			else if(c=='\t' && nlflg)
				continue;
			nlflg = 0;
			/* try matching delimiter when match>=0 */
			if(match>=0)
			{
			trymatch:
				if(iop->iodelim[match]==c)
				{
					match++;
					continue;
				}
				else if(--match>=0)
				{
					io_unreadc(c);
					dp = iop->iodelim;
					c = *dp++;
				}
			}
		copy:
			do
			{
				*bufp++ = c;
				if(bufp >= &obuff[IOBSIZE])
				{
					if(fd < 0)
						fd = here_tmp(iop);
					write(fd,bufp=obuff,(unsigned)IOBSIZE);
				}
			}
			while(c!='\n' && --match>=0 && (c= *dp++));
		}
		while(savec>0);
		if(c = (nosubst|!special))
                        iop->iofile &= ~IODOC;
		if(fd < 0)
		{
	                if(c)
				fd = here_tmp(iop);
			else
			{
	                        iop->iofile |= IOSTRG;
				*bufp = 0;
				iop->ioname = stak_copy(obuff);
				return(savec);
			}
		}
		if(bufp > obuff)
			write(fd, obuff, (unsigned)(bufp-obuff));
		close(fd);
	}
	return(savec);
}

/*
 * create a tempory file for a here document
 */

static int here_tmp(iop)
register struct ionod *iop;
{
	register int fd = io_mktmp((char*)0);
	iop->ioname = stak_copy(io_tmpname);
	iop->iolst=st.iotemp;
	st.iotemp=iop;
	return(fd);
}


/*
 * trim quotes and the escapes
 * returns non-zero if string is quoted 0 otherwise
 */

static int qtrim(string)
char *string;
{
	register char *sp = string;
	register char *dp = sp;
	register int c;
	register int quote = 0;
	while(c= *sp++)
	{
		if(c == ESCAPE)
		{
			quote = 1;
			c = *sp++;
		}
		else if(c == '"')
		{
			quote = 1;
			continue;
		}
		*dp++ = c;
	}
	*dp = 0;
	return(quote);
}
