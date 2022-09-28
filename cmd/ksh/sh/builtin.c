/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ksh:sh/builtin.c	1.5.4.1"

#include	<errno.h>
#include	"defs.h"
#include	"history.h"
#include	"builtins.h"
#include	"jobs.h"
#include	"sym.h"

#ifdef _sys_resource_
#   ifndef included_sys_time_
#	include <sys/time.h>
#   endif
#   include	<sys/resource.h>/* needed for ulimit */
#   define	LIM_FSIZE	RLIMIT_FSIZE
#   define	LIM_DATA	RLIMIT_DATA
#   define	LIM_STACK	RLIMIT_STACK
#   define	LIM_CORE	RLIMIT_CORE
#   define	LIM_CPU		RLIMIT_CPU
#   define	INFINITY	RLIM_INFINITY
#   ifdef RLIMIT_RSS
#	define	LIM_MAXRSS	RLIMIT_RSS
#   endif /* RLIMIT_RSS */
#else
#   ifdef VLIMIT
#	include	<sys/vlimit.h>
#   endif /* VLIMIT */
#endif	/* _sys_resource_ */
#ifdef UNIVERSE
#   ifdef _sys_universe_
#	include	<sys/universe.h>
#	define getuniverse(x)	(setuniverse(flag=setuniverse(0)),\
				(flag>=0?strncpy(x,univ_name[flag],TMPSIZ):0),\
				flag)
#   else
#	define univ_number(x)	(x)
#   endif /* _sys_universe_ */
#endif /* UNIVERSE */

#ifdef ECHOPRINT
#   undef ECHO_RAW
#   undef ECHO_N
#endif /* ECHOPRINT */

#define DOTMAX	32	/* maximum level of . nesting */
#define PHYS_MODE	H_FLAG
#define LOG_MODE	N_LJUST

/* This module references these external routines */
#ifdef ECHO_RAW
    extern char		*echo_mode();
#endif	/* ECHO_RAW */
#ifdef FS_3D
    extern char		*path_real();
#endif /* FS_3D */
extern int		gscan_all();
extern char		*utos();
extern void		ltou();

static int	flagset();
static void	cfailed();
static int	sig_number();
static int	scanargs();
static char	*cmd_name;
#ifdef JOBS
    static void	sig_list();
#endif	/* JOBS */
static int	argnum;
static int 	aflag;
static int	newflag;
static int	echon;

