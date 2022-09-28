/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/command.c	1.3"
/*	cscope - interactive C symbol or text cross-reference
 *
 *	command functions
 */

#include "global.h"
#include <curses.h>	/* LINES needed by constants.h */
#include <fcntl.h>	/* O_RDONLY */

BOOL	caseless;		/* ignore letter case when searching */
BOOL	*change;		/* change this line */
BOOL	changing;		/* changing text */
char	newpat[PATLEN + 1];	/* new pattern */
char	pattern[PATLEN + 1];	/* symbol or text pattern */

static	char	appendprompt[] = "Append to file: ";
static	char	pipeprompt[] = "Pipe to shell command: ";
static	char	toprompt[] = "To: ";

BOOL	changestring();
void	atchange(), editall(), editref(), help(), mark(), scrollbar();

/* execute the command */

BOOL
command(c)
register int	c;
{
	char	msg[MSGLEN + 1];	/* message */
	char	filename[PATHLEN + 1];	/* file path name */
	MOUSE	*p;			/* mouse data */
	register int	ch, i;
	register FILE	*file;
	struct cmd *curritem, *item;		/* command history */
	void	addcmd(), resetcmd();

	switch (c) {

	case ctrl('C'):	/* toggle caseless mode */
		if (caseless == NO) {
			caseless = YES;
			putmsg2("Caseless mode is now ON");
		}
		else {
			caseless = NO;
			putmsg2("Caseless mode is now OFF");
		}
		egrepcaseless(caseless);	/* turn on/off -i flag */
		return(NO);

	case ctrl('R'):	/* rebuild the cross reference */
		if (isuptodate == YES) {
			putmsg("The -d option prevents rebuilding the symbol database");
			return(NO);
		}
		move(LINES - 1, 0);	/* to lower left corner */
		clrtoeol();		/* clear bottom line */
		refresh();
		endwin();		/* restore terminal modes */
		incurses = NO;
		freefilelist();		/* remake the source file list */
		makefilelist();
		rebuild();
		if (errorsfound == YES) {
			errorsfound = NO;
			askforreturn();
		}		
		clear();
#if UNIXPC || !TERMINFO
		nonl();
		cbreak();	/* endwin() turns off cbreak mode so restore it */
		noecho();
#endif
		incurses = YES;
		mousemenu();	/* display an empty scrollbar */
		totallines = 0;
		topline = nextline = 1;
		break;

#if UNIXPC
	case ESC:	/* possible unixpc mouse selection */
#endif
	case ctrl('X'):	/* mouse selection */
		if ((p = getmouseaction(c)) == NULL) {
			return(NO);	/* unknown control sequence */
		}
		/* if the button number is a scrollbar tag */
		if (p->button == '0') {
			scrollbar(p);
			break;
		} 
		/* ignore a sweep */
		if (p->x2 >= 0) {
			return(NO);
		}
		/* if this is a line selection */
		if (p->y1 < FLDLINE) {

			/* find the selected line */
			/* note: the selection is forced into range */
			for (i = disprefs - 1; i > 0; --i) {
				if (p->y1 >= displine[i]) {
					break;
				}
			}
			/* display it in the file with the editor */
			editref(i);
		}
		else {	/* this is an input field selection */
			field = p->y1 - FLDLINE;
			
			/* force it into range */
			if (field >= FIELDS) {
				field = FIELDS - 1;
			}
			setfield();
			resetcmd();
			return(NO);
		}
		break;

	case '\t':	/* go to next input field */
	case '\n':
	case '\r':
	case ctrl('N'):
#if TERMINFO
	case KEY_DOWN:
	case KEY_ENTER:
	case KEY_RIGHT:
#endif
		field = (field + 1) % FIELDS;
		setfield();
		resetcmd();
		return(NO);

	case ctrl('P'):	/* go to previous input field */
#if TERMINFO
	case KEY_UP:
	case KEY_LEFT:
#endif
		field = (field + (FIELDS - 1)) % FIELDS;
		setfield();
		resetcmd();
		return(NO);
#if TERMINFO
	case KEY_HOME:	/* go to first input field */
		field = 0;
		setfield();
		resetcmd();
		return(NO);

	case KEY_LL:	/* go to last input field */
		field = FIELDS - 1;
		setfield();
		resetcmd();
		return(NO);
#endif
	case ' ':	/* display next page */
	case '+':
	case ctrl('V'):
#if TERMINFO
	case KEY_NPAGE:
#endif
		/* don't redisplay if there are no lines */
		if (totallines == 0) {
			return(NO);
		}
		/* note: seekline() is not used to move to the next 
		 * page because display() leaves the file pointer at
		 * the next page to optimize paging forward
		 */
		break;

	case '-':	/* display previous page */
#if TERMINFO
	case KEY_PPAGE:
#endif
		/* don't redisplay if there are no lines */
		if (totallines == 0) {
			return(NO);
		}
		/* go back two pages because already at next page */
		nextline -= 2 * mdisprefs;
		if (nextline < 1) {
			nextline = totallines - mdisprefs + 1;
		}
		if (nextline < 1) {
			nextline = 1;
		}
		seekline(nextline);
		break;

	case '>':	/* append the lines to a file */
		if (totallines == 0) {
			putmsg("There are no lines to append to a file");
		}
		else {	/* get the file name */
			move(PRLINE, 0);
			addstr(appendprompt);
			if (getline(newpat, COLS - sizeof(appendprompt), '\0', NO) > 0) {

				/* open the file */
				shellpath(filename, sizeof(filename), newpat);
				if ((file = fopen(filename, "a")) == NULL) {
					(void) sprintf(msg, "Cannot open file: %s", filename);
					putmsg(msg);
				}
				else {	/* append the lines */
					seekline(1);
					while ((c = getc(refsfound)) != EOF) {
						(void) putc(c, file);
					}
					seekline(topline);
					(void) fclose(file);
				}
			}

			/* clear the file name prompt */
			move(PRLINE, 0);
			clrtoeol();
		}
		/* return to the previous field */
		return(NO);

	case '|':	/* pipe the lines to a shell command */
		if (totallines == 0) {
			putmsg("There are no lines to pipe to a shell command");
			return(NO);
		}
		/* get the shell command */
		move(PRLINE, 0);
		addstr(pipeprompt);
		if (getline(newpat, COLS - sizeof(pipeprompt), '\0', NO) == 0) {
			move(PRLINE, 0);
			clrtoeol();
			return(NO);
		}
		move(LINES - 1, 0);	/* to lower left corner */
		clrtoeol();		/* clear bottom line */
		refresh();
		endwin();		/* restore the terminal modes */
		mousecleanup();
		(void) fflush(stdout);

		/* open the pipe */
		if ((file = mypopen(newpat, "w")) == NULL) {
			(void) fprintf(stderr, "cscope: cannot open pipe to shell command: %s\n", newpat);
		}
		else {	/* append the lines */
			seekline(1);
			while ((c = getc(refsfound)) != EOF) {
				(void) putc(c, file);
			}
			seekline(topline);
			(void) pclose(file);
		}
		askforreturn();
#if UNIXPC || !TERMINFO
		nonl();
		cbreak();	/* endwin() turns off cbreak mode so restore it */
		noecho();
#endif
		/* the menu and scrollbar may be changed by the command executed */
		mousemenu();
		drawscrollbar(topline, nextline);
		break;

	case ctrl('L'):	/* redraw screen */
#if TERMINFO
	case KEY_CLEAR:
#endif
		clearok(curscr, TRUE);
		wrefresh(curscr);
		drawscrollbar(topline, bottomline);
		return(NO);

	case '!':	/* shell escape */
#if BSD
		move(LINES - 1, 0);	/* to lower left corner */
		clrtoeol();		/* clear bottom line */
		refresh();
#endif
		execute(shell, shell, (char *) 0);
		clear();
		seekline(topline);
		break;

	case '?':	/* help */
		clear();
		help();
		clear();
		seekline(topline);
		break;

	case ctrl('E'):	/* edit all lines */
		editall();
		break;

	case ctrl('Y'):	/* repeat last pattern */
		if (*pattern != '\0') {
			addstr(pattern);
			goto repeat;
		}
		break;

	case ctrl('B'):		/* cmd history back */
	case ctrl('F'):		/* cmd history fwd */
		curritem = currentcmd();
		item = (c == ctrl('F')) ? nextcmd() : prevcmd();
		clearmsg2();
		if( curritem == item) {	/* inform user that we're at history end */
			putmsg2( "End of input field and search pattern history");
		}
		if( item) {
			field = (item->field);
			setfield();
			atfield();
			addstr( item->text);
			(void) strcpy( pattern, item->text);
			switch( ch = mygetch()) {
			case '\r':
			case '\n':
				goto repeat;
			default:
				(void) ungetch( ch);
				atfield();
				clrtoeol();	/* clear current field */
				break;
			}
		}
		return(NO);

	case '\\':	/* next character is not a command */
	case ctrl('Q'):
		addch('\\');	/* display the quote character */

		/* get a character from the terminal */
		if ((c = mygetch()) == EOF) {
			return(NO);	/* quit */
		}
		addstr("\b \b");	/* erase the quote character */
		goto ispat;

	case '.':
		putmsg("The . command has been replaced by ^Y");
		atfield();	/* move back to the input field */
		/* FALLTHROUGH */
	default:
		/* edit a selected line */
		if (isdigit(c) && c != '0' && mouse == NO) {
			editref(c - '1');
		}
		/* if this is the start of a pattern */
		else if (isprint(c)) {
	ispat:		if (getline(newpat, COLS - fldcolumn - 1, c, caseless) > 0) {
					strcpy(pattern, newpat);
					resetcmd();			/* reset command history ptr */
	repeat:
				addcmd( field, pattern);		/* add to command history */
				if (field == CHANGE) {
					
					/* prompt for the new text */
					move(PRLINE, 0);
					addstr(toprompt);
 					(void) getline(newpat, COLS - sizeof(toprompt), '\0', NO);
				}
				/* search for the pattern */
				if (search() == YES) {
					switch (field) {
					case DEFINITION:
					case FILENAME:
						if (totallines > 1) {
							break;
						}
						topline = 1;
						editref(0);
						break;
					case CHANGE:
						return(changestring());
					}
				}
				/* try to edit the file anyway */
				else if (field == FILENAME && 
				    access(newpat, READ) == 0) {
					edit(newpat, "1");
				}
			}
			else {	/* no pattern--the input was erased */
				return(NO);
			}
		}
		else {	/* control character */
			return(NO);
		}
	}
	return(YES);
}
/* change one text string to another */

