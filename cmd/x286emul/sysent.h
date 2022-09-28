/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)x286emul:sysent.h	1.1"

/* the syscalls table specifies how the arguments and return value
 * of each system call are to be processed.
 */

#define NARGMAX 6       /* max number of system call args */
#define NSYSENT 128     /* number of system call entries */

/* 286 system call argument types */
#define INT	1
#define UINT	2
#define PTR	3	/* data pointer, sized according to program's model */
#define LONG	4
#define ZERO	5
#define SPECIAL 6
#define TPTR	7	/* text pointer like PTR above */
#define LPTR	8	/* always a long data pointer, regardless of model */
#define	STACKFRAME 9	/* pointer to stack frame containing saved 286 regs */
#define	RDPTR	10	/* Data pointer, sized according to model,
			 * but converted to selector and offset rather than
			 * 386 address */

/* modifiers */
#define REG	0x20

/* masks */
#define TYPE(x)	((x)&0x1f)

/*  At system call entry, the stack looks like this:
 *
 *  (one word per line)
 *
 *        ...
 *        --          Dword cs because of 386 call gate
 *        cs
 *        ip (hi)     Dword ip because of 386 call gate
 *        ip (lo)
 *        flags
 *        bp
 *        ds
 *        es
 *	(four words if small model)
 *	  offset on 286 stack of system call arguments
 *        n       system call number
 */

#define N	0
#define ARGOFF	1
#define ES	(Ldata?2:6)
#define DS	(Ldata?3:7)
#define BP	(Ldata?4:8)
#define FLAGS   (Ldata?5:9)
#define IP	(Ldata?6:10)
#define IPHI	(Ldata?7:11)
#define CS	(Ldata?8:12)

#define CARRY   1               /* carry bit in flags word */

struct sysent {
	int (*routine)();       /* system call processing routine */
	char types[NARGMAX+1];  /* return value types */
};

extern struct sysent Sysent[];	/* system call table */
extern struct sysent Xsysent[];	/* cxenix system call table */
extern struct sysent Psysent[];

void SetupSysent();		/* initializes Sysent */

/*
 * errno_map maps System V.3 errno values into System V.0.
 */
extern unsigned char errno_map[];
