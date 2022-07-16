/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_IDTAB_H
#define _SYS_IDTAB_H

#ident	"@(#)head.sys:sys/idtab.h	11.7.3.1"
/*
 *
 *    defines for uid/gid translation.
 *
 */
#define MAXSNAME	20
#define CFREE		0
#define CINUSE		1
#define CINTER		2
#define GLOBAL_CH	'.'	/* name of the "global" table	*/
#define UID_DEV		0	/* minor device number for uid device	*/
#define	GID_DEV		1	/* minor device number for gid device	*/
#define UID_MAP		UID_DEV
#define GID_MAP		GID_DEV

struct idtab	{
	uid_t		i_rem;
	uid_t		i_loc;
};
#define i_defval i_rem
#define i_tblsiz i_loc

struct idhead {
	uid_t		i_default;
	uid_t		i_size;
	unsigned long	i_cend;
	unsigned long	i_next;
	unsigned long	i_tries;
	unsigned long	i_hits;
};
#define HEADSIZE \
    ((sizeof(struct idhead) + sizeof(struct idtab) -1) / sizeof(struct idtab))
#ifdef _KERNEL
extern char	rfheap[];
extern int	rfsize;

#define	gluid(a,b)	glid(UID_DEV,a,b)
#define glgid(a,b)	glid(GID_DEV,a,b)
#endif

#endif	/* _SYS_IDTAB_H */
