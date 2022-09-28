/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/macro.c	1.5.3.1"

#include	"defs.h"
#include	"sym.h"
#include	"builtins.h"
#ifdef MULTIBYTE
#   include	"national.h"
#endif /* MULTIBYTE */


/* These routines are defined by this module */
char	*mac_expand();
char	*mac_try();
char	*mac_trim();
int	mac_here();
void	mac_check();

/* These external routines are referenced by this module */
extern char		*ltos();
extern char		*match_paren();
extern char		*submatch();

#ifdef MULTIBYTE
    static int	charlen();
#endif /* MULTIBYTE */
static void	copyto();
static char	*substring();
static void	skipto();
static int	getch();
static int	comsubst();
static void	mac_error();
static void	mac_copy();
#ifdef POSIX
    static void	tilde_expand();
#endif /* POSIX */

static char	quote;	/* used locally */
static char	quoted;	/* used locally */
static char	mflag;	/* 0 for $x, 1 for here docs */
static const char *ifs;
static int	w_fd = -1;
static int mactry;
static char *mac_current;
static jmp_buf mac_buf;
static char	idb[2];
#ifdef FLOAT
static double numb;
#else
static long numb;
#endif /* FLOAT */

static void copyto(endch,newquote)
register char	endch;
{
	register int	c;
	register int count = 1;
	int saveq = quote;
#ifdef POSIX
	register char	*tilde = 0;
#endif /* POSIX */

	quote = newquote;
#ifdef POSIX
	/* check for tilde expansion */
	c = io_readc();
	if(c=='~' && !mflag && !quote)
		tilde = sh.staktop;
	io_unreadc(c);
#endif /* POSIX */
	while(c=getch(endch))
	{
		if((c==endch) && (saveq || !quote) && --count<=0)
			break;
		if(quote || c==ESCAPE)
		{
			if(c==ESCAPE)
			{
				c = io_readc();
				if(quote && !escchar(c) && c!= '"')
				{
					stak_push(ESCAPE);
					stak_push(ESCAPE);
				}
			}
			if(!mflag || !escchar(c))
				stak_push(ESCAPE);
		}
		if(sh.staktop >= sh.brkend)
			sh_addmem(BRKINCR);
		stak_push(c);
		if(c=='[' && endch==']')
			count++;
#ifdef POSIX
		else if(c=='/' && tilde)
		{
			tilde_expand(tilde,c);
			tilde = 0;
		}
#endif /* POSIX */
	}
#ifdef POSIX
	if(tilde)
		tilde_expand(tilde,0);
#endif /* POSIX */
	quote = saveq;
	stak_zero();
	if(c!=endch)
		mac_error();
}

#ifdef POSIX
/*
 * <argp> points to string on top of stack for tilde expansion
 * if <c> is non-zero, append <c> to expansion
 */

static void tilde_expand(arg,c)
register char *arg;
{
	extern char *sh_tilde();
	register char *cp;
	int offset = arg - sh.stakbot;
	stak_zero();
	if(cp = sh_tilde(arg))
	{
		struct fileblk	cb;
		stak_set(offset);
		io_push(&cb);
		io_sopen(cp);
		copyto(0,1);
		io_pop(1);
		if(c)
		{
			stak_push(c);
			stak_zero();
		}
	}
}
#endif /* POSIX */

/* skip chars up to } */

static void skipto(endch)
register char endch;
{
	register char	c;
	while((c=io_readc()) && c!=endch)
	{
		switch(c)
		{
			case ESCAPE:
				io_readc();
				break;

			case SQUOTE:	case DQUOTE:
				skipto(c);
				break;

			case DOLLAR:
				if((c=io_readc()) == LBRACE)
					skipto(RBRACE);
				else if(!dolchar(c))
					io_unreadc(c);
		}
	}
	if(c!=endch)
		mac_error();
}

