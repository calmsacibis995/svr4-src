/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_SYSINFO_H
#define _SYS_SYSINFO_H

#ident	"@(#)head.sys:sys/sysinfo.h	11.14.5.1"

/*
 *	System Information.
 */

struct sysinfo {
	time_t	cpu[5];
#define	CPU_IDLE	0
#define	CPU_USER	1
#define	CPU_KERNEL	2
#define	CPU_WAIT	3
#define CPU_SXBRK	4
	time_t	wait[3];
#define	W_IO	0
#define	W_SWAP	1
#define	W_PIO	2
	long	bread;
	long	bwrite;
	long	lread;
	long	lwrite;
	long	phread;
	long	phwrite;
	long	swapin;
	long	swapout;
	long	bswapin;
	long	bswapout;
	long	pswitch;
	long	syscall;
	long	sysread;
	long	syswrite;
	long	sysfork;
	long	sysexec;
	long	runque;
	long	runocc;
	long	swpque;
	long	swpocc;
	long	iget;
	long	namei;
	long	dirblk;
	ulong	readch;
	ulong	writech;
	long	rcvint;
	long	xmtint;
	long	mdmint;
	long	rawch;
	long	canch;
	long	outch;
	long	msg;
	long	sema;
	long	pnpfault;
	long	wrtfault;
    long    s5ipage;
    long    s5inopage;
    long    ufsipage;
    long    ufsinopage;
    long    xxipage;
    long    xxinopage;
};

extern struct sysinfo sysinfo;

struct syswait {
	short	iowait;
	short	swap;
	short	physio;
};

struct minfo {
	unsigned long 	freemem[2]; 	/* freemem in pages */
					/* "double" long format	*/
					/* freemem[0] least significant */
	long	freeswap;	/* free swap space */
	long    vfault;  	/* translation fault */
	long    demand;		/*  demand zero and demand fill pages */
	long    swap;		/*  pages on swap */
	long    cache;		/*  pages in cache */
	long    file;		/*  pages on file */
	long    pfault;		/* protection fault */
	long    cw;		/*  copy on write */
	long    steal;		/*  steal the page */
	long    freedpgs;	/* pages are freed */
	long    vfpg; 		/* pages are freed by vhand*/
	long    sfpg;		/* pages are freed by sched*/
	long    vspg;		/* pages are freed/swapped by vhand */
	long    sspg;		/* pages are freed/swapped by sched */
	long    unmodsw;	/* getpages finds unmodified pages on swap */
	long	unmodfl;	/* getpages finds unmodified pages in file */ 
#ifdef	i386			/* psk */
	long    psoutok;        /* swapping out a process */
	long    psinfail;       /* swapping in a process failed */
	long    psinok;         /* swapping in a process succeeded */
	long    rsout;          /* swapping out a region */
	long    rsin;           /* swapping in a region */
#endif
};

typedef struct fsinfo {
	ulong fsireadch;
	ulong fsiwritech;
	ulong fsivop_open;
	ulong fsivop_close;
	ulong fsivop_read;
	ulong fsivop_write;
	ulong fsivop_lookup;
	ulong fsivop_create;
	ulong fsivop_readdir;
	ulong fsivop_getpage;
	ulong fsivop_putpage;
	ulong fsivop_other;
} fsinfo_t;

extern struct minfo minfo;

struct vminfo {
	ulong	v_pgrec;
	ulong	v_xsfrec;
	ulong	v_xifrec;
	ulong	v_pgin;
	ulong	v_pgpgin;
	ulong	v_pgout;
	ulong	v_pgpgout;
	ulong	v_swpout;
	ulong	v_pswpout;
	ulong	v_swpin;
	ulong	v_pswpin;
	ulong	v_dfree;
	ulong	v_scan;
	ulong	v_pfault;
	ulong	v_vfault;
	ulong	v_sftlock;
};

extern struct vminfo vminfo;
extern struct syswait syswait;

struct syserr {
	long	inodeovf;
	long	fileovf;
	long	textovf;
	long	procovf;
};

extern struct syserr syserr;

struct shlbinfo {
	long	shlbs;		/* Max # of libs a process can link in	*/
				/*   at one time.			*/
	long	shlblnks;	/* # of times processes that have used	*/
				/*   static shared libraries.		*/
	long	shlbovf;	/* # of processes needed more shlibs	*/
				/*   than the system imposed limit.	*/
	long	shlbatts;	/* # of times processes have attached	*/
				/*   run time libraries.		*/
};

extern struct shlbinfo shlbinfo;

struct bpbinfo {
	long	usr;		/* usr time for the co-processor	*/
	long	sys;		/* system time for the co-processor	*/
	long	idle;		/* idle time for the co-processor	*/
	long	syscall;	/* # of system calls since boot on the  */
				/*	co-processor			*/
};

extern struct bpbinfo bpbinfo[];

struct rtminfo {
	long	ev_post;		/* # of evpost operations since boot	*/
	long	ev_poll;		/* # of evpoll operations since boot	*/
	long	ev_trap;		/* # of evtrap operations since boot	*/
};
extern struct rtminfo rtminfo;

#define KMEM_NCLASS 3		/* # of KMEM request classes		*/
#define KMEM_SMALL  0		/* small KMEM request index		*/
#define KMEM_LARGE  1		/* large KMEM request index		*/
#define KMEM_OSIZE  2		/* outsize KMEM request index		*/

struct kmeminfo {
	ulong	km_mem[KMEM_NCLASS];	/* amount of memory owned by KMEM  */
	ulong	km_alloc[KMEM_NCLASS];  /* amount of memory allocated	   */
	ulong	km_fail[KMEM_NCLASS];	/* # of failed requests		   */
};
extern struct kmeminfo kmeminfo;

#endif	/* _SYS_SYSINFO_H */
