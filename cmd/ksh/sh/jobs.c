/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/jobs.c	1.6.3.1"

#include	<errno.h>
#include	"defs.h"
#include	"jobs.h"
#include	"history.h"

#ifdef _sys_wait_
#  include	<sys/wait.h>
#  undef _wait_
#  define _wait_ 1
#else
#  ifdef _wait_
#     include	<wait.h>
#  endif /* _wait_*/
#endif	/* _sys_wait_ */

#ifdef _sys_sigaction_
#  include	<sys/sigaction.h>
#endif /* _sys_sigaction_ */

#ifdef WAIT1ARG
#   undef WNOHANG
#endif /* WAIT1ARG */

#undef	WIFSTOPPED
#define WIFSTOPPED(w)		(((w)&LOBYTE)==0177)
#undef	WSTOPSIG
#define WSTOPSIG(w)		(((w)>>8)&LOBYTE)
#undef	WIFKILLED
#define WIFKILLED(w)		((w)&LOBYTE)
#undef	WKILLSIG
#define WKILLSIG(w)		((w)&(LOBYTE&~HIGHBIT))
#undef	WEXITVAL
#define WEXITVAL(w)		((((unsigned)(w))>>8)&LOBYTE)
#undef	WIFDUMPED
#define WIFDUMPED(w)		(w&HIGHBIT)

static struct process	*job_bypid();
static struct process	*job_byjid();
static char		*job_sigmsg();
static int		job_alloc();
static void 		job_free();
static int		job_unpost();
static void		job_unlink();
static struct process	*freelist;
static char		beenhere;
static struct process	dummy;

#ifdef JOBS
   static void			job_set();
   static void			job_reset();
   static struct process	*job_byname();
   static struct process	*job_bystring();
   static struct termios	my_stty;  /* terminal state for shell */
   static char			*job_string;
#else
#  undef SIGCHLD
   extern const char		e_coredump[];
#endif /* JOBS */

#ifdef SIGTSTP
    static void		job_unstop();
    static void		job_fgrp();
#   ifdef waitpid
#	ifdef TIOCGPGRP
	   static int _i_;
#	   define tcgetpgrp(a) (ioctl(a, TIOCGPGRP, &_i_)>=0?_i_:-1)	
#	endif /* TIOCGPGRP */
	int tcsetpgrp(fd,pgrp)
	int fd;
	pid_t pgrp;
	{
	int pgid = pgrp;
#	ifdef TIOCGPGRP
		return(ioctl(fd, TIOCSPGRP, &pgid));	
#	else
		return(-1);
#	endif /* TIOCGPGRP */
	}
#   endif /* waitpid */
#else
#   define job_unstop(pw)
#endif /* SIGTSTP */

#ifndef OTTYDISC
#   undef NTTYDISC
#endif /* OTTYDISC */


#ifdef JOBS
#ifdef SIGTSTP
/*
 * initialize job control
 * if lflag is set the switching driver message will not print
 */