static int getch(endch)
int	endch;
{
	register int	c;
	register int	bra; /* {...} bra =1, {#...} bra=2 */
	int atflag;  /* set if $@ or ${array[@]} within double quotes */
retry:
	c = io_readc();
	if(c==DOLLAR)
	{
		register char *v;
		register char *argp;
		register struct namnod	*n=(struct namnod*)NULL;
		int 	dolg=0;
		int dolmax = st.dolc+1;
		int 	nulflg;
		char *id=idb;
		bra = 0;
		*id = 0;
	retry1:
		c = io_readc();
		switch(c)
		{
			case DOLLAR:
				v=sh_itos(sh.pid);
				break;

			case '!':
				if(sh.bckpid)
					v=sh_itos(sh.bckpid);
				else
					v = "";
				break;

			case LBRACE:
				if(bra++ ==0)
					goto retry1;

			case LPAREN:
				if(bra==0 && mactry==0)
				{
					if(comsubst(1))
						goto retry;
					v = ltos(numb,10);
				}
				else
					goto nosub;
				break;

			case RBRACE:
				if(bra!=2)
					goto nosub;
				bra = 0;
			case '#':
				if(bra ==1)
				{
					bra++;
					goto retry1;
				}
				v=sh_itos(st.dolc);
				break;

			case '?':
				v=sh_itos(sh.savexit&EXITMASK);
				break;

			case '-':
				v=arg_dolminus();
				break;
			
			default:
				if(isalpha(c))
				{
					int offset = stak_offset();
					while(isalnum(c))
					{
						if(sh.staktop >= sh.brkend)
							sh_addmem(BRKINCR);
						stak_push(c);
						c = io_readc();
					}
					while(c=='[' && bra)
					{
						stak_push('[');
						copyto(']',0);
						*id = sh.staktop[-1];
						stak_push(']');
						c = io_readc();
					}
					io_unreadc(c);
					stak_zero();
					n=env_namset(stak_address(offset),sh.var_tree,P_FLAG);
					stak_set(offset);
					v = nam_strval(n);
					if(nam_istype(n,N_ARRAY))
					{
						if((bra==2 && (c= *id) && astchar(c)) ||
							(array_next(n) && v))
							dolg = -1;
						else
							dolg = 0;
					}
					else
						id = n->namid;
					goto cont1;
				}
				*id = c;
				if(astchar(c))
				{
					dolg=1;
					c=1;
				}
				else if(isdigit(c))
				{
					c -= '0';
					if(bra)
					{
						int d;
						while((d=io_readc(),isdigit(d)))
							c = 10*c + (d-'0');
						io_unreadc(d);
					}
				}
				else
					goto nosub;
				if(c==0)
				{
					if((st.states&PROFILE) && !(st.states&FUNCTION))
						v = sh.shname;
					else
						v = st.cmdadr;
				}
				else if(c <= st.dolc)
					v = st.dolv[c];
				else
					dolg = 0, v = 0;
			}
	cont1:
		c = io_readc();
		if(bra==2)
		{
			if(c!=RBRACE)
				mac_error();
			if(dolg==0 && dolmax)
#ifdef MULTIBYTE
				c = (v?charlen(v):0);
#else
				c = (v?strlen(v):0);
#endif /* MULTIBYTE */
			else if(dolg>0)
				c = st.dolc;
			else if(dolg<0)
				c = array_elem(n);
			else
				c = (v!=0);
			v = sh_itos(c);
			dolg = 0;
			c = RBRACE;
		}
		/* check for quotes @ */
		if(idb[0]=='@' && quote && !atflag)
		{
			quoted--;
			atflag = 1;
		}
		if(c==':' && bra)	/* null and unset fix */
		{
			nulflg=1;
			c=io_readc();
		}
		else
			nulflg=0;
		if(!defchar(c) && bra)
			mac_error();
		argp = 0;
		if(bra)
		{
			if(c!=RBRACE)
			{
				bra = stak_offset();
				if(((v==0 || (nulflg && *v==0)) ^ (setchar(c)!=0))
					|| is_option(NOEXEC))
				{
					int newquote = quote;
					if(c=='#' || c == '%')
						newquote = 0;
					copyto(RBRACE,newquote);
				}
				else
					skipto(RBRACE);
				argp=stak_address(bra);
			}
		}
		else
		{
			io_unreadc(c);
			c=0;
		}
		/* check for substring operations */
		if(c == '#' || c == '%')
		{
			if(dolg != 0)
				mac_error();
			if(v && *v)
			{
#ifdef POSIX
				char *savarg = argp;
				char *pat;
				int savec;
#endif /* POSIX */
				/* allow room for escapes */
				bra = strlen(v);
				sh.staktop += bra;
				while(sh.staktop+bra >= sh.brkend)
					sh_addmem(BRKINCR);
#ifdef POSIX
				v = strcpy(sh.staktop,v);
				do
				{
					bra = 0;
					if(*argp==c)
					{
						bra++;
						argp++;
					}
					pat = argp;
					while(1)
					{
						switch(*argp)
						{
						case '#': case '%':
						case 0:
							goto endloop;

						case ESCAPE:
							argp++;
						default:
							argp++;
						}
					}
				endloop:
					savec = *argp;
					*argp++ = 0;
					if(c=='#')
					{
						char *savev = v;
						v = submatch(v,pat,bra);
						if(v==0)
							v = savev;
					}
					else
						v = substring(v,pat,bra);
					c = savec;
				}
				while(c);
				argp = savarg;
#else
				bra = 0;
				strcpy(sh.staktop,v);
				if(*argp==c)
				{
					bra++;
					argp++;
				}
				if(c=='#')
				{
					v = submatch(sh.staktop,argp,bra);
					if(v==0)
						v = sh.staktop;
				}
				else
					v = substring(sh.staktop,argp,bra);
				if(bra)
					argp--;
#endif /* POSIX */
			}
			sh.staktop = argp;
		}
	retry2:
		if(v && (!nulflg || *v ) && c!='+')
		{
			int type = *id;
#define sep bra
			if(*ifs)
				sep = *ifs;
			else
				sep = SP;
			while(1)
			{
				/* quoted null strings have to be marked */
				if(*v==0 && quote)
				{
					stak_push(ESCAPE);
					stak_push(0);
				}
				mac_copy(v);
				if(dolg==0)
					 break;
				if(dolg>0)
				{
					if(++dolg >= dolmax)
						break;
					v = st.dolv[dolg];
				}
				else
				{
					if(type == 0)
						break;
					v = nam_strval(n);
					type = array_next(n);
				}
				if(quote && *id=='*')
				{
					if(*ifs==0)
						continue;
					stak_push(ESCAPE);
				}
				stak_push(sep);
#undef sep
			}
		}
		else if(argp)
		{
			if(c=='?' && !is_option(NOEXEC))
			{
				sh_trim(argp);
				sh_fail(id,*argp?argp:e_nullset);
			}
			else if(c=='=')
			{
				if(n)
				{
					bra= argp-sh.stakbot;
					sh_trim(argp);
					nam_putval(n,argp);
					v = nam_strval(n);
					sh.staktop = argp = stak_address(bra);
					nulflg = 0;
					goto retry2;
				}
				else
					mac_error();
			}
		}
		else if(is_option(NOSET))
			sh_fail(id,e_notset);
		goto retry;
	}
	else if(c==endch)
		return(c);
	else if(c==SQUOTE && mactry==0)
	{
		comsubst(0);
		goto retry;
	}
	else if(c==DQUOTE && !mflag)
	{
		if(quote ==0)
		{
			atflag = 0;
			quoted++;
		}
		quote ^= 1;
		goto retry;
	}
	return(c);
nosub:
	if(bra)
		mac_error();
	io_unreadc(c);
	return(DOLLAR);
}

	/* Strip "" and do $ substitution
	 * Leaves result on top of stack
	 */
