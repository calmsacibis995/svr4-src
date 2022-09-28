/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_uexec.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_uexec.c	3.22	LCC);	/* Modified: 18:22:12 11/17/89 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

/* uexec.c						*/
/*							*/
/*							*/
/* Created	August 8, 1985	Niket K. Patwardhan	*/

/*               **** Sleaze Alert ****			*
 * Down where uexec() does it's fork, it is possible	*
 * for the child to die before the pid gets assigned	*	 
 * into the "ux" table.  If this happens the status	*	 
 * never gets saved and a future uwait will cause a 	*	 
 * hang on the PC.  The correct solution is to inhibit	* 
 * SIGCLD around the fork, but this requires different	*	 
 * approaches on different unix systems.  AT&T also 	*	 
 * makes vague warnings about expecting SIGCLD to 	*	 
 * continue operating as it currently does in future	*	 
 * versions of unix.  So I made a sleazy, but generic,	*	 
 * fix using the variables deadkid and deadstat.  This	*	 
 * fix does not close the hole completely, but makes it	*	 
 * very small.						*	 
 *			jta  4/29/86			*/	 
	 
#include "pci_types.h"
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#ifdef ULTRIX
#include <sys/wait.h>
#endif /* ULTRIX */

#ifndef NOFILE
#define	NOFILE	30
#endif

/*** TUNABLE PARAMETERS ***/

#define	NARG	29	/* Maximum number of arguments */
#define	NENV	29	/* Maximum number of environment strings */
#define NUXL	16	/* Maximum number of unwaited for uexecs */


/* Pointer arrays for execve */
static char *argp[NARG+3];
static char *envp[NENV+3];
static char *buf, *bufp, *ep;
static int n_uexec = 0;

extern char **environ;		/* Server's environment pointer */

/* Log of uexeced tasks */
static struct ux
{
	long pid;
	long stat;
	char cmd[55];
	char flag;
} ux[NUXL];
long deadkid, deadstat;
#define	GOOD	-1
#define BAD	0
#define	DONE	1
#define	WORKING	2

extern void sig_catcher();
extern char *malloc();
extern FILE *logFile;

#ifndef SIGNAL_KNOWN
#if	!defined(BSD43) && !defined(RIDGE)
#if	defined(ICT) || defined(CCI) || defined(UPORT)
extern int (*signal())();
#else
extern void (*signal())();
#endif /* ICT */
#endif /* !BSD43 */
#endif /* SIGNAL_KNOWN */

extern unsigned alarm();

