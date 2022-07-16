/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libnsl:nsl/_import.h	1.4.2.1"

#ifndef NO_IMPORT
#define free		(*__free)
#define calloc 		(*__calloc)
#define perror		(*__perror)
#define strlen		(*__strlen)
#define write		(*__write)
#define ioctl		(*__ioctl)
#define getmsg		(*__getmsg)
#define putmsg		(*__putmsg)
#define getpmsg		(*__getpmsg)
#define putpmsg		(*__putpmsg)
#define errno		(*__errno)
#define memcpy		(*__memcpy)
#define fcntl		(*__fcntl)
#define sigset		(*__sigset)
#define open		(*__open)
#define close		(*__close)
#define ulimit		(*__ulimit)
#else
#endif
