/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rcmds.d/setquote.c	1.1.2.1"
/*                       
 *		setquote.c
 *		Purpose is to setup the quotes for the
 *		restore command.
 */
#include <string.h>
#include <stdio.h>
#define CMDLENGTH	BUFSIZ /* max command line length */

main(argc, argv)
int argc;
char **argv;
{
	register optchar;
	extern char *optarg; 		
	char usercmd[CMDLENGTH]; 	/* command line from the user */
	int cflg=0, opterr=0;
	char *ptr;

	while ((optchar = getopt(argc, argv, "c:" )) != EOF)
		switch(optchar)
		{
		case 'c':
			/* check for multiple uses of -c. Only one is permissible */
			if(cflg) {  
			  opterr++; 
			  break;
			}
			cflg++;
			/* change : to " */
			while((ptr=strchr(optarg,':')) != NULL ) *ptr = '"';
			strcpy(usercmd,optarg);
			continue;
		case '?':
			opterr++;
			break;
		}


	if(opterr) 
	{
		fprintf(stderr, "Usage: setquote [-c \"command :arg1: :arg2: ...:argn:\"]\n");
		exit(1);
	}


	exit(system(usercmd));
}