void job_init(lflag)
{
	register int ntry = 0;

	/* use new line discipline when available */
#   ifdef NTTYDISC
#     ifdef FIOLOOKLD
	if((job.linedisc = ioctl (JOBTTY, FIOLOOKLD, 0)) <0)
#     else
	if(ioctl(JOBTTY,TIOCGETD,&job.linedisc) !=0)
#     endif /* FIOLOOKLD */
		return;
	if(job.linedisc!=NTTYDISC && job.linedisc!=OTTYDISC)
	{
		/* no job control when running with MPX */
#     ifdef VSH
		on_option(VIRAW);
#     endif /* VSH */
		return;
	}
	if(job.linedisc==NTTYDISC)
	{
		job.linedisc = -1;
	}
#   endif /* NTTYDISC */

	if((job.mypgid=getpgid(0))<=0)
		return;

	/* wait until we are in the foreground */
	while((job.mytgid=tcgetpgrp(JOBTTY)) != job.mypgid)
	{
		if(job.mytgid == -1)
			return;
		/* Stop this shell until continued */
		signal(SIGTTIN,SIG_DFL);
		kill(sh.pid,SIGTTIN);
		/* resumes here after continue tries again */
		if(ntry++ > MAXTRY)
		{
			p_str(e_no_start,NL);
			return;
		}
	}

#   ifdef NTTYDISC
	/* set the line discipline */
	if(job.linedisc>=0)
	{
		int linedisc = NTTYDISC;
#	ifdef FIOPUSHLD
		tty_get(JOBTTY,&my_stty);
		if (ioctl(JOBTTY, FIOPOPLD, 0) < 0)
			return;
		if (ioctl(JOBTTY, FIOPUSHLD, &linedisc) < 0)
		{
			ioctl(JOBTTY, FIOPUSHLD, &job.linedisc);
			return;
		}
		tty_set(JOBTTY,TCSANOW,&my_stty);
#	else
		if(ioctl(JOBTTY,TIOCSETD,&linedisc) !=0)
			return;
#	endif /* FIOPUSHLD */
		if(lflag==0)
			p_str(e_newtty,NL);
		else
			job.linedisc = -1;
	}
#   endif /* NTTYDISC */

	/* make sure that we are a process group leader */
	setpgid(0,sh.pid);
#   ifdef SA_NOCLDWAIT
	sigflag(SIGCLD, SA_NOCLDSTOP|SA_NOCLDWAIT, 0);
#   endif /* SA_NOCLDWAIT */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	tcsetpgrp(JOBTTY,sh.pid);
#   ifdef CNSUSP
	/* set the switch character */
	tty_get(JOBTTY,&my_stty);
	job.suspend = my_stty.c_cc[VSUSP];
	if(job.suspend == CNSUSP)
	{
		my_stty.c_cc[VSUSP] = CNSUSP;
		tty_set(JOBTTY,TCSAFLUSH,&my_stty);
	}
#   endif /* CNSUSP */
	on_option(MONITOR);
	job.jobcontrol++;
	return;
}


/*
 * see if there are any stopped jobs
 * restore tty driver and pgrp
 */

int job_close()
{
	register struct process *pw = job.pwlist;
	register int count = 0;
	if(!job.jobcontrol)
		return(0);
	for(;pw;pw=pw->p_nxtjob)
	{
		if(!(pw->p_flag&P_STOPPED))
			continue;
		if(beenhere)
			killpg(pw->p_pgrp,SIGTERM);
		count++;
	}
	if(beenhere++ == 0 && job.pwlist)
	{
		p_setout(ERRIO);
		if(count)
		{
			p_str(e_terminate,NL);
			return(-1);
		}
		else if(sh.login_sh)
		{
			p_str(e_running,NL);
			return(-1);
		}
	}
	setpgid(0,job.mypgid);
	tcsetpgrp(JOBTTY,job.mypgid);
#   ifdef NTTYDISC
	if(job.linedisc>=0)
	{
		/* restore old line discipline */
#	ifdef FIOPUSHLD
		tty_get(JOBTTY,&my_stty);
		if (ioctl(JOBTTY, FIOPOPLD, 0) < 0)
			return(0);
		if (ioctl(JOBTTY, FIOPUSHLD, &job.linedisc) < 0)
		{
			job.linedisc = NTTYDISC;
			ioctl(JOBTTY, FIOPUSHLD, &job.linedisc);
			return(0);
		}
		tty_set(JOBTTY,TCSAFLUSH,&my_stty);
#	else
		if(ioctl(JOBTTY,TIOCSETD,&job.linedisc) !=0)
			return(0);
#	endif /* FIOPUSHLD */
		p_str(e_oldtty,NL);
	}
#   endif /* NTTYDISC */
#   ifdef CNSUSP
	if(job.suspend==CNSUSP)
	{
		tty_get(JOBTTY,&my_stty);
		my_stty.c_cc[VSUSP] = CNSUSP;
		tty_set(JOBTTY,TCSAFLUSH,&my_stty);
	}
#   endif /* CNSUSP */
	job.jobcontrol = 0;
	return(0);
}