void sh_builtin(xbuiltin, argn, com,t)
int	argn;
register char	*com[];
union anynode	*t;
{
	register char *a1 = com[1];
	struct Amemory *troot;
	register int flag = 0;
	int scoped = 0;
	aflag = 0;
	argnum = 0;
	newflag = 0;
	if(a1)
		aflag = *a1;
	cmd_name = com[0];
	switch(xbuiltin)
	{
		case SYSEXEC:
			com++;
			st.ioset = 0;
			if(a1==0)
				break;

		case SYSLOGIN:
	
			if(is_option(RSHFLG))
				cfailed(e_restricted);
			else
			{
				if(job_close() < 0)
				{
					sh.exitval=1;
					break;
				}
				/* force bad exec to terminate shell */
				st.states &= ~(PROFILE|PROMPT|BUILTIN|LASTPIPE);
				sig_reset(0);
				io_rmtemp((struct ionod*)0);
				path_exec(com,(struct argnod*)0);
				sh_done(0);
			}

#ifdef OLDTEST
		case SYSTEST:	/* test	expression */
			sh.exitval = b_test(argn, com);
			break;
#endif /* OLDTEST */

		case SYSPWD:	/* pwd routine */
#if defined(LSTAT) || defined(FS_3D)
			while(a1 && *a1=='-'&& 
				(flag = flagset(a1,~(PHYS_MODE|LOG_MODE))))
			{
				if(flag&LOG_MODE)
					flag = 0;
				com++;
				a1 = com[1];
			}
#endif /* LSTAT||FS_3D */
			if(*(a1 = path_pwd(0))!='/')
				cfailed(e_pwd);
#if defined(LSTAT) || defined(FS_3D)
			if(flag)
			{
#   ifdef FS_3D
				a1 = path_real(a1);
#   else
				a1 = strcpy(stak_alloc(PATH_MAX),a1);
#   endif /* FS_3D */
#ifdef LSTAT
				path_physical(a1);
#endif /* LSTAT */
			}
#endif /* LSTAT||FS_3D */
			p_setout(st.standout);
			p_str(a1,NL);
			break;
			
		case SYSECHO:	/* system V echo routine */
#ifndef ECHOPRINT
#   ifdef ECHO_RAW
			/* This mess is because /bin/echo on BSD is archaic */
			com--;
			a1 = echo_mode();
#   else
#	ifdef ECHO_N
			/* same as echo except -n special */
			echon = 1;
#	else
			/* equivalent to print - */
			com--;
			a1 = (char*)e_minus;
#	endif /* ECHO_N */
#   endif	/* ECHO_RAW */
#endif /* ECHOPRINT */

		case SYSPRINT:	/* print routine */
		{
			register int fd;
			const char *msg = e_file;
			int raw = 0;
			argnum =  1;
			while(a1 && *a1 == '-')
			{
				int c = *(a1+1);
				com++;
				/* echon set when only -n is legal */
				if(echon)
				{
					if(strcmp(a1,"-n")==0)
						c = 0;
					else
					{
						com--;
						break;
					}
				}
#ifdef POSIX
				newflag = flagset(a1,~(N_FLAG|R_FLAG|P_FLAG|U_FLAG|S_FLAG|N_RJUST|E_FLAG|X_FLAG));
				if(newflag&E_FLAG)
					flag &= ~(R_FLAG|N_RJUST);
#else
				newflag = flagset(a1,~(N_FLAG|R_FLAG|P_FLAG|U_FLAG|S_FLAG|N_RJUST));
#endif /* POSIX */
				flag |= newflag;
				/* handle the -R flag for BSD style echo */
				if(flag&N_RJUST)
					echon = 1;
				if(c==0 || newflag==0)
					break;
				a1 = com[1];
			}
			echon = 0;
			argnum %= 10;
			if(flag&(R_FLAG|N_RJUST))
				raw = 1;
#ifdef POSIX
			if(flag&X_FLAG)
				raw |= 2;
#endif /* POSIX */
			if(flag&S_FLAG)
			{
				/* print to history file */
				if(!hist_open())
					cfailed(e_history);
				fd = hist_ptr->fixfd;
				st.states |= FIXFLG;
				goto skip;
			}
			else if(flag&P_FLAG)
			{
				fd = COTPIPE;
				msg = e_query;
			}
			else if(flag&U_FLAG)
				fd = argnum;
			else	
				fd = st.standout;
			if(sh.exitval = !fiswrite(fd))
			{
				if(fd==st.standout)
					break;
				cfailed(msg);
			}
		skip:
			p_setout(fd);
			if(echo_list(raw,com+1) && (flag&N_FLAG)==0)
				newline();
			if(flag&S_FLAG)
				hist_flush();
			break;
		}

		case SYSLET:
		{
			if(argn < 2)
				cfailed(e_nargs);
			while(--argn)
				sh.exitval = !sh_arith(*++com);
			break;
		}

		/*
		 * The following few builtins are provided to set,print,
		 * and test attributes and variables for shell variables,
		 * aliases, and functions.
		 * In addition, typeset -f can be used to test whether a
		 * function has been defined or to list all defined functions
		 * Note readonly is same as typeset -r.
		 * Note export is same as typeset -x.
		 */
		case SYSRDONLY:
			flag = R_FLAG;
			aflag = '-';
			goto typset;

		case SYSXPORT:
			flag = X_FLAG;
			aflag = '-';
			goto typset;

		case SYSALIAS:
		case SYSTYPESET:
		{
			register int fd;
			int type;	/* 0 for typeset, non-zero for alias */
			/* G_FLAG forces name to be in newest scope */
			if(st.fn_depth)
				scoped = G_FLAG;
			com += scanargs(com,~(N_LJUST|N_RJUST|N_ZFILL
					|N_INTGER|N_LTOU |N_UTOL|X_FLAG|R_FLAG
					|F_FLAG|T_FLAG|N_HOST
					|N_DOUBLE|N_EXPNOTE));
			flag = newflag;
			if((flag&N_INTGER) && (flag&(N_LJUST|N_RJUST|N_ZFILL|F_FLAG)))
				cfailed(e_option);

		typset:
			type = 0;
			if(xbuiltin == SYSALIAS)
			{
				if(flag&~(N_EXPORT|T_FLAG))
					cfailed(e_option);
				if(flag&T_FLAG)
					troot = sh.track_tree;
				else
					troot = sh.alias_tree;
				/* env_namset treats this value specially */
				type = (V_FLAG|G_FLAG);
			}
			else if(flag&F_FLAG)
			{
				if(flag&~(N_EXPORT|F_FLAG|T_FLAG|U_FLAG))
					cfailed(e_option);
				troot = sh.fun_tree;
				flag &= ~F_FLAG;
			}
			else
				troot = sh.var_tree;
			fd = st.standout;
			p_setout(fd);
			if(aflag == 0)
			{
				if(type)
					env_scan(fd,0,troot,0);
				else
					gscan_all(env_prattr,troot);
				break;
			}
			if(com[1])
			{
				if(st.subflag)
					break;
				while(a1 = *++com)
				{
					register unsigned newflag;
					register struct namnod *np;
					unsigned curflag;
					if(troot == sh.fun_tree)
					{
						/*
						 *functions can be exported or
						 * traced but not set
						 */
						if(flag&U_FLAG)
							np = env_namset(a1,sh.fun_tree,P_FLAG|V_FLAG);
						else
							np = nam_search(a1,sh.fun_tree,0);
						if(np)
						{
							if(flag==0)
							{
								env_prnamval(np,0);
								continue;
							}
							if(aflag=='-')
								nam_ontype(np,flag|N_FUNCTION);
							else if(aflag=='+')
								nam_offtype(np,~flag);
						}
						else
							sh.exitval++;
						continue;
					}
					np = env_namset(a1,troot,(type|scoped));
					/* tracked alias */
					if(troot==sh.track_tree && aflag=='-')
					{
						nam_ontype(np,NO_ALIAS);
						path_alias(np,path_absolute(np->namid));
						continue;
					}
					if(flag==0 && aflag!='-' && strchr(a1,'=') == NULL)
					{
						/* type==0 for TYPESET */
						if(type&&env_prnamval(np,0)==0)
						{
							p_str(a1,0);
							p_str(e_alias,NL);
							sh.exitval++;
						}
						continue;
					}
					curflag = namflag(np);
					if (aflag == '-')
					{
						newflag = curflag;
						if(flag&~NO_CHANGE)
							newflag &= NO_CHANGE;
						newflag |= flag;
						if (flag & (N_LJUST|N_RJUST))
						{
							if (flag & N_LJUST)
								newflag &= ~N_RJUST;
							else
								newflag &= ~N_LJUST;
						}
						if (flag & N_UTOL)
							newflag &= ~N_LTOU;
						else if (flag & N_LTOU)
							newflag &= ~N_UTOL;
					}
					else
						newflag = curflag & ~flag;
					if (aflag && (argnum>0 || (curflag!=newflag)))
					{
#ifdef apollo
						/* keep aliases from going
						   into environment */
						if(type)
							namflag(np) = newflag;
						else
							nam_newtype (np, newflag,argnum);
#endif /* apollo */
						nam_newtype (np, newflag,argnum);
					}
				}
			}
			else
				env_scan(fd,flag,troot,aflag=='+');
			break;
		}


		/*
		 * The removing of Shell variable names, aliases, and functions
		 * is performed here.
		 * Unset functions with unset -f
		 * Non-existent items being deleted give non-zero exit status
		 */
		case SYSUNALIAS:
		case SYSUNSET:
		{
			register struct namnod *np;
			register struct blk *bp;
#ifdef apollo
			short namlen;
#endif /* apollo */
			if(st.subflag)
				break;
			if(xbuiltin == SYSUNALIAS)
			{
				troot = sh.alias_tree;
				goto unall;
			}
			if(aflag == '-')
			{
				flag = flagset(a1,~F_FLAG);
				com++;
				troot = sh.fun_tree;
			}
			else
				troot = sh.var_tree;
		unall:
			if(argn < 2)
				cfailed(e_nargs);
			while(a1 = *++com)
			{
				np=env_namset(a1,troot,P_FLAG);
				if(np)
				{
					if(troot==sh.var_tree)
					{
						if (nam_istype (np, N_RDONLY))
							sh_fail(np->namid,e_readonly);
						else if (nam_istype (np, N_RESTRICT))
							sh_fail(np->namid,e_restricted);
#ifdef apollo
						namlen =strlen(np->namid);
						ev_$delete_var(np->namid,&namlen);
#endif /* apollo */
					}
					else if(bp=(struct blk*)(np->value.namenv))
					/* free function definition */
					{
					/*@ assert troot==sh.var.tree @*/
						while(bp)
						{
							free((char*)bp);
							bp = bp->word;
						}
						np->value.namenv = 0;
					}
					nam_free(np);
				}
				else
					sh.exitval = 1;
			}
			break;
		}


		case SYSDOT:
			if(a1)
			{
				if((sh.un.fd=path_open(a1,path_get(a1))) < 0)
					sh_fail(a1,e_found);
				if(st.dot_depth++ > DOTMAX)
					cfailed(e_recursive);
				if(argn > 2)
					arg_set(com+1);
				sh_eval((char*)0);
				st.dot_depth--;
			}
			else
				cfailed(e_argexp);
			break;
	
		case SYSTIMES:
		{
			struct tms tt;
			times(&tt);
			p_setout(st.standout);
			p_time(tt.tms_utime,' ');
			p_time(tt.tms_stime,NL);
			p_time(tt.tms_cutime,' ');
			p_time(tt.tms_cstime,NL);
			break;
		}
		
		case SYSRETURN:	/* return from a subroutine */
		case SYSEXIT:
			if(st.subflag)
				break;
			flag = ((a1?atoi(a1):sh.oldexit)&EXITMASK);
			if(xbuiltin==SYSRETURN && sh.freturn)
			{
				sh.exitval = flag;
				longjmp(*sh.freturn,1);
			}
			if(job_close() < 0)
				break;
			/* force exit */
			st.states &= ~(PROMPT|PROFILE|BUILTIN|FUNCTION|LASTPIPE);
			sh_exit(flag);
		
		case SYSNULL:
			t->tre.treio = 0;
			break;
		
		case SYSCONT:
			if(st.subflag)
				break;
			if(st.loopcnt)
			{
				st.execbrk = st.breakcnt = 1;
				if(a1)
					st.breakcnt = atoi(a1);
				if(st.breakcnt > st.loopcnt)
					st.breakcnt = st.loopcnt;
				else
					st.breakcnt = -st.breakcnt;
			}
			break;
		
		case SYSBREAK:
			if(st.subflag)
				break;
			if(st.loopcnt)
			{
				st.execbrk = st.breakcnt = 1;
				if(a1)
					st.breakcnt = atoi(a1);
				if(st.breakcnt > st.loopcnt)
					st.breakcnt = st.loopcnt;
			}
			break;
		
		case SYSTRAP:
			if(a1)
			{
				register int	clear;
				char *action = a1;
				if(st.subflag)
					break;
				if((clear=isdigit(*a1))==0)
				{
					++com;
					if(*a1=='-')
						clear++;
				}
				while(a1 = *++com)
				{
					flag = sig_number(a1);
					if(flag>=MAXTRAP || flag<MINTRAP)
						sh_fail(a1,e_trap);
					else if(clear)
						sig_clear(flag);
					else
					{
						free(st.trapcom[flag]);
						st.trapcom[flag] = sh_heap(action);
						if(*action)
							sig_ontrap(flag);
						else
							sig_ignore(flag);
					}
				}
			}
			else /* print out current traps */
#ifdef POSIX
				sig_list(-1);
#else
			{
				p_setout(st.standout);
				for(; flag<MAXTRAP; flag++)
					if(st.trapcom[flag])
					{
						p_num(flag,':');
						p_str(st.trapcom[flag],NL);
					}
			}
#endif /* POSIX */
			break;
		
		case SYSCD:
		{
			register const char *dp;
			register char *cdpath = NULLSTR;
			int rval;
			char *oldpwd;
			if(st.subflag)
				break;
			if(is_option(RSHFLG))
				cfailed(e_restricted);
#ifdef LSTAT
			while(a1 && *a1=='-' && a1[1]) 
			{
				flag = flagset(a1,~(PHYS_MODE|LOG_MODE));
				if(flag&LOG_MODE)
					flag = 0;
				com++;
				argn--;
				a1 =  com[1];
			}
#endif /* LSTAT */
			if(argn >3)
				cfailed(e_nargs);
			oldpwd = path_pwd(0);
			if(argn==3)
				a1 = sh_substitute(oldpwd,a1,com[2]);
			else if(a1==0 || *a1==0)
				a1 = nam_strval(HOME);
			else if(*a1 == '-' && *(a1+1) == 0)
				a1 = nam_strval(OLDPWDNOD);
			if(a1==0 || *a1==0)
				cfailed(argn==3?e_subst:e_direct);
			if(*a1 != '/')
			{
				cdpath = nam_strval(CDPNOD);
				if(oldpwd==0)
					oldpwd = path_pwd(1);
			}
			if(cdpath==0)
				cdpath = NULLSTR;
			if(*a1=='.')
			{
				/* test for pathname . ./ .. or ../ */
				if(*(dp=a1+1) == '.')
					dp++;
				if(*dp==0 || *dp=='/')
					cdpath = NULLSTR;
			}
			do
			{
				dp = cdpath;
				cdpath=path_join((char*)dp,a1);
				if(*stak_word()!='/')
				{
					char *last=(char*)stak_end(sh.staktop);
					char *cp = sh_copy(oldpwd,stak_begin());
					*cp++ = '/';
					sh.staktop = sh_copy(last,cp);
				}
#ifdef LSTAT
				if(!flag)
#endif /* LSTAT */
				{
#ifdef FS_3D
					dp = pathcanon(stak_word());
					/* eliminate trailing '/' */
					while(*--dp == '/' && dp>stak_word())
						*dp = 0;
#else
					pathcanon(stak_word());
#endif /* FS_3D */
				}
			}
			while((rval=chdir(path_relative(stak_word())))<0 && cdpath);
			if(rval<0)
			{
				switch(errno)
				{
#ifdef ENAMETOOLONG
				case ENAMETOOLONG:
					dp = e_longname;
					break;
#endif /* ENAMETOOLONG */
#ifdef EMULTIHOP
				case EMULTIHOP:
					dp = e_multihop;
					break;
#endif /* EMULTIHOP */
				case ENOTDIR:
					dp = e_notdir;
					break;

				case ENOENT:
					dp = e_found;
					break;

				case EACCES:
					dp = e_access;
					break;
#ifdef ENOLINK
				case ENOLINK:
					dp = e_link;
					break;
#endif /* ENOLINK */
				default: 	
					dp = e_direct;
					break;
				}
				sh_fail(a1,dp);
			}
			if(a1 == nam_strval(OLDPWDNOD) || argn==3)
				dp = a1;	/* print out directory for cd - */
#ifdef LSTAT
			if(flag)
			{
				a1 = stak_end(stak_word()+PATH_MAX);
				path_physical(a1);
			}
			else
#endif /* LSTAT */
				a1 = (char*)stak_fix();
			if(*dp && *dp!= ':' && (st.states&PROMPT) && strchr(a1,'/'))
			{
				p_setout(st.standout);
				p_str(a1,NL);
			}
			nam_fputval(OLDPWDNOD,oldpwd);
			free(oldpwd);
			nam_free(PWDNOD);
			nam_fputval(PWDNOD,a1);
			nam_ontype(PWDNOD,N_FREE|N_EXPORT);
			sh.pwd = PWDNOD->value.namval.cp;
			break;
		}
		
		case SYSSHFT:
		{
			flag = (a1?(int)sh_arith(a1):1);
			if(flag<0 || st.dolc<flag)
				cfailed(e_number);
			else
			{
				if(st.subflag)
					break;
				st.dolv += flag;
				st.dolc -= flag;
			}
			break;
		}
		
		case SYSWAIT:
			if(!st.subflag)
				job_bwait(com+1);
			break;
		
		case SYSREAD:
		{
			register int fd;
			com += scanargs(com,~(R_FLAG|P_FLAG|U_FLAG|S_FLAG));
			a1 = com[1];
			flag = newflag;
			if(flag&P_FLAG)
			{
				if((fd = sh.cpipe[INPIPE])<=0)
					cfailed(e_query);
			}
			else if(flag&U_FLAG)
				fd = argnum;
			else
				fd = 0;
			if(fd && !fisread(fd))
				cfailed(e_file);
			/* look for prompt */
			if(a1 && (a1=strchr(a1,'?')) && tty_check(fd))
			{
				p_setout(ERRIO);
				p_str(a1+1,0);
			}
			env_readline(&com[1],fd,flag&(R_FLAG|S_FLAG));
			if(fiseof(io_ftable[fd]))
			{
				sh.exitval=1;
				if(flag&P_FLAG)
				{
					io_pclose(sh.cpipe);
					break;
				}
			}
			clearerr(io_ftable[fd]);
			break;
		}
	
		case SYSSET:
			flag = is_option(EXECPR);
			if(a1)
			{
				arg_opts(argn,com);
				st.states &= ~(READPR|MONITOR);
				st.states |= is_option(READPR|MONITOR);
			}
			if(flag)
				sh_trace(com,1);
			if(a1==0 && t->com.comset==0)
				/*scan name chain and print*/
				env_scan(st.standout,0,sh.var_tree,0);
			break;
		
		case SYSEVAL:
			if(a1)
			{
				sh.un.com = com+2;
				sh_eval(a1);
			}
			break;

		case SYSFC:
		{
			register struct history *fp;
			int fdo;
			char *argv[2];
			char fname[TMPSIZ];
			int index2;
			int indx = -1; /* used as subscript for range */
			char *edit = NULL;		/* name of editor */
			char *replace = NULL;		/* replace old=new */
			int incr;
			int range[2];	/* upper and lower range of commands */
			int lflag = 0;
			int nflag = 0;
			int rflag = 0;
			histloc location;
			if(!hist_open())
				cfailed(e_history);
			fp = hist_ptr;
			while((a1=com[1]) && *a1 == '-')
			{
				argnum = -1;
				flag = flagset(a1,~(E_FLAG|L_FLAG|N_FLAG|R_FLAG));
				if(flag==0)
				{
					flag = fp->fixind - argnum-1;
					if(flag <= 0)
						flag = 1;
					range[++indx] = flag;
					argnum = 0;
					if(indx==1)
						break;
				}
				else
				{
					if(flag&E_FLAG)
					{
						/* name of editor specified */
						com++;
						if((edit=com[1]) == NULL)
							cfailed(e_argexp);
					}
					if(flag&N_FLAG)
						nflag++;
					if(flag&L_FLAG)
						lflag++;
					if(flag&R_FLAG)
						rflag++;
				}
				com++;
			}
			flag = indx;
			while(flag<1 && (a1=com[1]))
			{
				/* look for old=new argument */
				if(replace==NULL && strchr(a1+1,'='))
				{
					replace = a1;
					com++;
					continue;
				}
				else if(isdigit(*a1) || *a1 == '-')
				{
					/* see if completely numeric */
					do	a1++;
					while(isdigit(*a1));
					if(*a1==0)
					{
						a1 = com[1];
						range[++flag] = atoi(a1);
						if(*a1 == '-')
							range[flag] += (fp->fixind-1);
						com++;
						continue;
					}
				}
				/* search for last line starting with string */
				location = hist_find(com[1],fp->fixind-1,0,-1);
				if((range[++flag] = location.his_command) < 0)
					sh_fail(com[1],e_found);
				com++;
			}
			if(flag <0)
			{
				/* set default starting range */
				if(lflag)
				{
					flag = fp->fixind-16;
					if(flag<1)
						flag = 1;
				}
				else
					flag = fp->fixind-2;
				range[0] = flag;
				flag = 0;
			}
			if(flag==0)
				/* set default termination range */
				range[1] = (lflag?fp->fixind-1:range[0]);
			if((index2 = fp->fixind - fp->fixmax) <=0)
			index2 = 1;
			/* check for valid ranges */
			for(flag=0;flag<2;flag++)
				if(range[flag]<index2 ||
					range[flag]>=(fp->fixind-(lflag==0)))
					cfailed(e_number);
			if(edit && *edit=='-' && range[0]!=range[1])
				cfailed(e_number);
			/* now list commands from range[rflag] to range[1-rflag] */
			incr = 1;
			flag = rflag>0;
			if(range[1-flag] < range[flag])
				incr = -1;
			if(lflag)
			{
				fdo = st.standout;
				a1 = "\n\t";
			}
			else
			{
				fdo = io_mktmp(fname);
				a1 = "\n";
				nflag++;
			}
			p_setout(fdo);
			while(1)
			{
				if(nflag==0)
					p_num(range[flag],'\t');
				else if(lflag)
					p_char('\t');
				hist_list(hist_position(range[flag]),0,a1);
				if(lflag && (sh.trapnote&SIGSET))
					sh_exit(SIGFAIL);
				if(range[flag] == range[1-flag])
					break;
				range[flag] += incr;
			}
			if(lflag)
				return;
			io_fclose(fdo);
			hist_eof();
			p_setout(ERRIO);
			a1 = edit;
			if(a1==NULL && (a1=nam_strval(FCEDNOD)) == NULL)
				a1 = (char*)e_defedit;
			if(*a1 != '-')
			{
				sh.un.com = argv;
				argv[0] =  fname;
				argv[1] = NULL;
				sh_eval(a1);
			}
			fdo = io_fopen(fname);
			unlink(fname);
			/* don't history fc itself unless forked */
			if(!(st.states&FORKED))
				hist_cancel();
			st.states |= (READPR|FIXFLG);	/* echo lines as read */
			st.exec_flag--;  /* needed for command numbering */
			if(replace!=NULL)
				hist_subst(cmd_name,fdo,replace);
			else if(sh.exitval == 0)
			{
				/* read in and run the command */
				st.states &= ~BUILTIN;
				sh.un.fd = fdo;
				sh_eval((char*)0);
			}
			else
			{
				io_fclose(fdo);
				if(!is_option(READPR))
					st.states &= ~(READPR|FIXFLG);
			}
			st.exec_flag++;
			break;
		}

		case SYSGETOPTS:
		{
			register struct namnod *n;
			extern char opt_option[];
			extern char *opt_arg;
			static char value[2];
			const char *message = e_argexp;
			if(argn < 3)
				cfailed(e_argexp);
			n = env_namset(com[2],sh.var_tree,P_FLAG);
			switch(flag=optget((argn>3?com+2:st.dolv),a1))
			{
				case '?':
					message = e_option;
					/* fall through */
				case ':':
					if(*a1==':')
						opt_arg = opt_option+1;
					else
					{
						p_prp(cmd_name);
						p_str(e_colon,opt_option[1]);
						p_char(' ');
						p_str(message,NL);
						flag = '?';
					}
					*(a1 = value) = flag;
					break;

				case 0:
					a1 = NULLSTR;
					sh.exitval = 1;
					opt_char = 0;
					break;

				default:
					a1 = opt_option + (*opt_option=='-');
			}
			nam_putval(n, a1);
			break;
		}
	
		case SYSWHENCE:
		{
			com += scanargs(com,~(V_FLAG|P_FLAG));
			if(com[1] == 0)
				cfailed(e_nargs);
			p_setout(st.standout);
			sh_whence(com,newflag);
			break;
		}


		case SYSUMASK:
		{
			if(a1)
			{
				register int c;	
				if(st.subflag)
					break;
				if(isdigit(*a1))
				{
					while(c = *a1++)
					{
						if (c>='0' && c<='7')	
							flag = (flag<<3) + (c-'0');	
						else
							cfailed(e_number);
					}
				}
				else
				{
					char **cp = com+1;
					flag = umask(0);
					c = strperm(a1,cp,~flag);
					if(**cp)
					{
						umask(flag);
						cfailed(e_format);
					}
					flag = (~c&0777);
				}
				umask(flag);	
			}	
			else
			{
				p_setout(st.standout);
				a1 = utos((ulong)(flag=umask(0)),8);
				umask(flag);
				*++a1 = '0';
				p_str(a1,NL);
			}
			break;
		}

#ifdef LIM_CPU
#		define HARD	1
#		define SOFT	2
		 /* BSD style ulimit */
		case SYSULIMIT:
		{
#   ifdef RLIMIT_CPU
			struct rlimit rlp;
#   endif /* RLIMIT_CPU */
			const struct sysnod *sp;
			long i;
			int label;
			register int n;
			register int mode = 0;
			int unit;
			int save_index = opt_index;
			int save_char = opt_char;
			opt_char = 0;
			opt_index = 1;
			while((n = optget(com,":HSacdfmnstv")))
			{
				switch(n)
				{
					case 'H':
						mode |= HARD;
						continue;
					case 'S':
						mode |= SOFT;
						continue;
					case 'a':
						flag = (0x2f
#   ifdef LIM_MAXRSS
						|(1<<4)
#   endif /* LIM_MAXRSS */
#   ifdef RLIMIT_NOFILE
						|(1<<6)
#   endif /* RLIMIT_NOFILE */
#   ifdef RLIMIT_VMEM
						|(1<<7)
#   endif /* RLIMIT_VMEM */
							);
						break;
					case 't':
						flag |= 1;
						break;
#   ifdef LIM_MAXRSS
					case 'm':
						flag |= (1<<4);
						break;
#   endif /* LIM_MAXRSS */
					case 'd':
						flag |= (1<<2);
						break;
					case 's':
						flag |= (1<<3);
						break;
					case 'f':
						flag |= (1<<1);
						break;
					case 'c':
						flag |= (1<<5);
						break;
#   ifdef RLIMIT_NOFILE
					case 'n':
						flag |= (1<<6);
						break;
#   endif /* RLIMIT_NOFILE */
#   ifdef RLIMIT_VMEM
					case 'v':
						flag |= (1<<7);
						break;
#   endif /* RLIMIT_VMEM */
					default:
						cfailed(e_option);
				}
			}
			a1 = com[opt_index];
			opt_index = save_index;
			opt_char = save_char;
			/* default to -f */
			if(flag==0)
				flag |= (1<<1);
			/* only one option at a time for setting */
			label = (flag&(flag-1));
			if(a1 && label)
				cfailed(e_option);
			sp = limit_names;
			if(mode==0)
				mode = (HARD|SOFT);
			for(; flag; sp++,flag>>=1)
			{
				if(!(flag&1))
					continue;
				n = sp->sysval>>11;
				unit = sp->sysval&0x7ff;
				if(a1)
				{
					if(st.subflag)
						break;
					if(strcmp(a1,e_unlimited)==0)
						i = INFINITY;
					else
					{
						if((i=sh_arith(a1)) < 0)
							cfailed(e_number);
						i *= unit;
					}
#   ifdef RLIMIT_CPU
					if(getrlimit(n,&rlp) <0)
						cfailed(e_number);
					if(mode&HARD)
						rlp.rlim_max = i;
					if(mode&SOFT)
						rlp.rlim_cur = i;
					if(setrlimit(n,&rlp) <0)
						cfailed(e_ulimit);
#   endif /* RLIMIT_CPU */
				}
				else
				{
#   ifdef  RLIMIT_CPU
					if(getrlimit(n,&rlp) <0)
						cfailed(e_number);
					if(mode&HARD)
						i = rlp.rlim_max;
					if(mode&SOFT)
						i = rlp.rlim_cur;
#   else
					i = -1;
				}
				if((i=vlimit(n,i)) < 0)
					cfailed(e_number);
				if(a1==0)
				{
#   endif /* RLIMIT_CPU */
					p_setout(st.standout);
					if(label)
						p_str(sp->sysnam,SP);
					if(i!=INFINITY)
					{
						i = (i+unit-1)/unit;
						p_str(utos((ulong)i,10),NL);
					}
					else
						p_str(e_unlimited,NL);
				}
			}
			break;
		}
#else
		case SYSULIMIT:
		{
#   ifndef VENIX
			long i;
			long ulimit();
			register int mode = 2;
			if(aflag == '-')
			{
#	ifdef RT
				flag = flagset(a1,~(F_FLAG|P_FLAG));
#	else
				flag = flagset(a1,~F_FLAG);
#	endif /* RT */
				a1 = com[2];
			}
			if(flag&P_FLAG)
				mode = 5;
			if(a1)
			{
				if(st.subflag)
					break;
				if((i=sh_arith(a1)) < 0)
					cfailed(e_number);
			}
			else
			{
				mode--;
				i = -1;
			}
			if((i=ulimit(mode,i)) < 0)
				cfailed(e_number);
			if(a1==0)
			{
				p_setout(st.standout);
				p_str(utos((ulong)i,10),NL);
			}
#   endif /* VENIX */
			break;
		}
#endif /* LIM_CPU */

#ifdef JOBS
#   ifdef SIGTSTP
		case SYSBG:
			flag = 1;
		case SYSFG:
			if(!is_option(MONITOR) || !job.jobcontrol)
			{
				sh.exitval = 1;
				if(st.states&PROMPT)
					cfailed(e_no_jctl);
				break;
			}
			if(job_walk(job_switch,flag,com+1))
				cfailed(e_no_job);
			break;
#    endif /* SIGTSTP */

		case SYSJOBS:
			com += scanargs(com,~(N_FLAG|L_FLAG|P_FLAG));
			if(*++com==0)
				com = 0;
			p_setout(st.standout);
			if(job_walk(job_list,newflag,com))
				cfailed(e_no_job);
			break;

		case SYSKILL:
		{
			if(argn < 2)
				cfailed(e_nargs);
			/* just in case we send a kill -9 $$ */
			p_flush();
			flag = SIGTERM;
			if(aflag == '-')
			{
				a1++;
				if(*a1 == 'l')
				{
#ifdef POSIX
					if(argn>2)
					{
						com++;
						while(a1= *++com)
						{
							if(isdigit(*a1))
								sig_list((atoi(a1)&0177)+1);
							else
							{
								if((flag=sig_number(a1)) < 0)
									cfailed(e_option);
								p_num(flag,NL);
							}
						}
					}
					else
#endif /* POSIX */
					sig_list(0);
					return;
				}
				else if(*a1=='s')
				{
					com++;
					a1 = com[1];
				}
				flag = sig_number(a1);
				if(flag <0 || flag > NSIG)
					cfailed(e_option);
				com++;
			}
			if(*++com==0)
				cfailed(e_nargs);
			if(job_walk(job_kill,flag,com))
				sh.exitval = 2;
			break;
		}
#endif
		
#ifdef apollo
		/*
		 *  Apollo system support library loads into the virtual address space
		 */
		case SYSINLIB:
		{
			int status,xfer;
			short len;
			std_$call void pm_$load();
			std_$call void pm_$call();
			if(st.subflag)
				break;
			if(a1)
			{
				len = strlen(a1);
				pm_$load(*a1, len, 2 , 0, xfer,status);
				if(status!=0)
				sh_fail(a1,"cannot inlib");
				else if(xfer)
					pm_$call(xfer);
			}
			break;
		}

		case SYSINPROCESS:
			if(argn < 2)
				on_option(INPROC);
			else
				sh.exitval = exec_here(com);
			break;
#endif	/* apollo */

#ifdef FS_3D
#define VLEN		14
		case SYSVMAP:
			flag = 1;
		case SYSVPATH:
		{
			char version[VLEN+1];
			char *vend; 
			char toggle;
			switch(argn)
			{
			case 1:
				p_setout(st.standout);
				vend = stak_alloc(v_size(flag)+1);
				v_getbuf(vend,flag);
				toggle = 0;
				while(flag = *vend++)
				{
					if(flag==' ')
					{
						flag  = e_sptbnl[toggle+1];
						toggle = !toggle;
					}
					p_char(flag);
				}
				if(toggle)
					newline();
				break;
			case 2:
				if(flag)
				{
					if(getvmap(a1,version)<0)
						cfailed("cannot get mapping");
					a1 = version;
				}
				else
				{
					if((a1 = path_real(a1)) ==0)
						cfailed("cannot get vpath");
				}
				p_str(a1,NL);
				break;
			case 3:
				if(st.subflag)
					break;
				if(flag)
				{
					if(setvmap(a1,com[2])<0)
						cfailed("cannot set mapping");
				}
				else
				{
					if(setvpath(a1,com[2])<0)
						cfailed("cannot set vpath");
				}
				break;
			default:
				cfailed(e_nargs);
			}
			break;
		}
#endif /* FS_3D */
#ifdef UNIVERSE
    /*
     * there are two styles of universe 
     * Pyramid universes have <sys/universe.h> file
     * Masscomp universes do not
     */

		case SYSUNIVERSE:
		if(a1)
		{
			register char *sp;
			if(setuniverse(univ_number(a1)) < 0)
				cfailed("invalid name");
			/* set directory in new universe */
			if(*(a1 = path_pwd(0))=='/')
				chdir(a1);
			/* clear out old tracked alias and echo binding */
			a1 = nam_fstrval(PATHNOD);
			sp = stak_alloc(6+strlen(a1));
			strcpy(sp,"PATH=");
			strcpy(sp+5,a1);
			env_namset(sp,sh.var_tree,0);
		}
		else
		{
			char universe[TMPSIZ];
			if(getuniverse(universe) < 0)
				cfailed("not accessible");
			else
				p_str(universe,NL);
		}
		break;
#endif /* UNIVERSE */


#ifdef SYSSLEEP
		/* fine granularity sleep builtin someday */
		case SYSSLEEP:
		{
			extern double atof();
			if(a1)
			{
				if(strmatch(a1,"*([0-9])?(.)*([0-9])"))
					sh_delay(atof(a1));
				else
					cfailed(e_number);
			}
			else
				cfailed(e_argexp);
			break;
		
		}
#endif /* SYSSLEEP */
	}
}

