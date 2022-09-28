/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/cmd.c	1.4.3.1"

#include	"defs.h"
#include	"sym.h"
#include	"history.h"
#include	"builtins.h"

/* These routines are referenced by this module */

static union anynode	*makelist();
static struct argnod	*qscan();
static struct ionod	*inout();
static void		chkword();
static void		chkflags();
static void		chksym();
static union anynode	*term();
static union anynode	*list();
#ifdef POSIX
   static union anynode	*andlist();
#endif /* POSIX */
static struct regnod	*syncase();
static union anynode	*item();
static int		skipnl();
static void		prsym();
int			getlineno();
#ifdef NEWTEST
#   include	"test.h"
    static union anynode	*test_expr();
    static union anynode	*test_and();
    static union anynode	*test_or();
    static union anynode	*test_primary();
#endif /* NEWTEST */


static int	heredoc;

#define getnode(type)	((union anynode*)stak_alloc(sizeof(struct type)))
/*
 * ========	command line decoding	========
 *
 *  This is the parser for a shell command line
 */




/*
 * Make a node which will cause the shell to fork
 */

union anynode	*sh_mkfork(flgs, i)
int 	flgs;
union anynode		*i;
{
	register union anynode	*t;
	t = getnode(forknod);
	t->fork.forktyp = flgs|TFORK;
	t->fork.forktre = i;
	t->fork.forkio = 0;
	return(t);
}

/*
 *  Make a node corresponding to a command list
 */

static union anynode	*makelist(type,i,r)
int 	type;
union anynode		*i, *r;
{
	register union anynode	*t;
	if(i==0 || r==0)
		sh_syntax();
	else
	{
		t = getnode(lstnod);
		t->lst.lsttyp = type;
		t->lst.lstlef = i;
		t->lst.lstrit = r;
	}
	return(t);
}

/*
 * cmd
 *	empty
 *	list
 *	list & [ cmd ]
 *	list [ ; cmd ]
 */

union anynode	*sh_parse(sym,flg)
register int 	sym;
int 	flg;
{
	register int flag = FINT|FAMP;
	register union anynode	*i, *e;
	struct ionod *saviotemp = st.iotemp;
	/* parser output goes on standard error */
	sh.wdset = 0;
	if(sym==NL)
		sh.owdval = 0;
	p_setout(ERRIO);
	i = list(flg);
	if(sh.wdval==NL)
	{
		if(flg&NLFLG)
		{
			sh.wdval=';';
			sh_prompt(0);
		}
	}
	else if(i==0 && (flg&MTFLG)==0)
		sh_syntax();
	switch(sh.wdval)
	{
		case COOPSYM:		/* set up a cooperating process */
			flag |= (FPIN|FPOU|FPCL|FCOOP);
				
		case '&':
			if(i)
			{
				if(saviotemp!=st.iotemp || heredoc)
					flag |= FTMP;
				i = sh_mkfork(flag, i);
			}
			else
				 sh_syntax();

		case ';':
			if(e=sh_parse(sym,flg|MTFLG))
				i=makelist(TLST, i, e);
			else if(i==0)
			{
				sh.wdval = ';';
				sh_syntax();
			}
			break;

		case EOFSYM:
			if(sym==NL)
				break;

		default:
			if(sym)
				chksym(sym);

	}
	return(i);
}

#ifdef POSIX
   /* precedence of && higher than || */
   /*
    * list
    *	andlist
    *	list || andlist
    */
static union anynode	*list(flg)
{
	register union anynode	*r = andlist(flg);
	while(r && sh.wdval==ORFSYM)
		r = makelist(TORF , r, andlist(NLFLG));
	return(r);
}
	
/*
 * andlist
 *	term
 *	and && term
 */

static union anynode	*andlist(flg)
{
	register union anynode	*r = term(flg);
	while(r && sh.wdval==ANDFSYM)
		r = makelist(TAND, r, term(NLFLG));
	return(r);
}
	
#else
/*
 * list
 *	term
 *	list && term
 *	list || term
 */

static union anynode	*list(flg)
{
	register union anynode	*r;
	register int 	b;
	r = term(flg);
	while(r && ((b=(sh.wdval==ANDFSYM)) || sh.wdval==ORFSYM))
	{
		r = makelist((b ? TAND : TORF), r, term(NLFLG));
	}
	return(r);
}
#endif /* POSIX */

/*
 * term
 *	item
 *	item | term
 */