/***** uexec() ***********************************************************/
/*									  */
/*	ap	Pointer to text area of incoming packet			  */
/*	type	Indicator of first and last packets			  */
/*	msize	Text area size						  */
/*	asize	If first packet, size of all text areas together	  */
/*	eoff	If packet with first environment, its offset, else msize  */
/*	flag	Save status flag. (0 = don't save status)		  */
/*									  */
/*	RETURNS pid of uexec'ed process, 0 = failure, -1 = not done yet	  */
/**************************************************************************/
long
uexec(ap, type, msize, asize, eoff, flag)
char *ap;
int type, msize, asize, eoff, flag;
{
	register int i;
	register char *cp;
	long j,k;
	char **environp, **envpp;

	log("uexec: %s %x %d %d %x %x\n", ap, type, msize, asize, eoff, flag);
	/* Collect strings from all packets. Assign memory on first packet */
	if(type & 1)
	{
		log("allocing memory %d\n", asize);
		buf = malloc(asize);
		if(buf <= (char *)0)
		{
			log("uexec: No buffer\n");
			return(BAD);
		}
		bufp = buf;
		ep = buf+eoff;
	}
	memcpy(bufp, ap, msize);
	bufp += msize;
	if(!(type & 2)) return(GOOD);
	log("uexec: Last packet received\n");
	
	/*** Now all arguments have been collected ***/
	cp = buf;

	cp += strlen(cp) + 1;	/* strip command to get argv[0] */
	/** Collect string pointers **/
	for(i = 0; (i < NARG+2) && (cp < ep); i++)
	{
	    log("%s\n", cp);
	    argp[i] = cp;
	    cp += strlen(cp) + 1;
	}
	log("uexec: %d arguments\n", i-2);
	if(cp != ep) { free(buf, bufp-buf); return(BAD); }
	argp[i] = 0;

	/** Now get environment pointers **/
	log("uexec: Getting env pointers\n");
	envpp = envp;
	for (environp = environ, i = 0; *environp && i < NENV; i++)
		*envpp++ = *environp++;
#ifdef JANUS
	for (environp = envp; environp < envpp; environp++)
		if (envcmp(*environp, "ONUNIX="))
			break;
	if (environp == envpp) {
#ifdef MERGE386
		*envpp++ = "ONUNIX=MERGE386";
#else
		*envpp++ = "ONUNIX=MERGE286";
#endif
		i++;
	}
#endif /* JANUS */
	while (cp < bufp) {
		log("%s\n", cp);
		for (environp = envp; environp < envpp; environp++) {
			if (envcmp(*environp, cp)) {
				for (ep = cp; *ep && *ep != '='; ep++)
					;
				if (*ep && *++ep)
					*environp = cp;
				else {
					*environp-- = *--envpp;
					i--;
				}
				break;
			}
		}
		if (environp == envpp && i <= NENV) {
			*envpp++ = cp;
			i++;
		}
		cp += strlen(cp) + 1;
	}
	for (environp = envp; environp < envpp; environp++)
		if (envcmp(*environp, "PATH="))
			break;
	if (environp == envpp) {
		*envpp++ = "PATH=/bin:/usr/bin";
		i++;
	}
	log("uexec: Got %d env pointers\n", i-1);
	if(cp != bufp) { free(buf, bufp-buf); return(BAD); }
	envp[i] = 0;

	/** All exec args setup, so go **/
	/*** Remember the command, and then fork and remember the PID ***/
	log("Looking for free slot\n");
	for(i=0; i<NUXL; i++)
	{
		if(ux[i].pid) continue;
		strncpy(ux[i].cmd, buf, 55);
		ux[i].flag = WORKING;
		log("Forking\n");
		if(ux[i].pid = fork())		/* Really mean an assign! */
		{
			if (deadkid == ux[i].pid) {
				ux[i].stat = deadstat;
				ux[i].flag = DONE;
				deadkid = 0;
			}
			free(buf, bufp - buf);
			if(ux[i].pid > 0)
			{
				j = ux[i].pid;
				if(!flag) {
					ux[i].pid = 0;
				}
				else {
					n_uexec++;
				}
				log("Uexec'ed process %d\n", j);
				return(j);
			}
			log("Bad fork!\n");
			ux[i].pid = 0;
			return(BAD);
		}

		/** Child process **/
#ifdef SYS5
		setpgrp();
#else
		setpgrp(0,getpid());    
#endif
		for(i=0; i<NOFILE; i++)
		close(i);
		while (open("/dev/null", 2) == -1 && errno == EINTR)
			;
		dup(0);
		dup(0);
		log("uexec: %s\n", buf);
		environ = envp;
#ifdef ULTRIX
/* If on.exe is execing /bin/sh (all the currently extant versions do) */
/* then really execute sh5, ULTRIX's System V shell.  The regular */
/* ULTRIX sh does a few things that confuse on.exe (such as create the */
/* output file even if the command is not found). */

		if (!strcmp(buf, "/bin/sh"))
			execvp("/bin/sh5", argp);
		else
#endif
		execvp(buf, argp);
		exit(1);
	}
	free(buf, bufp-buf);
	return(BAD);
}

/******	uwait() ***********************************************************/
/*									  */
/*	wf	Wait flag						  */
/*	addr	Pointer to output packet				  */
/*									  */
/*	RETURNS nothing!						  */
/**************************************************************************/	
void
uwait(wf, addr)
int wf;
struct output *addr;
{
	register int i,j;
	int statbuf;
#if defined(SYS5_3) || defined(SYS5_4) || defined(ULTRIX)
	void (*saved)();
#else
	int (*saved)();
#endif	/* SYS5_3 || SYS5_4 || ULTRIX */
	unsigned oldtime;

