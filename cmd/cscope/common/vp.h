/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/vp.h	1.1"
/*
 *	VPATH assumptions:
 *		VPATH is the environment variable containing the view path 
 *		where each path name is followed by ':', '\n', or '\0'.
 *		Embedded blanks are considered part of the path.
 */

#define MAXPATH	200		/* max length for entire name */

#if !NOMALLOC
extern	char	**vpdirs;	/* directories (including current) in view path */
#else
#define	MAXDIR	25		/* same as libVP */
#define	DIRLEN	80		/* same as libVP */
extern	char	vpdirs[MAXDIR][DIRLEN + 1];
#endif
extern	int	vpndirs;	/* number of directories in view path */

void	vpinit();
