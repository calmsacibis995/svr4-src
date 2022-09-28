/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/fault.c	1.5.3.1"

#include	"defs.h"
#include	"jobs.h"
#include	"sym.h"
#include	"timeout.h"


/* ========	fault handling routines	   ======== */

#ifndef JOBS
#   undef SIGCHLD
#endif /* JOBS */

void	sh_fault(sig)
register int 	sig;
{
	register int 	flag;
#ifdef OLDTERMIO
	/* This .1 sec delay eliminates break key problems on 3b consoles */
#   ifdef _poll_
	if(sig==2)
		poll("",0,100);
#   endif /* _poll_ */
#endif /* OLDTERMIO */
#ifdef	SIGCHLD
	if(sig==SIGCHLD)
	{
		job.waitsafe++;
		if(st.trapcom[SIGCHLD])
		{
			sh.trapnote |= SIGSLOW;
#   ifndef SIG_NORESTART
			if(st.intfn)
			{
				sigrelease(sig);
				(*st.intfn)();
			}
#   endif	/* SIG_NORESTART */
		}
		return;
	}
#endif	/* SIGCHLD */
	signal(sig, sh_fault);
	if(sig==SIGALRM)
	{
		if((st.states&WAITING) && sh_timeout>0)
		{
			if(st.states&GRACE)
			{
				/* force exit */
					st.states &= ~GRACE;
					st.states |= FORKED;
					sh_fail(e_timeout,NIL);
			}
			else
			{
				st.states |= GRACE;
				alarm((unsigned)TGRACE);
				p_str(e_timewarn,NL);
				p_flush();
			}
		}
	}
	else
	{
		if(st.trapcom[sig])
			flag = TRAPSET;
		else
		{
			sh.lastsig = sig;
			flag = SIGSET;
		}
		sh.trapnote |= flag;
		st.trapflg[sig] |= flag;
		if(sig <= SIGQUIT)
			sh.trapnote |= SIGSLOW;
	}
#ifndef SIG_NORESTART
	/* This is needed because interrupted reads automatically restart */
	if(st.intfn)
	{
		sigrelease(sig);
		(*st.intfn)();
	}
#endif	/* SIG_NORESTART */
}

void sig_init()
{
	register int i;
	register int n;
	register const struct sysnod	*syscan = sig_names;
	sig_begin();
	while(*syscan->sysnam)
	{
		n = syscan->sysval;
		i = n&((1<<SIGBITS)-1);
		n >>= SIGBITS;
		st.trapflg[--i] = (n&~SIGIGNORE);
		if(n&SIGFAULT)
			signal(i,(VOID(*)())sh_fault);
		else if(n&SIGIGNORE)
			sig_ignore(i);
		else if(n&SIGCAUGHT)
			sig_ontrap(i);
		else if(n&SIGDONE)
		{
			sh.trapnote |= SIGBEGIN;
			if(signal(i,(VOID(*)())sh_done)==SIG_IGN)
			{
				sig_ignore(i);
				st.trapflg[i] = SIGOFF;
			}
			else
				st.trapflg[i] = SIGMOD|SIGDONE;
			sh.trapnote &= ~SIGBEGIN;
		}
		syscan++;
	}
	for(syscan=sig_messages; n=syscan->sysval; syscan++)
	{
		if(n > NSIG+1)
			continue;
		if(*syscan->sysnam)
			sh.sigmsg[n-1] = (char*)syscan->sysnam;
	}
}

/*
 * set signal n to ignore
 * returns 1 if signal was already ignored, 0 otherwise
 */
int	sig_ignore(n)
register int n;
{
	if(n < MAXTRAP-1 && !(st.trapflg[n]&SIGIGNORE))
	{
		if(signal(n,SIG_IGN) != SIG_IGN)
		{
			st.trapflg[n] |= SIGIGNORE;
			st.trapflg[n] &= ~SIGFAULT;
			return(0);
		}
		st.trapflg[n] = SIGOFF;
	}
	return(1);
}

/*
 * Turn on trap handler for signal <n>
 */