static union anynode	*term(flg)
register int flg;
{
	register union anynode	*t;
	heredoc = 0;
	sh.reserv++;
	if(flg&NLFLG)
		skipnl();
	else
		 sh_lex();
	/* check to see if pipeline is to be timed */
#ifdef POSIX
	if(sh.wdval==TIMSYM || sh.wdval==NOTSYM)
#else
	if(sh.wdval==TIMSYM)
#endif /* POSIX */
	{
		t = getnode(parnod);
		t->par.partyp=TTIME;
#ifdef POSIX
		if(sh.wdval==NOTSYM)
			t->par.partyp |= COMSCAN;
#endif /* POSIX */
		t->par.partre = term(0);
	}
	else if((t=item(NLFLG|MTFLG)) && sh.wdval=='|')
	{
		register union anynode	*tt;
		flg = heredoc|FPOU;
		if(tt=term(NLFLG))
			t=makelist(TFIL,sh_mkfork(flg,t),sh_mkfork(FPIN|FPCL,tt));
		else
			chksym(0);
	}
	return(t);
}

/*
 * case statement
 */

static struct regnod*	syncase(esym)
register int esym;
{
	sh.wdset |= IN_CASE; 	/* set to avoid aliasing expressions */
	skipnl();
	if(sh.wdval==esym)
	{
		sh.wdset &= ~IN_CASE;
		return(0);
	}
	else
	{
		register struct regnod	*r=(struct regnod*) stak_alloc(sizeof(struct regnod));
		r->regptr=0;
		r->regflag=0;
		if(sh.wdval == '(')
			skipnl();
		while(1)
		{
			chkflags(A_EXP);
			sh.wdarg->argnxt.ap=r->regptr;
			r->regptr=sh.wdarg;
			if(sh.wdval==')' || sh.wdval=='|' || ( sh_lex()!=')' && sh.wdval!='|' ))
				sh_syntax();
			if(sh.wdval=='|')
				sh_lex();
			else
				break;
		}
		sh.wdset &= ~IN_CASE;
		r->regcom=sh_parse(0,NLFLG|MTFLG);
		if(sh.wdval==ECSYM)
			r->regnxt=syncase(esym);
		else if(sh.wdval==ECASYM)
		{
			r->regflag++;
			r->regnxt=syncase(esym);
		}
		else
		{
			chksym(esym);
			r->regnxt=0;
		}
		return(r);
	}
}

/*
 * item
 *
 *	( cmd ) [ < in ] [ > out ]
 *	word word* [ < in ] [ > out ]
 *	if ... then ... else ... fi
 *	for ... while ... do ... done
 *	case ... in ... esac
 *	begin ... end
 */

