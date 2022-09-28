/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rcmds.d/display.c	1.1.3.1"

#include <stdio.h>
#include <string.h>

#define	truncat(s, n)	(strlen(s) >= n ? (s[n] = '\0', s) : s)

#define	TRUE	1
#define	FALSE	0
#define	READ	0
#define	WRITE	1
#define	STRLEN	256
#define ARGLEN	60

main(argc, argv)
int	argc;
char	*argv[];
{
	char	type, month[ARGLEN], date[ARGLEN], day[ARGLEN], hour[ARGLEN];
	char	min[ARGLEN], tmp[ARGLEN], week[ARGLEN], task[ARGLEN];
	char	instr[STRLEN];
	char	*gettok(), *remain;
	int	exist = FALSE, i, j, p[2];
	FILE	*fp;

	if (pipe(p) < 0) {
		printf("Can't open pipe.\n");
		exit(1);
	}
	if (fork() == 0) {
		close(p[READ]);
		close(1); dup(p[WRITE]);
		close(p[WRITE]);
		setuid(0);
		execlp("crontab", "crontab", "-l", (char *) 0);
		exit(1);
	}
	close(p[WRITE]);
	close(0); dup(p[READ]);
	close(p[READ]);

	fp = fopen(argv[1], "w");

	while(gets(instr) != NULL) {
		if (instr[0] == '#')
			continue;
		remain = gettok(instr, min);
		remain = gettok(remain, hour);
		remain = gettok(remain, date);
		remain = gettok(remain, month);
		remain = gettok(remain, week);

		if (strncmp(remain, "echo \'\\n\' | /usr/bin/backup", 27) == 0) {
			exist = TRUE;
			while ((remain = gettok(remain, tmp)) != NULL)
				if (strcmp(tmp, "-p") == 0) {
					type = 'I';
					break;
				} else if (strcmp(tmp, "-c") == 0) {
					type = ' ';
					break;
				}

			if (strcmp(week, "*") == 0) 
				strcpy(day, "all");
			else {
				for (i = j = 0; week[j] != '\0'; j++, i++)
					switch(week[j]) {
					case '0':
						day[i] = 'S';
						day[++i] = 'u';
						break;
					case '1':
						day[i] = 'M';
						break;
					case '2':
						day[i] = 'T';
						break;
					case '3':
						day[i] = 'W';
						break;
					case '4':
						day[i] = 'T';
						day[++i] = 'h';
						break;
					case '5':
						day[i] = 'F';
						break;
					case '6':
						day[i] = 'S';
						break;
					default:
						day[i] = week[j];
						break;
					}
				day[i] = '\0';
			}

			if (strcmp(month, "*") == 0)
				strcpy(month, "all");
	
			if (strcmp(date, "*") == 0)
				strcpy(date, "all");
		 
			fprintf(fp, "%c  Month:%-11s  Date:%-23s  Day:%-7s  %s:%s\n", type, truncat(month, 11), truncat(date, 23), truncat(day, 7), hour, min);
		}
	}
	fclose(fp);
	if (exist) {
		chmod(argv[1], 0666);
		printf("1\n");
	} else {
		unlink(argv[1]);
		printf("0\n");
	}
}

char *
gettok(in, out)
char	*in, *out;
{
	char	*i, *j;

	if ((i = in) == NULL) return((char *)NULL);
	while(*i == ' ' || *i == '\t') i++;
	for (j = out; *i != ' ' && *i != '\0' && *i != '\t'; i++, j++)
		*j = *i;
	*j = '\0';
	if (*i == '\0')
		return((char *)NULL);
	else
		return(++i);
}
