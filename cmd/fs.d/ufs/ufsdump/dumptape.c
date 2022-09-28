/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ident	"@(#)ufs.cmds:ufs/ufsdump/dumptape.c	1.8.3.1"

#include <setjmp.h>
#include <errno.h>
#include "dump.h"
#include "sys/param.h"

static char (*tblock)[TP_BSIZE]; /* Pointer to malloc()ed buffer for tape */
static int writesize;		/* Size of malloc()ed buffer for tape */
static int trecno = 0;
extern int ntrec;		/* blocking factor on tape */
extern int tenthsperirg;	/* tenths of an inch per inter-record gap */
extern int read(), write();

/*
 * Concurrent dump mods (Caltech) - disk block reading and tape writing
 * are exported to several slave processes.  While one slave writes the
 * tape, the others read disk blocks; they pass control of the tape in
 * a ring via signals.	The parent process traverses the filesystem and
 * sends spclrec()'s and lists of daddr's to the slaves via pipes.
 */
struct req {			/* instruction packets sent to slaves */
	int count;
	daddr_t dblk;
} *req;
static int reqsiz;

#define SLAVES 3		/* 1 reading pipe, 1 reading disk, 1 writing */

static int slavefd[SLAVES];	/* pipes from master to each slave */
static int slavepid[SLAVES];	/* used by killall() */
static int rotor;		/* next slave to be instructed */
static int master;		/* pid of master, for sending error signals */
static int bufrecs;		/* tape records (not blocks) per buffer */
union u_spcl *nextspcl; 	/* where to copy next taprec record */

/*
 * Allocate tape buffer, on page boundary for tape write() efficiency,
 * with array of req packets immediately preceeding it so both can be
 * written together by flusht().
 */
alloctape()
{
	int pgoff = PAGEOFFSET;	    /* pagesize better be power of 2 */
	char *buf, *malloc();

	writesize = ntrec * TP_BSIZE;

	bufrecs = ntrec + (20/ntrec)*ntrec;
	reqsiz = (bufrecs+1) * sizeof (struct req);
	if ((buf=malloc(reqsiz + bufrecs*TP_BSIZE + pgoff)) == NULL)
		return (0);

	tblock = (char (*)[TP_BSIZE]) (((long)buf + reqsiz + pgoff) &~ pgoff);
	nextspcl = (union u_spcl *)tblock;
	req = (struct req *) (tblock[0] - reqsiz);
	return (1);
}

/* write enough TS_END records to bring us to an ntrec boundry */

runout()
{
	register int i;

	spcl.c_type = TS_END;
	i = trecno;
	do {
		spclrec();
	} while (++i % ntrec);
}

taprec(dp)
	char *dp;
{

	req[trecno].dblk = (daddr_t)0;
	req[trecno].count = 1;
	*nextspcl++ = *(union u_spcl *)dp;			/* block move */
	trecno++;
	spcl.c_tapea++;
	if (trecno >= bufrecs || spcl.c_type == TS_END && trecno % ntrec == 0)
		flusht();
}

dmpblk(blkno, size)
	daddr_t blkno;
	int size;
{
	register int avail, tpblks;
	daddr_t dblkno;

	dblkno = fsbtodb(sblock, blkno);
	tpblks = size / TP_BSIZE;
	while ((avail = MIN(tpblks, bufrecs - trecno)) > 0) {
		req[trecno].dblk = dblkno;
		req[trecno].count = avail;
		trecno += avail;
		spcl.c_tapea += avail;
		if (trecno >= bufrecs)
			flusht();
		dblkno += avail * (TP_BSIZE / DEV_BSIZE);
		tpblks -= avail;
	}
}

static int nogripe = 0;

tperror()
{

	if (pipeout) {
		msg("Tape write error on %s\n", tape);
		msg("Cannot recover\n");
		dumpabort();
		/* NOTREACHED */
	}
	msg("Tape write error %ld feet into tape %d\n", asize/120L, tapeno);
	broadcast("TAPE ERROR!\n");
	if (!query("Do you want to restart?"))
		dumpabort();
	msg("This tape will rewind.  After it is rewound,\n");
	msg("replace the faulty tape with a new one;\n");
	msg("this dump volume will be rewritten.\n");
	killall();
	nogripe = 1;
	close_rewind();
	Exit(X_REWRITE);
}

/* compatibility routine */
tflush(i)
	int i;
{

	for (i = 0; i < ntrec; i++)
		spclrec();
}