/*
 * Set the ttygroup id to a previously running job
 */
#else
void job_init(flag)
{
}

int job_close()
{
	if(!(st.states&MONITOR) || (st.states&FORKED))
		return(0);
	if(beenhere++ == 0 && job.pwlist && sh.login_sh)
	{
		p_str(e_running,NL);
		return(-1);
	}
	return(0);
}
#endif	/* SIGTSTP */

static void job_set(pw)
register struct process *pw;
{
	/* save current terminal state */
	tty_get(JOBTTY,&my_stty);
	if(pw->p_flag&P_STTY)
	{
		/* restore terminal state for job */
		tty_set(JOBTTY,TCSAFLUSH,&pw->p_stty);
	}
#ifdef SIGTSTP
	tcsetpgrp(JOBTTY,pw->p_fgrp);
	/* if job is stopped, resume it in the background */
	job_unstop(pw);
#endif	/* SIGTSTP */
}

static void	job_reset(pw)
register struct process *pw;
{
	/* save the terminal state for current job */
#ifdef SIGTSTP
	job_fgrp(pw,tcgetpgrp(JOBTTY));
	if(tcsetpgrp(JOBTTY,sh.pid) !=0)
		return;
#endif	/* SIGTSTP */
	if(pw && (pw->p_flag&P_SIGNALLED))
	{
		if(tty_get(JOBTTY,&pw->p_stty) == 0)
			pw->p_flag |= P_STTY;
		/* restore terminal state for job */
		tty_set(JOBTTY,TCSAFLUSH,&my_stty);
	}
	beenhere = 0;
}
#endif /* JOBS */

/*
 * wait built-in command
 */

void job_bwait(jobs)
char *jobs[];
{
	register char *job;
	register struct process *pw;
	register pid_t pid;
	if(*jobs==0)
		job_wait((pid_t)-1);
	else while(job = *jobs++)
	{
#ifdef JOBS
		if(*job == '%')
		{
			if(pw = job_bystring(job))
				pid = pw->p_pid;
			else
				return;
		}
		else
#endif /* JOBS */
			pid = atoi(job);
		job_wait(-pid);
	}
}

#ifdef JOBS
/*
 * execute function <fun> for each job
 */

int job_walk(fun,arg,jobs)
int (*fun)();
char *jobs[];
{
	register struct process *pw = job.pwlist;
	register int r = 0;
	register char *job;
	register struct process *px;
	job_string = 0;
	if(jobs==0)
	{
		/* do all jobs */
		for(;pw;pw=px)
		{
			px = pw->p_nxtjob;
			if((*fun)(pw,arg))
				r = 2;
		}
	}
	else if(*jobs==0)	/* current job */
	{
		/* skip over non-stop jobs */
		while(pw && pw->p_pgrp==0)
			pw = pw->p_nxtjob;
		if((*fun)(pw,arg))
			r = 2;
	}
	else while(job = *jobs++)
	{
		job_string = job;
		if(*job==0)
			sh_fail(job_string,e_jobusage);
		if(*job == '%')
			pw = job_bystring(job);
		else
		{
			int pid = atoi(job);
			if(pid<0)
				job++;
			while(isdigit(*job))
				job++;
			if(*job)
				sh_fail(job_string,e_jobusage);
			if(!(pw = job_bypid(pid)))
			{
				pw = &dummy;
				pw->p_pid = pid;
				pw->p_pgrp = pid;
			}
			pw->p_flag |= P_BYNUM;
		}
		if((*fun)(pw,arg))
			r = 2;
		if(pw)
			pw->p_flag &= ~P_BYNUM;
	}
	return(r);
}

/*
 * list the given job
 * flag L_FLAG for long listing
 * flag N_FLAG for list only jobs marked for notification
 * flag P_FLAG for process id(s) only
 */

