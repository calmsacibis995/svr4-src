/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rcmds.d/format.c	1.1.3.1"

#include	<stdio.h>
#include	<string.h>
#include	<sys/types.h>

#define	MAXNUM	60
#define	MAXOUT	120
#define	TRUE	1
#define	FALSE	0

main(argc, argv)
int	argc;
char	*argv[];
{
	char	instr[BUFSIZ];
	char	outstr[MAXOUT];
	char	*tmp;
	int	num[MAXNUM];
	int	num1, num2, num3;
	int	max, upper;
	int	ind = 0, range = FALSE, more = FALSE;
	int	week = FALSE;
	char	weekout[MAXOUT];
	int	i;

	if (argc < 2) {
		printf("Usage: %s [max. number of choices]\n", argv[0]);
		exit(1);
	} else 	if((max = atoi(argv[1])) == 7)
			week = TRUE;
	if (gets(instr) == NULL) {
		printf("%s: can't get input\n", argv[0]);
		exit(1);
	}
	if (strlen(instr) == 0) {
		printf("\n");
		exit(0);
	}

	tmp = strtok(instr, " ,");
	do {
		if (strcmp(tmp, "all") && ind < max - 1)
			num[ind] = atoi(tmp);
		else {
			printf("all\n");
			exit(0);
		}
		ind++;
	} while ((tmp = strtok((char *)NULL, " ,")) != NULL);

	/*
	 * Format the number array to a shorter format by grouping consecutive
	 * numbers into range.
	 */
	upper = ind - 1;
	strcpy(outstr, "");
	ind = 0;
	num1 = num[ind];
	do {
		sprintf(outstr, "%s%d", outstr, num1);
		if (ind >= upper)
			break;
		num2 = num[++ind];
		if (num2 != num1 + 1) {
			sprintf(outstr, "%s ", outstr);
			num1 = num2;
			continue;
		}
		if (ind >= upper) {
			sprintf(outstr, "%s %d", outstr, num2);
			break;
		}
		while (ind < upper) {
			num3 = num[++ind];
			if (num3 != num2 + 1) {
				more = TRUE;
				break;
			}
			range = TRUE;
			num2 = num3;
			more = FALSE;
		}
		if (range)
			if (num2 == num3) {
				sprintf(outstr, "%s-%d", outstr, num2);
				break;
			}else{
				sprintf(outstr, "%s-%d ", outstr, num2);
				range = FALSE;
			}
		else
			sprintf(outstr, "%s %d ", outstr, num2);
		num1 = num3;
	} while (ind <= upper);
	if (! week) {
		printf("%s\n", outstr);
		exit(0);
	}
	strcpy(weekout, "");
	max = strlen(outstr);
	for (i = ind = 0; ind < max; ind++, i++) {
		switch(outstr[ind]) {
		case '0':
			weekout[i] = 'S';
			weekout[++i] = 'u';
			break;
		case '1':
			weekout[i] = 'M';
			break;
		case '2':
			weekout[i] = 'T';
			break;
		case '3':
			weekout[i] = 'W';
			break;
		case '4':
			weekout[i] = 'T';
			weekout[++i] = 'h';
			break;
		case '5':
			weekout[i] = 'F';
			break;
		case '6':
			weekout[i] = 'S';
			break;
		case ' ':
		case '-':
			weekout[i] = outstr[ind];
			break;
		}
	}
	weekout[i] = '\0';
	printf("%s\n", weekout);
}
