/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)boot:boot/sys/boot.h	1.1.5.1"

#include "sys/types.h"
#include "sys/bootinfo.h"
#include "sys/immu.h"

/* definitions for generic AT(386) hard/floppy disk boot code */

#define	MAX_BSIZE		2048	/* size of secondary block (bytes) */

#define SBUFSIZE	1024		/* System buffer size */

#define ROOTINO ((ino_t) 2) /* i-number of all roots */

/* common macros */

#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) < (b) ? (b) : (a))

#ifdef DEBUG
#define debug(x)	x
#else
#define debug(x)	/*x/**/
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#define NULL	0

/* 0 == old bootinfo, 1 == new bootinfo */

#define BKINAME		('B' + ('K' << 8) + ('I' << 16))
#define BKIVERSION	2		

/* size of bootstrap reserved memory */

#define BOOTSIZE 	(40*1024)

#define ELFMAGIC	0x457f
/* INTERVAL(b, e, p) returns true if p lies in the range defined by b and e */

#define INTERVAL( b, e, p)	( e > 0 ? ((p >= b) && ( p < b + e)) : \
					  ((p < b) && ( p >= b + e)) )

/* externals from disk driver */

extern	int	unix_start;
extern	int	dev_gran;
extern	int	spt;
extern	int	spc;

/* externals from the low-level bootstrap */

/* the address of this is the address of the start of the GDT */
extern	int	GDTstart; 

/* /etc/default/boot parameters */

extern int		autoboot;
extern int		timeout;
extern char		defbootstr[];
extern char		bootmsg[];
extern char		bootprompt[];
extern char		initprog[];
extern struct bootmem	memrng[];
extern int		memrngcnt;
extern char		mreqmsg1[];
extern char		mreqmsg2[];
extern int		memreq;

extern short bnami();
extern short biget();
extern long  breadi();
extern unsigned long  bload();

extern char *strtok();
extern char *strncpy();
extern unsigned long atol();

extern paddr_t	physaddr();

extern struct bootinfo binfo;

/* Assembly definitions */

#ifdef	WINI
#define BOOTDRIVE	0x80			/* hard disk boot drive */
#else
#define BOOTDRIVE	0x0			/* floppy disk boot drive */
#endif

#define PROTMASK	0x1 			/* MSW protection bit */
#define NOPROTMASK	0xfffffffe

#define BOOTIND		0			/* offset of boot indicator */
#define RELSECT		8			/* offset of relative starting sector */

#define DISKBUF		0x8000		/* offset of disk buffer */
#define STACK		0x9ffe		/* initial stack location */

#define GDTSIZE		0x40			/* size of the GDT */

#define HDBIOS_NHEAD	2
#define HDBIOS_SPT	14

#define HDBIOS_NCYL	0
#define HDBIOS_PRECOMP	5
#define HDBIOS_LZ	12

#if defined(MB1) || defined(MB2)
/* the following is allow some area of RAM used by the monitor, kernel
 * page directory table, page tables, and we don't want to load the 
 * kernel there.
 */
#define	RESERVED_SIZE	0x11000L
#endif

#ifdef AT386
enum	kbd_ck_val	{ IGNORE, ENFORCE };
typedef enum kbd_ck_val kbd_ck_val_t;
#endif
