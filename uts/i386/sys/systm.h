/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_SYSTM_H
#define _SYS_SYSTM_H

#ident	"@(#)head.sys:sys/systm.h	11.53.4.2"
/*
 * Random set of variables used by more than one routine.
 */

#ifdef _KERNEL
extern struct vnode *rootdir;	/* pointer to vnode of root directory */
extern short cputype;		/* type of cpu = 40, 45, 70, 780, 0x3b15 */
extern clock_t lbolt;		/* time in HZ since last boot */
extern int Dstflag;     /* configurable timezone */
extern int Timezone;        /* configurable DST flag */
extern int Hz;          /* Ticks/second of the clock */


extern char runin;		/* scheduling flag */
extern char runout;		/* scheduling flag */

extern int	maxmem;		/* max available memory (clicks) */
extern int	physmem;	/* physical memory (clicks) on this CPU */
extern int	maxclick;	/* Highest physical click + 1.		*/
extern int	nswap;		/* size of swap space in blocks*/
extern dev_t	rootdev;	/* device of the root */
extern dev_t	swapdev;	/* swapping device */
extern dev_t    dumpdev;    /* dump device */
extern char	*panicstr;	/* panic string pointer */
extern int	blkacty;	/* active block devices */
extern int	pwr_cnt, pwr_act;
extern int 	(*pwr_clr[])();

extern	int	rstchown;	/* 1 ==> restrictive chown(2) semantics */

#if defined(__STDC__)
extern void iomove(caddr_t, int, int);
extern int is32b(void);
extern void wakeprocs(caddr_t, int);
extern void wakeup(caddr_t);
extern int sleep(caddr_t, int);
extern void trap_ret(void);
extern int grow(int *);
extern int timeout(void (*)(), caddr_t, long);
extern int untimeout(int);
extern int nodev(void);
extern int nulldev(void);
extern int getudev(void);
extern int bcmp(char *, char *, size_t);
extern int memlow(void);
extern int stoi(char **);
extern void numtos(u_long, char *);
extern char *strncpy(char *, char *, size_t);
extern char *strcat(char *, char *);
extern char *strncat(char *, char *, size_t);
extern int strcmp(char *, char *);
extern int strncmp(char *, char *, size_t);
extern int ucopyin(caddr_t, caddr_t, size_t, u_int);
extern int copyin(caddr_t, caddr_t, size_t);
extern int ucopyout(caddr_t, caddr_t, size_t, u_int);
extern int copyout(caddr_t, caddr_t, size_t);
extern int copyinstr(char *, char *, size_t, size_t *);
extern int copystr(char *, char *, size_t, size_t *);
extern void bcopy(caddr_t, caddr_t, size_t);
extern void ovbcopy(char *, char *, size_t);
extern void fbcopy(int *, int *, size_t);
extern void bzero(caddr_t, size_t);
extern void bzeroa(caddr_t, size_t);
extern void bzeroba(caddr_t, size_t);
extern int kzero(caddr_t, size_t);
extern int upath(caddr_t, caddr_t, size_t);
extern int spath(caddr_t, caddr_t, size_t);
extern int fubyte(caddr_t);
extern int lfubyte(caddr_t);
extern int fuword(int *);
extern int lfuword(int *);
extern int subyte(caddr_t, char);
extern int suword(int *, int);
extern int setjmp(label_t *);
extern void longjmp(label_t *);
extern void xrele(struct vnode *);
extern int arglistsz(caddr_t *, int *, int *, int);
extern int copyarglist(int, caddr_t *, int, caddr_t *, caddr_t, int);
extern int userstrlen(caddr_t);
#else
extern void iomove();
extern int is32b();
extern void wakeprocs();
extern void wakeup();
extern int sleep();
extern void trap_ret();
extern int grow();
extern int timeout();
extern int untimeout();
extern int nodev();
extern int nulldev();
extern int getudev();
extern int bcmp();
extern int memlow();
extern int stoi();
extern void numtos();
extern char *strncpy();
extern char *strcat();
extern char *strncat();
extern int strcmp();
extern int strncmp();
extern int ucopyin();
extern int copyin();
extern int ucopyout();
extern int copyout();
extern int copyinstr();
extern int copystr();
extern void bcopy();
extern void ovbcopy();
extern void fbcopy();
extern void bzero();
extern void bzeroa();
extern void bzeroba();
extern int kzero();
extern int upath();
extern int spath();
extern int fubyte();
extern int lfubyte();
extern int fuword();
extern int lfuword();
extern int subyte();
extern int suword();
extern int setjmp();
extern void longjmp();
extern void xrele();
extern int arglistsz();
extern int copyarglist();
extern int userstrlen();

#endif

/*
 * Arguments to wakeprocs() to specify preemptive vs.
 * non-preemptive wakeup
 */
#define	NOPRMPT	0
#define	PRMPT	1