char *mac_expand(as)
char *as;
{
	register int	savqu =quoted;
	register int	savq = quote;
	register int	savpeekn = st.peekn;
	struct fileblk	cb;
	mac_current = as;
	st.peekn = 0;
	io_push(&cb);
	io_sopen(as);
	stak_begin();
	mflag = 0;
	quote=0;
	quoted=0;
	if(!(ifs = nam_fstrval(IFSNOD)))
		ifs = e_sptbnl;
	copyto(0,0);
	io_pop(1);
	st.peekn = savpeekn;
	if(quoted && (sh.stakbot == sh.staktop))
	{
		stak_push(ESCAPE);
		stak_push(0);
	}
	/* above is the fix for *'.c' bug	*/
	quote=savq;
	quoted=savqu;
	return(stak_fix());
}

/*
 * command substitution
 * type==0 for ``
 * type==1 for $()
*/

static int comsubst(type)
int type;
{
	struct fileblk	cb;
	register int	fd;
#define d	fd
	register union anynode *t;
	register char *argc;
	struct ionod *saviotemp = st.iotemp;
	int savem = mflag;
	STKPTR savtop = sh.staktop;
	STKPTR savptr = stak_fix();
	char inbuff[IOBSIZE+1];
	int saveflags = (st.states&FIXFLG);
	register int waitflag = 0;
	stak_begin();
	if(type)
	{
		sh.staktop = (STKPTR)(match_paren((char*)sh.stakbot,LPAREN,RPAREN)-1);
		if(*sh.stakbot==LPAREN && *(sh.staktop-1)==RPAREN)
		{
			argc = stak_end(sh.staktop-1);
			numb = sh_arith(mac_trim(argc+1,0));
			stak_reset(savtop);
			sh.stakbot = savptr;
			return(0);
		}
	}
	else
	{
		while((d=io_readc())!=SQUOTE && d)
		{
			if(d==ESCAPE)
			{
				d = io_readc();
				/*
				 * This is wrong but it preserves compatibility with
				 * the SVR2 shell
				 */
				if(!(escchar(d) || (d=='"' && quote)))
					stak_push(ESCAPE);
			}
			if(sh.staktop >= sh.brkend)
				sh_addmem(BRKINCR);
			stak_push(d);
		}
	}
	argc=stak_fix();
	st.states &= ~FIXFLG;		/* do not save command subs in fc file */
	if(w_fd>=0)
	{
		p_setout(w_fd);
		p_flush();	/* flush before executing command */
	}
	io_push(&cb);
	io_sopen(argc);
	sh.nested_sub = 0;
	st.exec_flag++;
	t = sh_parse(EOFSYM,MTFLG|NLFLG);
	st.exec_flag--;
	if(!t || is_option(NOEXEC))
		goto readit;
	if(!sh.nested_sub && !t->tre.treio && is_rbuiltin(t))
	{
		/* nested command subs not handled specially */
		/* handle command substitution of most builtins separately */
		/* exec, login, cd, ., eval and shift not handled this way */
		/* put output into tmpfile */
		int save1_out = st.standout;
		if((st.states&IS_TMP)==0)
		{
			char tmp_fname[TMPSIZ];
			/* create and keep open a /tmp file for command subs */
			fd = io_mktmp(tmp_fname);
			fd = io_renumber(fd,TMPIO);
			st.states |= IS_TMP;
			/* root cannot unlink because fsck could give bad ref count */
			if(sh.userid || !is_option(INTFLG))
				unlink(tmp_fname);
			else
				st.states |= RM_TMP;
		}
		else
			fd = TMPIO;
		st.standout = fd;
		/* this will only flush the buffer if output is fd already */
		p_setout(fd);
		p_char(0);
		st.subflag++;
		sh_funct(t,(char**)0,(int)(st.states&ERRFLG),(struct argnod*)0);
		st.subflag = 0;
		p_setout(fd);
		p_char(0);
		if(*_sobuf != 0)
		{
			/* file is larger than buffer, read from it */
			p_flush();
			io_seek(fd,(off_t)1,SEEK_SET);
			io_init(input=fd,st.standin,inbuff);
			waitflag = -1;
		}
		else
		{
			/* The file is all in the buffer */
			strcpy(inbuff,_sobuf+1);
			io_sopen(inbuff);
			io_ftable[fd]->ptr = io_ftable[fd]->base;
		}
		st.standout = save1_out;
		goto readit;
	}
	else if(t->tre.tretyp==0 && t->com.comarg==0)
	{
		if(t->tre.treio && (((t->tre.treio)->iofile)&IOUFD)==0)
		{
			argc = (t->tre.treio)->ioname;
			if(!((t->tre.treio)->iofile&IORAW))
				argc=mac_trim(argc,1);
		}
		else
			argc = (char*)e_devnull;
		fd = io_fopen(argc);
	}
	else
	{
		int 	pv[2];
		int forkflag = FPOU|FCOMSUB;
		waitflag++;
		if(st.iotemp!=saviotemp)
			forkflag |= FTMP;
		t = sh_mkfork(forkflag,t);
		  /* this is done like this so that the pipe
		   * is open only when needed
		   */
		io_popen(pv);
		sh.inpipe = 0;
		sh.outpipe = pv;
		sh_exec(t, (int)(st.states&ERRFLG));
		fd = pv[INPIPE];
		io_fclose(pv[OTPIPE]);
	}
	io_init(input=fd,st.standin,inbuff);

readit:
	mflag = savem;
	stak_reset(savptr);
	d = savtop - savptr;
	while(d--)
		*sh.staktop++ = *savptr++;
	mac_copy((char*)0);
	if(waitflag>0)
		job_wait(sh.subpid);
	while(sh.stakbot!=sh.staktop)
	{
		if(*--sh.staktop != NL)
		{
			++sh.staktop;
			break;
		}
		else if(quote)
			sh.staktop--;
	}
	io_pop(waitflag>=0?0:1);
	if(w_fd >=0)
		p_setout(w_fd);
	st.states |= saveflags;
	return(1);
#undef d
}