static union anynode	*item(flag)
int	flag;
{
	register union anynode	*t;
	register struct ionod	*io;
	int savwdval = sh.owdval;
	int savline = sh.olineno;
	if(flag)
		io=inout((struct ionod*)0,1);
	else
		io=0;
	if(sh.wdval && sh.wdval!=EOFSYM && sh.wdval!=PROCSYM)
	{
		sh.olineno =  getlineno(0);
		sh.owdval = sh.wdval;
	}
	switch(sh.wdval)
	{
#ifdef NEWTEST
		/* [[ ... ]] */
		case BTSTSYM:
			sh.wdset |= (IN_TEST|TEST_OP1); 
			t = test_expr(ETSTSYM);
			sh.wdset &= ~(IN_TEST|TEST_OP1); 
			break;
#endif /* NEWTEST */
		/* case statement */
		case CASYM:
		{
			t = getnode(swnod);
			chkword();
			chkflags(0);
			t->sw.swarg=sh.wdarg;
			skipnl();
			chksym(INSYM|BRSYM);
			t->sw.swlst=syncase(sh.wdval==INSYM?ESSYM:KTSYM);
			t->sw.swtyp=TSW;
			break;
		}

		/* if statement */
		case IFSYM:
		{
			register union anynode	*tt;
			register int w;
			t = getnode(ifnod);
			t->if_.iftyp=TIF;
			t->if_.iftre=sh_parse(THSYM,NLFLG);
			t->if_.thtre=sh_parse(ELSYM|FISYM|EFSYM,NLFLG);
			w = sh.wdval;
			t->if_.eltre=(w==ELSYM?sh_parse(FISYM,NLFLG):
				(w==EFSYM?(sh.wdval=IFSYM, tt=item(0)):0));
			if(w==EFSYM)
			{
				if(tt->tre.tretyp!=TSETIO)
					goto done;
				t->if_.eltre = tt->fork.forktre;
				tt->fork.forktre = t;
				t = tt;
				goto done;
			}
			break;
		}

		/* for and select statement */
		case FORSYM:
		case SELSYM:
		{
			t = getnode(fornod);
			t->for_.fortyp=(sh.wdval==FORSYM?TFOR:TSELECT);
			t->for_.forlst=0;
			chkword();
			t->for_.fornam=(char*) sh.wdarg->argval;
			if(skipnl()==INSYM)
			{
				chkword();
				t->for_.forlst=(struct comnod*) item(0);
				if(sh.wdval!=NL && sh.wdval!=';')
					sh_syntax();
				if(sh.wdval==NL)
					sh_prompt(0);
				skipnl();
			}
			/* 'for i;do cmd' is valid syntax */
			else if(sh.wdval==';')
			{
				sh.reserv = 1;
				sh_lex();
			}
			chksym(DOSYM|BRSYM);
			t->for_.fortre=sh_parse(sh.wdval==DOSYM?ODSYM:KTSYM,NLFLG);
			break;
		}

		/* This is the code for parsing function definitions */
		case PROCSYM:
		funct_5_2:
		{
			struct blk *blokptr;
			int savstates = st.states;
			int saveline = st.firstline;
			register struct fileblk *fd = NULL;
			struct ionod *saviotemp = st.iotemp;
			t = getnode(procnod);
			t->proc.proctyp=TPROC;
			t->proc.procloc = -1;
			st.firstline = st.standin->flin;
			t->proc.procline = st.firstline;
			flag = (sh.wdval==PROCSYM);
			if(flag)
				chkword();
			t->proc.procnam= (char*)sh.wdarg->argval;
			skipnl();
			if(is_option(INTFLG) && !is_option(NOLOG) && 
				st.standin->ftype==F_ISFILE)
			{
				/* just in case history file not open yet */
				if(hist_open())
				{
					fd = io_ftable[hist_ptr->fixfd];
					st.states |= FIXFLG;
					t->proc.procloc = 
						hist_position(hist_ptr->fixind) +
						fd->ptr - fd->base;
				}
			}
			if(flag)
				chksym(BRSYM);
			/* force a new stak frame to compile the command */
			sh_addblok(-1);
			blokptr = sh.stakbsy;
			if(t->proc.proctre=item(0))
			{
				/* force stak frame to save the command */
				sh_addblok(-1);
				/* save the entry point in block */
				t->proc.procblk = sh.stakbsy;
				while(sh.stakbsy->word < blokptr)
       	                         sh.stakbsy = sh.stakbsy->word;
				/*@ assert blokptr==sh.stakbsy @*/
				sh.stakbsy->word = 0;
				sh.stakbsy = blokptr;
			}
			else
				t->proc.procblk = blokptr;
			if(st.iotemp != saviotemp)
			{
				st.iotemp = saviotemp;
				st.states |= RM_TMP;
			}
			if(fd && !(savstates&FIXFLG))
			{
				hist_flush();
				hist_cancel();
				st.states &= ~FIXFLG;
			}
			st.firstline = saveline;
			return(t);
		}

		/* while and until */
		case WHSYM:
		case UNSYM:
		{
			t = getnode(whnod);
			t->wh.whtyp=(sh.wdval==WHSYM ? TWH : TUN);
			t->wh.whtre = sh_parse(DOSYM,NLFLG);
			t->wh.dotre = sh_parse(ODSYM,NLFLG);
			break;
		}

		/* command group with {...} */
		case BRSYM:
			t = sh_parse(KTSYM,NLFLG);
			break;

		case '(':
		{
			register union anynode	* p;
			p = getnode(parnod);
			p->par.partre=sh_parse(')',NLFLG);
			p->par.partyp=TPAR;
			t=sh_mkfork(0,p);
			break;
		}

		default:
			if(io==0)
				return(0);

		/* simple command */
		case 0:
		{
			register struct argnod	*argp;
			struct argnod	**argtail;
			struct argnod	**settail;
			int 	keywd=KEYFLG;
			int	argno = 0;
			int	bltin = 0;
			int	key_on = (flag?is_option(KEYFLG):0);
			t = getnode(comnod);
			t->com.comio=io; /*initial io chain*/
			/* set command line number for error messages */
			t->com.comline = getlineno(st.firstline);
			argtail = &(t->com.comarg);
			t->com.comset = 0;
			settail = &(t->com.comset);
			while(sh.wdval==0)
			{
				argp = sh.wdarg;
				argp->argchn = 0;
				/* test for keyword argument */
				if(sh.wdset&keywd)
				{
					chkflags(0);
					if(bltin&SYSDECLARE)
						goto cmdarg;
					*settail = argp;
					settail = &(argp->argnxt.ap);
					/* alias substitutions allowed */
					sh.wdset |= (KEYFLG|CAN_ALIAS);
				}
				else
				{
					sh.wdset = 0;	/* don't hunt for aliases*/
					chkflags(A_SPLIT|A_EXP);
				cmdarg:
					if((argp->argflag&A_RAW) == 0)
						argno = -1;
					if(argno>=0 && argno++==0)
					{
						/* check for builtin command */
						bltin=sh_lookup(argp->argval,tab_builtins);
						if(bltin&SYSDECLARE)
							key_on = KEYFLG;
					}
					*argtail = argp;
					argtail = &(argp->argnxt.ap);
					sh.wdset = keywd= key_on;
				}
#ifdef DEVFD
			retry:
				sh_lex();
				if((sh.wdval==IPROC ||sh.wdval==OPROC))
				{
					union anynode *t;
					int flag = (sh.wdval==OPROC);
					t = sh_parse(')',NLFLG);
					argp = (struct argnod*)stak_begin();
					argno = -1;
					*argtail = argp;
					argtail = &(argp->argnxt.ap);
					stak_end(sh_copy(e_nullstr,argp->argval));
					argp->argchn = (struct argnod*)sh_mkfork(flag?FPIN|FAMP|FPCL:FPOU,t);
					argp->argflag =  (A_EXP|flag);
					goto retry;
				}
#else
				sh_lex();
#endif	/* DEVFD */
				if(argno==1 && !t->com.comset && sh.wdval== '(')
				{
					/* SVR2 style function */
					sh_lex();
					if(sh.wdval == ')')
					{
						sh.wdarg = argp;
						goto funct_5_2;
					}
					sh.wdval = '(';
				}
				if(flag)
				{
					if(io)
					{
						while(io->ionxt)
							io = io->ionxt;
						io->ionxt = inout((struct ionod*)0,0);
					}
					else
						t->com.comio = io = inout((struct ionod*)0,0);
				}
			}
			*argtail = 0;
			t->com.comtyp = (TCOM|((bltin&SYSMAX)<<(COMBITS+1)));
			/* expand argument list if possible */
			if(argno>0)
				t->com.comarg = qscan(&t->com,argno);
			else if(t->com.comarg)
				t->com.comtyp |= COMSCAN;
			sh.wdset &= ~CAN_ALIAS;
			return(t);
		}
	}
	sh.reserv++;
	sh_lex();
	if(io=inout(io,0))
	{
		int type = t->tre.tretyp&COMMSK;
		t=sh_mkfork(0,t);
		t->tre.treio=io;
		t->fork.forkline = getlineno(st.firstline)-1;
		if(type != TFORK)
			t->tre.tretyp = TSETIO;
	}
done:
	sh.owdval = savwdval;
	sh.olineno = savline;
	return(t);
}


