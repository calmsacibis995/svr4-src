/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/pfdat.h	11.1.4.1"

typedef struct pfdat {
	unsigned	pf_blkno : 24,	/* Disk block nummber.	*/
			pf_flags : 8;	/* page flags		*/
	cnt_t		pf_ndref;	/* Need ref cnt for	*/
					/* shared pg stealing	*/
	dev_t		pf_dev;		/* Disk device code.	*/
	long		pf_inumber;	/* inode number of 	*/
					/* matching file	*/
	char		pf_swpi;	/* Index into swaptab.	*/
	char		pf_rawcnt;	/* Cnt of processes	*/
					/* doing raw I/O to 	*/
					/* page.		*/
	short		pf_waitcnt;	/* Number of processes	*/
					/* waiting for PG_DONE	*/
	struct pfdat	*pf_next;	/* Next free pfdat.	*/
	struct pfdat	*pf_prev;	/* Previous free pfdat.	*/
	struct pfdat	*pf_hchain;	/* Hash chain link.	*/
	ulong		pf_use;		/* dbd share use count	*/
} pfd_t;

#define	P_QUEUE		0x01	/* Page on free queue		*/
#define	P_BAD		0x02	/* Bad page (parity error, etc.)*/
#define	P_HASH		0x04	/* Page on hash queue		*/
#define P_DONE		0x08	/* I/O to read page is done	*/
#define	P_SWAP		0x10	/* Page on swap (not file).	*/

extern struct pfdat phead;
extern struct pfdat pbad;
extern struct pfdat *pfdat;
extern struct pfdat **phash;
extern struct pfdat ptfree;
extern int phashmask;
extern struct pfdat	*pfind();


#define BLKNULL		0	/* pf_blkno null value		*/

extern int	mem_lock;
extern int	memlock();
extern int	memunlock();
extern int	memlocked();

/* DBD allocation defines */
#define DBDSIZE	128
#define DBDSZSHFT	7
#define NDBDPP		32
#define NDBDPPSHFT	5
#define NPGEPDBD	32	/* Number of page entries per DBD chunk */
#define PGEPDBDSHFT	5
#define pgetodbd(X)	(((X) + NPGEPDBD-1) >> PGEPDBDSHFT)
#define pgetodbdt(X)	((X) >> PGEPDBDSHFT)
#define dbdtopge(X)	((X) << PGEPDBDSHFT)
#define dbdtop(X)	(((X) + NDBDPP-1) >> NDBDPPSHFT)
#define dbdtob(X)	((X) << DBDSZSHFT)
extern struct pfdat dbdfree;
