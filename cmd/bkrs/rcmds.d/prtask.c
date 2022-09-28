/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rcmds.d/prtask.c	1.1.3.1"

/* Usage:	prtask [-b|-t] [-s|-l] [-d|-c|-f|-p] outfile [taskfile]
		1st arg: -b schedule automatic backup
			 -t task scheduling
		2nd arg: -s standard input
			 -l from "crontab -l"
		3rd arg: -d output delete menu format for task scheduling
			 -f output delete confirmation format for task scheduling
			 -c output change menu format for task scheduling
			 -p output display text frame format for task scheduling
			 -D output delete menu format for backup
			 -F output delete confirmation format for backup
			 -C output change menu format for backup
			 -A output scheduled backup format for add
		4th arg: the output file name
		5th arg: output file for crontab entry, for change and delete only
 */

#include <stdio.h>
#include <string.h>

#define	truncat(s, n)	(strlen(s) > n ? (s[n] = '\0', s) : s)

#define	TRUE	1
#define	FALSE	0
#define	READ	0
#define	WRITE	1

main(argc, argv)
int	argc;
char	*argv[];
{
	char	month[40], date[100], day[30], hour[80];
	char	min[200], week[30], task[BUFSIZ], type;
	char	instr[BUFSIZ];
	char	*slashchar(), *gettok(), *remain;
	int	i, j, p[2];
	int	seq = 0;
	FILE	*fp, *fptask;

	if (argv[2][1] == 'l') {
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
	}

	fp = fopen(argv[4], "w");
	if (argv[3][1] == 'd' || argv[3][1] == 'c' || argv[3][1] == 'D' || argv[3][1]== 'C')
		fptask = fopen(argv[5], "w");

	while(gets(instr) != NULL) {
		if (instr[0] == '#')
			continue;
		if ((remain = gettok(instr, min)) == NULL)
			continue;
		if ((remain = gettok(remain, hour)) == NULL)
			continue;
		if ((remain = gettok(remain, date)) == NULL)
			continue;
		if ((remain = gettok(remain, month)) == NULL)
			continue;
		if ((remain = gettok(remain, week)) == NULL)
			continue;

		if (argv[1][1] == 'b' && strncmp(remain, "echo \'\\n\' | /usr/bin/backup", 27) != 0)
			continue;
		if (argv[1][1] == 'b')
			while ((remain = gettok(remain, task)) != NULL)
				if (strcmp(task, "-p") == 0) {
					type = 'I';
					break;
				} else {
					if (strcmp(task, "-c") == 0) {
						type = ' ';
						break;
					}
				}
		else
			strcpy(task, remain);
		seq++;

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

		if (strcmp(hour, "*") == 0)
			strcpy(hour, "all");

		if (strcmp(min, "*") == 0)
			strcpy(min, "all");

		switch(argv[3][1]) {
		case 'D':
			fprintf(fp, "name=\"%c  Month:%-11s  Date:%-23s  Day:%-7s  %s:%s\"\n", type, truncat(month, 11), truncat(date, 23), truncat(day, 7), hour, min);
			fprintf(fp, "lininfo=%d\n", seq);
			fprintf(fp, "itemmsg=const \"Mark items to delete with the MARK function key, and strike the ENTER key.\"\n\n");
			fprintf(fptask, "%d %s\n", seq, instr);
			break;

		case 'C':
			fprintf(fp, "name=\"%c  Month:%-11s  Date:%-23s  Day:%-7s  %s:%s\"\n", type, truncat(month, 11), truncat(date, 23), truncat(day, 7), hour, min);
			fprintf(fp, "lininfo=%d\n", seq);
			fprintf(fp, "action=open $OBJ_DIR/sched/Form.chg \"$LININFO\" %s\n", argv[5]);
			fprintf(fp, "itemmsg=const \"Move to an item with the arrow keys and strike ENTER to select.\"\n\n");
			fprintf(fptask, "%s\n", instr);
			break;

		case 'A':
			fprintf(fp, "%c  Month:%-11s  Date:%-23s  Day:%-7s  %s:%s\n", type, truncat(month, 11), truncat(date, 23), truncat(day, 7), hour, min);
			break;
	
		case 'F':
			fprintf(fp, "\n\tType of Backup: %s\n\tMonth(s): %s\n\tDate(s): %s\n\tWeekday(s): %s\n\tTime: %s:%s\n", (type == 'I' ? "Incremental System Backup" : "System Backup"), month, date, day, hour, min);
			break;
		case 'd':
			fprintf(fp, "name=Task %2d: %s\n", seq, slashchar(task, 66));
			fprintf(fp, "lininfo=%d\n", seq);
			fprintf(fp, "itemmsg=const \"Mark items to delete with the MARK function key, and strike the ENTER key.\"\n\n");
			fprintf(fptask, "%d %s\n", seq, instr);
			break;

		case 'c':
			fprintf(fp, "name=Task %2d: %s\n", seq, slashchar(task, 66));
			fprintf(fp, "lininfo=%d\n", seq);
			fprintf(fp, "action=open $OBJ_DIR/Form.chg \"$LININFO\" %s\n", argv[5]);
			fprintf(fp, "itemmsg=const \"Move to an item with the arrow keys and strike ENTER to select.\"\n\n");
			fprintf(fptask, "%s\n", instr);
			break;

		case 'p':
			fprintf(fp, "\n Task %2d: %s\n Month(s): %s\n Date(s): %s\n Weekday(s): %s\n Hour(s): %s\n Minutes: %s\n", seq, truncat(task, 65), month, truncat(date, 65), day, hour, truncat(min, 65));
			break;

		case 'f':
			fprintf(fp, "\n   Task %2d: %s\n   Month(s): %s\n   Date(s): %s\n   Weekday(s): %s\n   Hour(s): %s\n   Minutes: %s\n", seq, slashchar(task, BUFSIZ), month, date, day, hour, min);
			break;
		}
	}
	fclose(fp);
	if (argv[3][1] == 'd' || argv[3][1] == 'c')
		fclose(fptask);
	if (seq) {
		if (argv[1][1] == 'b') {
			chmod(argv[4], 066);
			printf("2\n");
		} else 
			printf("1\n");
	} else {
		unlink(argv[4]);
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

char *
slashchar(in, n)
char	*in;
int	n;
{
	
	static	char	out[BUFSIZ], *i, *j;

	if (strlen(in) > n)
		in[n] = '\0';
	for (i = in, j = out; *i != '\0'; i++, j++)
		switch(*i) {
		case '\"':
		case '\\':
		case '\'':
			*j = '\\';
			*++j = *i;
			break;
		default:
			*j = *i;
			break;
		}
	*j = '\0';
	return(out);
}
