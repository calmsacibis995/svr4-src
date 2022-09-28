/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)xtract:xtract.c	1.1"

#include <stdio.h>
#include <signal.h>

	/*
	 * xtract: extract a file from a cpio archive and stop.
	 */

main(argc, argv)
int argc;
char *argv[];
{
	int	rv = 0;
	char	cmd[1024],
		result[1024];
	FILE	*fdes,
		*popen();

	(void)fclose(stderr);
	(void)setpgrp();
	if (argc != 4)
		{
		(void)printf("USAGE: xtract cpio_options pattern archive\n");
		exit(1);
		}
	(void)sprintf(cmd, "cpio -iv%s \"%s\" < %s", argv[1], argv[2], argv[3]);
	if ((fdes = popen(cmd, "r")) == (FILE *)NULL)
		{
		(void)printf("xtract: could not popen() cpio\n");
		exit(1);
		}
	if ((fgets(result, 1024, fdes)) != NULL)
		{
		if (strlen(result))
			result[strlen(result) - 1] = '\0';
		if (!gmatch(result, argv[2]))
			{
			(void)printf("xtract: error - %s not found\n", argv[2]);
			rv = 1;
			}
		else
			(void)printf("xtract: done\n");
		}
	else 
		{
		(void)printf("xtract: error - %s not found\n", argv[2]);
		rv = 1;
		}
	(void)signal(SIGTERM, SIG_IGN);
	(void)kill(0, 15);
	(void)pclose(fdes);
	exit(rv);
}
