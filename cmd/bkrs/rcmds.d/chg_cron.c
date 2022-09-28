/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rcmds.d/chg_cron.c	1.1.3.1"

#include <stdio.h>
#include <string.h>

#define	READ	0
#define	WRITE	1

main(argc, argv)
int	argc;
char	*argv[];
{
	int	p[2];
	FILE	*taskfp, *cronfp, *fopen();
	int	pid, w, status;
	char	newcron[BUFSIZ], task[BUFSIZ], newtask[BUFSIZ], oldtask[BUFSIZ];
	char	month[40], date[100], day[30];
	char	minute[180], hour[80];

	if (strcmp(argv[2], "all"))
		strcpy(month, argv[2]);
	else
		strcpy(month, "*");

	if (strcmp(argv[3], "all"))
		strcpy(date, argv[3]);
	else
		strcpy(date, "*");

	if (strcmp(argv[4], "all"))
		strcpy(day, argv[4]);
	else
		strcpy(day, "*");

	if (strcmp(argv[5], "all"))
		strcpy(hour, argv[5]);
	else
		strcpy(hour, "*");

	if (strcmp(argv[6], "all"))
		strcpy(minute, argv[6]);
	else
		strcpy(minute, "*");

	taskfp = fopen(argv[7], "r");
	fgets(oldtask, BUFSIZ, taskfp);
	fclose(taskfp);
	unlink(argv[7]);

	if (strcmp(argv[1], "System Backup") == 0) {
		sprintf(newtask, "%s %s %s %s %s echo \'\\n\' | /usr/bin/backup -t -c -d /dev/rmt/c0s0\n", minute, hour, date, month, day);
	} else if (strcmp(argv[1], "Incremental System Backup") == 0) {
		sprintf(newtask, "%s %s %s %s %s echo \'\\n\' | /usr/bin/backup -t -p -d /dev/rmt/c0s0\n", minute, hour, date, month, day);
	     } else {
		taskfp = fopen(argv[1], "r");
		fgets(task, BUFSIZ, taskfp);
		fclose(taskfp);
		unlink(argv[1]);
		sprintf(newtask, "%s %s %s %s %s %s", minute, hour, date, month, day, task);
	     }

	if (pipe(p) < 0) {
		printf("Can't open pipe.\n");
		exit(1);
	}
	if (fork() == 0) {
		close(p[READ]);
		close(1); dup(p[WRITE]);
		close(p[WRITE]);
		execlp("crontab", "crontab", "-l", (char *) 0);
		exit(1);
	}
	close(p[WRITE]);
	close(0); dup(p[READ]);
	close(p[READ]);
	sprintf(newcron, "/tmp/tmpcron.%d", getpid());
	cronfp = fopen(newcron, "w");
	while (fgets(task, BUFSIZ, stdin) != NULL) {
		if (task[0] == '#')
			continue;
		if (strncmp(oldtask, task, strlen(task) - 1) == 0)
			strcpy(task, newtask);
		fputs(task, cronfp);
	}
	fclose(cronfp);
	if ((pid = fork()) == 0) {
		execlp("crontab", "crontab", newcron, (char *) 0);
		exit(1);
	}
	while ((w = wait(&status)) != pid && w != -1)
		;
	unlink(newcron);
}
