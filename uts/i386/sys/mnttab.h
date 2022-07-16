/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_MNTTAB_H
#define _SYS_MNTTAB_H

#ident	"@(#)head.sys:sys/mnttab.h	1.2.8.1"

#define	MNTTAB	"/etc/mnttab"
#define	MNT_LINE_MAX	1024

#define	MNT_TOOLONG	1	/* entry exceeds MNT_LINE_MAX */
#define	MNT_TOOMANY	2	/* too many fields in line */
#define	MNT_TOOFEW	3	/* too few fields in line */

#define	mntnull(mp)\
	((mp)->mnt_special = (mp)->mnt_mountp =\
	 (mp)->mnt_fstype = (mp)->mnt_mntopts =\
	 (mp)->mnt_time = NULL)

#define	putmntent(fd, mp)\
	fprintf((fd), "%s\t%s\t%s\t%s\t%s\n",\
		(mp)->mnt_special ? (mp)->mnt_special : "-",\
		(mp)->mnt_mountp ? (mp)->mnt_mountp : "-",\
		(mp)->mnt_fstype ? (mp)->mnt_fstype : "-",\
		(mp)->mnt_mntopts ? (mp)->mnt_mntopts : "-",\
		(mp)->mnt_time ? (mp)->mnt_time : "-")

struct mnttab {
	char	*mnt_special;
	char	*mnt_mountp;
	char	*mnt_fstype;
	char	*mnt_mntopts;
	char	*mnt_time;
};

#ifdef __STDC__
extern int	getmntent(FILE *, struct mnttab *);
extern int	getmntany(FILE *, struct mnttab *, struct mnttab *);
#else
extern int	getmntent();
extern int	getmntany();
#endif

#endif	/* _SYS_MNTTAB_H */
