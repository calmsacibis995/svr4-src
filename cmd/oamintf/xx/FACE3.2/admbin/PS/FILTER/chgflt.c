/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FILTER/chgflt.c	1.1"

#include <stdio.h>
#include <string.h>

char line[BUFSIZ];

main (argc,argv)
int argc;
char *argv[];
{
	char *pattern;

	pattern=argv[1];
	getline(pattern);
	getform();
	exit (0);
}


getline (p)
char  *p;
{
	FILE *filter;
	int  col, i, j;
	char tline[BUFSIZ], tp[14];

	if ((filter = fopen ("/usr/spool/lp/admins/lp/filter.table", "r")) 
		== (FILE*) NULL)  {
		printf ("open failed\n");
		exit (1);
		}
	while (fgets(tline, BUFSIZ, filter) != (char *) NULL) {
		for (col=0, i=0; col<4; i++) 
			if (tline[i] == ':')
				col++; 
		strcpy (tp, p);
		for (j=0; tline[i] != ':' && tline[i]==tp[j]; i++,j++);
		if (tline[i] == ':') {
			strcpy (line, tline);
			fclose (filter);
			return;
			}
		}
	fclose (filter);
	return;
}

getform()
{
	char tname[500];
	char c;
	int  i, j, icount;

	for (i=1, icount=0; line[i] != '\0' && icount <= 6; icount++, i++) {
		for (j=0; line[i] != ':'; j++, i++)
			if (line[i] == ',') 
				tname[j] = ' ';
			else
				tname[j] = line[i];
		tname[j] = '\0';
		switch (icount) {
			case 0 : /*prt type*/
				printf ("ptype=%s\n", tname);
				break;
			case 1 : /*printers*/
				printf ("prt=%s\n", tname);
				break;
			case 2 : /*input*/
				printf ("in=%s\n", tname);
				break;
			case 3 : /*filter name */
				break;
			case 4 : /*output*/
				printf ("out=%s\n",tname);
				break;
			case 5 : /*fltr type */
				if (strcmp (tname, "slow") == 0)
					printf ("slow=Yes\n");
				else printf ("slow=No\n");
				break;
			case 6 : /*command */
				printf ("command=%s\n", tname);
				break;
			}

		}
	icount=0;
	while ( line[i] != '\0' ) {
		switch (line[i]) {
			case 'I' : /* INPUT */
				while (line[i] != '-')
					i++;
				i++;
				printf ("INPUT=%c\n", line[i]);
				break;	
			case 'O' : /* OUTPUT */
				while (line[i] != '-')
					i++;
				i++;
				printf ("OUTPUT=%c\n", line[i]);
				break;	
			case 'T' : /* TERM */
				while (line[i] != '-')
					i++;
				i++;
				printf ("TERM=%c\n", line[i]);
				break;	
			case 'C' : /* CPI, CHARSET, COPIES */
				i++;
				c = line[i];
				while (line[i] != '-')
					i++;
				i++;
				if ( c == 'P' )
					printf ("CPI=%c\n", line[i]);
				else if ( c == 'H' )
					printf ("CHARSET=%c\n", line[i]);
				else 	printf ("COPIES=%c\n", line[i]);
				break;	
			case 'L' : /* LPI, LENGTH */
				i++;
				c = line[i];
				while (line[i] != '-')
					i++;
				i++;
				if ( c == 'P' )
					printf ("LPI=%c\n", line[i]);
				else	printf ("LENGTH=%c\n", line[i]);
				break;	
			case 'W' : /* WIDTH */
				while (line[i] != '-')
					i++;
				i++;
				printf ("WIDTH=%c\n", line[i]);
				break;	
			case 'P' : /* PAGES */
				while (line[i] != '-')
					i++;
				i++;
				printf ("PAGES=%c\n", line[i]);
				break;	
			case 'F' : /* FORM */
				while (line[i] != '-')
					i++;
				i++;
				printf ("FORM=%c\n", line[i]);
				break;	
			case 'M' : /* MODES */
				icount++;
				while ( line[i] != ' ' )
					i++;
				i++;
				for (j=0; line[i] != ' '; j++, i++) 
					tname[j] = line[i];
				tname[j] = '\0';
				while ( line[i] != '-' ) 
					i++;
				i++;
				if (icount <= 6) {
					printf ("MODE%d=%s\n",icount,tname);
					printf ("opt%d=%c\n",icount,line[i]);
					}
				break;	
			default : 
				break;
			}
		while ( line[i] != ',' && line[i] != '\0' )
			i++;
		if ( line[i] == ',')
			i++;
		}
}

