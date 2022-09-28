/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/main.c	1.5.3.1"

#include	"defs.h"
#include	"jobs.h"
#include	"sym.h"
#include	"history.h"
#include	"timeout.h"
#include	"builtins.h"
#ifdef pdp11
#   include	<execargs.h>
#endif	/* pdp11 */
#ifdef TIOCLBIC
#   undef TIOCLBIC
#   ifdef _sys_ioctl_
#	include	<sys/ioctl.h>
#   endif /* _sys_ioctl_ */
#endif /* TIOCLBIC */


/* These routines are referenced by this module */
extern int	gscan_some();

static void	exfile();
static void	pr_prompt();
static void	chkmail();
static int	notstacked();
static void	dfault();

extern char	**environ;

static struct fileblk topfile;
static struct stat lastmail;
static time_t	mailtime;
static char	beenhere = 0;

main(c, v)
int 	c;
register char *v[];
{
	register char *sim;
	register int 	rsflag;	/* local restricted flag */
	char *command;
	int prof;
#ifdef apollo
	extern char pm_$unix_env ;
	pm_$unix_env = -1;
#endif	/* apollo */
	st.standout = 1;
	sh.cpipe[0] = -1;
	sh.userid=getuid();
	sh.euserid=geteuid();
	sh.groupid=getgid();
	sh.egroupid=getegid();
	time(&mailtime);
	sig_init();

	/* initialize storage allocation */
	sh_addblok(0);

	/* set up memory for name-value pairs */
	nam_init();

	/* set name-value pairs from userenv
	 *  'rsflag' is zero if SHELL variable is
	 * set in environment and contains an'r' in
	 * the basename part of the value.
	 */
	rsflag=env_init();
	/* a shell is also restricted if argv(0) has
	 *  an 'rsh' for its basename
	 */
	sim = path_basename(*v);
	if(*sim=='-')
	{
		sim++;
		sh.login_sh = 2;
	}
	/* check for restricted shell */
	if(c>0 && strmatch(sim,"*r*sh"))
		rsflag=0;

	/* look for options */
	/* st.dolc is $#	*/
	st.dolc=arg_opts(c,v);
	st.dolv=v+c-st.dolc--;
	st.dolv[0] = v[0];
	if(st.dolc < 1)
		on_option(STDFLG);
	if(is_option(STDFLG|CFLAG)==0)
	{
		st.dolc--;
		st.dolv++;
	}
	/* set[ug]id scripts require the -p flag */
	prof = !is_option(PRIVM);
	if(sh.userid!=sh.euserid || sh.groupid!=sh.egroupid)
	{
#ifdef P_UID
		/* require sh -p to run setuid and/or setgid */
		if(!is_option(PRIVM) && sh.userid >= P_UID)
		{
			setuid(sh.euserid=sh.userid);
			setgid(sh.egroupid=sh.groupid);
		}
		else
#else
		{
			on_option(PRIVM);
			prof = 0;
		}
#endif /* P_UID */
#ifdef SHELLMAGIC
		/* careful of #! setuid scripts with name beginning with - */
		if(sh.login_sh && eq(v[0],v[1]))
			sh_fail(e_prohibited,NIL);
#endif /*SHELLMAGIC*/
	}
	else
		off_option(PRIVM);
	if(is_option(RSHFLG))
	{
		rsflag = 0;
		off_option(RSHFLG);
	}
	sh.shname = st.dolv[0]; /* shname for $0 in profiles */
	/*
	 * return here for shell file execution
	 * but not for parenthesis subshells
	 */
	st.cmdadr = st.dolv[0]; /* cmdadr is $0 */
	setjmp(sh.subshell);
	command = st.cmdadr;
	/* set pidname '$$' */
	sh.pid = getpid();
	io_settemp(sh.pid);
	srand(sh.pid&0x7fff);
	sh.ppid = getppid();
	dfault(PS4NOD,e_traceprompt);
	path_pwd(1);
	if((beenhere++)==0)
	{
		st.states |= PROFILE;
		/* decide whether shell is interactive */
		if(is_option(ONEFLG|CFLAG)==0 && is_option(STDFLG) &&
			tty_check(0) && tty_check(ERRIO))
			on_option(INTFLG|BGNICE);
		if(sh.ppid==1)
			sh.login_sh++;
		if(sh.login_sh >= 2)
		{
			/* ? profile */
			sh.login_sh += 2;
			job_init(1);
			/*	system profile	*/
			if((input=path_open(e_sysprofile,NULLSTR)) >= 0)
			{
				st.cmdadr = (char*)e_sysprofile;
				exfile();	/* file exists */
				io_fclose(input);
			}
			if(prof &&  (input=path_open(mac_try((char*)e_profile),NULLSTR)) >= 0)
			{
				st.cmdadr = (char*)e_profile;
				exfile();
				io_fclose(input);
			}
		}
		/* make sure PWD is set up correctly */
		path_pwd(1);
		if(prof)
			sim = mac_try(nam_strval(ENVNOD));
		else if(is_option(PRIVM))
			sim = (char*)e_suidprofile;
		else
			sim = NULLSTR;
		if(*sim && (input = path_open(sim,NULLSTR)) >= 0)
		{
			st.cmdadr = sh_heap(sim);
			exfile();
			free(st.cmdadr);
			st.cmdadr = command;
			io_fclose(input);
		}
		st.cmdadr = command;
		st.states &= ~PROFILE;
		if(rsflag==0)
		{
			on_option(RSHFLG);
			nam_ontype(PATHNOD,N_RESTRICT);
			nam_ontype(ENVNOD,N_RESTRICT);
			nam_ontype(SHELLNOD,N_RESTRICT);
		}
		/* open input file if specified */
		st.cmdline = 0;
		st.standin = 0;
		if(sh.comdiv)
		{
			st.standin = &topfile;
			io_sopen(sh.comdiv);
			input = F_STRING;
		}
		else
		{
			sim = st.cmdadr;
			st.cmdadr = v[0];
			if(is_option(STDFLG))
				input = 0;
			else
			{
				char *sp;
				/* open stream should have been passed into shell */
				if(strmatch(sim,e_devfdNN))
				{
					struct stat statb;
					input = atoi(sim+8);
					if(fstat(input,&statb)<0)
						sh_fail(st.cmdadr,e_open);
					sim = st.cmdadr;
					off_option(EXECPR|READPR);
				}
				else
				{
#ifdef VFORK
				if(strcmp(sim,path_basename(*environ))==0)
					sp = (*environ)+2;
				else
#endif /* VFORK */
				sp = path_absolute(sim);
				if(sp==0 || (input=io_fopen(sp))<0)
#ifndef VFORK
					/* for compatibility with bsh */
					if((input=io_fopen(sim))<0)
#endif /* VFORK */
						sh_fail(sim,e_open);
				}
			}
			st.cmdadr = sim;
			sh.comdiv--;
#ifdef ACCT
			initacct();
			if(input != 0)
				preacct(st.cmdadr);
#endif	/* ACCT */
		}
		if(!is_option(INTFLG))
		{
			/* eliminate local aliases and functions */
			gscan_some(env_nolocal,sh.alias_tree,N_EXPORT,0);
			gscan_some(env_nolocal,sh.fun_tree,N_EXPORT,0);
		}
	}
#ifdef pdp11
	else
		*execargs=(char *) st.dolv;	/* for `ps' cmd */
#endif	/* pdp11 */
	st.states |= is_option(INTFLG);
	nam_fputval(IFSNOD,(char*)e_sptbnl);
	exfile();
	if(st.states&PROMPT)
	        newline();
	sh_done(0);
	/* NOTREACHED */
}

