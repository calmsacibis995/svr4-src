/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	_SYS_SOCKMOD_H
#define	_SYS_SOCKMOD_H

#ident	"@(#)head.sys:sys/sockmod.h	1.13.4.1"

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
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#include <sys/un.h>	/* for sockaddr_un */

/* internal flags - in addition to the ones in timod.h */
#define		S_WINFO		0x01000	/* waiting for T_info to complete */
#define 	S_WRDISABLE	0x02000	/* write service queue disabled */
#define 	S_WUNBIND	0x04000	/* waiting on T_OK_ACK for 
					 * T_UNBIND_REQ
					 */
#define 	S_RBLOCKED	0x08000 /* read side is/was blocked */
#define 	S_WBLOCKED	0x10000 /* write side is/was blocked */
#define		S_WCLOSE	0x20000 /* Waiting to free the so_so, but
					 * have pending esballoc'ed msgs.
					 */

/* socket module ioctls */
#define		SIMOD 		('I'<<8)

/*
 * The following are ioctl handled specially by the socket
 * module which were not handled by timod.
 */
#define		SI_GETUDATA		(SIMOD|101)
#define		SI_SHUTDOWN		(SIMOD|102)
#define		SI_LISTEN		(SIMOD|103)
#define		SI_SETMYNAME		(SIMOD|104)
#define		SI_SETPEERNAME		(SIMOD|105)
#define		SI_GETINTRANSIT		(SIMOD|106)
#define		SI_TCL_LINK		(SIMOD|107)
#define		SI_TCL_UNLINK		(SIMOD|108)


struct si_udata {
	int	tidusize;	/* TIDU size          */
	int	addrsize;	/* address size	      */
	int	optsize;	/* options size	      */
	int	etsdusize;	/* expedited size     */
	int	servtype;	/* service type       */
	int	so_state;	/* socket states      */
	int	so_options;	/* socket options     */
};

struct _si_user {
	struct	_si_user 	*next;		/* next one 	      */
	struct	_si_user 	*prev;		/* previous one	      */
	int		  	fd;		/* file descripter    */
	int		  	ctlsize;	/* ctl buffer size    */
	char   		 	*ctlbuf;	/* ctl buffer         */
	int			family;		/* protocol family    */
	struct	si_udata	udata;		/* socket info	      */
	int			flags;
};

/*
 * Flag bits.
 */
#define		S_SIGIO		0x1	/* If set, user has SIGIO enabled */
#define		S_SIGURG	0x2	/* If set, user has SIGURG enabled */

/*
 * Used for the tortuous UNIX domain
 * naming.
 */
struct ux_dev {
	dev_t	dev;
	ino_t	ino;
};

struct ux_extaddr {
	size_t	size;				/* Size of following address */
	union	{
		struct ux_dev	tu_addr;	/* User selected address */
		int		tp_addr;	/* TP selected address */
	} addr;
};
#define		extdev		ux_extaddr.addr.tu_addr.dev
#define		extino		ux_extaddr.addr.tu_addr.ino
#define		extsize		ux_extaddr.size
#define		extaddr		ux_extaddr.addr

struct bind_ux {
	struct	sockaddr_un	name;
	struct	ux_extaddr	ux_extaddr;
};

/*
 * Doubly linked list of so_so that
 * represent a UNIX domain socket.
 */
struct so_ux {
	struct so_so *next;
	struct so_so *prev;
};

struct so_so {
	long 			flags;
	queue_t			*rdq;
	mblk_t  		*iocsave;
	struct	t_info		tp_info;
	struct	netbuf		raddr;
	struct	netbuf		laddr;
	struct	ux_extaddr	lux_dev;
	struct	ux_extaddr	rux_dev;
	int			so_error;
	mblk_t  		*oob;
	struct	so_so		*so_conn;
	mblk_t 			*consave;
	struct	si_udata	udata;
	int			so_option;
	mblk_t  		*bigmsg;
	struct	so_ux		so_ux;
	int			hasoutofband;
	mblk_t			*urg_msg;
	int			sndbuf;
	int			rcvbuf;
	int			sndlowat;
	int			rcvlowat;
	int			linger;
	int			sndtimeo;
	int			rcvtimeo;
	int			prototype;
	int			esbcnt;
};

extern struct _si_user 	*_s_checkfd();
extern struct _si_user 	*_s_open();
extern void 		 _s_aligned_copy();
extern struct netconfig	*_s_match();
extern int 	 	 _s_sosend();
extern int		 _s_soreceive();
extern int 		 _s_getudata();
extern int 		 _s_is_ok();
extern int 		 _s_do_ioctl();
extern int 		 _s_min();
extern int		 _s_max();
extern void		 _s_close();
extern int		 _s_getfamily();
extern int		 _s_uxpathlen();
extern void		 (*sigset())();

/*
 * Socket library debugging
 */
extern int		_s_sockdebug;
#define	SOCKDEBUG(S, A, B)	\
			if ((((S) && (S)->udata.so_options & SO_DEBUG)) || \
						_s_sockdebug) { \
				(void)syslog(LOG_ERR, (A), (B)); \
			}
#endif /* _SYS_SOCKMOD_H */