static const char flgchar[] = "efgilmnprstuvxEFHLPRZ";
static const int flgval[] = {E_FLAG,F_FLAG,G_FLAG,I_FLAG,L_FLAG,M_FLAG,
			N_FLAG,P_FLAG,R_FLAG,S_FLAG,T_FLAG,U_FLAG,V_FLAG,
			X_FLAG,N_DOUBLE|N_INTGER|N_EXPNOTE,N_DOUBLE|N_INTGER,
			N_HOST,N_LJUST,H_FLAG,N_RJUST,N_RJUST|N_ZFILL};
/*
 * process option flags for built-ins
 * flagmask are the invalid options
 */

static int flagset(flaglist,flagmask)
char *flaglist;
{
	register int flag = 0;
	register int c;
	register char *cp,*sp;
	int numset = 0;

	for(cp=flaglist+1;c = *cp;cp++)
	{
		if(isdigit(c))
		{
			if(argnum < 0)
			{
				argnum = 0;
				numset = -100;
			}
			else
				numset++;
			argnum = 10*argnum + (c - '0');
		}
		else if(sp=strchr(flgchar,c))
			flag |= flgval[sp-flgchar];
		else if(c!= *flaglist)
			goto badoption;
	}
	if(numset>0 && flag==0)
		goto badoption;
	if((flag&flagmask)==0)
		return(flag);
badoption:
	cfailed(e_option);
	/* NOTREACHED */
}

