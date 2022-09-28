/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)setmnt:setmnt.c	4.15"

#include	<stdio.h>
#include	<sys/mnttab.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/statvfs.h>

#define	LINESIZ	BUFSIZ
#define	OPTSIZ	64

extern char	*fgets();
extern char	*strtok();
extern char	*strrchr();
extern char	*opts();
extern time_t	time();

char	line[LINESIZ];
char	sepstr[] = " \t\n";

char	mnttab[] = MNTTAB;

main(argc, argv)
	char	**argv;
{
	char	*lp = line;
	char	*myname;
	time_t	date;
	FILE	*fd;
	struct mnttab	mm;
	struct statvfs	sbuf;

	myname = strrchr(argv[0], '/');
	if (myname)
		myname++;
	else
		myname = argv[0];

	if (argc > 1) {
		fprintf(stderr, "Usage: %s\n", myname);
		exit(1);
	}

	umask(~(S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) & S_IAMB);
	if ((fd = fopen(mnttab, "w")) == NULL) {
		fprintf(stderr, "%s: Cannot open %s for writing\n", myname, mnttab);
		exit(1);
	}

	time(&date);
	while ((lp = fgets(line, LINESIZ, stdin)) != NULL) {
		if ((mm.mnt_special = strtok(lp, sepstr)) != NULL &&
		    (mm.mnt_mountp = strtok(NULL, sepstr)) != NULL &&
		     statvfs(mm.mnt_mountp, &sbuf) == 0) {
			fprintf(fd, "%s\t%s\t%s\t%s\t%d\n",
				mm.mnt_special,
				mm.mnt_mountp,
				sbuf.f_basetype ? sbuf.f_basetype : "-",
				opts(sbuf.f_flag),
				date);
		}
	}
	fclose(fd);

	exit(0);
}

char	*
opts(flag)
	ulong	flag;
{
	static char	mntopts[OPTSIZ];

	sprintf(mntopts, "%s,%s",
		(flag & ST_RDONLY) ? "ro" : "rw",
		(flag & ST_NOSUID) ? "nosuid" : "suid");

	return	mntopts;
}
