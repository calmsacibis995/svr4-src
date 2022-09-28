/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:libinst/qstrdup.c	1.1.4.1"

#include <string.h>

#ifdef PRESVR4
#define NULL	0
#endif

extern int	errno;
extern void	*calloc();
extern void	progerr(), 
		quit();

#define ERR_MEMORY	"memory allocation failure, errno=%d"

char *
qstrdup(s)
char *s;
{
	register char *pt;

	pt = (char *) calloc((unsigned)(strlen(s)+1), sizeof(char));
	if(pt == NULL) {
		progerr(ERR_MEMORY, errno);
		quit(99);
	}
	(void) strcpy(pt, s);
	return(pt);
}