int job_list(pw,flag)
struct process *pw;
register int flag;
{
	register struct process *px = pw;
	register int  n;
	register const char *msg;
	static char m[2];
	int msize;
	if(pw==0)
		return(1);
	if(job.pwlist==NULL)
		return(0);
	if(job.waitsafe)
		job_wait((pid_t)0);
	if((flag&N_FLAG) && (!(px->p_flag&P_NOTIFY)||px->p_pgrp==0))
		return(0);
	if((flag&P_FLAG))
	{
		p_num(px->p_pgrp,NL);
		return(0);
	}
	n = px->p_job;
	p_sub(n,' ');
	if(px==job.pwlist)
		m[0] = '+';
	else if(px==job.pwlist->p_nxtjob)
		m[0] = '-';
	else
		m[0] = ' ';
	p_str(m,' ');
	do
	{
		n = 0;
		if(flag&L_FLAG)
			p_num(px->p_pid,'\t');
		if(px->p_flag&P_SIGNALLED)
			msg = job_sigmsg((int)(px->p_exit));
		else if(px->p_flag&P_NOTIFY)
		{
			msg = e_Done;
			n = px->p_exit;
		}
		else
			msg = e_Running;
		p_str(msg,0);
		msize = strlen(msg);
		if(n)
		{
			p_str(e_nullstr,'(');
			p_num((int)n,')');
			msize += (3+(n>10)+(n>100));
		}
		if(px->p_flag&P_COREDUMP)
		{
			p_str(e_coredump,0);
			msize += strlen(e_coredump);
		}
		p_nchr(SP,MAXMSG-msize);
		if(flag&L_FLAG)
			px = px->p_nxtproc;
		else
			px = NULL;
		if(px==NULL)
			hist_list(pw->p_name,0,";");
		else
			p_str(e_nlspace,0);
	}
	while(px);
	px = pw;
	if(px->p_flag&P_STOPPED)
		px->p_flag &= ~P_NOTIFY;
	else if(px->p_flag&P_DONE)
	{
		if(!job_unpost(px))
			px->p_flag &= ~P_NOTIFY;
	}
	return(0);
}

/*
 * get the process group given the job number
 * This routine returns the process group number or -1
 */

static struct process *job_bystring(ajob)
register char *ajob;
{
	register struct process *pw=job.pwlist;
	register int c;
	if(*ajob++ != '%' || pw==NULL)
		return(NULL);
	c = *ajob;
	if(isdigit(c))
		pw = job_byjid(atoi(ajob));
	else if(c=='+' || c=='%')
		;
	else if(c=='-')
	{
		if(pw)
			pw = job.pwlist->p_nxtjob;
	}
	else
		pw = job_byname(ajob);
	if(pw && pw->p_flag)
		return(pw);
	return(NULL);
}

/*
 * Kill a job or process
 */

int job_kill(pw,sig)
register struct process *pw;
int sig;
{
	register pid_t pid;
	register int r;
	const char *msg;
	errno = 0;
	if(pw==0)
		goto error;
	pid = pw->p_pid;
	if(pw->p_flag&P_BYNUM)
	{
#ifdef SIGTSTP
		if(sig==SIGSTOP && pid==sh.pid && sh.ppid==1)
		{
			/* can't stop login shell */
			errno = EPERM;
			r = -1;
		}
		else
		{
			if(pw->p_flag&P_STOPPED)
				pw->p_flag &= ~(P_STOPPED|P_SIGNALLED);
			if(pid>=0)
			{
				kill(pid,SIGCONT);
				r = kill(pid,sig);
			}
			else
			{
				killpg(pid,SIGCONT);
				r = killpg(-pid,sig);
			}
		}
#else
		if(pid>=0)
			r = kill(pid,sig);
		else
			r = killpg(-pid,sig);
#endif	/* SIGTSTP */
	}
	else
	{
		job_unstop(pw);
		if(pid = pw->p_pgrp)
		{
			r = killpg(pid,sig);
			if(r>=0)
#   ifdef _poll_
				poll("",0,100);
#   else
				time((time_t)0); /* delay a little for SIGCLD */
#   endif /* _poll_ */
		}
		while(pw && pw->p_pgrp==0 && (r=kill(pw->p_pid,sig))>=0) 
			pw = pw->p_nxtproc;
	}
	if(r<0 && job_string)
	{
	error:
		p_setout(ERRIO);
		p_str(e_killcolon,0);
		if(pw && pw->p_flag&P_BYNUM)
			msg = e_no_proc;
		else
			msg = e_no_job;
		p_str(job_string,0);
		p_str(e_colon,0);
		if(errno == EPERM)
			msg = e_access;
		p_str(msg,NL);
		r = 2;
	}
	return(r);
}

