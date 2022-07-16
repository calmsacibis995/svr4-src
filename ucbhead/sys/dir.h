/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbhead:sys/dir.h	1.1.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
 * This header file provides BSD compatibility for DIR and direct structures.
 * The fields in the BSD DIR structure are identical to to the SVR4 DIR
 * structure, except for the fact that the dd_buf field in SVR4 is not
 * statically allocated. 
 * The BSD direct structure is similar (not identical) to the dirent
 * structure. All fields of the direct structure can be obtained using
 * the information provided by dirent.
 * All routines manipulating DIR structures are compatible, only readdir
 * is not. The BSD version of this routine returns a direct structure. 
 */
#if !defined(KERNEL) && !defined (DEV_BSIZE)
#define	DEV_BSIZE	512
#endif
#define DIRBUF		1048
#define DIRBLKSIZ	DIRBUF
#define	MAXNAMLEN	255

struct	direct {
	u_long	d_ino;			/* inode number of entry */
	u_short	d_reclen;		/* length of this record */
	u_short	d_namlen;		/* length of string in d_name */
	char	*d_name;		/* name of entry */
};


/*
 * The macro DIRSIZ(dp) gives an amount of space required to represent
 * a directory entry. 
 */
#undef DIRSIZ
#define DIRSIZ(dp)  \
        ((sizeof (struct direct) - sizeof ((dp)->d_name) + \
        (strlen((dp)->d_name)+1) + 3) & ~3)


#ifndef KERNEL
/*
 * Definitions for library routines operating on directories.
 */
typedef struct _dirdesc {
	int	dd_fd;
	long	dd_loc;
	long	dd_size;
	char	*dd_buf;
} DIR;

#ifndef NULL
#define NULL 0
#endif
#if defined(__STDC__)

extern DIR              *opendir( const char * );
extern struct direct    *readdir( DIR * );
extern long             telldir( DIR * );
extern void             seekdir( DIR *, long );
extern void             rewinddir( DIR * );
extern int              closedir( DIR * );

#else

extern	DIR *opendir();
extern	struct direct *readdir();
extern	long telldir();
extern	void seekdir();
extern  void rewinddir();
extern	void closedir();

#endif

#define rewinddir(dirp)	seekdir((dirp), (long)0)

#endif
