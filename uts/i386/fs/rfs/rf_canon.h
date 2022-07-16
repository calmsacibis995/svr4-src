/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FS_RFS_RF_CANON_H
#define _FS_RFS_RF_CANON_H

#ident	"@(#)kern-fs:rfs/rf_canon.h	1.3"

/*
 * Macros describing the format of various structures.
 */
#define COMMV1_FMT	"llllllllll"		/* version 1 request/response */
#define MSGCOM_FMT	"llllllllllllllllll"	/* message and common headers */
#define RESPV2_FMT	"llllllllllllll"
#define REQV2_FMT	"llllllllllllllllllllllllllllllllllllllllllllll"
#define DIRENT_FMT	"llsc0"
#define FLOCK_FMT	"ssllllllll"
#define O_FLOCK_FMT	"ssllss"
#define STAT_FMT	"sssssssllll"
#define STATFS_FMT	"sllllllc6c6"
#define USTAT_FMT	"lsc6c6"
#define ATTR_FMT	"llllllllllllllllllllllllll"		/* rf_attr */
#define RFLKC_FMT	"lllllllllllllllllllllllllllll"		/* rflkc_info */
#define SYMLNK_FMT	"llllllllllllllllllllllllllc256c0"	/* symlnkarg */
#define STATVFS_FMT	"lllllllllc16llc32llllllllllllllll"
#define MKDENT_FMT	"llllllllllllllllllllllllllc0"

/*
 * String, character array, scalar character, short, integer type expansion;
 * string is worst-case estimate.
 */
#define STRXPAND	(2 * sizeof(long))
#define CXPAND(len)	(sizeof(long) + ((len) % sizeof(long) ?	\
			  sizeof(long) - (len) % sizeof(long) : 0))
#define BXPAND		(sizeof(long) - 1)
#define SXPAND		(sizeof(long) - sizeof(short))
#define IXPAND		(sizeof(long) - sizeof(int))

/*
 * Macros for heterogeneity buffer expansion.  These are used to allocate
 * streams messages and for sanity checking on responses.
 *
 * NOTE to porters:  These are necessarily machine and compiler-specific,
 * and derived after considering structure layout and padding.
 *
 * TO DO:  do this with canonical structure representations and expressions
 * like sizeof(struct candirent) - sizeof(struct dirent).  Just make sure
 * that the canonical representation allows for aligning strings.  The
 * worst case is demonstrated by
 *
 *	--------------------------------------
 *	|             'a' | 'b' 'c' 'd' '\0' |
 *	--------------------------------------
 *
 * for the native representation, because the 'a' will require an additional
 * long in the canonical representation.
 */
#define DIRENT_XP	(SXPAND + STRXPAND)
#define FLOCK_XP	(2 * SXPAND)
#define OFLOCK_XP	(4 * SXPAND)
#define STAT_XP		(6 * SXPAND)
#define STATFS_XP	(2 * CXPAND(6))
#define USTAT_XP	(SXPAND + 2 * CXPAND(6) - 2)
#define ATTR_XP		0
#define RFLKC_XP	0
#define SYMLNK_XP	(CXPAND(256) + STRXPAND)
#define STATVFS_XP	(CXPAND(16) + CXPAND(32))
#define MKDENT_XP	STRXPAND

/*
 * Generic data canonization
 */
extern int	rf_fcanon();
extern int	rf_tcanon();

/*
 * RFS header-specific data canonization
 */
extern int	rf_mcfcanon();
extern void	rf_hdrtcanon();
extern int	rf_rhfcanon();

/*
 * Directory entry canonization.
 */
extern int	rf_dentcanon();
extern int	rf_denfcanon();

#endif /* _FS_RFS_RF_CANON_H */