/*
 * Get process structure from first letters of jobname
 *
 */

static struct process *job_byname(name)
char *name;
{
	register struct process *pw = job.pwlist;
	register struct process *pz = 0;
	register int flag = 0;
	register char *cp = name;
	if(hist_ptr==NULL)
		return(NULL);
	if(*cp=='?')
		(cp++,flag++);
	for(;pw;pw=pw->p_nxtjob)
	{
		if(hist_match(pw->p_name,cp,flag)>=0)
		{
			if(pz)
				sh_fail(name-1,e_ambiguous);
			pz = pw;
		}
	}
	return(pz);
}

#else
#   define job_set(x)
#   define job_reset(x)
#endif /* JOBS */



/*
 * Initialize the process posting array
 */

void	job_clear()
{
	register struct process *pw, *px;
	register struct process *pwnext;
	register int j = JBYTES;
	for(pw=job.pwlist; pw; pw=pwnext)
	{
		pwnext = pw->p_nxtjob;
		for(px=pw; px; px=px->p_nxtproc)
			free((char*)px);
	}
	job.pwlist = NULL;
	job.numpost=0;
#ifdef JOBS
	job.jobcontrol = 0;
#endif /* JOBS */
	while(--j >=0)
		job.freejobs[j]  = 0;
}

/*
 * put the process <pid> on the process list and return the job number
 */

int job_post(pid)
pid_t pid;
{
	register struct process *pw;
	register struct history *fp = hist_ptr;
	if(pw=freelist)
		freelist = pw->p_nxtjob;
	else
		pw = new_of(struct process,0);
	job.numpost++;
	if(job.pipeflag && job.pwlist)
	{
		/* join existing current job */
		pw->p_nxtjob = job.pwlist->p_nxtjob;
		pw->p_nxtproc = job.pwlist;
		pw->p_job = job.pwlist->p_job;
	}
	else
	{
		/* create a new job */
		pw->p_nxtjob = job.pwlist;
		pw->p_job = job_alloc();
		pw->p_nxtproc = 0;
	}
	job.pwlist = pw;
	pw->p_pid = pid;
	pw->p_flag = P_RUNNING;
	if(st.states&MONITOR)
		pw->p_fgrp = job.curpgid;
	else
		pw->p_fgrp = 0;
	pw->p_pgrp = pw->p_fgrp;
#ifdef DBUG
	p_setout(ERRIO);
	p_num(getpid(),':');
	p_str("posting",' ');
	p_num(pw->p_job,' ');
	p_str("pid",'=');
	p_num(pw->p_pid,' ');
	p_str("pgid",'=');
	p_num(pw->p_pgrp,NL);
	p_flush();
#endif /* DBUG */
#ifdef JOBS
	if(fp!=NULL)
		pw->p_name=hist_position(fp->fixind-1);
	else
		pw->p_name = -1;
#endif /* JOBS */
	if(job.numpost >= MAXJ-1)
		job_wait((pid_t)1);
	return(pw->p_job);
}

/*
 * Returns a process structure give a process id
 */

