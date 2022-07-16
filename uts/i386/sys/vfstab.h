/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define	VFSTAB	"/etc/vfstab"
#define	VFS_LINE_MAX	1024
#ident	"@(#)head.sys:sys/vfstab.h	1.2.8.1"

#define	VFS_TOOLONG	1	/* entry exceeds VFS_LINE_MAX */
#define	VFS_TOOMANY	2	/* too many fields in line */
#define	VFS_TOOFEW	3	/* too few fields in line */

#define	vfsnull(vp)	((vp)->vfs_special = (vp)->vfs_fsckdev =\
			 (vp)->vfs_mountp = (vp)->vfs_fstype =\
			 (vp)->vfs_fsckpass = (vp)->vfs_automnt =\
			 (vp)->vfs_mntopts = NULL)

#define	putvfsent(fd, vp)\
	fprintf((fd), "%s\t%s\t%s\t%s\t%s\t%s\t%s\n",\
		(vp)->vfs_special ? (vp)->vfs_special : "-",\
		(vp)->vfs_fsckdev ? (vp)->vfs_fsckdev : "-",\
		(vp)->vfs_mountp ? (vp)->vfs_mountp : "-",\
		(vp)->vfs_fstype ? (vp)->vfs_fstype : "-",\
		(vp)->vfs_fsckpass ? (vp)->vfs_fsckpass : "-",\
		(vp)->vfs_automnt ? (vp)->vfs_automnt : "-",\
		(vp)->vfs_mntopts ? (vp)->vfs_mntopts : "-")

struct vfstab {
	char	*vfs_special;
	char	*vfs_fsckdev;
	char	*vfs_mountp;
	char	*vfs_fstype;
	char	*vfs_fsckpass;
	char	*vfs_automnt;
	char	*vfs_mntopts;
};

#ifdef __STDC__
extern int	getvfsent(FILE *, struct vfstab *);
extern int	getvfsspec(FILE *, struct vfstab *, char *);
extern int	getvfsfile(FILE *, struct vfstab *, char *);
extern int	getvfsany(FILE *, struct vfstab *, struct vfstab *);
#else
extern int	getvfsent();
extern int	getvfsspec();
extern int	getvfsfile();
extern int	getvfsany();
#endif
