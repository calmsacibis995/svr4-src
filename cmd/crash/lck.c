/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:lck.c	1.9.8.1"

/*
 * This file contains code for the crash function lck.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#ifndef i386
#include <sys/sbd.h>
#endif
#include <sys/var.h>
#include <a.out.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/fs/s5inode.h>
#include <sys/flock.h>
#include "crash.h"

static struct syment *Flckinfo,*Sleeplcks;	/* namelist symbol */
extern struct syment *Vnode,*Inode,*Ninode;	/* pointers */
struct procid {			/* effective and sys ids */
	pid_t epid;
	long sysid;
	int valid;
};
struct procid *procptr;		/* pointer to procid structure */
extern char *malloc();

/* get effective and sys ids into table */
int
getprocid()
{
	struct proc *prp, prbuf;
	struct pid pid;
	static int lckinit = 0;
	register i;
	proc_t *slot_to_proc();

	if(lckinit == 0) {
		procptr = (struct procid *)malloc((unsigned)
			(sizeof (struct procid) * vbuf.v_proc));
		lckinit = 1;
	}

	for (i = 0; i < vbuf.v_proc; i++) {
		prp = slot_to_proc(i);
		if (prp == NULL)
			procptr[i].valid = 0;
		else {
			readmem((long)prp,1, -1,(char *)&prbuf,sizeof (proc_t),
				"proc table");
			readmem((long)prbuf.p_pidp,1,-1,
				(char *)&pid,sizeof(struct pid), "pid table");
			procptr[i].epid = prbuf.p_epid;
			procptr[i].sysid = prbuf.p_sysid;
			procptr[i].valid = 1;
		}
	}
}

/* find process with same id and sys id */
int
findproc(pid,sysid)
pid_t pid;
short sysid;
{
	int slot;

	for (slot = 0; slot < vbuf.v_proc; slot++) 
		if ((procptr[slot].valid) &&
		    (procptr[slot].epid == pid) &&
		    (procptr[slot].sysid == sysid))
			return slot;
	return(-1);
}