static struct process *job_bypid(pid)
pid_t 	pid;
{
	register struct process  *pw, *px;
	for(pw=job.pwlist; pw; pw=pw->p_nxtjob)
		for(px=pw; px; px=px->p_nxtproc)
		{
			if(px->p_pid==pid)
				return(px);
		}
	return(NULL);
}

/*
 * return a pointer to a job given the job id
 */

static struct process *job_byjid(jobid)
{
	register struct process *pw;
	for(pw=job.pwlist;pw; pw = pw->p_nxtjob)
	{
		if(pw->p_job==jobid)
			break;
	}
	return(pw);
}

#ifndef SIG_NORESTART
static jmp_buf waitintr;
static VOID interrupt()
{
	st.intfn = 0;
	longjmp(waitintr,1);
}
#endif /* SIG_NORESTART */

/*
 * Wait for process pid to complete
 * If pid < -1, then wait can be interrupted, -pid is waited for (wait builtin)
 * pid=0 to wait for any process
 * pid=1 to wait for at least one process to complete
 * pid=-1 to wait for all runing processes
 */

void	job_wait(pid)
register pid_t 	pid;
{
	register struct process *pw = NULL;
	register int 	wstat;
	register pid_t	myjob;
	register pid_t 	p;
	int		jobid = 0;
	int		w;
#ifdef WUNTRACED
	int		waitflag = WUNTRACED;
#endif /* WUNTRACED */
	char		intr = 0;
	char		bgjob = 0;
	if(pid==0)
	{
		if(!job.waitsafe)
#ifdef WNOHANG
			waitflag |= WNOHANG;
#else
			return;
#endif /* WNOHANG */
	}
	if(pid < 0)
	{
		pid = -pid;
		intr = 1;
#ifndef SIG_NORESTART
		st.intfn = interrupt;
		if(setjmp(waitintr))
			goto done;
#endif /* SIG_NORESTART */
	}
	if(pid > 1)
	{
		if((pw=job_bypid(pid))==NULL)
			return;
		jobid = pw->p_job;
		if(intr==0 && pw->p_pgrp)
			job_set(job_byjid(jobid));
		if(pw->p_flag&P_DONE)
			goto jobdone;
	}
#ifdef DBUG
	p_setout(ERRIO);
	p_num(getpid(),':');
	p_str(" job_wait job",'=');
	p_num(jobid,' ');
	p_str("pid",'=');
	p_num(pid,NL);
#endif /* DBUG*/
	p_flush();
#ifdef SIGCHLD
	signal(SIGCHLD, SIG_DFL);
	job.waitsafe = 0;
#endif /* SIGCHLD */
	while(1)
	{
		if((p=waitpid((pid_t)-1,&w,waitflag))== -1)
		{
			if(errno==EINTR &&!intr)
				continue;
			goto done;
		}
		if(p==0)
			goto done;
		if((pw = job_bypid(p))==0)
#ifdef JOBS
			goto tryagain;
#else
			continue;
#endif /* JOBS */
		wstat = w;
		myjob= (pw->p_job==jobid);
		if(!myjob && pw->p_pgrp && pw->p_pgrp!=job.curpgid)
			bgjob = 1;
		pw->p_flag &= ~P_RUNNING;
#ifdef DBUG
		p_num(getpid(),':');
		p_str(" job with pid",' ');
		p_num(p,' ');
		p_str("complete with status",'=');
		p_num(w,' ');
		p_str("jobid",'=');
		p_num(pw->p_job,NL);
		p_flush();
#endif /* DBUG*/
#ifdef SIGTSTP
		if (WIFSTOPPED(wstat))
		{
			pw->p_exit = WSTOPSIG(wstat);
			if(myjob && (pw->p_exit==SIGTTIN||pw->p_exit==SIGTTOU))
			{
				/* job stopped prematurely, restart it */
				killpg(pw->p_pgrp,SIGCONT);
				continue;
			}
			pw->p_flag |= (P_NOTIFY|P_SIGNALLED|P_STOPPED);
			if(myjob)
			{
				if((pw=job_byjid(jobid)) && (pw->p_flag&P_RUNNING))
						goto tryagain;
				job_wait((pid_t)0);
				goto done;
			}
			goto tryagain;
		}
		else
#endif /* SIGTSTP */
		{
			if(p==sh.cpid)
			{
				io_fclose(COTPIPE);
				io_fclose(sh.cpipe[1]);
				sh.cpipe[1] = -1;
			}
			if (WIFKILLED(wstat))
			{
				pw->p_flag |= (P_DONE|P_SIGNALLED);
				if (WIFDUMPED(wstat))
					pw->p_flag |= P_COREDUMP;
				pw->p_exit = WKILLSIG(wstat);
				if(!myjob)
					pw->p_flag |= P_NOTIFY;
				else if(pw->p_exit==SIGINT)
					sh_fault(SIGINT); 
#ifdef JOBS
				if(myjob)
#endif /* JOBS */
				{
					p_setout(ERRIO);
					if(pw->p_exit!=SIGINT &&
						pw->p_exit!=SIGPIPE)
					{
						register char *msg;
						if(pid!=p||!(st.states&PROMPT))
						{
							char n[20];
							sh_copy(sh_itos(p),n);
							p_prp(n);
							p_str(e_nullstr,SP);
						}
						if(pw->p_flag&P_COREDUMP)
							p = 0;
						else
							p = NL;
						msg = job_sigmsg((int)(pw->p_exit));
						p_str(msg,p);
						if(p==0)
							p_str(e_coredump,NL);
					}
					if(!intr)
						pw = job_byjid(jobid);
				}
			}
			else
			{
				pw->p_flag |= (P_DONE|P_NOTIFY);
				pw->p_exit = WEXITVAL(wstat);
			}
		}
		if(myjob)
		{
		jobdone:
			/* wstat is true if last process in pipeline */
			if(wstat = (pw==job_byjid(jobid)))
			{
				sh.exitval = pw->p_exit;
				if(pw->p_flag&P_SIGNALLED)
					sh.exitval |= SIGFAIL;
			}
			if(job_unpost(pw)) /* job is complete */
				break;
			else if(intr || (wstat && (!(st.states&MONITOR))))
				break;
			continue;
		}
		else if(pw->p_pgrp==0 && p!=sh.subpid)
			job_unpost(pw);
#ifdef JOBS
	tryagain:
		if(pid==0)
		{
#   ifdef WNOHANG
			waitflag |= WNOHANG;
#   else
			signal(SIGCHLD,sh_fault);
			if(!job.waitsafe)
				goto done;
			job.waitsafe = 0;
#   endif /* WNOHANG */
		}
#endif /* JOBS */
	}
	exitset();
done:
	sh.trapnote &= ~SIGSLOW;
#ifdef SIGCHLD
	if(bgjob && st.trapcom[SIGCHLD])
	{
		st.trapflg[SIGCHLD] |= TRAPSET;
		sh.trapnote |= TRAPSET;
	}
#endif /* SIGCHLD */
#ifndef SIG_NORESTART
	st.intfn = 0;
#endif /* SIG_NORESTART */
#ifdef SIGCHLD
	signal(SIGCHLD,sh_fault);
#endif /* SIGCHLD */
	if(pid>1 && intr==0 && pw->p_pgrp)
		job_reset(pw);
}

