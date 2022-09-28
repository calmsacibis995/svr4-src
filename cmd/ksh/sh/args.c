/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/args.c	1.4.3.1"

#include	"defs.h"
#ifdef DEVFD
#   include	"jobs.h"
#endif	/* DEVFD */
#include	"sym.h"
#include	"builtins.h"


#ifdef DEVFD
    void	close_pipes();
#endif	/* DEVFD */

extern void	gsort();
extern int	strcmp();

static int		arg_expand();
static struct dolnod*	copyargs();
static void		print_opts();
static int		split();

static char		*null;
static struct dolnod	*argfor; /* linked list of blocks to be cleaned up */
static struct dolnod	*dolh;
static char flagadr[12];
static const char flagchar[] =
{
	'i',	'n',	'v',	't',	's',	'x',	'e',	'r',	'k',
	'u', 'f',	'a',	'm',	'h',	'p',	'c', 0
};
static const optflag flagval[]  =
{
	INTFLG,	NOEXEC,	READPR,	ONEFLG, STDFLG,	EXECPR,	ERRFLG,	RSHFLG,	KEYFLG,
	NOSET,	NOGLOB,	ALLEXP,	MONITOR, HASHALL, PRIVM, CFLAG, 0
};

/* ======== option handling	======== */

/*
 *  This routine turns options on and off
 *  The options "sicr" are illegal from set command.
 *  The -o option is used to set option by name
 *  This routine returns the number of non-option arguments
 */

int arg_opts(argc,argv)
register char **argv;
int  argc;
{
	register char *cp;
	register int c;
	register char *flagc;
	register optflag newflags=opt_flags;
	register optflag opt;
	char minus;
	struct namnod *np = (struct namnod*)0;
	char sort = 0;
	char minmin = 0;
	int setflag = (st.states&BUILTIN);
	while((cp= *++argv) && ((c= *cp) == '-' || c=='+'))
	{
		minus = (c == '-');
		argc--;
		if((c= *++cp)==0)
		{
			newflags &= ~(EXECPR|READPR);
			argv++;
			break;
		}
		else if(c == '-')
		{
			minmin = 1;
			argv++;
			break;
		}
		while(c= *cp++)
		{
			if(setflag)
			{
				if(c=='s')
				{
					sort = 1;
					continue;
				}
				else if(c=='A')
				{
					if(argv[1]==0)
						sh_fail(*argv, e_argexp);
					np = env_namset(*++argv,sh.var_tree,P_FLAG|V_FLAG);
					argc--;
					if(minus)
						nam_free(np);
					continue;
				}
				else if(strchr("icr",c))
					sh_fail(*argv, e_option);
			}
			if(c=='c' && minus && argc>=2 && sh.comdiv==0)
			{
				sh.comdiv= *++argv;
				argc--;
				newflags |= CFLAG;
				continue;
			}
			if(flagc=strchr(flagchar,c))
				opt = flagval[flagc-flagchar];
			else if(c != 'o')
				sh_fail(*argv,e_option);
			else
			{
				argv++;
				if(*argv==NIL)
				{
					print_opts(newflags);
					argv--;
					continue;
				}
				else
				{
					argc--;
					c=sh_lookup(*argv,tab_options);
					opt = 1L<<c;
					if(opt&(1|INTFLG|RSHFLG))
						sh_fail(*argv,e_option);
				}
			}
			if(minus)
			{
#if ESH || VSH
				if(opt&(EDITVI|EMACS|GMACS))
					newflags &= ~ (EDITVI|EMACS|GMACS);
#endif
				newflags |= opt;
			}
			else
				newflags &= ~opt;
		}
	}
	/* cannot set -n for interactive shells since there is no way out */
	if(is_option(INTFLG))
		newflags &= ~NOEXEC;
#ifdef RAWONLY
	if(is_option(EDITVI))
		newflags |= VIRAW;
#endif	/* RAWONLY */
	if(!setflag)
		goto skip;
	if(sort)
	{
		if(argc>1)
			gsort(argv,argc-1,strcmp);
		else
			gsort(st.dolv+1,st.dolc,strcmp);
	}
	if((newflags&PRIVM) && !is_option(PRIVM))
	{
		if((sh.userid!=sh.euserid && setuid(sh.euserid)<0) ||
			(sh.groupid!=sh.egroupid && setgid(sh.egroupid)<0) ||
			(sh.userid==sh.euserid && sh.groupid==sh.egroupid))
			newflags &= ~PRIVM;
	}
	else if(!(newflags&PRIVM) && is_option(PRIVM))
	{
		setuid(sh.userid);
		setgid(sh.groupid);
		if(sh.euserid==0)
		{
			sh.euserid = sh.userid;
			sh.egroupid = sh.groupid;
		}
	}
skip:
	opt_flags = newflags;
	if(setflag)
	{
		argv--;
		if(np)
			env_arrayset(np,argc,argv);
		else if(argc>1 || minmin)
			arg_set(argv);
	}
	return(argc);
}

/*
 * returns the value of $-
 */