/* get arguments for lck function */
int
getlcks()
{
	int slot = -1;
	int phys = 0;
	int all = 0;
	unsigned long addr = -1;
	unsigned long arg1 = -1;
	unsigned long arg2 = -1;
	int c;
	struct flckinfo infobuf;

	if(!Flckinfo)
		if(!(Flckinfo = symsrch("flckinfo")))
			error("flckinfo not found in symbol table\n");
	if(!Sleeplcks)
		if(!(Sleeplcks = symsrch("sleeplcks")))
			error("sleeplcks not found in symbol table\n");
	optind = 1;
	while((c = getopt(argcnt,args,"epw:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			case 'e' :	all = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	getprocid();
	readmem((long)Flckinfo->n_value,1,-1,(char *)&infobuf,
		sizeof infobuf,"flckinfo table");
	fprintf(fp,"\nAdministrative Info:\n");
	fprintf(fp,"Currently_in_use  Total_used\n");
	fprintf(fp,"     %5d             %5d\n\n",
		infobuf.reccnt,
		infobuf.rectot);
	if(args[optind]) {
		fprintf(fp,"TYP WHENCE     START     LEN       EPID       SYSID   STAT         PREV     NEXT\n");
			do {
				if((addr = strcon(args[optind], 'h')) == -1)
					error("\n");
				prlcks(phys,addr);
			}while(args[++optind]);
	} else
		prilcks();
}

/* print lock information relative to s5inodes (default) */
int
prilcks()
{
	struct	filock	*actptr,*slptr,fibuf;
	struct flckinfo info;
	unsigned long iptr;
	struct inode ibuf;
	int active = 0;
	int free = 0;
	int sleep = 0;
	int slot,next,prev;

	if(!Inode)
		if(!(Inode = symsrch("inode")))
			error("S5 inode table not found in symbol table\n");
	if(!Ninode)
		if(!(Ninode = symsrch("ninode")))
			error("cannot determine S5 inode table size\n");

	readmem(Ninode->n_value,1,-1,(char *)&ninode, sizeof ninode,
		"size of S5 inode table");
	readmem((long)(Inode->n_value),1,-1,(char *)&iptr, sizeof iptr, "S5 inode");

	fprintf(fp,"Active Locks:\n");
	fprintf(fp,"INO TYP WHENCE   START    LEN    PROC   EPID     SYSID  WAIT    PREV     NEXT\n");
	for(slot = 0; slot < ninode; slot++) {
		readmem((long)(iptr+slot*sizeof ibuf),1,-1,
			(char *)&ibuf,sizeof ibuf,"inode table");
		if(ibuf.i_mode == 0)
			continue;
		actptr = (struct filock *)ibuf.i_vnode.v_filocks;
		while(actptr) {
			readmem((long)actptr,1,-1,(char *)&fibuf,
				sizeof fibuf,"filock information");
			++active;
			fprintf(fp,"%3d ",slot);
			if(fibuf.set.l_type == F_RDLCK) 
				fprintf(fp," r  ");
			else if(fibuf.set.l_type == F_WRLCK) 
				fprintf(fp," w  ");
			else fprintf(fp," ?  ");
			fprintf(fp,"%d %10ld %10ld %4d %8d %8d    %x    %8x %8x\n",
				fibuf.set.l_whence,
				fibuf.set.l_start,
				fibuf.set.l_len,
				findproc(fibuf.set.l_pid,fibuf.set.l_sysid),
				fibuf.set.l_pid,
				fibuf.set.l_sysid,
				fibuf.stat.wakeflg,
				fibuf.prev,
				fibuf.next);
			actptr = fibuf.next;

		}
	}

	fprintf(fp,"\nSleep  Locks:\n");
	fprintf(fp,"TYP WHENCE   START    LEN    LPROC   EPID    SYSID  BPROC    EPID   SYSID    PREV     NEXT\n");
	readmem((long)Sleeplcks->n_value,1,-1,(char *)&slptr,
		sizeof slptr,"sleep lock information table");
	while (slptr) {
		readmem((long)slptr,1,-1,(char *)&fibuf,sizeof fibuf,
			"sleep lock information table slot");
		++sleep;
		if(fibuf.set.l_type == F_RDLCK) 
			fprintf(fp," r  ");
		else if(fibuf.set.l_type == F_WRLCK) 
			fprintf(fp," w  ");
		else fprintf(fp," ?  ");
		fprintf(fp,"%1d %10ld %10ld %4d %6d %8d %4d %6d %8d %8x %8x",
			fibuf.set.l_whence,
			fibuf.set.l_start,
			fibuf.set.l_len,
			findproc(fibuf.set.l_pid,fibuf.set.l_sysid),
			fibuf.set.l_pid,
			fibuf.set.l_sysid,
			findproc(fibuf.stat.blk.pid,fibuf.stat.blk.sysid),
			fibuf.stat.blk.pid,
			fibuf.stat.blk.sysid,
			fibuf.prev,
			fibuf.next);
		slptr = fibuf.next;
	}

	fprintf(fp,"\nSummary From Actual Lists:\n");
	fprintf(fp," TOTAL    ACTIVE  SLEEP\n");
	fprintf(fp," %4d    %4d    %4d\n",
		active+sleep,
		active,
		sleep);
}    

/* print linked list of locks */
int
prlcks(phys,addr)
int phys;
unsigned long addr;
{
	struct filock fibuf;

	readbuf(addr,0,phys,-1,(char *)&fibuf,sizeof fibuf,"frlock");
	fprintf(fp," %c%c%c",
	(fibuf.set.l_type == F_RDLCK) ? 'r' : ' ',
	(fibuf.set.l_type == F_WRLCK) ? 'w' : ' ',
	(fibuf.set.l_type == F_UNLCK) ? 'u' : ' ');
	fprintf(fp," %1d %10ld %10ld %8d %8d %8x %8x %8x\n",
		fibuf.set.l_whence,
		fibuf.set.l_start,
		fibuf.set.l_len,
		fibuf.set.l_pid,
		fibuf.set.l_sysid,
		fibuf.stat.wakeflg,
		fibuf.prev,
		fibuf.next);
}
