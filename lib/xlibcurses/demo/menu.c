/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:demo/menu.c	1.1"
/*
				menu
				====

	This program allows the user to quickly build a menu of
	selectable options. A file of options may be specified,
	or if none is requested the program will look for 
	'.menu.data'.

	The input file begins with records describing the valid options
	and the commands they represent. Each line begins with an
	option. The option is followed by whitespace then the command
	to be substituted. A maximum of 50 commands may be specified.
	The end of the option records is denoted by a line containing
	only the word 'END'.

	The options records are followed by the menu to be presented to
	the user. The remaining lines found after the END record are
	displayed to the user each time the user is prompter for an 
	option.

	In addition to the options supplied by the file, 3 other 
	'built-in' options exist. If BANG is defined as 1 then the 
	user may enter !CMD and the command CMD will be executed.
	If DOLLAR is defined as 1 then entering ${FILE} causes menu to
	load its menu from FILE; if FILE is omitted, menu loads from
	'.menu.data'.

	At least one option must be specified that has the string
	EXIT defined. This is the option to terminate the program.

	The program uses libcurses, so the terminal being used should
	be known to the terminfo database, and it must be properly
	initialized.

	Comments and or suggestions, as always, are welcome.

		Author: Ernest H. Rice III
			AT&T Bell Laboratories
			SF 5-349
			190 River Road
			Summit, New Jersey
					07901

			(201) 522-6352
			btlunix!ehr3

*/
#include 	<curses.h>
#include	<stdio.h>
#include	<setjmp.h>
#include	<signal.h>
#include	<string.h>
#include	<term.h>

#define BANG 1		/* 1 says allow !CMD, 0 says dont */
#define DOLLAR 1	/* 1 says allow $FILE, 0 says dont */
#define MAXLINE 256
#define MAXOPTS	50

jmp_buf	holdenv;

int	interupt();

extern	char *ctime();
extern	char *fgets();
extern	FILE *fopen();
extern	long time();

struct	line {
	int	inuse;	/* says this line has data */
	char	data[MAXLINE];
} usrscr[MAXOPTS];

struct	table {
	int	inuse;		/* 1 says the entry in use */
	char	opt[MAXLINE];	/* Holds option value */
	char	cmd[MAXLINE];	/* holds command to be executed */
} cmdtab[MAXOPTS];

char	*cptr;
char	date[25];
int	done=0;
int	exitsw = 0;	/* 1 says EXIT found */
char	file[256];	/* Holds file name of options */
FILE	*fptr;		/* General purpose file pointer */
int	pad;
char	*program;	/* Holds program name */
char	record[MAXLINE];	/* Holds a record of input */
char	*rptr;	/* work char pointer */

