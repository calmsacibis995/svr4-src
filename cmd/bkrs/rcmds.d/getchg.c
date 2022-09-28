/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rcmds.d/getchg.c	1.1.3.1"

#include <stdio.h>
#include <string.h>

main(argc, argv)
int	argc;
char	*argv[];
{
	char	task[BUFSIZ], month[40], date[100], day[30], hour[80], min[200], week[30];
	char	instr[BUFSIZ];
	char	*remain, *gettok(), tmp[80], *comtosp();
	FILE	*fp, *fpin;
	int	BACKUP = 0, i, j;

	fpin = fopen(argv[3], "r");
	i = atoi(argv[1]);
	while (i--)
		fgets(instr, BUFSIZ, fpin);
	fclose(fpin);
	fpin = fopen(argv[3], "w");
	fprintf(fpin, "%s", instr);
	fclose(fpin);

	remain = gettok(instr, min);
	remain = gettok(remain, hour);
	remain = gettok(remain, date);
	remain = gettok(remain, month);
	remain = gettok(remain, week);

	while (*remain == ' ' || *remain == '\t') remain++;

	if (argc == 4) {
		BACKUP = 1;
		while ((remain = gettok(remain, tmp)) != NULL)
			if (strcmp(tmp, "-p") == 0) {
				strcpy(task, "Incremental System Backup");
				break;
			} else if (strcmp(tmp, "-c") == 0) {
				strcpy(task, "System Backup");
				break;
			}
	} else
		strcpy(task, remain);

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

	if (! BACKUP) {
		fp = fopen(argv[4], "w");
		fprintf(fp, "%s", task);
		fclose(fp);
		printf("Month%s Day%s WeekN%s Hour%s Min%s WeekD%s\n", month, date, day, hour, min, week);

	} else
		printf("Task%s Month%s Day%s WeekN%s Hour%s Min%s WeekD%s\n", task, month, date, day, hour, min, week);
	fp = fopen(argv[2], "w");
	fprintf(fp, "Month=%s\nDay=%s\nWeek=%s\nHour=%s\nMinute=%s\n", comtosp(month), comtosp(date), comtosp(day), comtosp(hour), comtosp(min));
	fclose(fp);
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
comtosp(str)
char	*str;
{
	char	*out, *i;

	out = str;
	for (i = str; *i != '\0'; i++)
		if (*i == ',')
			*i = ' ';
	return(out);
}
