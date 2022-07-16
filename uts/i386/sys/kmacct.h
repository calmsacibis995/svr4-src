/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/kmacct.h	1.1.8.1"
/*
 * Kernel Memory Allocator accounting package
 */

/*
 * Command types for KMACCT ioctl()
 */

#define	KMACCT_ON	1	/* Turn KMEM accounting on */
#define	KMACCT_OFF	2	/* Turn KMEM accounting off */
#define	KMACCT_ZERO	3	/* Zero the counters */
#define	KMACCT_SIZE	4	/* Return size of accounting data */
#define	KMACCT_NDICT	5	/* Return number of dictionary entries */
#define	KMACCT_DEPTH	6	/* Return depth of trace stack */
#define	KMACCT_STATE	7	/* Return state of accounting (on or off) */

/*
 * The kernel text start address and kernel text length parameters should
 * agree with the kernel text origin and length values given in the
 * kernel ifile.  These are used to detect when we traced back out of
 * the kernel stack.
 */

#define	KTXTSTRT	KVSBASE
#define	KTXTLEN		0x00400000

/*
 * The number of different sizes to track should agree with the 
 * number of sizes allocatable by KMEM; in 4.0 this includes all
 * sizes in powers of 2 from 16 to 4096, plus outsize requests
 * greater than 4096 bytes.
 */

#define	KMSIZES		10

/*
 * MAXDEPTH is the maximum amount of stack that we can trace back through.
 * The master.d tunable must be no greater than MAXDEPTH.
 */

#define	MAXDEPTH	10	

/*
 * Calling sequence symbol table structure.
 * Note: if this structure changes, so must master.d/kmacct.  Also, *next
 * pointer must be first item in structure so hash chain will work.
 */

struct	kmasym {
	struct kmasym	*next;		/* Next structure on hash chain */
	caddr_t		*pc;		/* Pointer to pc calling sequence */
	ulong		reqa[KMSIZES];	/* Requests to allocate in each class */
	ulong		reqf[KMSIZES];	/* Requests to free in each class */
	};

typedef	struct kmasym	kmasym_t;

/*
 * Header for allocated buffers.  Each buffer that has been allocated while
 * accounting is enabled has a buffer header associated with it.
 */

struct	kmabuf	{
	struct	kmabuf	*next;	/* Next header in hash chain or free list */
	struct	kmabuf	*prev;	/* Previous header in hash chain */
	caddr_t		*addr;	/* Address of this buffer */
	kmasym_t	*kp;	/* Pointer to  dictionary entry */
	};

typedef	struct kmabuf	kmabuf_t;

/*
 * Calls to kmaccount() must identify whether they are allocating
 * or freeing space.
 */

#define	KMACCT_ALLOC	0x0100
#define	KMACCT_FREE	0x0200