void	sig_ontrap(n)
register int n;
{
	register int flag;
	if(n==DEBUGTRAP)
		sh.trapnote |= TRAPSET;
	/* don't do anything if already set or off by parent */
	else if(!(st.trapflg[n]&(SIGFAULT|SIGOFF)))
	{
		flag = st.trapflg[n];
		if(signal(n,(VOID(*)())sh_fault)==SIG_IGN) 
		{
			/* has it been set to ignore by shell */
			if(flag&SIGIGNORE)
				flag |= SIGFAULT;
			else
			{
				/* It ignored already, keep it ignored */ 
				sig_ignore(n);
				flag = SIGOFF;
			}
		}
		else
			flag |= SIGFAULT;
		flag &= ~(SIGSET|TRAPSET|SIGIGNORE|SIGMOD);
		st.trapflg[n] = flag;
	}
}

/*
 * Restore to default signals
 * Do not free the trap strings if flag is non-zero
 */

void	sig_reset(flag)
{
	register int 	i;
	register char *t;
	i=MAXTRAP;
	while(i--)
	{
		t=st.trapcom[i];
		if(t==0 || *t)
		{
			if(flag)
				st.trapcom[i] = 0; /* don't free the traps */
			sig_clear(i);
		}
		st.trapflg[i] &= ~(TRAPSET|SIGSET);
	}
	sh.trapnote=0;
}

/*
 * reset traps at start of function execution
 * keep track of which traps are caught by caller in case they are modified
 * flag==0 before function, flag==1 after function
 */

void	sig_funset(flag)
{
	register int 	i;
	register char *tp;
	i=MAXTRAP;
	while(i--)
	{
		tp = st.trapcom[i];
		if(flag==0)
		{
			if(tp && *tp==0)
				st.trapflg[i] = SIGOFF;
			else
			{
				if(tp)
					st.trapflg[i] |= SIGCAUGHT;
				st.trapflg[i] &= ~(TRAPSET|SIGSET);
			}
			st.trapcom[i] = 0;
		}
		else if(tp)
			sig_clear(i);
	}
	sh.trapnote = 0;
}

/*
 * free up trap if set and restore signal handler if modified
 */

void	sig_clear(n)
register int 	n;
{
	register int flag = st.trapflg[n];
	register char *t;
	if(t=st.trapcom[n])
	{
		free(t);
		st.trapcom[n]=0;
		flag &= ~(TRAPSET|SIGSET);
	}
	if(flag&(SIGFAULT|SIGMOD|SIGIGNORE))
	{
		if(flag&SIGCAUGHT)
		{
			if(flag&(SIGMOD|SIGIGNORE))
				signal(n, sh_fault);
		}
		else if((flag&SIGDONE))
		{
			if(t || (flag&SIGIGNORE))
				signal(n, sh_done);
		}
		else
			 signal(n, SIG_DFL);
		flag &= ~(SIGMOD|SIGFAULT|SIGIGNORE);
		if(flag&SIGCAUGHT)
			flag |= SIGFAULT;
		else if(flag&SIGDONE)
			flag |= SIGMOD;
	}
	st.trapflg[n] = flag;
}


/*
 * check for traps
 */

void	sh_chktrap()
{
	register int 	i=MAXTRAP;
	register char *t;
#ifdef JOBS
	if(job.waitsafe)
		job_wait((pid_t)0);
#endif /* JOBS */
	/* process later if doing command subsitution */
	if(st.subflag)
		return;
	sh.trapnote &= ~(TRAPSET|SIGSLOW);
	if((st.states&ERRFLG) && sh.exitval)
	{
		if(st.trapcom[ERRTRAP])
			st.trapflg[ERRTRAP] = TRAPSET;
		if(is_option(ERRFLG))
			sh_exit(sh.exitval);
	}
	while(--i)
	{
		if(st.trapflg[i]&TRAPSET)
		{
			st.trapflg[i] &= ~TRAPSET;
			if(t=st.trapcom[i])
			{
				int savxit=sh.exitval;
				sh_eval(t);
				p_flush();
				sh.exitval=savxit;
				exitset();
			}
		}
	}
	if(st.trapcom[DEBUGTRAP])
	{
		st.trapflg[DEBUGTRAP] |= TRAPSET;
		sh.trapnote |= TRAPSET;
	}
}

