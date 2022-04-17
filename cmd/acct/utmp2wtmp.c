/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)acct:utmp2wtmp.c	1.2.3.1"
/*
 *	create entries for users who are still logged on when accounting
 *	is being run. Look at utmp, and update the time stamp. New info
 *	goes to wtmp. Call by runacct. 
 */

#include <stdio.h>
#include <sys/types.h>
#include <utmp.h>

long time();

main()
{
	struct utmp *getutent(), *utmp;
	FILE *fp;

	fp = fopen(WTMP_FILE, "a+");
	while ((utmp=getutent()) != NULL) {
		if (utmp->ut_type == USER_PROCESS) {
			time( &utmp->ut_time );
			fwrite( utmp, sizeof(*utmp), 1, fp);
		}
	}
	fclose(fp);
}
