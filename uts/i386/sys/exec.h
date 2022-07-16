/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_EXEC_H
#define _SYS_EXEC_H

#ident	"@(#)head.sys:sys/exec.h	1.15.4.1"

#include <sys/types.h>
#include <sys/vnode.h>
#include <sys/cred.h>
#include <sys/resource.h>
#include <sys/proc.h>


#define getexmag(x)   (x[1] << 8) + x[0]


/*
 * User argument structure for stack image management
 */

struct uarg {
	caddr_t estkstart;
	int estksize;
	u_int estkhflag;
	int stringsize;
	int argsize;
	int envsize;
	int argc;
	int envc;
	int prefixc;	/* intp argument prefix invisible to psargs */
	int prefixsize;
	caddr_t *prefixp;
	int auxsize;
	addr_t stacklow;
	caddr_t stackend;
	char **argp;
	char **envp;
	char *fname;
	int traceinval;
	caddr_t auxaddr;
	caddr_t argaddr;
	int flags;
};

typedef struct execenv {
	caddr_t ex_brkbase;
	short   ex_magic;
	vnode_t *ex_vp;
} execenv_t; 

/* flags definition */

#define   RINTP   0x1 /* A run-time interpreter is active */
#define   EMULA   0x2 /* Invoking emulator */


struct execsw {
	short *exec_magic;
	int   (*exec_func)();
	int   (*exec_core)();
};

extern int nexectype;		/* number of elements in execsw */
extern struct execsw execsw[];

typedef struct exhdmap {
	struct exhdmap	*nextmap;
	off_t		curbase;
	off_t		curoff;
	int		cureoff;
	caddr_t		bndrycasep;
	long		bndrycasesz;
	struct fbuf	*fbufp;
	int		keepcnt;
} exhdmap_t;

typedef struct exhda {
	vnode_t	*vp;
	u_long	vnsize;
	exhdmap_t	*maplist;
	int state;
	int nomap;
} exhda_t;

#define EXHDA_HADERROR	1

#if defined(__STDC__)

extern int exhd_getmap(exhda_t *, off_t, int, int, char *);
extern void exhd_release(exhda_t *);
extern int remove_proc(struct uarg *);
extern int execmap(vnode_t *, caddr_t, size_t, size_t, off_t, int);
extern void setexecenv(struct execenv *);
extern int setregs(struct uarg *);
extern int core_seg(proc_t *, vnode_t *, off_t, caddr_t, size_t, rlim_t, cred_t
*);
extern int gexec(vnode_t **, struct uarg *, int, long *);
extern caddr_t execstk_addr(int, u_int *);
extern int execpermissions(struct vnode *, struct vattr *, exhda_t *, struct uarg *);
extern int arglistsz(char **, int *, int *, int);
extern int copyarglist(int, char **, int, char **, char *, int);

#else

extern int exhd_getmap();
extern void exechd_release();
extern int execmap();
extern int remove_proc();
extern void setexecenv();
extern int setregs();
extern int core_seg();
extern int gexec();
extern caddr_t execstk_addr();
extern int execpermissions();
extern int arglistsz();
extern int copyarglist();

#endif	/* __STDC__ */

/* flags for exhd_getmap(): */
#define	EXHD_NOALIGN	0
#define EXHD_4BALIGN	1	/* align on 4 byte boundary */
#define EXHD_KEEPMAP	2	/* keep for parallel use with other maps */
				/* if not set, map will be freed
				 * automatically on next getmap
				 */
#define EXHD_COPY	4	/* Copy to the provided address */

/* the following macro is a machine dependent encapsulation of
 * postfix processing to hide the stack direction from elf.c
 * thereby making the elf.c code machine independent.
 */
#ifdef i386
#define execpoststack(ARGS, ARRAYADDR, BYTESIZE)  \
	(copyout((caddr_t)(ARRAYADDR), (ARGS)->auxaddr, BYTESIZE) ? EFAULT : \
			(((ARGS)->auxaddr += (BYTESIZE)), 0))
#else
#define execpoststack(ARGS, ARRAYADDR, BYTESIZE)  \
	(copyout((caddr_t)(ARRAYADDR), (ARGS)->stackend, BYTESIZE) ? EFAULT \
		: (((ARGS)->stackend += (BYTESIZE)), 0))
#endif

#endif /* _SYS_EXEC_H */
