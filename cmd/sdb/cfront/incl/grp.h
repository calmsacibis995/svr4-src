/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/grp.h	1.1"
/*ident	"@(#)cfront:incl/grp.h	1.6"*/

#ifndef GRPH
#define GRPH

#ifndef FILE
#       include <stdio.h>
#endif

struct	group {	/* see getgrent(3) */
	char	*gr_name;
	char	*gr_passwd;
	int	gr_gid;
	char	**gr_mem;
};

extern void endgrent ();
extern group *fgetgrent (FILE*);
extern group *getgrent ();
extern group *getgrgid (int);
extern group *getgrnam (const char*);
extern void setgrent ();

#endif
