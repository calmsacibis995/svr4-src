/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H

#ident	"@(#)head.sys:sys/utsname.h	11.14.3.1"

/*
 * If you are compiling the kernel, the value used in initializing
 * the utsname structure in the master.d/kernel file better be the
 * same as SYS_NMLN.
 */
#if (defined (_POSIX_SOURCE) || defined(_XOPEN_SOURCE)) && !defined(_KERNEL)

#if !defined(_STYPES)
#define _SYS_NMLN	257	/* 4.0 size of utsname elements.*/
				/* Must be at least 257 to 	*/
				/* support Internet hostnames.  */
#else
#define _SYS_NMLN	9	/* old size of utsname elements */
#endif	/* _STYPES */


struct utsname {
	char	sysname[_SYS_NMLN];
	char	nodename[_SYS_NMLN];
	char	release[_SYS_NMLN];
	char	version[_SYS_NMLN];
	char	machine[_SYS_NMLN];
};

#else /*!defined(POSIX_SOURCE) && !defined(_XOPEN_SOURCE) || defined(_KERNEL)*/

#if !defined(_STYPES)
#define SYS_NMLN	257	/* 4.0 size of utsname elements.*/
				/* Must be at least 257 to 	*/
				/* support Internet hostnames.  */
#else
#define SYS_NMLN	9	/* old size of utsname elements */
#endif	/* _STYPES */

struct utsname {
	char	sysname[SYS_NMLN];
	char	nodename[SYS_NMLN];
	char	release[SYS_NMLN];
	char	version[SYS_NMLN];
	char	machine[SYS_NMLN];
};


#define XSYS_NMLN   9   /* size of utsname elements for XENIX */
struct xutsname {
    char    sysname[XSYS_NMLN];
    char    nodename[XSYS_NMLN];
    char    release[XSYS_NMLN];
    char    version[XSYS_NMLN];
    char    machine[XSYS_NMLN];
    char    reserved[15];
    unsigned short sysorigin;   /* original supplier of the system */
    unsigned short sysoem;      /* OEM for this system */
    long    sysserial;      /* serial number of this system */
};

extern struct xutsname xutsname;

#endif /* defined (_POSIX_SOURCE) || defined(_XOPEN_SOURCE) */

#if !defined(_POSIX_SOURCE) 
extern struct utsname utsname;
#endif /* !defined(_POSIX_SOURCE) */

#if !defined(_KERNEL)
#if defined(__STDC__)

#if !defined(_STYPES)
static int uname(struct utsname *);
#else 
int uname(struct utsname *);
#endif /* !defined(_STYPES) */

int nuname(struct utsname *);
#else

#if !defined(_STYPES)
static int uname();
#else 
int uname();
#endif /* !defined(_STYPES) */

int nuname();
#endif	/* (__STDC__) */
#endif	/* !(KERNEL) */

#if !defined(_KERNEL) && !defined(_STYPES)
static int
uname(buf)
struct utsname *buf;
{
	int ret;

	ret = nuname(buf);
	return ret;
}
#endif /* !defined(_KERNEL) && !defined(_STYPES) */

#endif	/* _SYS_UTSNAME_H */