	i = -1;
	if(n_uexec > 0)
	{
		i = 0;
		for(j = 0; j<NUXL; j++)
		if(ux[j].pid && (ux[j].flag == DONE))
		{
			i = ux[j].pid;
			ux[j].pid = 0;
			n_uexec--;
			statbuf = ux[j].stat;
			goto done;
		}

/* retry: */
		log("uwait: at 'retry' label %d\n", n_uexec);
		saved = signal(SIG_CHILD, SIG_DFL);
#ifndef ULTRIX
		/* Blocking mode is not supported. */
		/* The PCILIB doc states this clearly. We sort of fake */
		/* it for system V by waiting 30 secs.  I tried */
		/* using the ULTRIX sigvec call to allow the alarm */
		/* to interrupt wait() below, but there's a bug */
		/* in ULTRIX that prevents that from working */
		/* (occasionally gets a SIGILL in wait() itself). */

		signal(SIGALRM, sig_catcher);
		if(wf)
			oldtime = alarm(30);
		else
			oldtime = alarm(2);
#endif /* !ULTRIX */
		log("uwait: waiting......\n");
		u_wait(&statbuf);
		signal(SIG_CHILD, saved);
#ifndef ULTRIX
		alarm(oldtime);
#endif /* !ULTRIX */

		for(j = 0; j<NUXL; j++)
		if(ux[j].pid && (ux[j].flag == DONE))
		{
			i = ux[j].pid;
			ux[j].pid = 0;
			n_uexec--;
			statbuf = ux[j].stat;
			goto done;
		}
	}
done:
	log("uwait: Process %d status %x count %d\n", i, statbuf, j);
	addr->hdr.res = 0;
	addr->hdr.f_size = i;
	if((statbuf & 0xFF) == 0) addr->hdr.b_cnt = (statbuf >> 8) & 0xFF;
	else if((statbuf & 0xFF) == 0x7F)
	   addr->hdr.b_cnt = ((statbuf >> 8) & 0xFF) | 0x400;
	else if((statbuf & 0xFF00) == 0)
	   addr->hdr.b_cnt = (statbuf & 0x7F) | ((statbuf & 0x80)?0x200:0x100);
}

u_wait(statbuf)
int *statbuf;
{
	register int i,j;

#ifdef ULTRIX
	i = wait3(statbuf, WNOHANG, 0);
#else /* ! ULTRIX */
	i = wait(statbuf);
#endif /* ULTRIX */

	if (i == -1)
	{
		log("u_wait: wait failed, errno %d\n", errno);
		return(-1);
	}
	log("uwait: Process %d just died\n", i);
	if ((j = find_ux(i)) == -1) {
		deadkid = i;
		deadstat = *statbuf;
	}
	else {
		ux[j].stat = *statbuf;
		ux[j].flag = DONE;
	}
	return(i);
}

/************************************************************************
* Name: 	pci_ukill()
* 
* Purpose:	Allow PCI user to send signals to unix processes.
*
* Input:	pid	Process id for kill function
*		sig	Signal to send
*		addr	Pointer to reply packet
*
* Output:	addr->hdr.res contains code reflecting success/failure
*
*************************************************************************/

void
pci_ukill(pid, sig, addr)
long pid;
int sig;
struct output *addr;
{
    if (kill((int) pid, sig)) {
	addr->hdr.res = INVALID_FUNCTION;
    }
    else {
	addr->hdr.res = SUCCESS;
    }
}
	    

int
find_ux(procid)
int procid;
{
	register int j;
	
	for(j=0; j<NUXL; j++) 
		if (ux[j].pid == procid) return (j);
	return (-1);
}

kill_uexecs()
{
    int i;
    
    for (i=0; i<NUXL; i++) {
	if (ux[i].flag == WORKING)
	    kill(-ux[i].pid,SIGHUP);
    }
}

/*
 *  Compare two environment strings for name equality
 */

envcmp(s1, s2)
register char *s1, *s2;
{
	while (*s1 == *s2) {
		if (*s1 == '=')
			return 1;
		s1++;
		s2++;
	}
	return 0;
}