static void	exfile()
{
	time_t curtime;
	union anynode *t;
	register struct fileblk *fp = st.standin;
	register int fno = input;
	int maxtry = MAXTRY;
	int execflags;
	/* move input */
	if(fno!=F_STRING)
	{
		if(fno > 0)
		{
			fno = input = io_renumber(fno,INIO);
			io_ftable[0] = 0;
		}
		io_init(input=fno,(struct fileblk*)0,NIL);
		st.standin = fp = io_ftable[fno];
		if(is_option(ONEFLG) && !(st.states&PROFILE))
			fp->flag |= IONBF;
	}
	if(st.states&INTFLG)
	{
		dfault(PS1NOD, (sh.euserid?e_stdprompt:e_supprompt));
		fp->flag |= IOEDIT;
		sig_ignore(SIGTERM);
		if(hist_open())
		{
			on_option(FIXFLG);
		}
		if(sh.login_sh<=1)
			job_init(0);
	}
	else
	{
		if(!(st.states&PROFILE))
		{
			on_option(HASHALL|INPROC);
			off_option(MONITOR);
		}
		st.states &= ~(PROMPT|MONITOR|FIXFLG);
		off_option(FIXFLG);
	}
	if(setjmp(sh.errshell) && (st.states&PROFILE))
	{
		io_fclose(fno);
		return;
	}
	/* error return here */
	opt_char = 0;
	opt_index = 1;
	sh.freturn = (jmp_buf*)sh.errshell;
	st.loopcnt = 0;
	st.peekn = 0;
	st.iopend = 0;
	st.linked = 0;
	sh.trapnote = 0;
	if(fiseof(fp))
		goto eof_or_error;
	/* command loop */
	while(1)
	{
		stak_reset((STKPTR)0);
		stak_check(); /* may reduce sbrk */
		exitset();
		st.states &= ~(ERRFLG|READPR|GRACE|MONITOR);
		st.states |= is_option(READPR)|WAITING|ERRFLG;
		/* -eim  flags don't apply to profiles */
		if(st.states&PROFILE)
			st.states &= ~(INTFLG|ERRFLG|MONITOR);
		p_setout(ERRIO);
		if((st.states&PROMPT) && notstacked() && !fiseof(fp))
		{
			register char *mail;
#ifdef JOBS
			st.states |= is_option(MONITOR);
			job_walk(job_list,N_FLAG,(char**)0);
#endif	/* JOBS */
			if((mail=nam_strval(MAILPNOD)) || (mail=nam_strval(MAILNOD)))
			{
				time(&curtime);
				if ((curtime - mailtime) >= sh_mailchk)
				{
					chkmail(mail);
					mailtime = curtime;
				}
			}
			if(hist_ptr)
				hist_eof();
			pr_prompt(mac_try(nam_strval(PS1NOD)));
			/* sets timeout for command entry */
#ifdef TIMEOUT
			if(sh_timeout <= 0 || sh_timeout > TIMEOUT)
				sh_timeout = TIMEOUT;
#endif /* TIMEOUT */
			if(sh_timeout>0)
				alarm((unsigned)sh_timeout);
			fp->flin = 1;
		}
		st.peekn = io_readc();
		if(fiseof(fp) || fiserror(fp))
		{
		eof_or_error:
			if((st.states&PROMPT) && notstacked() && fiserror(fp)==0) 
			{
				clearerr(fp);
				st.peekn = 0;
				if(--maxtry>0 && is_option(NOEOF) &&
					 !fiserror(&io_stdout) && tty_check(fno))
				{
					p_str(e_logout,NL);
					continue;
				}
				else if(job_close()<0)
					continue;
				hist_close();
			}
			return;
		}
		maxtry = MAXTRY;
		if(sh_timeout>0)
			alarm(0);
		if((st.states&PROMPT) && hist_ptr && st.peekn)
		{
			hist_eof();
			p_char(st.peekn);
			p_setout(ERRIO);
		}
		st.states |= is_option(FIXFLG);
		st.states &= ~ WAITING;
		st.cmdline = fp->flin;
		t = sh_parse(NL,MTFLG|TOPFLG);
		sh.readscript = 0;
		if(hist_ptr)
			hist_flush();
		if(t)
		{
			execflags = (ERRFLG|MONITOR|PROMPT);
			/* sh -c simple-command may not have to fork */
			if(!(st.states&PROFILE) && is_option(CFLAG) && nextchar(fp)==0)
				execflags |= 1;
			st.execbrk = 0;
			p_setout(ERRIO);
			sh_exec(t,execflags);
			/* This is for sh -t */
			if(is_option(ONEFLG))
			{
				io_unreadc(NL);
				fp->flag |= IOEOF;
			}
		}
	}
}

