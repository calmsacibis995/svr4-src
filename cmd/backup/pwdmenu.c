/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)backup:pwdmenu.c	1.1"
#include <stdio.h>
#include <pwd.h>
#include <string.h>

main(argc, argv)
	int argc;
	char *argv[];
{
	struct passwd *getpwent();
	struct passwd *pwdentry;
	short flag=0;
	char name[30];

	if (argc > 1)
	    while ( (pwdentry = getpwent()) != NULL ) {
		if ((pwdentry->pw_uid > 99) && 
				strcmp("NONE",pwdentry->pw_passwd) &&
				strcmp(argv[1],pwdentry->pw_name)){
			flag=1;
			strcpy (name,"Name=");
			strcat(name,pwdentry->pw_name);
			puts(name);
		}
	    }
	else
	    while ( (pwdentry = getpwent()) != NULL ) {
		if ((pwdentry->pw_uid > 99) && 
				strcmp("NONE",pwdentry->pw_passwd)) {
			flag=1;
			strcpy (name,"Name=");
			strcat(name,pwdentry->pw_name);
			puts(name);
		}
	    }
	if (flag == 0)
	   puts("Name=N O N E");
}