/*
 * Copy and expand a here-document
 */

int mac_here(iop)
register struct ionod	*iop;
{
	register char	c;
	register int 	in;
	register int	ot;
	struct fileblk 	fb;
	char inbuff[IOBSIZE+1];
	quote = 0;
	ifs = e_nullstr;
	mflag = 1;
	ot = io_mktmp(inbuff);
	unlink(inbuff);
	w_fd = ot;
	io_push(&fb);
	if(iop->iofile&IOSTRG)
	{
		io_sopen(iop->ioname);
		in = F_STRING;
	}
	else
	{
		in = io_fopen(iop->ioname);
		io_init(in,&fb,inbuff);
		}
	p_setout(ot);
	stak_begin();
	if(is_option(EXECPR))
		sh.heretrace=1;
	while(1)
	{
		c=getch(0);
		if(c==ESCAPE)
		{
			c = io_readc();
			if(!escchar(c))
				stak_push(ESCAPE);
		}
		if(sh.staktop!=sh.stakbot)
		{
			*sh.staktop = 0;
			p_str(sh.stakbot,c);
			sh.staktop = sh.stakbot;
		}
		else if(c)
			p_char(c);
		if(c==0)
			break;
	}
	p_flush();
	sh.heretrace=0;
	mflag = 0;
	io_pop(0);
	w_fd = -1;
	io_ftable[ot] = 0;
	lseek(ot,(off_t)0,SEEK_SET);
	return(ot);
}