char *arg_dolminus()
{
	register const char *flagc=flagchar;
	register char *flagp=flagadr;
	while(*flagc)
	{
		if(opt_flags&flagval[flagc-flagchar])
			*flagp++ = *flagc;
		flagc++;
	}
	*flagp = 0;
	return(flagadr);
}

/*
 * set up positional parameters 
 */

void arg_set(argi)
char *argi[];
{
	register char **argp=argi;
	register int size = 0; /* count number of bytes needed for strings */
	register char *cp;
	register int 	argn;
	/* count args and number of bytes of arglist */
	while((cp=(char*)*argp++) != ENDARGS)
	{
		size += strlen(cp);
	}
	/* free old ones unless on for loop chain */
	argn = argp - argi;
	arg_free(dolh,0);
	dolh=copyargs(argi, --argn, size);
	st.dolc=argn-1;
}

/*
 * free the argument list if the use count is 1
 * If count is greater than 1 decrement count and return same blk
 * Free the argument list if the use count is 1 and return next blk
 * Delete the blk from the argfor chain
 * If flag is set, then the block dolh is not freed
 */

struct dolnod *arg_free(blk,flag)
struct dolnod *	blk;
{
	register struct dolnod*	argr=blk;
	register struct dolnod*	argblk;
	if(argblk=argr)
	{
		if((--argblk->doluse)==0)
		{
			if(flag && argblk==dolh)
				dolh->doluse = 1;
			else
			{
				/* delete from chain */
				if(argfor == argblk)
					argfor = argblk->dolnxt;
				else
				{
					for(argr=argfor;argr;argr=argr->dolnxt)
						if(argr->dolnxt==argblk)
							break;
					if(argr==0)
					{
						return(NULL);
					}
					argr->dolnxt = argblk->dolnxt;
				}
				free((char*)argblk);
			}
			argr = argblk->dolnxt;
		}
	}
	return(argr);
}

/*
 * grab space for arglist and link argblock for cleanup
 * The strings are copied after the argment vector
 */

static struct dolnod *copyargs(from, n, size)
char *from[];
{
	register struct dolnod *dp=new_of(struct dolnod,n*sizeof(char*)+size+n);
	register char **pp;
	register char *sp;
	dp->doluse=1;	/* use count */
	/* link into chain */
	dp->dolnxt = argfor;
	argfor = dp;
	pp= dp->dolarg;
	st.dolv=pp;
	sp = (char*)dp + sizeof(struct dolnod) + n*sizeof(char*);
	while(n--)
	{
		*pp++ = sp;
		sp = sh_copy(*from++,sp) + 1;
	}
	*pp = ENDARGS;
	return(dp);
}

/*
 *  used to set new argument chain for functions
 */

struct dolnod *arg_new(argi,savargfor)
char *argi[];
struct dolnod **savargfor;
{
	register struct dolnod *olddolh = dolh;
	*savargfor = argfor;
	dolh = NULL;
	argfor = NULL;
	arg_set(argi);
	return(olddolh);
}

/*
 * reset arguments as they were before function
 */

void arg_reset(blk,afor)
struct dolnod *blk;
struct dolnod *afor;
{
	while(argfor=arg_free(argfor,0));
	dolh = blk;
	argfor = afor;
}

void arg_clear()
{
	/* force `for' $* lists to go away */
	while(argfor=arg_free(argfor,1));
	argfor = dolh;
#ifdef DEVFD
	close_pipes();
#endif	/* DEVFD */
}

/*
 * increase the use count so that an arg_set will not make it go away
 */

struct dolnod *arg_use()
{
	register struct dolnod *dh;
	if(dh=dolh)
		dh->doluse++;
	return(dh);
}

/*
 *  Print option settings on standard output
 */

static void print_opts(oflags)
#ifndef pdp11
register
#endif	/* pdp11 */
optflag oflags;
{
	register const struct sysnod *syscan = tab_options;
#ifndef pdp11
	register
#endif	/* pdp11 */
	optflag value;
	p_setout(st.standout);
	p_str(e_heading,NL);
	while(value=syscan->sysval)
	{
		value = 1<<value;
		p_str(syscan->sysnam,SP);
		p_nchr(SP,16-strlen(syscan->sysnam));
		if(oflags&value)
			p_str(e_on,NL);
		else
			p_str(e_off,NL);
		syscan++;
	}
}

#ifdef DEVFD
static int to_close[15];
static int indx;

void close_pipes()
{
	register int *fd = to_close;
	while(*fd)
	{
		close(*fd);
		*fd++ = -1;
	}
	indx = 0;
}
#endif	/* DEVFD */

#ifdef VPIX
#   define EXTRA 2
#else
#   define EXTRA 1
#endif /* VPIX */

/*
 * build an argument list
 */

