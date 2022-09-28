/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)proto-cmd:not_found.c	1.3"

char STR1[] = "An attempt was made to execute the ";
char STR2[] = " program.  This program is installed as part of the ";
char STR3[] = " package which you do not have on your system.  Please install this package before attempting to run this program again.  Consult your 'Operations/System Administration Guide' for further assistance.";

#define CNT 2

main(argc, argv)
int argc;
char *argv[];
{
#ifndef CNT
	int CNT;
	for (CNT=1; CNT<=2; i++)
#endif
	char buf[5120];
	{
		strcpy (buf,"message -d \"");
		strcat (buf, STR1);
		strcat (buf, argv[0]);
		strcat (buf, STR2);
		strcat (buf, PACKAGE);
		strcat (buf, STR3);
		strcat (buf, "\" 1>&2");
		system (buf);
	}
	exit (2);
}
