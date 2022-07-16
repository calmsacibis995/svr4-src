/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_SESSION_H
#define _SYS_SESSION_H

#ident	"@(#)head.sys:sys/session.h	1.17.3.1"
typedef struct sess {

	short s_ref; 			/* reference count */
	short s_mode;			/* /sess current permissions */
	uid_t s_uid;			/* /sess current user ID */
	gid_t s_gid;			/* /sess current group ID */
	ulong s_ctime;			/* /sess change time */
	dev_t s_dev;			/* tty's device number */
	struct vnode *s_vp;		/* tty's vnode */
	struct pid *s_sidp;		/* session ID info */
	struct cred *s_cred;		/* allocation credentials */

} sess_t;

#define s_sid s_sidp->pid_id

extern sess_t session0;

/*
 * Enumeration of the types of access that can be requested for a 
 * controlling terminal under job control.
 */

enum jcaccess {
	JCREAD,			/* read data on a ctty */
	JCWRITE,		/* write data to a ctty */
	JCSETP,			/* set ctty parameters */
	JCGETP			/* get ctty parameters */
};

#define SESS_HOLD(sp)	(++(sp)->s_ref)
#define SESS_RELE(sp)	(--(sp)->s_ref > 0 ? 0 : sess_rele(sp))

#if defined(__STDC__)

extern int sess_rele(sess_t *);
extern void sess_create(void);
extern void freectty(sess_t *);
extern void alloctty(proc_t *, vnode_t *);
extern int realloctty(proc_t *, pid_t);
extern dev_t cttydev(proc_t *);
extern int hascttyperm(sess_t *, cred_t *, mode_t);

#else

extern int sess_rele();
extern void sess_create();
extern void freectty();
extern void alloctty();
extern int realloctty();
extern dev_t cttydev();
extern int hascttyperm();

#endif

#endif