/*
 * copy value of string or file onto the stack inserting backslashes
 * as needed to prevent word splitting and file expansion
 */

static void mac_copy(str)
register char *str;
{
	register int c;
	while(c = (str?*str++:io_readc()))
	{
		if(sh.staktop >= sh.brkend)
			sh_addmem(BRKINCR);
		/*@ assert (!mflag&&quote)==0; @*/
		if(quote || (!mflag&&addescape(c)&&(c==ESCAPE||!strchr(ifs,c))))
	 		stak_push(ESCAPE); 
 		stak_push(c); 
	}
}
 
/*
 * Deletes the right substring of STRING using the expression PAT
 * the longest substring is deleted when FLAG is set.
 */

static char *substring(string,pat,flag)
register char *string;
char *pat;
int flag;
{
	register char *sp = string;
	register char *cp;
	sp += strlen(sp);
	cp = sp;
	while(sp>=string)
	{
		if(strmatch(sp,pat))
		{
			cp = sp;
			if(flag==0)
				break;
		}
		sp--;
#ifdef MULTIBYTE
		if(*sp&HIGHBIT)
		{
			if(*(sp-in_csize(3))==ESS3)
				sp -= in_csize(3);
			else if(*(sp-in_csize(2))==ESS2)
				sp -= in_csize(2);
			else
				sp -= (in_csize(1)-1);
		}
#endif /* MULTIBYTE */
	}
	*cp = 0;
	return(string);
}


/*
 * do parameter and command substitution and strip of quotes
 * attempt file name expansion if <type> not zero
 */

char *mac_trim(s,type)
char *s;
{
	register char *t=mac_expand(s);
	struct argnod *schain = st.gchain;
	if(type && f_complete(t,e_nullstr)==1)
		t = st.gchain->argval;
	st.gchain = schain;
	sh_trim(t);
	return(t);
}

/*
 * perform only parameter substitution and catch failures
 */

char *mac_try(s)
register char *s;
{
	if(s)
	{
		mactry++;
		if(setjmp(mac_buf)==0)
			s = mac_trim(s,0);
		else
			io_pop(1);
		mactry = 0;
	}
	if(s==0)
		return(NULLSTR);
	return(s);
}

static void mac_error()
{
	sh_fail(mac_current,e_subst);
}

/*
 * check to see if the error occured while expanding prompt
 */

void mac_check()
{
	if(mactry)
		longjmp(mac_buf,1);
}



#ifdef MULTIBYTE
static int	charlen(str)
register char *str;
{
	register int n = 0;
	register int c;
	while(*str)
	{
		c = echarset(*str);		/* find character set */
		str += (in_csize(c)+(c>=2));	/* move to next char */
		n += out_csize(c);		/* add character size */
	}
	return(n);
}
#endif /* MULTIBYTE */
