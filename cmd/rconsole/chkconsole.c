/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rconsole:chkconsole.c	1.1"

/* chkconsole is called out of inittab as the first user process.  It
 * checks if the console nodes and makes sure /dev/sysmsg is link of /dev/console.
 */

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define CONSOLE	"/dev/console"
#define SYSMSG	"/dev/sysmsg"

char *cmd;

main(argc, argv)
int	argc;
char	**argv;
{
	int remade;
	struct stat console_buf, sysmsg_buf;

	cmd = argv[0];

	if (argc != 1)
	{
		fprintf(stderr, "usage: %s\n", cmd);
		exit(1);
	}

	if (stat(CONSOLE, &console_buf) == -1)
	{
		fprintf(stderr, "%s: can't stat %s, errno = %d\n", cmd, CONSOLE, errno);
		console_buf.st_rdev = -1;
	} else
	if ((console_buf.st_mode & S_IFMT) != S_IFCHR)
	{
		fprintf(stderr, "%s: %s not a character special file\n", cmd, CONSOLE);
		console_buf.st_rdev = -1;
	}

	umask(0);
	remade = 0;

	/* ensure SYSMSG is linked to CONSOLE (same device number is enough) */
	if (stat(SYSMSG, &sysmsg_buf) == -1)
	{
		remade = 1;
		fprintf(stderr, "%s: can't stat %s, errno = %d\n", cmd, SYSMSG, errno);
	}

	errno = 0;
	if (remade || sysmsg_buf.st_rdev != console_buf.st_rdev)
	{
		if (unlink(SYSMSG) == -1)
		   if (errno != ENOENT)
		{
			fprintf(stderr, "%s: can't unlink %s, errno = %d\n", cmd, SYSMSG, errno);
			exit(1);
		}
		if (link(CONSOLE, SYSMSG) == -1)
		{
			fprintf(stderr, "%s: can't link %s, errno = %d\n", cmd, SYSMSG, errno);
			exit(1);
		}
	}

	exit(0);
}