/*
 * skip past newlines but issue prompt if interactive
 */

static int	skipnl()
{
	while((sh.reserv++, sh_lex()==NL))
		sh_prompt(0);
	return(sh.wdval);
}

/*
 * check for and process and i/o redirections
 * if flag is set then an alias can be in the next word
 */

static struct ionod	*inout(lastio,flag)
struct ionod		*lastio;
{
	register int 	iof;
	register struct ionod	*iop;
	register int c;
	iof=sh.wdnum;
	switch(sh.wdval)
	{
		case DOCSYM:	/*	<<	*/
			iof |= (IODOC|IORAW);
			heredoc = FTMP;
			break;

		case APPSYM:	/*	>>	*/
		case '>':
			if(sh.wdnum==0)
				iof |= 1;
			iof |= IOPUT;
			if(sh.wdval==APPSYM)
			{
				iof |= IOAPP;
				break;
			}

		case '<':
			if((c=io_nextc())=='&')
				iof |= IOMOV;
			else if(c=='|' && sh.wdval=='>')
				iof |= IOCLOB;
			else if(c=='>')
				iof |= IORDW;
			else
				io_unreadc(c);
			break;

		default:
			return(lastio);
	}
	chkword();
	iop=(struct ionod*) stak_alloc(sizeof(struct ionod));
	iop->ioname=sh.wdarg->argval;
	iop->iolink = 0;
	iop->iodelim = 0;
	if(iof&IODOC)
	{
		iop->iolst=st.iopend;
		st.iopend=iop;
	}
	else
	{
		iop->iolst = 0;
		chkflags(A_EXP);
		if(sh.wdarg->argflag&A_RAW)
			iof |= IORAW;
	}
	iop->iofile=iof;
	if(flag)
		/* allow alias substitutions and parameter assignments */
		sh.wdset |= (KEYFLG|CAN_ALIAS);
	sh_lex();
	iop->ionxt=inout(lastio,flag);
	return(iop);
}

