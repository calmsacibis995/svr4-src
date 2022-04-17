/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)acct:acctdef.h	1.17.2.2"

#ifdef i386
#ifndef HZ
#define	HZ	100
#endif
#endif

#define PACCT		"/var/adm/pacct"
#define HOLFILE		"/etc/acct/holidays"
#define	NHOLIDAYS	20	/* max number of company holidays per year */
#define NSYS		20
#define NFILE		100	/* max number of files that acctmerg handles */

#ifdef uts
#define CSIZE 		10001
#define	MAXUSERS	5003
#define A_SSIZE 	60001     /* max num of sessions in 1 acct run */
#define A_TSIZE 	10001     /* max num of line names in 1 acct run */
#define A_USIZE 	20001     /* max num of distinct login names in 1 acct run */
#else
#define CSIZE 		5001
#define	MAXUSERS	3001
#define A_SSIZE 	6001     /* max num of sessions in 1 acct run */
#define A_TSIZE 	1001     /* max num of line names in 1 acct run */
#define A_USIZE 	2001     /* max num of distinct login names in 1 acct run */
#endif
#define TSIZE1		100	/* # distinct names, for speed only */
#define USIZE1		100

#define	MAXIGN		10
#define	UNUSED		-1
#define	FAIL		-1
#define	MAXNAME		8
#define	SUCCEED		0
#define	TRUE		1
#define	FALSE		0
#define PRIME		0
#define NONPRIME	1
#define MEANSIZE	01
#define KCOREMIN	02
#define HOGFACTOR	04
#define	SEPTIME		010
#define	CPUFACTOR	020
#define IORW		040
#define	ROOT		0
#define	ERR		(-1)
#define	OK		0
#define	NOGOOD		1
#define	VALID		0
#define	INVALID		1
#define LSZ		14 * 4	/* maximum length of a line name */
#define MAX_DEV_PATH	LSZ + 5		/* maximum size of absolute line name path */
#define MAX_SRCH_DEPTH	4	/* maximum search depth in /dev directory for line name */
#define NSZ		8	/* sizeof login name */

#define MYKIND(flag)	((flag & ACCTF) == 0)
#define SU(flag)	((flag & ASU) == ASU)
#define TOTAL(a)	(a[PRIME] + a[NONPRIME])
#define	okay(time)	((time/100>=0) && (time/100<=24) \
			&& (time%100>=0) && (time%100<60))
#define	pf(dble)	fprintf(stdout, "%8.2lf", dble)
#define	ps(s)		fprintf(stdout, "%8.8s", s)
#define	diag(string)	fprintf(stderr, "\t%s\n", string)
#define DATE_FMT	"%a %b %e %H:%M:%S %Y\n"
#define DATE_FMT1	" %H:%M:%S"
#define CBEMPTY   	(ctab[i].ct_sess == 0)
#define UBEMPTY   	(ub[i].ut_pc == 0 && ub[i].ut_cpu[0] == 0 && \
ub[i].ut_cpu[1] == 0 && ub[i].ut_kcore[0] ==0 && ub[i].ut_kcore[1] == 0)


#define EQN(s1, s2)	(strncmp(s1, s2, sizeof(s1)) == 0)
#define CPYN(s1, s2)	strncpy(s1, s2, sizeof(s1))

#define SECSINDAY	86400L
#define SECS(tics)	((double) tics)/HZ
#define MINS(secs)	((double) secs)/60
#define MINT(tics)	((double) tics)/(60*HZ)

#ifdef pdp11
#define KCORE(clicks)	((double) clicks/16)
#endif
#ifdef vax
#define KCORE(clicks)	((double) clicks/2)
#endif

#ifdef i386
#define KCORE(clicks)	((double) clicks/2)
#endif

#if defined(u3b) || defined(u3b5) || defined(u3b2)
#define KCORE(clicks)	((double) clicks*2)
#endif
#if UTS
#define KCORE(clicks)   ((double) BSIZE * (clicks/1024.0))
#endif

/*
 *	total accounting (for acct period), also for day
 */

struct	tacct	{
	uid_t		ta_uid;		/* userid */
	char		ta_name[8];	/* login name */
	float		ta_cpu[2];	/* cum. cpu time, p/np (mins) */
	float		ta_kcore[2];	/* cum kcore-minutes, p/np */
	float		ta_con[2];	/* cum. connect time, p/np, mins */
	float		ta_du;		/* cum. disk usage */
	long		ta_pc;		/* count of processes */
	unsigned short	ta_sc;		/* count of login sessions */
	unsigned short	ta_dc;		/* count of disk samples */
	unsigned short	ta_fee;		/* fee for special services */
};


/*
 *	connect time record 
 */
struct ctmp {
	dev_t	ct_tty;			/* major minor */
	uid_t	ct_uid;			/* userid */
	char	ct_name[8];		/* login name */
	long	ct_con[2];		/* connect time (p/np) secs */
	time_t	ct_start;		/* session start time */
};

/*
 *	per process temporary data
 */
struct ptmp {
	uid_t	pt_uid;			/* userid */
	char	pt_name[8];		/* login name */
	long	pt_cpu[2];		/* CPU (sys+usr) P/NP time tics */
	unsigned pt_mem;		/* avg. memory size (64byte clicks) */
};	
