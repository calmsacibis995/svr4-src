/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/sd.h	1.2.5.1"
/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */


/*
 *	@(#) sd.h 1.3 87/06/22 
 */




/* Shared Data flags */
#define	SD_RDONLY	0x00
#define	SD_WRITE	0x01
#define	SD_CREAT	0x02
#define	SD_UNLOCK	0x04
#define	SD_NOWAIT	0x08

#if defined(_KERNEL) || defined(_KMEMUSER)

#define	SD_LOCKED	0x10
#define	SD_NTFY		0x20
#define	SD_BTWN		0x40

#define SDI_LOCKED	0x10
#define	SDI_NTFY	0x20
#define SDI_DEST	0x40
#define SDI_CLEAR	0x80


struct sd {                 /* XENIX shared data table */
	struct xnamnode *sd_xnamnode;	/* pointer to inode for segment */
	char 	     *sd_addr;	/* address in this proc's data space */
	char	     *sd_cpaddr;/* version # for local copy */
	char	     sd_flags;	/* describing state of this proc */
	struct sd    *sd_link;	/* ptr to next shared data seg for this proc */
};

extern struct sd sdtab[];	/* the XENIX shared data table itself */
#endif /* _KERNEL || KMEMUSER */

#ifndef _KERNEL
#ifdef __STDC__
extern char *sdget(char *path, int flags, ...);
int sdenter(char *addr, int flags);
int sdleave(char *addr);
int sdfree(char *addr);
int sdgetv(char *addr);
int sdwaitv(char *addr, int vnum);
#else
extern char *sdget();
int sdenter();
int sdfree();
int sdgetv();
int sdwaitv();
#endif /* __STDC__  */
#endif /* #ifndef _KERNEL */
