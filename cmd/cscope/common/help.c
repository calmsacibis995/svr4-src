/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/help.c	1.2"
/*	cscope - interactive C symbol cross-reference
 *
 *	display help
 */

#include "global.h"
#include <curses.h>	/* LINES needed by constants.h */
/* 
	max num of lines of help screen - 
	this number needs to be increased if more that n help items are needed
*/
#define MAXHELP 50

void
help()
{
	char **ep, **tp, *text[MAXHELP];	
	int ln;

	tp = text;
	if (changing == NO) {
#if UNIXPC
		if (mouse == YES || unixpcmouse == YES) {
#else
		if (mouse == YES) {
#endif
			*tp++ = "Point with the mouse and click button 1 to move to the desired input field,\n";
			*tp++ = "type the pattern to search for, and then press the RETURN key.  For the first 4\n";
			*tp++ = "and last 2 input fields, the pattern can be a regcmp(3X) regular expression.\n";
			*tp++ = "If the search is successful, you can edit the file containing a displayed line\n";
			*tp++ = "by pointing with the mouse and clicking button 1.\n";
			*tp++ = "\nYou can either use the button 2 menu or these single-character commands:\n\n";
		}
		else {
			*tp++ = "Press the TAB key repeatedly to move to the desired input field, type the\n";
			*tp++ = "pattern to search for, and then press the RETURN key.  For the first 4 and\n";
			*tp++ = "last 2 input fields, the pattern can be a regcmp(3X) regular expression.\n";
			*tp++ = "If the search is successful, you can use these single-character commands:\n\n";
			*tp++ = "1-9\t\tEdit the file containing the displayed line.\n";
		}
		*tp++ = "space bar\tDisplay next lines.\n";
		*tp++ = "+\t\tDisplay next lines.\n";
		*tp++ = "-\t\tDisplay previous lines.\n";
		*tp++ = "^E\t\tEdit all lines.\n";
		*tp++ = ">\t\tAppend all lines to a file.\n";
		*tp++ = "|\t\tPipe all lines to a shell command.\n";
		if (mouse == NO) {
			*tp++ = "\nAt any time you can use these single-character commands:\n\n";
		}
		*tp++ = "^Y\t\tSearch with the last pattern typed.\n";
		*tp++ = "^B\t\tRecall previous input field and search pattern.\n";
		*tp++ = "^F\t\tRecall next input field and search pattern.\n";
		if(caseless)
			*tp++ = "^C\t\tToggle ignore/use letter case when searching (IGNORE).\n";
		else
			*tp++ = "^C\t\tToggle ignore/use letter case when searching (USE).\n";
		*tp++ = "^R\t\tRebuild the symbol database.\n";
		*tp++ = "!\t\tStart an interactive shell (type ^D to return to cscope).\n";
		*tp++ = "^L\t\tRedraw the screen.\n";
		*tp++ = "?\t\tDisplay this list of commands.\n";
		*tp++ = "^D\t\tExit cscope.\n";
		*tp++ = "\nNote: If the first character of the pattern you want to search for matches\n";
		*tp++ = "a command, type a \\ character first.\n";
	}
	else {
		if (mouse == YES) {
			*tp++ = "Point with the mouse and click button 1 to mark or unmark the line to be\n";
			*tp++ = "changed.  You can also use the button 2 menu or these single-character\n";
			*tp++ = "commands:\n\n";
		}
		else {
			*tp++ = "When changing text, you can use these single-character commands:\n\n";
			*tp++ = "1-9\t\tMark or unmark the line to be changed.\n";
		}
		*tp++ = "*\t\tMark or unmark all displayed lines to be changed.\n";
		*tp++ = "space bar\tDisplay next lines.\n";
		*tp++ = "+\t\tDisplay next lines.\n";
		*tp++ = "-\t\tDisplay previous lines.\n";
		*tp++ = "a\t\tMark or unmark all lines to be changed.\n";
		*tp++ = "^D\t\tChange the marked lines and exit.\n";
		*tp++ = "ESC\t\tExit without changing the marked lines.\n";
		*tp++ = "!\t\tStart an interactive shell (type ^D to return to cscope).\n";
		*tp++ = "^L\t\tRedraw the screen.\n";
		*tp++ = "?\t\tDisplay this list of commands.\n";
	}

	/* print help, a screen at a time */
	ep = tp;
	for( ln = 0, tp = text; tp < ep;) {
		if( ln++ < LINES-4)
			addstr( *tp++);
		else {
			addstr( "\n");
			askforchar();
			clear();
			ln = 0;
		}
	}
	if( ln) {
		addstr( "\n");
		askforchar();
	}
}