/*
 * if there is pending input, the prompt is not printed.
 * prints PS2 if flag is zero otherwise PS3
 */

void	sh_prompt(flag)
register int flag;
{
	register char *cp;
	register struct fileblk *fp;
	if(flag || (st.states&PROMPT) && notstacked())
	{
		if(flag)
			fp = io_ftable[0];
		else
			fp = st.standin;
		/* if characters are in input buffer don't issue prompt */
		if(fp && finbuff(fp))
			return;
		if(cp = nam_fstrval(flag?PS3NOD:PS2NOD))
		{
			flag = output;
			p_setout(ERRIO);
			p_str(cp,0);
			p_setout(flag);
		}
	}
}



/* prints out messages if files in list have been modified since last call */
static void chkmail(files)
char *files;
{
	register char *cp,*sp,*qp;
	register char save;
	struct stat	statb;
	if(*(cp=files) == 0)
		return;
	sp = cp;
	do
	{
		/* skip to : or end of string saving first '?' */
		for(qp=0;*sp && *sp != ':';sp++)
			if((*sp == '?' || *sp=='%') && qp == 0)
				qp = sp;
		save = *sp;
		*sp = 0;
		/* change '?' to end-of-string */
		if(qp)
			*qp = 0;
		st.gchain = NULL;
		do
		{
			/* see if time has been modified since last checked
			 * and the access time <= the modification time
			 */
			if(stat(cp,&statb) >= 0 && statb.st_mtime >= mailtime
				&& statb.st_atime <= statb.st_mtime)
			{
				/* check for directory */
				if(st.gchain==NULL && (statb.st_mode&S_IFMT)==S_IFDIR) 
				{
					/* generate list of directory entries */
					f_complete(cp,"/*");
				}
				else
				{
					/*
					 * If the file has shrunk,
					 * or if the size is zero
					 * then don't print anything
					 */
					if(statb.st_size &&
						(  statb.st_ino != lastmail.st_ino
						|| statb.st_dev != lastmail.st_dev
						|| statb.st_size > lastmail.st_size))
					{
						/* save and restore $_ */
						char *save = sh.lastarg;
						sh.lastarg = cp;
						p_str(mac_try(qp==0?(char*)e_mailmsg:qp+1),NL);
						sh.lastarg = save;
					}
					lastmail = statb;
					break;
				}
			}
			if(st.gchain)
			{
				cp = st.gchain->argval;
				st.gchain = st.gchain->argchn;
			}
			else
				cp = NULL;
		}
		while(cp);
		if(qp)
			*qp = '?';
		*sp++ = save;
		cp = sp;
	}
	while(save);
	stak_reset((STKPTR)0);
}

/*
 * print the primary prompt
 */

static void pr_prompt(str)
register char *str;
{
	register int c;
	static short cmdno;
#ifdef TIOCLBIC
	int mode;
	mode = LFLUSHO;
	ioctl(ERRIO,TIOCLBIC,&mode);
#endif	/* TIOCLBIC */
	p_flush();
	p_setout(ERRIO);
	for(;c= *str;str++)
	{
		if(c==FC_CHAR)
		{
			/* look at next character */
			c = *++str;
			/* print out line number if not !! */
			if(c!= FC_CHAR)
				p_num(hist_ptr?hist_ptr->fixind:++cmdno,0);
			if(c==0)
				break;
		}
		p_char(c);
	}
#if VSH || ESH
	*io_ftable[ERRIO]->ptr = 0;
	/* prompt flushed later */
#else
	p_flush();
#endif
}

static void dfault(np,value)
register struct namnod *np;
char *value;
{
	if(isnull(np))
		nam_fputval(np,value);
}

static notstacked()
{
	register struct fileblk *fp;
	/* clean up any completed aliases */
	while((fp=st.standin)->ftype==F_ISALIAS && fp->flast==0)
		io_pop(1);
	return(fp->fstak==0);
}