flusht()
{
	int siz = (char *)nextspcl - (char *)req;

	req[trecno].count = 0;			/* Sentinel */
	if (atomic(write, slavefd[rotor], req, siz) != siz) {
		perror("  UFSDUMP: error writing command pipe");
		dumpabort();
	}
	if (++rotor >= SLAVES)
		rotor = 0;
	nextspcl = (union u_spcl *)tblock;
	asize += (writesize/density + tenthsperirg) * trecno / ntrec;
	blockswritten += trecno;
	trecno = 0;
	if (!pipeout && asize > tsize) {
		close_rewind();
		otape();
	}
	timeest();
}

ufsrewind()
{
	int f;

	for (f = 0; f < SLAVES; f++)
		close(slavefd[f]);
	while (wait(NULL) >= 0)
		;	/* wait for any signals from slaves */
	if (pipeout)
		return;
	msg("Tape rewinding\n");
	close(to);
	while ((f = open(tape, 0)) < 0)
		sleep(10);
	close(f);
}

close_rewind()
{

	ufsrewind();
	if (!nogripe) {
		msg("Change Tapes: Mount tape #%d\n", tapeno+1);
		broadcast("CHANGE TAPES!\7\7\n");
	}
	while (!query("Is the new tape mounted and ready to go?"))
		if (query ("Do you want to abort?")) {
			dumpabort();
			/*NOTREACHED*/
		}
}

/*
 *	We implement taking and restoring checkpoints on the tape level.
 *	When each tape is opened, a new process is created by forking; this
 *	saves all of the necessary context in the parent.  The child
 *	continues the dump; the parent waits around, saving the context.
 *	If the child returns X_REWRITE, then it had problems writing that tape;
 *	this causes the parent to fork again, duplicating the context, and
 *	everything continues as if nothing had happened.
 */
otape()
{
	int parentpid;
	int childpid;
	int status;
	int waitpid;
	void (*interrupt)() = signal(SIGINT, SIG_IGN);

	parentpid = getpid();

    restore_check_point:
	signal(SIGINT, interrupt);
	fflush(stderr);
	/*
	 *	All signals are inherited...
	 */
	childpid = fork();
	if (childpid < 0) {
		msg("Context save fork fails in parent %d\n", parentpid);
		Exit(X_ABORT);
	}
	if (childpid != 0) {
		/*
		 *	PARENT:
		 *	save the context by waiting
		 *	until the child doing all of the work returns.
		 *	don't catch the interrupt
		 */
		signal(SIGINT, SIG_IGN);
#ifdef TDEBUG
		msg("Tape: %d; parent process: %d child process %d\n",
			tapeno+1, parentpid, childpid);
#endif /* TDEBUG */
		while ((waitpid = wait(&status)) != childpid)
			msg("Parent %d waiting for child %d has another child %d return\n",
			    parentpid, childpid, waitpid);
		if (status & 0xFF) {
			msg("Child %d returns LOB status %o\n",
				childpid, status&0xFF);
		}
		status = (status >> 8) & 0xFF;
#ifdef TDEBUG
		switch (status) {
		case X_FINOK:
			msg("Child %d finishes X_FINOK\n", childpid);
			break;
		case X_ABORT:
			msg("Child %d finishes X_ABORT\n", childpid);
			break;
		case X_REWRITE:
			msg("Child %d finishes X_REWRITE\n", childpid);
			break;
		default:
			msg("Child %d finishes unknown %d\n", childpid, status);
			break;
		}
#endif /* TDEBUG */
		switch (status) {
		case X_FINOK:
			Exit(X_FINOK);
		case X_ABORT:
			Exit(X_ABORT);
		case X_REWRITE:
			goto restore_check_point;
		default:
			msg("Bad return code from dump: %d\n", status);
			Exit(X_ABORT);
		}
		/*NOTREACHED*/
	} else {	/* we are the child; just continue */
#ifdef TDEBUG
		sleep(4);	/* allow time for parent's message to get out */
		msg("Child on Tape %d has parent %d, my pid = %d\n",
		    tapeno+1, parentpid, getpid());
#endif
		while (( to = pipeout ? 1 : creat(tape, 0666)) < 0)
			if (!query("Cannot open tape.  Do you want to retry the open?"))
				dumpabort();

		enslave();  /* Share open tape file descriptor with slaves */

		asize = 0;
		tapeno++;		/* current tape sequence */
		newtape++;		/* new tape signal */
		spcl.c_volume++;
		spcl.c_type = TS_TAPE;
		spclrec();
		if (tapeno > 1)
			msg("Tape %d begins with blocks from ino %d\n",
			    tapeno, ino);
	}
}

