/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ifndef	MEMDUP_H
#define	MEMDUP_H
/*==================================================================*/
/*
**
*/
#ident	"@(#)lp:include/memdup.h	1.2.2.1"

#ifdef	__STDC__
void	*memdup (void *, int);
#else
void	*memdup ();
#endif
/*==================================================================*/
#endif
