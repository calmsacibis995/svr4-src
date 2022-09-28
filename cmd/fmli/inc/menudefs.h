/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 *
 */
#ident	"@(#)fmli:inc/menudefs.h	1.3"

#define MENU_UNDEFINED	(-1)
#define MENU_MRK	(1)
#define MENU_INACT	(2)

struct menu_line {
	char	*highlight;
	char	*lininfo;
	char	*description;
	short	flags;
};