#ifdef SIGTSTP


/*
 * move job to foreground if bgflag == 0
 * move job to background if bgflag != 0
 */

job_switch(pw,bgflag)
register struct process *pw;
{
	register const char *msg;
	if(!pw || !(pw=job_byjid((int)pw->p_job)))
		return(1);
	p_setout(st.standout);
	if(bgflag)
	{
		p_sub((int)pw->p_job,'\t');
		msg = "&";
	}
	else
	{
		job_unlink(pw);
		pw->p_nxtjob = job.pwlist;
		job.pwlist = pw;
		msg = e_nullstr;
	}
	hist_list(pw->p_name,'&',";");
	p_str(msg,NL);
	p_flush();
	if(!bgflag)
		job_wait(pw->p_pid);
	else if(pw->p_flag&P_STOPPED)
		job_unstop(pw);
	return(0);
}


/*
 * Set the foreground group associated with a job
 */

static void job_fgrp(pw,new)
register struct process *pw;
register int new;
{
	for(; pw; pw=pw->p_nxtproc)
		pw->p_fgrp = new;
}

/*
 * turn off STOP state of a process group and send CONT signals
 */

static void job_unstop(px)
register struct process *px;
{
	register struct process *pw;
	register int num = 0;
	for(pw=px ;pw ;pw=pw->p_nxtproc)
	{
		if(pw->p_flag&P_STOPPED)
		{
			num++;
			pw->p_flag &= ~(P_STOPPED|P_SIGNALLED|P_NOTIFY);
			pw->p_flag |= P_RUNNING;
		}
	}
	if(num!=0)
	{
		if(px->p_fgrp != px->p_pgrp)
			killpg(px->p_fgrp,SIGCONT);
		killpg(px->p_pgrp,SIGCONT);
	}
}
#endif	/* SIGTSTP */

