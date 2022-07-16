/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_SWAP_H
#define _SYS_SWAP_H

#ident	"@(#)head.sys:sys/swap.h	11.16.2.1"

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

/* The following are for the swapctl system call */

#define	SC_ADD		1	/* add a specified resource for swapping */
#define	SC_LIST		2	/* list all the swapping resources */
#define	SC_REMOVE	3	/* remove the specified swapping resource */
#define SC_GETNSWP	4	/* get number of swapping resources configued */

typedef struct swapres {
	char	*sr_name;	/* pathname of the resource specified */
	off_t	sr_start;	/* starting offset of the swapping resource */
	off_t 	sr_length;	/* length of the swap area */
} swapres_t;

typedef struct swapent {
	char 	*ste_path;	/* get the name of the swap file */
	off_t	ste_start;	/* starting block for swapping */
	off_t	ste_length;	/* length of swap area */
	long	ste_pages;	/* numbers of pages for swapping */
	long	ste_free;	/* numbers of ste_pages free */
	long	ste_flags;	/* see below */
} swapent_t;

/* ste_flags values */

#define	ST_INDEL	0x01		/* this file is in the process */
					/* of being deleted. Don't     */
					/* allocate from it. This can  */
					/* be turned of by swapadd'ing */
					/* this device again.          */
#define	ST_DOINGDEL	0x02		/* this file is in the process */
					/* of being deleted. Fail      */
					/* another deletion attempt    */
					/* if this flag is on.         */
#define	ST_DELETED	0x04		/* this file has been deleted  */
					/* but the data structures have*/
					/* not been freed up yet.      */

typedef struct	swaptable {
	int	swt_n;			/*number of swapents following */
	struct	swapent	swt_ent[1];	/* array of swt_n swapents */
} swaptbl_t;


/* obsolete sys3b system call structures */

typedef struct swapint {
	char	si_cmd;		/* One of the command codes	*/
				/* listed below.		*/
	char	*si_buf;	/* For an SI_LIST function, this*/
				/* is a pointer to a buffer of	*/
				/* sizeof(swpt_t)*MSFILES bytes.*/
				/* For the other cases, it is a	*/
				/* pointer to a pathname of a	*/
				/* swap file.			*/
	int	si_swplo;	/* The first block number of the*/
				/* swap file.  Used only for	*/
				/* SI_ADD and SI_DEL.		*/
	int	si_nblks;	/* The size of the swap file in	*/
				/* blocks.  Used only for an	*/
				/* SI_ADD request.		*/
} swpi_t;

/*	The following are the possible values for si_cmd. */

#define	SI_LIST		0	/* List the currently active	*/
				/* swap files.			*/
#define	SI_ADD		1	/* Add a new swap file.		*/
#define	SI_DEL		2	/* Delete one of the currently	*/
				/* active swap files.		*/


/*
 * The following structure contains the data describing a swap file
 * that is returned by the SI_LIST command.  
 */

typedef struct swaptab {
	o_dev_t		st_dev;		/* The swap device.		*/
	short		st_flags;	/* Flags defined below.		*/
	use_t		st_ucnt;	/* in use flag for compatibility*/
	int		pad1;		/* use_t	*st_next;	*/
	int		st_swplo;	/* First block number on device	*/
					/* to be used for swapping.	*/
	int		st_npgs;	/* Number of pages of swap	*/
					/* space on device.		*/
	int		st_nfpgs;	/* Nbr of free pages on device.	*/
	int		pad2;		/* struct vnode	*st_vp;		*/
	int		pad3;		/* Round size to 2^n.		*/
} swpt_t;

#ifdef _KERNEL
/*
 * VM - virtual swap device.
 */

struct	swapinfo {
	struct	vnode *si_vp;		/* vnode for this swap device */
	struct	vnode *si_svp;		/* svnode for this swap device */
	uint	si_soff;		/* starting offset (bytes) of file */
	uint	si_eoff;		/* ending offset (bytes) of file */
	struct	anon *si_anon;		/* pointer to anon array */
	struct	anon *si_eanon;		/* pointer to end of anon array */
	struct	anon *si_free;		/* anon free list for this vp */
	int	si_allocs;		/* # of conseq. allocs from this area */
	struct	swapinfo *si_next;	/* next swap area */
	short	si_flags;		/* flags defined below */
	ulong	si_npgs;		/* number of pages of swap space */
	ulong	si_nfpgs;		/* number of free pages of swap space */
	char 	*si_pname;		/* swap file name */
};

#define	IS_SWAPVP(vp)	(((vp)->v_flag & VISSWAP) != 0)

#if defined(__STDC__)
extern int swapfunc(swpi_t *);
extern int swap_init(struct vnode *);
extern void swap_free(struct anon *);
extern void swap_xlate(struct anon *, struct vnode **, uint *);
extern struct anon *swap_alloc(void);
extern struct anon *swap_anon(struct vnode *, uint);
#else
extern int swapfunc();
extern int swap_init();
extern void swap_free();
extern void swap_xlate();
extern struct anon *swap_alloc();
extern struct anon *swap_anon();
#endif	/* __STDC__ */

#endif	/* _KERNEL */

#endif	/* _SYS_SWAP_H */
