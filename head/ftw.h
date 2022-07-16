/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:ftw.h	1.3.1.9"
/*
 *	Codes for the third argument to the user-supplied function
 *	which is passed as the second argument to ftwalk
 */

#define	FTW_F	0	/* file */
#define	FTW_D	1	/* directory */
#define	FTW_DNR	2	/* directory without read permission */
#define	FTW_NS	3	/* unknown type, stat failed */
#define FTW_SL	4	/* symbolic link */
#define	FTW_DP	6	/* directory */
#define FTW_SLN	7	/* symbolic link that points to nonexistent file */

/*
 *	Codes for the fourth argument to ftwalk.  You can specify the
 *	union of these flags.
 */

#define FTW_PHYS	01  /* use lstat instead of stat */
#define FTW_MOUNT	02  /* do not cross a mount point */
#define FTW_CHDIR	04  /* chdir to each directory before reading */
#define FTW_DEPTH	010 /* call descendents before calling the parent */

struct FTW
{
	int	quit;
	int	base;
	int	level;
};

/*
 * legal values for quit
 */

#define FTW_SKD		1
#define FTW_FOLLOW	2
#define FTW_PRUNE	4

#if defined(__STDC__)

#include <sys/types.h>
#include <sys/stat.h>
extern int ftw(const char *, int (*)(const char *, const struct stat *, int), int);
extern int _xftw(const int, const char *, int (*)(const char *, const struct stat *, int), int);
extern int nftw(const char *, int (*)(const char *, const struct stat *, int, struct FTW *), int, int);

#else

extern int ftw(), nftw();

#endif

#if !defined(_STYPES)

#define XFTWVER	2	/* version of file tree walk */

static int ftw(path, fn, depth)
const char *path;
int (*fn) ();
int depth;
{
	return(_xftw(XFTWVER, path, fn, depth));
}
#endif

/* nftw not available to non-EFT applications */

#if defined(_STYPES)
#include <errno.h>

static int nftw(path, fn, depth, flags)
const char *path;
int (*fn) ();
int depth;
int flags;
{
	return(EINVAL);
}
#endif


