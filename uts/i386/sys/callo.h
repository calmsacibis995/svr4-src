/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_CALLO_H
#define _SYS_CALLO_H

#ident	"@(#)head.sys:sys/callo.h	11.3.4.1"
/*
 *	The callout structure is for a routine arranging
 *	to be called by the clock interrupt
 *	(clock.c) with a specified argument,
 *	in a specified amount of time.
 *	Used, for example, to time tab delays on typewriters.
 */

struct	callo
{
	int	c_time;		/* incremental time */
	int	c_id;		/* timeout id */
	caddr_t	c_arg;		/* argument to routine */
	void	(*c_func)();	/* routine */
};
extern	struct	callo	callout[];

#endif	/* _SYS_CALLO_H */
