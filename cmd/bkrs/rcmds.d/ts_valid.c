/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rcmds.d/ts_valid.c	1.1.3.1"

#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>

#define	TRUE	1
#define	FALSE	0

char	*itoweek[] = { "Su", "M", "T", "W", "Th", "F", "S" };
char	*itoa[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
	"10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
	"20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
	"30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
	"40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
	"50", "51", "52", "53", "54", "55", "56", "57", "58", "59" };

main(argc, argv)
int	argc;
char	*argv[];
{
	int	innum[60];
	char	instr[BUFSIZ], outstr[BUFSIZ];
	char	*tmp;
	int	num, num1, num2;
	int	upper, lower;
	int	i = 0, ind = 0, week = FALSE;
	char	weekout[BUFSIZ];

	if (argc < 4) {
		printf("Usage: %s lower upper input \n", argv[0]);
		exit(1);
	}
	
	lower = atoi(argv[1]);
	upper = atoi(argv[2]);

	if (upper == 6) {
		week = TRUE;
		if (weektoi(instr, argv[3]) == 1) {
			printf("E%s\n", instr);
			exit(0);
		}
	} else
		strcpy(instr, argv[3]);

	if (strcmp(instr, "all") == 0) {
		printf("all\n");
		exit(0);
	}

	while (instr[ind] == ' ') ind++;

	if (instr[ind] == '\0') {
		if (upper == 12)
			printf("Nul\n");
		else
			printf("all\n"); 
		exit(0);
	}

	for (num = 0; num <= upper; num++)
		innum[num] = 0;

	do { 
		if (! isdigit(instr[ind])) {
			tmp = strtok(&instr[ind], " ,");
			if (strcmp(tmp, "all") == 0)
				printf("all\n");
			else
				printf("E%s\n", tmp);
			exit(0);
		}
		num1 = 0;
		do {
			num1 = num1 * 10 + (instr[ind] - '0');
			outstr[i++] = instr[ind];
		} while (isdigit(instr[++ind]));
		if ((num1 < lower) || (num1 > upper)) {
			printf("E%d\n", num1);
			exit(0);
		}

		if (innum[num1]) {
			printf("R%s\n", week ? itoweek[num1] : itoa[num1]);
			exit(0);
		} else
			innum[num1] = 1;

		if (instr[ind] == '-') {
			if (!isdigit(instr[++ind])) {
				if (instr[ind] == '\0') {
					printf("E-\n");
					exit(0);
				}
				tmp = strtok(&instr[ind], " ,");
				if (strcmp(tmp, "all") == 0)
					printf("all\n");
				else
					printf("E%s\n", tmp);
				exit(0);
			}
			outstr[i++] = '-';
			num2 = 0;
			do {
				num2 = num2 * 10 + (instr[ind] - '0');
				outstr[i++] = instr[ind];
			} while (isdigit(instr[++ind]));
			if ((num2 < lower) || (num2 > upper)) {
				printf("E%d\n", num2);
				exit(0);
			}
			for (num = num1 + 1; num <= num2; num++)
				if (innum[num]) {
					printf("R%s\n", week ? itoweek[num] : itoa[num]);
					exit(0);
				} else
					innum[num] = 1;
		}
		outstr[i++] = ',';
		while (instr[ind] == ' ' || instr[ind] == '\n' || instr[ind] == ',')
			ind++;
	} while (instr[ind] != '\0');
	outstr[--i] = '\0';

	if (! week) {
		printf("%s\n", outstr);
		exit(0);
	}
	for (i = ind = 0; outstr[ind] != '\0'; ind++, i++) {
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
		case ',':
		case '-':
			weekout[i] = outstr[ind];
			break;
		}
	}
	weekout[i] = '\0';
	printf("N%s D%s\n", weekout, outstr);
}

weektoi(out, in)
char	*out, *in;
{
	int	ind;
	char	*i, *j, tmp[60];

	j = out;
	i = in;

	do {
		while (*i == ' ' || *i == '-')
			i++;
		for (ind = 0; *i != ' ' && *i != '-' && *i != '\0'; i++, ind++)
			tmp[ind] = *i;
		tmp[ind] = '\0';

		if (strcmp(tmp, "") == 0)
			continue;
		if (strcmp(tmp, "all") == 0) {
			strcpy(out, "all");
			return(0);
		}
		if (strcmp(tmp, "Su") == 0) {
			*j++ = '0';
			*j++ = *i;
			continue;
		}
		if (strcmp(tmp, "M") == 0) {
			*j++ = '1';
			*j++ = *i;
			continue;
		}
		if (strcmp(tmp, "T") == 0) {
			*j++ = '2';
			*j++ = *i;
			continue;
		}
		if (strcmp(tmp, "W") == 0) {
			*j++ = '3';
			*j++ = *i;
			continue;
		}
		if (strcmp(tmp, "Th") == 0) {
			*j++ = '4';
			*j++ = *i;
			continue;
		}
		if (strcmp(tmp, "F") == 0) {
			*j++ = '5';
			*j++ = *i;
			continue;
		}
		if (strcmp(tmp, "S") == 0){
			*j++ = '6';
			*j++ = *i;
			continue;
		}
		
		strcpy(out, tmp);
		return(1);
	} while (*i != '\0');
	return(0);
}