/*
 * process command line options and store into newflag
 */

static int scanargs(com,flags)
char *com[];
{
	register char **argv = ++com;
	register int flag;
	register char *a1;
	if(aflag!='+' && aflag!='-')
		return(0);
	while((a1 = *argv) && *a1==aflag)
	{
		if(a1[1] && a1[1]!=aflag)
			flag = flagset(a1,flags);
		else
			flag = 0;
		argv++;
		if(flag==0)
			break;
		newflag |= flag;
	}
	return(argv-com);
}

/*
 * evaluate the string <s> or the contents of file <un.fd> as shell script
 * If <s> is not null, un is interpreted as an argv[] list instead of a file
 */

void sh_eval(s)
register char *s;
{
	struct fileblk	fb;
	union anynode *t;
	char inbuf[IOBSIZE+1];
	io_push(&fb);
	if(s)
	{
		io_sopen(s);
		if(sh.un.com)
		{
			fb.feval=sh.un.com;
			if(*fb.feval)
				fb.ftype = F_ISEVAL;
		}
	}
	else if(sh.un.fd>=0)
	{
		io_init(input=sh.un.fd,&fb,inbuf);
	}
	sh.un.com = 0;
	st.exec_flag++;
	t = sh_parse(NL,NLFLG|MTFLG|TOPFLG);
	st.exec_flag--;
	if(is_option(READPR)==0)
		st.states &= ~READPR;
	if(s==NULL && hist_ptr)
		hist_flush();
	p_setout(ERRIO);
	io_pop(0);
	sh_exec(t,(int)(st.states&(ERRFLG|MONITOR)));
}


