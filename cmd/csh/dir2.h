/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)csh:dir2.h	1.1.3.1"

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
 * Definitions for library routines operating on directories.
 */
 
#ifndef _KERNEL
typedef struct _dirdesc {
        int     dd_fd;                  /* file descriptor */
        long    dd_loc;             /* buf offset of entry from last readddir() */
        long    dd_size;                /* amount of valid data in buffer */
        long    dd_bsize;               /* amount of entries read at a time */
        long    dd_off;                 /* Current offset in dir (for telldir) */
        char    *dd_buf;                /* directory data buffer */
} DIR;  
 
#ifndef NULL
#define NULL 0
#endif  
extern  DIR *opendir();
extern  struct direct *readdir();
extern  long telldir();
extern  void seekdir();
#define rewinddir(dirp) seekdir((dirp), (long)0)
extern  int closedir();
#endif
