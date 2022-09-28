#ident	"@(#)xdir.h	1.2	92/07/24	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/xdir.h	1.1"
/* SCCSID(@(#)xdir.h	3.6	LCC);	/* Modified: 16:55:48 6/27/89 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#if defined(SYS5_3) || defined(SYS5_4)

#undef	MAXNAMLEN
#include	<sys/fs/s5dir.h>
#include	<dirent.h>
#define	direct	dirent
#define	MAXDIRLEN	MAXNAMLEN

#else	/* not (SYS5 or SYS5_4) */

#include <sys/dir.h>

#if	defined(LOCUS)
#define direct	ldirect			/* kludge for LOCUS, force direct */
#ifndef	ld_ino
#define d_ino	ld_ino			/* and his merry men to look like */
#endif	/* ld_ino */
#ifndef	ld_name
#define d_name	ld_name			/* locus struct ldirect in sys/dir.h */
#endif	/* ld_name */
#endif	/* LOCUS */

#ifndef	BERK42FILE

#define	BLKSIZ		1024

#ifndef MAXPATHLEN			/* be sure we use MAXPATHLEN from */
#define MAXPATHLEN	80 		/* sys/dir.h if it exists */
#endif

#ifndef	MAXDIRLEN
#define	MAXDIRLEN	DIRSIZ
#endif	/* !MAXDIRLEN */

/*
 * Definitions for access routines operating on directories.
 */


#if	!(defined(LOCUS))			/* dirdesc already defined */
typedef struct _dirdesc {
	int	dd_fd;
	long	dd_loc;
	long	dd_size;

	/* the union nonsense below is so we can guarantee null-termination
	*  on names in directories. The 4.2 versions already provide this
	*/

	union {
		struct direct udd_dir;
		char udd_dirchars[sizeof (struct direct)+1];
	} dd_udir;

#define	dd_dir	dd_udir.udd_dir
#define	dd_dirchars	dd_udir.udd_dirchars

	char	dd_buf[BLKSIZ];
} DIR;
#endif


/*			External Functions Declarations 		*/


extern	DIR
	*opendir();		/* Opens a directory for reading */

extern	struct direct
	*readdir();		/* Scans directory entry at a time */

extern	long
	telldir();		/* Returns current directory offset */

extern	void
	seekdir(),		/* Seeks to a new directory offset */
	closedir();		/* Closes directory */

#else	/* BERKELEY42 */
#ifndef	MAXDIRLEN
#define	MAXDIRLEN	MAXNAMLEN
#endif	/* !MAXDIRLEN */
#endif	/* BERKELEY42 */
#endif	/* not (SYS5 or SYS5_4) */
