/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi:cmd/scsicomm.h	1.3"

#define	TRUE		1
#define	FALSE		0
#define	NORMEXIT	0
#define	ERREXIT		-1
#define	REMOTE(x)	((x) & 2)

#define MDEVICE		"/etc/conf/cf.d/mdevice"
#define	MNTTAB		"/etc/mnttab"
#define	MAXFIELD	64
#define	MAXMAJNUMS	7
#define	MAXMINOR	255
#define	MFIELDS		9	/* number of fields in a mdevice file */
#define	NDISKPARTS	2	/* # of disk partitions per mirror partition */
#define NO_MAJOR	-1
#define TEMPNODE	"/dev/scsiXXXXXX"

extern struct scsi_edt	*readedt(int *);
extern struct drv_majors *GetDriverMajors(char *, int *);
extern int		ckmirror(char *);
extern int		rd_vtoc(char *, struct vtoc *);
extern void		error();
extern void		warning();
extern int		get_chardevice(char *, char *);
extern int		get_blockdevice(char *, char *);
extern short		scl_swap16(unsigned long);
extern long		scl_swap24(unsigned long);
extern long		scl_swap32(unsigned long);

#ifdef i386
#define LUN(x)		(x & 0x03)
#else
#define LUN(x)		(x & 0x03)
#endif /* i386 */

#ifdef i386
#define TC(x)		((x >> 2) & 0x07)
#else
#define TC(x)		((x >> 2) & 0x07)
#endif /* i386 */