/*
 * get next token and make sure that it is not a keyword or meta-character
 */

static void	chkword()
{
	if(sh_lex())
		sh_syntax();
}

/*
 * see if this token is syntactically correct
 */

static void	chksym(sym)
register int sym;
{
	register int 	x = sym&sh.wdval;
	if(((x&SYMFLG) ? x : sym) != sh.wdval)
		sh_syntax();
}

/*
 * print the name of a syntactic token
 */

static void	prsym(sym)
register int sym;
{
	if(sym&SYMFLG)
	{
		register const struct sysnod	*sp=tab_reserved;
		while(sp->sysval && sp->sysval!=sym)
			sp++;
		p_str(sp->sysnam,0);
	}
	else if(sym==EOFSYM)
		p_str(e_endoffile,0);
	else
	{
		if(sym&SYMREP)
			p_char(sym);
		if(sym==NL)
			p_str("newline or ;",0);
		else
			p_char(sym);
	}
	p_char('\'');
}

/*
 * print a bad syntax message
 */

void	sh_syntax()
{
	register const char *cp = e_unexpected;
	register int w = sh.wdval;
	p_setout(ERRIO);
	p_prp(e_synbad);
	if((w==EOFSYM) && sh.owdval)
	{
		w = sh.owdval;
		cp = e_unmatched;
	}
	else
		sh.olineno = st.standin->flin;
	if(!(st.states&(PROMPT|PROFILE)))
	{
		p_str(e_atline,0);
		p_num(sh.olineno,SP);
	}
	p_str(e_colon,'`');
	if(w)
		prsym(w);
	else
	{
		sh_trim(sh.wdarg->argval);
		p_str(sh.wdarg->argval,'\'');
	}
	p_str(cp,NL);
	sh_exit(SYNBAD);
}

/*
 * check argument for possible optimizations
 * in many cases we can skip mac_expand and file name expansion
 * fexp is set to the union of A_SPLIT and A_EXP when splitting and/or
 * file expansion are possible.
 */

#define EXP_MACRO	2	/* macro expansion needed */
#define EXP_TRIM	4	/* quoted characters in string */
#define EXP_FILE	8	/* file expansion characters*/
#define EXP_QUOTE	16	/* string contains " character */

static void chkflags(fexp)
{
	register struct argnod* argp = sh.wdarg;
	register int c;
	argp->argflag = 0;
	{
		register int flag = 0;
		char nquote = 0;
		char *sp=argp->argval;
		while(c= *sp++)
		{
			if(c==ESCAPE)
			{
				flag |= EXP_TRIM;
				sp++;
			}
			else if(isexp(c))
			{
				if(c == '$' || c == '`')
				{
					flag |= EXP_MACRO;
					if(!nquote)
					{
						flag |= EXP_FILE;
						break;
					}
					if(c=='`')
						sh.nested_sub++;
				}
				else if(!nquote)
				{
					/* special case of '[' */
					if(*sp || c!='[')
						flag |= EXP_FILE;
				}
			}
			else if(c == '"')
			{
				/* toggle the quote count */
				nquote = !nquote;
				flag |= EXP_QUOTE;
			}
		}
		if(!(flag&EXP_FILE))
			fexp &= ~A_EXP;
		/* return if no macro expansion, file expansion or trimming required */
		if(flag==0)
		{
			argp->argflag |= A_RAW;
			return;
		}
		/* return if macro or command substitution needed */
		if(flag&EXP_MACRO)
		{
			argp->argflag |= (A_MAC|fexp);
			if(is_option(NOEXEC))
			{
				int wdset = sh.wdset;
				STKPTR savst = stak_fix();
				st.cmdline = getlineno(st.firstline);
				mac_expand(argp->argval);
				stak_reset(savst);
				sh.wdarg = argp;
				sh.wdset = wdset;
			}
			return;
		}
		/* check to see if file expansion is required */
		if(fexp&A_EXP)
		{
			argp->argflag|= A_EXP;
			/* return if no quotes otherwise don't optimize */
			if(flag&(EXP_QUOTE|EXP_TRIM))
			{
				argp->argflag= A_MAC|A_EXP;
				return;
			}
			return;
		}
		argp->argflag |= A_RAW;
	}
	/* just get rid of quoting stuff and consider argument as expanded */
	{
		register char *dp,*sp;
		char nquote = 0;	/* set within quoted string */
		dp = sp =  argp->argval;
		while(c= *sp++)
		{
			if(c != '"')
			{
				if(c==ESCAPE)
				{
					/* strip escchar's in double quotes */
					c = *sp++;
					if(nquote && !escchar(c) && c!='"')
						*dp++ = ESCAPE;
				}
				*dp++ = c;
			}
			else	/* toggle quote marker */
				nquote = !nquote;
		}
		*dp = 0;
	}
}