extern struct proc *old_curproc;    /* previous curproc */
extern struct proc *oldproc;        /* previous proc that exited */

#endif /* _KERNEL */

/*
 * Structure of the system-entry table.
 */
extern struct sysent {
	char	sy_narg;		/* total number of arguments */
	char	sy_flags;		/* various flags as defined below */
	int	(*sy_call)();		/* handler */
} sysent[];

extern unsigned	sysentsize;

/* Various flags in sy_flags */

#define	SETJUMP	0x01			/* when set, indicate that systrap */
					/* should do a setjmp()		   */
#define	ASYNC	0x02			/* async extension is allowed	   */
#define IOSYS	0x04			/* an I/O system call, the first   */
					/* arg. passed is file descriptor  */

/*
 * Structure of the return-value parameter passed by reference to
 * system entries.
 */
union rval {
	struct	{
		int	r_v1;
		int	r_v2;
	} r_v;
	off_t	r_off;
	time_t	r_time;
};
#define r_val1	r_v.r_v1
#define r_val2	r_v.r_v2
	
typedef union rval rval_t;

extern int Dstflag;
extern int Timezone;

#ifdef	KPERF

/*	This is the structure for the  kernel performance		*/ 
/*	measurement code.                                      		*/
/*                                                      		*/
#define NUMRC 512
#define NUMPHASE 64
#define PFCHAR 10

#define KPFCHILDSLP 35
#define KPFTRON    36
#define KPFTRON2    37
#define KPFTROFF    38

/*	The following structure describes the records written
**	by the kernel performance measurement code.
**
**	Not all fields of the structure have meaningful values for
**	records types.
*/
typedef struct kernperf {
	unsigned char	kp_type;	/* the record type as defined below */
	unsigned char	kp_level;	/* A priority level.		*/
	pid_t 		kp_pid;		/* A process id.	*/
	clock_t 	kp_time;	/* A relative time in 10 	*/
					/* microseconds units		*/
	unsigned long	kp_pc;		/* A pc (kernel address).	*/
} kernperf_t;

/* the possible record types are as follows.
*/

#define KPT_SYSCALL	0	/* System call - pc determines which	*/
				/* one.					*/
#define KPT_INTR	1	/* An interrupt - pc determines which 	*/
				/* one.					*/
#define KPT_TRAP_RET	2	/* Return from trap to user level	*/

#define KPT_INT_KRET	3	/* Return from interrupt to kernel	*/
				/* level.				*/
#define	KPT_INT_URET	4	/* Return from interrupt to user level	*/

#define	KPT_SLEEP	5	/* Call to "sleep" - pc is caller. The  */
				/* pid is that of the caller		*/
#define	KPT_WAKEUP	6	/* Call of "wakeup" - pc is caller. The	*/
				/* pid is that of process being		*/
				/* awakened.				*/ 
#define	KPT_PSWTCH	7	/* Process switch.  The pid is the new	*/
				/* process about to be run		*/
#define	KPT_SPL		8	/* Change of priority level.  The pc is	*/
				/* that of the caller.  The level is 	*/
				/* the new priority level.		*/
#define	KPT_CSERVE	9	/* Call of a streams service procedure.	*/
				/* the pc tells which one.		*/
#define	KPT_RSERVE	10	/* Return from a streams service 	*/
				/* procedure.  the pc tells which one.	*/
#define KPT_UXMEMF	11	/* memory fault because of paging	*/
				/* or stack exception.			*/
#define KPT_SWTCH	12	/* call to swtch			*/
#define KPT_QSWTCH	13	/* call to qswtch			*/
#define KPT_STKBX	14	/* stack boundary exceptions		*/
#define KPT_END		15	/* end of trace				*/
#define KPT_IDLE	16	/* in scheduler sitting idle		*/
#define KPT_PREEMPT	17	/* hit a preemption point		*/
				/* however preemption did not occur	*/
#define KPT_P_QSWTCH	18	/* reached a preemption point, and will */
				/* Qswtch				*/
#define KPT_LAST	19	/* last record of a proc                */

#define swtch() \
{\
	if (kpftraceflg) {\
		asm(" MOVAW 0(%pc),Kpc"); \
		kperf_write(KPT_SWTCH,Kpc,curproc); \
	} \
	if (kpftraceflg && exitflg) {\
		kperf_write(KPT_LAST,Kpc,curproc); \
		exitflg = 0; \
	} \
	KPswtch(); \
}
extern int kpchildslp;
extern int pre_trace;
extern int kpftraceflg;
extern int takephase;
extern int putphase;
extern int outbuf;
/* extern int out_of_tbuf; */
extern int numrc;
extern int numrccount;
extern int Kpc;
extern int KPF_opsw;
extern kernperf_t kpft[];
extern int exitflg;

#else

extern void swtch();

#endif	/* KPERF */

#endif	/* _SYS_SYSTM_H */
