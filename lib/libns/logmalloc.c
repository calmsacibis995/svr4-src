/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libns:logmalloc.c	1.3.3.1"

#include <stdio.h>
#include "nslog.h"
#undef free
#undef malloc
#undef calloc
#undef realloc
char	*malloc();
char	*realloc();
char	*calloc();

void
xfree(p)
char	*p;
{
	LOG2(L_MALLOC,"free(0x%x)\n",p);
	fflush(Logfd);
	free(p);
	return;
}
char	*
xmalloc(size)
unsigned size;
{
	char	*ret;
	LOG2(L_MALLOC,"malloc(%d)",size);
	fflush(Logfd);
	ret = malloc(size);
	LOG2(L_MALLOC,"returns 0x%x\n",ret);
	return(ret);
}
char	*
xrealloc(p,size)
char	*p;
unsigned size;
{
	char	*ret;
	LOG3(L_MALLOC,"realloc(0x%x,%d) ",p,size);
	fflush(Logfd);
	ret = realloc(p,size);
	LOG2(L_MALLOC,"returns 0x%x\n",ret);
	return(ret);
}
char	*
xcalloc(n,size)
unsigned n,size;
{
	char	*ret;
	LOG3(L_MALLOC,"calloc(%d,%d) ",n,size);
	fflush(Logfd);
	ret = calloc(n,size);
	LOG2(L_MALLOC,"returns 0x%x\n",ret);
	return(ret);
}
