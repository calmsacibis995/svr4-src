/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:user/putusrdefs.c	1.3.7.1"



#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <userdefs.h>
#include "messages.h"

FILE *defptr;		/* default file - fptr */

extern void errmsg();
extern char *ctime();
extern void exit();
extern time_t time();
extern int lockf();

/* putusrdef - 
 * 	changes default values in defadduser file
 */
putusrdef( defs )
struct userdefs *defs;
{
	time_t timeval;		/* time value from time */

	/* 
	 * file format is:
	 * #<tab>Default values for adduser.  Changed mm/dd/yy hh:mm:ss.
	 * defgroup=m	(m=default group id)
	 * defgname=str1	(str1=default group name)
	 * defparent=str2	(str2=default base directory)
	 * definactive=x	(x=default inactive)
	 * defexpire=y		(y=default expire)
	 */

	if((defptr = fopen( DEFFILE, "w")) == NULL) {
		errmsg( M_FAILED );
		return( EX_UPDATE );
	}

	if(lockf(fileno(defptr), F_LOCK, 0) != 0) {
		/* print error if can't lock whole of DEFFILE */
		errmsg( M_UPDATE, "created" );
		return( EX_UPDATE );
	}

	/* get time */
	timeval = time((long *) 0);

	/* write it to file */
	if( fprintf( defptr,
		"%s%s\n%s%d\n%s%s\n%s%s\n%s%s\n%s%s\n%s%d\n%s%s\n",
		FHEADER, ctime(&timeval), GIDSTR, defs->defgroup,
		GNAMSTR, defs->defgname, PARSTR, defs->defparent,
		SKLSTR, defs->defskel, SHELLSTR, defs->defshell, 
		INACTSTR, defs->definact, EXPIRESTR, defs->defexpire ) <= 0 ) {

		errmsg( M_UPDATE, "created" );
	}

	(void) lockf(fileno(defptr), F_ULOCK, 0);
	(void) fclose(defptr);

	return( EX_SUCCESS );
}
