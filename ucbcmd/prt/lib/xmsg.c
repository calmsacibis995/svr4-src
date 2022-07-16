/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*      Portions Copyright (c) 1988, Sun Microsystems, Inc.     */ 
/*      All Rights Reserved.                                    */ 
 
#ident	"@(#)ucbprt:lib/xmsg.c	1.1.1.1"

#include	"../hdr/defines.h"
#include	<errno.h>


/*
	Call fatal with an appropriate error message
	based on errno.  If no good message can be made up, it makes
	up a simple message.
	The second argument is a pointer to the calling functions
	name (a string); it's used in the manufactured message.
*/
int
xmsg(file,func)
char *file, *func;
{
	register char *str;
	char d[FILESIZE];
	extern int errno;
	extern char Error[];
	int	fatal();

	switch (errno) {
	case ENFILE:
		str = "no file (ut3)";
		break;
	case ENOENT:
		sprintf(str = Error,"`%s' nonexistent (ut4)",file);
		break;
	case EACCES:
		str = d;
		copy(file,str);
		file = str;
		sprintf(str = Error,"directory `%s' unwritable (ut2)",
			dname(file));
		break;
	case ENOSPC:
		str = "no space! (ut10)";
		break;
	case EFBIG:
		str = "write error (ut8)";
		break;
	default:
		sprintf(str = Error,"errno = %d, function = `%s' (ut11)",errno,
			func);
		break;
	}
	return(fatal(str));
}