/*
 * Given the name or number of a signal return the signal number
 */

static int sig_number(string)
register char *string;
{
	register int n;
	if(isdigit(*string))
		n = atoi(string);
	else
	{
		ltou(string,string);
		n = sh_lookup(string,sig_names);
		n &= (1<<SIGBITS)-1;
		n--;
	}
	return(n);
}

#ifdef JOBS
/*
 * list all the possible signals
 * If flag is 1, then the current trap settings are displayed
 */
static void sig_list(flag)
{
	register const struct sysnod	*syscan;
	register int n = MAXTRAP;
	const char *names[MAXTRAP+3];
	syscan=sig_names;
	p_setout(st.standout);
	/* not all signals may be defined */
#ifdef POSIX
	if(flag<0)
		n += 2;
#endif /* POSIX */
	while(--n >= 0)
		names[n] = e_trap;
	while(*syscan->sysnam)
	{
		n = syscan->sysval;
		n &= ((1<<SIGBITS)-1);
		names[n] = syscan->sysnam;
		syscan++;
	}
	n = MAXTRAP-1;
#ifdef POSIX
	if(flag<0)
		n += 2;
#endif /* POSIX */
	while(names[--n]==e_trap);
	names[n+1] = NULL;
#ifdef POSIX
	if(flag<0)
	{
		while(--n >= 0)
		{
			if(st.trapcom[n])
			{
				p_str("trap",' ');
				p_qstr(st.trapcom[n],' ');
				p_str(names[n+1],NL);
			}
		}
	}
	else if(flag)
	{
		if(flag <= n && names[flag])
			p_str(names[flag],NL);
		else
			p_num(flag-1,NL);
	}
	else
#endif /* POSIX */
		p_list(n-1,names+2);
}
#endif	/* JOBS */

