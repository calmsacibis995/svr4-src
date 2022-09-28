/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:usermgmt/moddate.c	1.1.1.1"
#include <sys/types.h>
#include <time.h>

main(argc, argv)
char *argv[];
int argc;
{
time_t tstamp;
	struct tm *tmp, *localtime();
	tstamp = (atoi(argv[1])*24L*60*60) + ((24L*60*60)/2);
	tmp = localtime(&tstamp);
	printf("%d/%d/%d", tmp->tm_mon+1, tmp->tm_mday, tmp->tm_year);
}