/*
 * convert argument chain to argument list when no special arguments
 */

#ifdef VPIX
#   define EXTRA 2
#else
#   define EXTRA 1
#endif /* VPIX */

static struct argnod *qscan(ac,argn)
struct comnod	*ac;
int argn;
{
	register char **cp;
	register struct argnod *ap;
	register struct dolnod* dp;
	/* leave space for an extra argument at the front */
	dp = (struct dolnod*)stak_alloc((unsigned)sizeof(struct dolnod) + EXTRA*sizeof(char*) + argn*sizeof(char*));
	cp = dp->dolarg+EXTRA;
	dp->doluse = argn;
	ap = ac->comarg;
	while(ap)
	{
		*cp++ = ap->argval;
		ap = ap->argnxt.ap;
	}
	*cp = NULL;
	return((struct argnod*)dp);
}

int getlineno(offset)
register int offset;
{
	if(st.exec_flag)
		return(st.cmdline);
	return(st.standin->flin-offset-((st.peekn&STRIP)==NL));
}

#ifdef NEWTEST
static union anynode *test_expr(sym)
{
	register union anynode *t = test_or();
	if(sh.wdval!=sym)
		sh_syntax();
	return(t);
}

static union anynode *test_or()
{
	register union anynode *t = test_and();
	while(sh.wdval==ORFSYM)
		t = makelist(TORF,t,test_and());
	return(t);
}

static union anynode *test_and()
{
	register union anynode *t = test_primary();
	while(sh.wdval==ANDFSYM)
		t = makelist(TAND,t,test_primary());
	return(t);
}

static union anynode *test_primary()
{
	register int num;
	register struct argnod *arg;
	register union anynode *t;
	sh.wdset |= TEST_OP1;
	skipnl();
	num = sh.wdnum;
	switch(sh.wdval)
	{
		case '(':
			t = test_expr(')');
			t = makelist(TTST|TPAREN ,t, t);
			sh_lex();
			break;

		case '!':
			t = test_primary();
			t->tre.tretyp |= TNEGATE;
			break;

		case TESTUNOP:
			chkword();
			chkflags(0);
			t = makelist(TTST|TUNARY|(num<<TSHIFT),
				(union anynode*)sh.wdarg,(union anynode*)sh.wdarg);
			skipnl();
			break;

		/* binary test operators */
		case 0:
			chkflags(0);
			arg = sh.wdarg;
			sh.wdset |= TEST_OP2;
			sh_lex();
			if(sh.wdval==TESTBINOP)
				num = sh.wdnum;
			else if(sh.wdval=='<')
				num = TEST_SLT;
			else if(sh.wdval=='>')
				num = TEST_SGT;
			else
				sh_syntax();
			chkword();
			if(num&TEST_PATTERN)
			{
				chkflags(A_EXP);
				if(sh.wdarg->argflag&A_EXP)
					num &= ~TEST_PATTERN;
			}
			else
				chkflags(0);
			t = makelist(TTST|TBINARY|(num<<TSHIFT),
				(union anynode*)arg,(union anynode*)sh.wdarg);
			skipnl();
	}
	return(t);
}

#endif /* NEWTEST */