dumpabort()
{

	if (master != 0 && master != getpid())
		kill(master, SIGTERM);	/* Signals master to call dumpabort */
	else {
		killall();
		msg("The ENTIRE dump is aborted.\n");
	}
	Exit(X_ABORT);
}

Exit(status)
{

#ifdef TDEBUG
	msg("pid = %d exits with status %d\n", getpid(), status);
#endif /* TDEBUG */
	exit(status);
}

sigpipe()
{

	msg("Broken pipe\n");
	dumpabort();
}

killall()
{
	register int i;

	for (i = 0; i < SLAVES; i++)
		if (slavepid[i] > 0)
			kill(slavepid[i], SIGKILL);
}

static int ready, caught;
static jmp_buf jbuf;

proceed()
{

	if (ready)
		longjmp(jbuf, 1);
	caught++;
}

enslave()
{
	int cmd[2];			/* file descriptors */
	register int i, j;

	master = getpid();
	signal(SIGTERM, dumpabort); /* Slave sends SIGTERM on dumpabort() */
	signal(SIGPIPE, sigpipe);
	signal(SIGIOT, tperror);    /* Slave sends SIGIOT on tape errors */
	signal(SIGTRAP, proceed);
	for (i = 0; i < SLAVES; i++) {
		if (pipe(cmd) < 0 || (slavepid[i] = fork()) < 0) {
			perror("  UFSDUMP: can't create child");
			dumpabort();
		}
		slavefd[i] = cmd[1];
		if (slavepid[i] == 0) {		/* Slave starts up here */
			int next;		    /* pid of neighbor */

			for (j = 0; j <= i; j++)
				close(slavefd[j]);
			close(fi);		    /* Need our own seek ptr */
			if ((fi = open(disk, 0)) < 0) {
				perror("  UFSDUMP: can't reopen disk");
				dumpabort();
			}
			signal(SIGINT, SIG_IGN);    /* Master handles this */
			atomic(read, cmd[0], &next, sizeof (next));
			doslave(cmd[0], next, i);
			Exit(X_FINOK);
		}
		close(cmd[0]);
	}
	for (i = 0; i < SLAVES; i++)
		atomic(write, slavefd[i], slavepid + (i+1)%SLAVES, sizeof (int));
	kill(slavepid[0], SIGTRAP);
	master = 0;
	rotor = 0;
}

doslave(cmd, next, mynum)
	int cmd, next, mynum;
{
	int nread;

	while ((nread = atomic(read, cmd, req, reqsiz)) == reqsiz) {
		register struct req *p;
		register int nrec = 0, trec;
		register char *tp;

		for (p = req, trecno = 0; p->count > 0;
		    trecno += p->count, p += p->count) {
			if (p->dblk == 0)
				trec = trecno - nrec++;
		}
		if (nrec > 0 &&
		    atomic(read, cmd, tblock[trec], nrec*TP_BSIZE) !=
		    nrec*TP_BSIZE) {
			msg("Master/slave protocol botched\n");
			dumpabort();
		}
		for (p = req, trecno = 0; p->count > 0;
		    trecno += p->count, p += p->count) {
			if (p->dblk)
				bread(p->dblk, tblock[trecno], p->count*TP_BSIZE);
			else if (trecno < trec)
				*(union u_spcl *)tblock[trecno] =
				    *(union u_spcl *)tblock[trec++];
		}
		if (setjmp(jbuf) == 0) {
			ready = 1;
			if (!caught)
				pause();
		}
		ready = caught = 0;
		for (tp = tblock[0]; (trecno -= ntrec) >= 0; tp += writesize) {
			if (write(to, tp, writesize) == writesize)
				continue;
			kill(master, SIGIOT);	/* Restart from checkpoint */
			pause();
		}
		kill(next, SIGTRAP);	    /* Next slave's turn */
	}
	if (nread != 0) {
		perror("  UFSDUMP: error reading command pipe");
		dumpabort();
	}
}

/*
 * Since a read from a pipe may not return all we asked for,
 * or a write may not write all we ask if we get a signal,
 * loop until the count is satisfied (or error).
 */
atomic(func, fd, buf, count)
	int (*func)(), fd, count;
	char *buf;
{
	int got, need = count;
	extern int errno;

	while (need > 0) {
		got = (*func)(fd, buf, MIN(need, 4096));
		if (got < 0 && errno == EINTR)
			continue;
		if (got <= 0)
			break;
		buf += got;
		need -= got;
	}
	return ( (count-=need) == 0 ? got : count);
}