main(argc, argv)
char	**argv;
{
	char	c;	/* work char */
	int	i;	/* work integer */

	/*
	 * Strip off path name of this command
	 */
	for (i=strlen(argv[0]); i>=0 && (c=argv[0][i])!='/'; --i);
        if (i>=0) argv[0]+=i+1;
	program=argv[0];

	setupterm(0,1,0);

	if (argc == 1) strcpy(file, ".menu.data");
	else {
		if (argc == 2) strcpy(file, argv[1]);
		else {
			fprintf(stderr,"%s: usage %s {FILE}\n", program, program);
			exit(1);
		}
	}

	initscr();
	load(file);
	/*
		Signals SIGINT and SIGQUIT will
		always bring us back to just past the 
		following 'setjmp' statement
	*/
	setjmp(holdenv);
	signal(SIGQUIT, interupt);	/* Always reset signals	*/
	signal(SIGINT, interupt);	/* Same goes here...    */

	done = 0;
	record[0] = '\0';
	while (!done) {
		if (record[0] == '\0') pmenu();
		getrep();
	}
	clear();
	refresh();
	resetterm();
	endwin();
}
interupt()
{
	longjmp(holdenv);
}
execute(string)
char	*string;
{
	int pid, pos,rc;

	/*
		Parent temporarily ignores signals so it 
		will remain around for command to finish
	*/
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	clear();
	standout();
	pos = ((columns-strlen(string))/2) - 1;
	if (pos < 0) pos = 0;
	mvaddstr(0,pos,string);
	printw("\n\n");
	standend();
	refresh();
	resetterm();
	if ((pid = fork()) == 0) {
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		execl("/bin/sh", "sh", "-c", string, NULL);
	}

	if (pid != -1) {
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
	}

	while ((rc = wait(0)) != pid && rc != -1) ;
	printf("\n");
	fixterm();
	standout();
	mvprintw(lines-1,0,"%s: Press RETURN to continue ", program);
	refresh();
	standend();
	flushinp();
}
load(fname)
char	*fname;
{
	int	i, j, k, rc;

	exitsw = 0;
	resetterm();
	/*
		reset tables in case we are reloading
	*/
	for (i=0; i < MAXOPTS; i++) {
		cmdtab[i].inuse = 0;
		usrscr[i].inuse = 0;
	}
	/*
		open file of options, moan and exit if you cannot
	*/
	if ((fptr = fopen(file, "r")) == NULL) {
		fprintf(stderr,"%s: Cannot open file '%s' for reading\n", program, file);
		endwin();
		exit(2);
	}
	/*
		Load up cmdtab with options and commands
	*/
	rptr = record;
	for (j=0;(cptr = fgets(rptr, MAXLINE, fptr)) != NULL && (rc = strncmp(rptr, "END",3)) != 0; j++) {
		cmdtab[j].inuse = 1;
		for (i=0; record[i] != ' ' && record[i] != '\t' && record[i] != '\0'; ++i) cmdtab[j].opt[i] = record[i];
		cmdtab[j].opt[i] = '\0';
		/* 
			We have the option, now get the command
		*/
		for (; record[i] == ' ' || record[i] == '\t'; ++i);
		if (record[i] == '\0') {
			fprintf(stderr, "%s: Error in line No. %d of input file '%s', NO COMMAND\n", program, j+1, file);
			endwin();
			exit(3);
		}
		for (k=0; record[i] != '\n' && record[i] != '\0'; ++i) cmdtab[j].cmd[k++] = record[i];
		cmdtab[j].cmd[k] = '\0';

		if ((k = strcmp(cmdtab[j].cmd, "EXIT")) == 0) exitsw = 1;
	}
	for (j=0; (cptr = fgets(rptr, MAXLINE, fptr)) != NULL; j++) {
		usrscr[j].inuse = 1;
		strcpy(usrscr[j].data, record);
	}
	if (j == 0) {
		fprintf(stderr, "%s: Error, at least one line of screen data must be present\n", program);
		endwin();
		exit(4);
	}
	if (!exitsw) {
		fprintf(stderr, "%s: Error, at least one option must do 'EXIT'\n", program);
		endwin();
		exit(5);
	}
	pad = (lines - j - 1)/2;
	fclose(fptr);
	fixterm();
}
pmenu()
{
	long	*lptr, lval;
	char	*cptr;
	int	j, k;

	clear();
	lptr = &lval;
	time(lptr);
	cptr = ctime(lptr);
	strncpy(date,cptr,24);
	date[25] = '\0';
	standout();
	mvprintw(0,0,"File: %s", file);
	mvprintw(0,columns-26,"%s", date);
	standend();
	k=pad;
	for (j=0; usrscr[j].inuse == 1; j++) mvaddstr(k++, 0, usrscr[j].data);
	mvaddstr(k,0, "Enter option:");
	refresh();
}
getrep()
{
	int	found = 0, j, rc, rtrn = 0;

	rptr = record;
	resetterm();
	if ((cptr = gets(rptr)) != NULL) {
		fixterm();
		move(lines-1,0);
		clrtoeol();
		refresh();
		if (record[0] == '\0') rtrn = 1;
		if (BANG && record[0] == '!') {
			found = 1;
			if (record[1] != '\0') execute(++rptr);
			else execute("sh");
			return;
		}
		if (DOLLAR && record[0] == '$') {
			found = 1;
			if ((rc = strlen(rptr)) > 1) strcpy(file, ++rptr);
			else strcpy(file, ".menu.data");
			load(file);
			mvprintw(lines-1,0,"%s: %s loaded --- Press RETURN to continue", program,file);
			refresh();
			return;
		}
		for (j=0; cmdtab[j].inuse == 1; j++) {
			if ((rc = strcmp(cmdtab[j].opt, record)) == 0) {
				found = 1;
				if ((rc = strcmp(cmdtab[j].cmd, "EXIT")) != 0) execute(cmdtab[j].cmd);
				else done = 1;
			}
		}
		if (!done && !found && !rtrn) {
			flash();
			standout();
			mvprintw(lines-1,0,"%s: Invalid Option. Press RETURN to continue ", program);
			refresh();
			flushinp();
			standend();
		}
	}
	else done = 1;
}
