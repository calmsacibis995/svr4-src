/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbsendmail:aux/mailstats.c	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

# include <sys/types.h>
# include <sys/stat.h>
# include <stdio.h>
# include "conf.h"
# include "mailstats.h"


/*
**  MAILSTATS -- print mail statistics.
**
**	Arguments: 
**		file		Name of statistics file.
**
**	Exit Status:
**		zero.
*/

main(argc, argv)
	char  **argv;
{
	register int fd;
	struct statistics st;
	char *sfile = "/usr/ucblib/sendmail.st";
	register int i;
	struct stat sbuf;
	extern char *ctime();

	if (argc > 1) sfile = argv[1];

	fd = open(sfile, 0);
	if (fd < 0)
	{
		perror(sfile);
		exit(1);
	}
	fstat(fd, &sbuf);
	if (read(fd, &st, sizeof st) != sizeof st ||
	    st.stat_size != sizeof st)
	{
		(void) sprintf(stderr, "File size change\n");
		exit(1);
	}

	printf("Mail statistics from %24.24s", ctime(&st.stat_itime));
	printf(" to %s\n", ctime(&sbuf.st_mtime));
	printf("  Mailer   msgs from  bytes from    msgs to    bytes to\n");
	for (i = 0; i < MAXMAILERS; i++)
	{
		if (st.stat_nf[i] == 0 && st.stat_nt[i] == 0)
			continue;
		printf("%2d %-10s", i, st.stat_names[i]);
		if (st.stat_nf[i])
		  printf("%6ld %10ldK ", st.stat_nf[i], st.stat_bf[i]);
		else
		  printf("                   ");
		if (st.stat_nt[i])
		  printf("%10ld %10ldK\n", st.stat_nt[i], st.stat_bt[i]);
		else
		  printf("\n");
	}
}