char **arg_build(nargs,comptr)
int 	*nargs;
struct comnod	*comptr;
{
	register struct argnod	*argp;
	{
		register struct comnod	*ac = comptr;
		register struct argnod	*schain;
		/* see if the arguments have already been expanded */
		if(ac->comarg==NULL)
		{
			*nargs = 0;
			return(&null);
		}
		else if((ac->comtyp&COMSCAN)==0)
		{
			*nargs = ((struct dolnod*)ac->comarg)->doluse;
			return(((struct dolnod*)ac->comarg)->dolarg+EXTRA);
		}
		schain = st.gchain;
		st.gchain = NULL;
#ifdef DEVFD
		close_pipes();
#endif	/* DEVFD */
		*nargs = 0;
		if(ac)
		{
			argp = ac->comarg;
			while(argp)
			{
				*nargs += arg_expand(argp);
				argp = argp->argnxt.ap;
			}
		}
		argp = st.gchain;
		st.gchain = schain;
	}
	{
		register char	**comargn;
		register int	argn;
		register char	**comargm;
		argn = *nargs;
		argn += EXTRA;	/* allow room to prepend args */
		comargn=(char**)stak_alloc((unsigned)(argn+1)*sizeof(char*));
		comargm = comargn += argn;
		*comargn = ENDARGS;
		while(argp)
		{
			struct argnod *nextarg = argp->argchn;
			argp->argchn = 0;
			*--comargn = argp->argval;
			if((argp->argflag&A_RAW)==0)
				sh_trim(*comargn);
			if((argp=nextarg)==0 || (argp->argflag&A_MAKE))
			{
				if((argn=comargm-comargn)>1)
					gsort(comargn,argn,strcmp);
				comargm = comargn;
			}
		}
		return(comargn);
	}
}

/* Argument expansion */

static int arg_expand(argp)
register struct argnod *argp;
{
	register int count = 0;
	argp->argflag &= ~A_MAKE;
#ifdef DEVFD
	if(*argp->argval==0 && (argp->argflag&A_EXP))
	{
		/* argument of the form (cmd) */
		register struct argnod *ap;
		char *cp;
		int pv[2];
		int fd;
		ap = (struct argnod*)stak_begin();
		ap->argflag |= A_MAKE;
		ap->argflag &= ~A_RAW;
		ap->argchn= st.gchain;
		st.gchain = ap;
		count++;
		cp = sh_copy(e_devfd,ap->argval);
		io_popen(pv);
		fd = argp->argflag&A_RAW;
		stak_end(sh_copy(sh_itos(pv[fd]),cp));
		sh.inpipe = sh.outpipe = 0;
		if(fd)
		{
			sh.inpipe = pv;
			sh_exec((union anynode*)argp->argchn,(int)(st.states&ERRFLG));
		}
		else
		{
			sh.outpipe = pv;
			sh_exec((union anynode*)argp->argchn,(int)(st.states&ERRFLG));
		}
#ifdef JOBS
		job.pipeflag++;
#endif	/* JOBS */
		close(pv[1-fd]);
		to_close[indx++] = pv[fd];
	}
	else
#endif	/* DEVFD */
	if((argp->argflag&A_RAW)==0)
	{
		register char *ap = argp->argval;
		if(argp->argflag&A_MAC)
			ap = mac_expand(ap);
		count = split(ap,argp->argflag&(A_SPLIT|A_EXP));
	}
	else
	{
		argp->argchn= st.gchain;
		st.gchain = argp;
		argp->argflag |= A_MAKE;
		count++;
	}
	return(count);
}

static int split(s,macflg) /* blank interpretation routine */
char *s;
{
	register char *argp;
	register int 	c;
	register struct argnod *ap;
	int 	count=0;
	int expflag = (!is_option(NOGLOB) && (macflg&A_EXP));
	const char *seps;
	if(macflg &= A_SPLIT)
		seps = nam_fstrval(IFSNOD);
	else
		seps = e_nullstr;
	if(seps==NULL)
		seps = e_sptbnl;
	while(1)
	{
		if(sh.trapnote&SIGSET)
			sh_exit(SIGFAIL);
		ap = (struct argnod*)stak_begin();
		argp = ap->argval;
		while(c= *s++)
		{
			if(c == ESCAPE)
			{
				c = *s++;
				if(c!='/')
					*argp++ = ESCAPE;
			}
			else if(strchr(seps,c))
				break;
			if(argp >= sh.brkend)
				sh_addmem(BRKINCR);
			*argp++ = c;
		}
	/* This allows contiguous visible delimiters to count as delimiters */
		if(argp==ap->argval)
		{
			if(c==0)
				return(count);
			if(macflg==0 || strchr(e_sptbnl,c))
				continue;
		}
		else if(c==0)
		{
			s--;
		}
		/* file name generation */
		stak_end(argp);
		ap->argflag &= ~(A_RAW|A_MAKE);
#ifdef BRACEPAT
		if(expflag)
			count += expbrace(ap);
#else
		if(expflag && (c=path_expand(ap->argval)))
			count += c;
#endif /* BRACEPAT */
		else
		{
			count++;
			ap->argchn= st.gchain;
			st.gchain = ap;
		}
		st.gchain->argflag |= A_MAKE;
	}
}

