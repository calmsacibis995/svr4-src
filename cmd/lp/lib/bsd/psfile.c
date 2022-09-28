/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/bsd/psfile.c	1.1.2.1"

#include <fcntl.h>

#define	PSCOM	"%!"

#if defined(__STDC__)
psfile(char * fname)
#else
psfile(fname)
char	*fname;
#endif
{
	int		fd;
	register int	ret = 0;
	char		buf[sizeof(PSCOM)-1];

	if ((fd = open(fname, O_RDONLY)) >= 0 &&
    	    read(fd, buf, sizeof(buf)) == sizeof(buf) &&
    	    strncmp(buf, PSCOM, sizeof(buf)) == 0)
			ret++;
	(void)close(fd);
	return(ret);
}
