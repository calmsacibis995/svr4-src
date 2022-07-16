/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_FILE_H
#define _SYS_FILE_H

#ident	"@(#)head.sys:sys/file.h	11.28.5.4"
/*
 * One file structure is allocated for each open/creat/pipe call.
 * Main use is to hold the read/write pointer associated with
 * each open file.
 */
typedef struct file {
	struct file  *f_next;		/* pointer to next entry */
	struct file  *f_prev;		/* pointer to previous entry */
	ushort	f_flag;
	cnt_t	f_count;		/* reference count */
	struct vnode *f_vnode;		/* pointer to vnode structure */
	off_t	f_offset;		/* read/write character pointer */
	struct	cred *f_cred;		/* credentials of user who opened it */
	struct	aioreq *f_aiof;		/* aio file list forward link	*/
	struct	aioreq *f_aiob;		/* aio file list backward link	*/
	union {
		off_t f_off;
/* XENIX Support */
		struct	file *f_slnk;	/* XENIX semaphore queue */
/* End XENIX Support */
	} f_un;
} file_t;

#define f_offset	f_un.f_off	/* read/write character pointer */

/* flags */

#define	FOPEN		0xFFFFFFFF
#define	FREAD		0x01
#define	FWRITE		0x02
#define	FNDELAY		0x04
#define	FAPPEND		0x08
#define	FSYNC		0x10

/*
 * The new flag is added for raw disk async I/O feature.
 */
#define	FRAIOSIG	0x20	/* cause a signal for a completed RAIO request */ 

#define	FNONBLOCK	0x80

#define	FMASK		0xFF	/* should be disjoint from FASYNC */

/* open-only modes */

#define FCREAT      0x0100
#define FTRUNC      0x0200
#define FEXCL       0x0400
#define FNOCTTY     0x0800

/* Internal flag used by SVR4 async i/o feature */
#define FASYNC      0x1000

/* file descriptor flags */
#define FCLOSEXEC	001	/* close on exec */

/* miscellaneous defines */

#define NULLFP ((struct file *)0)

#ifndef L_SET
#define	L_SET	0	/* for lseek */
#endif /* L_SET */

/*
 * Count of number of entries in file list.
 */
extern unsigned int filecnt;

/*
 * Routines dealing with user per-open file flags and
 * user open files.  
 */

#if defined(__STDC__)
extern int getf(int, file_t **);
extern void closeall(int);
extern int closef(file_t *);
extern int ufalloc(int, int *);
extern int falloc(struct vnode *, int, file_t **, int *);
extern void finit(void);
extern void unfalloc(file_t *);
extern void setf(int, file_t *);
extern char getpof(int);
extern void setpof(int, char);
extern int filesearch(struct vnode *);
extern int fassign(struct vnode **, int, int*);

#else

extern int getf();
extern void closeall();
extern int closef();
extern int ufalloc();
extern int falloc();
extern void finit();
extern void unfalloc();
extern void setf();
extern char getpof();
extern void setpof();
extern int filesearch();
extern int fassign();

#endif	/* __STDC__ */

#endif	/* _SYS_FILE_H */