/*
 * remove a job from table
 * If all the processes have not completed then unpost returns 0
 * Otherwise the job is removed and unpost returns 1.
 * pwlist is reset if the first job is removed
 */

static int job_unpost(pwtop)
register struct process *pwtop;
{
	register struct process *pw;
	/* make sure all processes are done */
#ifdef DBUG
	p_setout(ERRIO);
	p_num(getpid(),':');
	p_str(" unpost pid",'=');
	p_num(pwtop->p_pid,NL);
	p_flush();
#endif /* DBUG */
	pwtop = pw = job_byjid((int)pwtop->p_job);
	for(; pw && (pw->p_flag&P_DONE); pw=pw->p_nxtproc);
	if(pw)
		return(0);
	/* all processes complete, unpost job */
	job_unlink(pwtop);
	for(pw=pwtop; pw ; pw=pw->p_nxtproc)
	{
		pw->p_flag &= ~P_DONE;
		job.numpost--;
		pw->p_nxtjob = freelist;
		freelist = pw;
	}
#ifdef DBUG
	p_str("free job",'=');
	p_num(pwtop->p_job,NL);
	p_flush();
#endif /* DBUG */
	job_free((int)pwtop->p_job);
	return(1);
}

/*
 * unlink a job form the job list
 */

static void job_unlink(pw)
register struct process *pw;
{
	register struct process *px;
	if(pw==job.pwlist)
	{
		job.pwlist = pw->p_nxtjob;
		return;
	}
	for(px=job.pwlist;px;px=px->p_nxtjob)
		if(px->p_nxtjob == pw)
		{
			px->p_nxtjob = pw->p_nxtjob;
			return;
		}
}

/*
 * get an unused job number
 * freejobs is a bit vector, 0 is unused
 */

static int job_alloc()
{
	register int j=0;
	register unsigned mask = 1;
	register unsigned char *freeword;
	/* skip to first word with a free slot */
	while(job.freejobs[j] == 0xff)
		j++;
	freeword = &job.freejobs[j];
	j *= 8;
	for(j++;mask&(*freeword);j++,mask <<=1);
	*freeword  |=mask;
	return(j);
}

/*
 * return a job number
 */

static void job_free(n)
register int n;
{
	register int j = (--n)/8;
	register unsigned mask;
	n -= j*8;
	mask = 1 << n;
	job.freejobs[j]  &= ~mask;
}

static char *job_sigmsg(sig)
int sig;
{
	static char signo[] = "Signal xxx";
	if(sig < (MAXTRAP-1) && sh.sigmsg[sig])
		return(sh.sigmsg[sig]);
	sh_copy(sh_itos(sig),signo+7);
	return(signo);
}
