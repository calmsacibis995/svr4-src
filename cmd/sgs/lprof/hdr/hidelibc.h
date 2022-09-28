/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lprof:hdr/hidelibc.h	1.3"

#ifdef __STDC__

#define	access	_access
#define	chmod	_chmod
#define	chown	_chown
#define	close	_close
#define	creat	_creat
#define	getpid	_getpid
#define	link	_link
#define	open	_open
#define	read	_read
#define	tempnam	_tempnam
#define	unlink	_unlink
#define	write	_write

#endif
