/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_ASYNCIO_H
#define _SYS_ASYNCIO_H

#ident	"@(#)head.sys:sys/asyncio.h	1.4.8.1"
/*
**		File Contents
**		=============
**
**	This file contains data which must be included in an 
**	application program using the Async system call facility.
*/

/*
**	Structure user used to pass async I/O request parameters to asyncio()
*/

typedef	struct aioop {
	ushort		aio_cmd;	/* one of the command codes defined below */
	ushort		aio_flags;	/* the flags defined below 		  */
	int		aio_error;	/* error code; the field is used to save  */
					/* error encountered in processing the    */
					/* asyncio() function. It is not for      */
					/* any error found in the actual I/O	  */
					/* run asynchronously because the    	  */
					/* aioop struct may be reused after       */
					/* returning from asyncio() function.	  */
	char		*aio_bufp;	/* ptr to the buffer for the operation	  */
	uint		aio_bufs;	/* size of buffer pointed to by aio_bufp  */
	int		aio_fd;		/* file descriptor on which to do I/O	  */
	off_t		aio_offset;	/* relative offset in bytes               */
	pcparms_t	aio_pri;	/* I/O priority  			  */
	ecb_t		aio_ecb;	/* ecb for event notification      	  */
} aioop_t;

/*	The possible values for the aio_cmd field 	*/

#define	AIOC_READ	0x01		/* read from the file descriptor aio_fd   */

#define	AIOC_WRITE	0x02		/* write to the file descriptor aio_fd    */

/*	The flags in the aio_flags and a_flags field are defined as follows.	  */

#define AIOF_SEEK_SET	0x01		/* This flag has the same meaning as an   */
					/* lseek with whence set to 0. An explicit*/
					/* seek will be done before an I/O request*/
					/* is queued. This flag has no effect on  */
					/* a non-I/O async call.		  */

#define AIOF_SEEK_CUR	0x02		/* This flag has the same meaning as an   */
					/* lseek with whence set to 1. An explicit*/
					/* seek will be done before an I/O request*/
					/* is queued. This flag has no effect on  */
					/* a non-I/O async call.		  */

#define AIOF_SEEK_FLAGS	(AIOF_SEEK_SET | AIOF_SEEK_CUR) 

#define	AIOF_NONBLOCK	0x04		/* This flag has the same meaning as the  */
					/* O_NONBLOCK flag for the file descriptor*/
					/* ar_ofilep. If set, it overrides the    */
					/* O_NONBLOCK setting of ar_ofilep and 	  */  
					/* will be used for this I/O. Note, the   */
					/* effect of this flag is local to this   */
					/* request only and does not apply to     */
					/* any I/Os on the same file descriptor.  */

#define	AIOF_SCHEDPRI	0x08		/* When set, indicate the request comes	  */
					/* with a priority parameter to override  */
					/* default FIFO operation. The request	  */
					/* should be processed with the specified */
					/* priority				  */

#define AIOF_UFLAGS	(AIOF_SEEK_SET|AIOF_SEEK_CUR|AIOF_NONBLOCK|AIOF_SCHEDPRI) 
					/* User defined flags			  */

#define	AIOF_ACCEPTED	0x10		/* When set, indicate the request has 	  */
					/* been accepted and will be processed by */
					/* the server.				  */

#define	AIOF_ERROR	0x20		/* When set, indicate the request has 	  */
					/* been rejected during the validity 	  */
					/* check. The actual error code will be   */
					/* set in rt_error.			  */

#define AIOF_PROCESS	(AIOF_ACCEPTED | AIOF_ERROR)

					/* If neither of AIOF_ACCEPTED nor AIOF_ERROR
					** is set, the request was not processed at all
					** because some system resources are not avail.
					*/
/*
**	The following structure describes the data
**	returned for an ET_ASYNC event.
*/
   
typedef struct evd_async {
 	long		ea_rval;	/* # of bytes read/write*/
 	int		ea_errno;	/* error code		*/
 	off_t		ea_offset;	/* initial file offest	*/
} evd_async_t;

/*
**		The Async System Call Facility Version Number
**		=============================================
**
**	In order to be able to make changes to the Async Facility in the
**	future, we define a version number which is automatically passed
**	through from the application program to the libraries and then
**	to the kernel.  If we want to change the Async interface, we
**	just increase the version number so the library routines
**	and the kernel can tell the difference.  This works with either
**	private or shared libraries.
**	The current version number is as follows.
**
*/

#define	ASYNC_VERSION	2	/* The current version number.	*/

/*	The following are previously used version numbers which are
**	still supported.
*/

/*	None yet.
*/

/*	The following are previously used version numbers which are no
**	longer supported.
*/

#define	ASYNC_VER_PROTO	1	/* Version number used in the	*/
				/* first prototype of Async I/O.*/
/*
**		The Async Facility Interfaces
**		=============================
**
**	When a user program calls one of the functions described on one of
**	the async facility manual pages, it is actually invoking one of the
**	following macros.  The purpose of the macros is to pass the
**	version number to the library routines.  The library routines
**	will, in turn, pass the version number down to the kernel.  This
**	will allow us to make changes to the Async facility in the future 
**	and still support old *.o and a.out files.
**
*/

#define	aread(fildes, bufp, bufs, ecbp)	\
	__aread(ASYNC_VERSION, fildes, bufp, bufs, ecbp)

#define	awrite(fildes, bufp, bufs, ecbp)	\
	__awrite(ASYNC_VERSION, fildes, bufp, bufs, ecbp)

#define	asyncio(asyncp, asyncs)	\
	__asyncio(ASYNC_VERSION, asyncp, asyncs)

#define	acancel(eidp, eids) \
	__acancel(ASYNC_VERSION, eidp, eids)


/*
**		Function Prototypes
**		===================
**
**	The following are prototypes for the library functions which
**	users call (indirectly using the macros defined above).
**
*/

#if defined(__STDC__)

int	__aread(const int, const int, char *, const uint, \
		ecb_t *);
int	__awrite(const int, const int, char *, const uint, \
		ecb_t *);
int	__asyncio(const int, aioop_t *, const int);
int	__acancel(const int, const long *const, const int);

#else

int	__aread();
int	__awrite();
int	__asyncio();
int	__acancel();

#endif

#endif	/* _SYS_ASYNCIO_H */