BOOL
changestring()
{
	char	newfile[PATHLEN + 1];	/* new file name */
	char	oldfile[PATHLEN + 1];	/* old file name */
	char	linenum[NUMLEN + 1];	/* file line number */
	char	msg[MSGLEN + 1];	/* message */
	FILE	*script;		/* shell script file */
	BOOL	anymarked = NO;		/* any line marked */
	MOUSE	*p;			/* mouse data */
	int	c, i;
	char	*s;

	/* open the temporary file */
	if ((script = fopen(temp2, "w")) == NULL) {
		putmsg("Cannot open temporary file");
		return(NO);
	}
	/* create the line change indicators */
	change = (BOOL *) mycalloc(totallines, sizeof(BOOL));
	changing = YES;
	mousemenu();

	/* until the quit command is entered */
	for (;;) {
		/* display the current page of lines */
		display();
	same:
		atchange();
		
		/* get a character from the terminal */
		if ((c = mygetch()) == EOF || c == ctrl('D') || c == ctrl('Z')) {
			break;	/* change lines */
		}
		/* see if the input character is a command */
		switch (c) {
		case ' ':	/* display next page */
		case '+':
		case ctrl('V'):
#if TERMINFO
		case KEY_NPAGE:
#endif
		case '-':	/* display previous page */
#if TERMINFO
		case KEY_PPAGE:
#endif
		case '!':	/* shell escape */
		case '?':	/* help */
			command(c);
			break;

		case ctrl('L'):	/* redraw screen */
#if TERMINFO
		case KEY_CLEAR:
#endif
			command(c);
			goto same;

		case ESC:	/* don't change lines */
#if UNIXPC
			if((p = getmouseaction(c)) == NULL) {
				goto nochange;	/* unknown escape sequence */
			}
			break;
#endif
		case ctrl('G'):
			goto nochange;

		case '*':	/* mark/unmark all displayed lines */
			for (i = 0; topline + i < nextline; ++i) {
				mark(i);
			}
			goto same;

		case 'a':	/* mark/unmark all lines */
			for (i = 0; i < totallines; ++i) {
				if (change[i] == NO) {
					change[i] = YES;
				}
				else {
					change[i] = NO;
				}
			}
			/* show that all have been marked */
			seekline(totallines);
			break;
		case ctrl('X'):	/* mouse selection */
			if ((p = getmouseaction(c)) == NULL) {
				goto same;	/* unknown control sequence */
			}
			/* if the button number is a scrollbar tag */
			if (p->button == '0') {
				scrollbar(p);
				break;
			}
			/* find the selected line */
			/* note: the selection is forced into range */
			for (i = disprefs - 1; i > 0; --i) {
				if (p->y1 >= displine[i]) {
					break;
				}
			}
			mark(i);
			goto same;
		default:
			/* if a line was selected */
			if (isdigit(c) && c != '0' && mouse == NO) {
				mark(c - '1');
			}
			goto same;
		}
	}
	/* for each line containing the old text */
	(void) fprintf(script, "ed - <<\\!\n");
	*oldfile = '\0';
	seekline(1);
	for (i = 0; fscanf(refsfound, "%s%*s%s%*[^\n]", newfile, linenum) == 2;
	    ++i) {
		/* see if the line is to be changed */
		if (change[i] == YES) {
			anymarked = YES;
		
			/* if this is a new file */
			if (strcmp(newfile, oldfile) != 0) {
				
				/* make sure it can be changed */
				if (access(newfile, WRITE) != 0) {
					(void) sprintf(msg, "Cannot write to file %s", newfile);
					putmsg(msg);
					anymarked = NO;
					break;
				}
				/* if there was an old file */
				if (*oldfile != '\0') {
					(void) fprintf(script, "w\n");	/* save it */
				}
				/* edit the new file */
				(void) strcpy(oldfile, newfile);
				(void) fprintf(script, "e %s\n", oldfile);
			}
			/* output substitute command */
			(void) fprintf(script, "%ss/", linenum);	/* change */
			for (s = pattern; *s != '\0'; ++s) {	/* old text */
				if (strchr("/\\[.^*$", *s) != NULL) {
					(void) putc('\\', script);
				}
				(void) putc(*s, script);
			}
			(void) putc('/', script);			/* to */
 			for (s = newpat; *s != '\0'; ++s) {	/* new text */
				if (strchr("/\\&", *s) != NULL) {
					(void) putc('\\', script);
				}
				(void) putc(*s, script);
			}
			(void) fprintf(script, "/gp\n");	/* and print */
		}
	}
	(void) fprintf(script, "w\nq\n!\n");	/* write and quit */
	(void) fclose(script);

	/* if any line was marked */
	if (anymarked == YES) {
		
		/* edit the files */
		move(PRLINE, 0);
		clrtoeol();
		refresh();
		(void) fprintf(stderr, "Changed lines:\n\r");
		execute("sh", "sh", temp2, (char *) 0);
		askforreturn();
		seekline(1);
	}
	else {
nochange:
		move(PRLINE, 0);
		clrtoeol();
	}
	changing = NO;
	mousemenu();
	free((char *) change);
	return(anymarked);
}

/* mark/unmark this displayed line to be changed */

void
mark(i)
int	i;
{
	int	j;
	
	j = i + topline - 1;
	if (j < totallines) {
		move(displine[i], 1);
		if (change[j] == NO) {
			change[j] = YES;
			addch('>');
		}
		else {
			change[j] = NO;
			addch(' ');
		}
	}
}

/* scrollbar actions */

void
scrollbar(p)
MOUSE	*p;
{
	/* reposition list if it makes sense */
	if (totallines == 0) {
		return;
	}
	switch (p->percent) {
		
	case 101: /* scroll down one page */
		if (nextline + mdisprefs > totallines) {
			nextline = totallines - mdisprefs + 1;
		}
		break;
		
	case 102: /* scroll up one page */
		nextline = topline - mdisprefs;
		if (nextline < 1) {
			nextline = 1;
		}
		break;

	case 103: /* scroll down one line */
		nextline = topline + 1;
		break;
		
	case 104: /* scroll up one line */
		if (topline > 1) {
			nextline = topline - 1;
		}
		break;
	default:
		nextline = p->percent * totallines / 100;
	}
	seekline(nextline);
}