static void cfailed(message)
MSG message;
{
	sh_fail(cmd_name,message);
}

#ifdef SYSSLEEP
/*
 * delay execution for time <t>
 */

#ifdef _poll_
#   include	<poll.h>
#endif /* _poll_ */
#ifdef HZ
#   define TIC_SEC	HZ	/* number of ticks per second */
#else
#   define TIC_SEC	60	/* number of ticks per second */
#endif /* HZ */


int	sh_delay(t)
double t;
{
	register int n = t;
#ifdef _poll_
	struct pollfd fd;
	if(t<=0)
		return;
	else if(n > 30)
	{
		sleep(n);
		t -= n;
	}
	if(n=1000*t)
		poll(&fd,0,n);
#else
#   ifdef _SELECT5_
	struct timeval timeloc;
	if(t<=0)
		return;
	timeloc.tv_sec = n;
	timeloc.tv_usec = 1000000*(t-(double)n);
	select(0,(fd_set*)0,(fd_set*)0,(fd_set*)0,&timeloc);
#   else
#	ifdef _SELECT4_
		/* for 9th edition machines */
		if(t<=0)
			return;
		if(n > 30)
		{
			sleep(n);
			t -= n;
		}
		if(n=1000*t)
			select(0,(fd_set*)0,(fd_set*)0,n);
#	else
		struct tms tt;
		if(t<=0)
			return;
		sleep(n);
		t -= n;
		if(t)
		{
			clock_t begin = times(&tt);
			if(begin==0)
				return;
			t *= TIC_SEC;
			n += (t+.5);
			while((times(&tt)-begin) < n);
		}
#	endif /* _SELECT4_ */
#   endif /* _SELECT5_ */
#endif /* _poll_ */
}
#endif /* SYSSLEEP */

#ifdef UNIVERSE
#   ifdef _sys_universe_
	int univ_number(str)
	char *str;
	{
		register int n = 0;
		while( n < NUMUNIV)
		{
			if(strcmp(str,univ_name[n])==0)
				return(n);
			n++;
		}
		return(-1);
	}
#   endif /* _sys_universe_ */
#endif /* UNIVERSE */
