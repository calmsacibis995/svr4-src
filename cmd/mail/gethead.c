/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:gethead.c	1.7.3.1"
#include "mail.h"
/*
	display headers, indicating current and status

	current is the displacement into the mailfile of the 
	current letter

	all indicates how many headers should be shown.
		0	->	show window +/-6 around current
		1	->	show all messages
		2	->	show deleted messages
*/
gethead(current,all)
int	current,all;
{

	int	displayed = 0;
	FILE	*file;
	char	*hold;
	char	holdval[100];
	char	*wline;
	char	wlineval[100];
	int	ln;
	char	mark;
	int	rc, size, start,stop,ix;
	char	userval[20];

	hold = holdval;
	wline = wlineval;

	printf("%d letters found in %s, %d scheduled for deletion, %d newly arrived\n", nlet, mailfile, changed, nlet - onlet);

	if (all==2 && !changed) return(0);

	file = doopen(lettmp,"r",E_TMP);
	if (!flgr) {
		stop = current - 6;
		if (stop < -1) stop = -1;
		start = current + 5;
		if (start > nlet - 1) start = nlet - 1;
		if (all) {
			start = nlet -1;
			stop = -1;
		}
	}
	else {
		stop = current + 6;
		if (stop > nlet) stop = nlet;
		start = current - 5;
		if (start < 0) start = 0;
		if (all) {
			start = 0;
			stop = nlet;
		}
	}
	for (ln = start; ln != stop; ln = flgr ? ln + 1 : ln - 1) {
		size = let[ln+1].adr - let[ln].adr;
		if ((rc = fseek(file, let[ln].adr, 0)) != 0) {
			errmsg(E_FILE,"Cannot seek header");
			fclose(file);
			return(1);
		}
		if (fgets(wline, 100, file) == NULL) {
			errmsg(E_FILE,"Cannot read header");
			fclose(file);
			return(1);
		}
		if ((rc = strncmp(wline, header[H_FROM].tag, 5)) != SAME) {
			errmsg(E_FILE,"Invalid header encountered");
			fclose(file);
			return(1);
		}
		strcpy(hold, wline + 5);
		fgets(wline, 100, file);
		while ((rc = strncmp(wline, header[H_FROM1].tag, 6)) == SAME
			&& substr(wline,"remote from ") != -1) {
			strcpy(hold, wline + 6);
			fgets(wline, 100, file);
		}
	
		for (ix = 0,rc = 0; hold[rc] != ' ' && hold[rc] != '\t'; ++rc) {
			userval[ix++] = hold[rc];
			if (hold[rc] == '!') ix = 0;
		}
		userval[ix] = '\0';

		for (; hold[rc] == ' ' || hold[rc] == '\t'; ++rc);
		strcpy(wline, hold + rc);
		for (rc = 0; wline[rc] != '\n'; ++rc);
		wline[rc] = '\0';
	
		if (!flgh && current == ln) mark = '>';
		else mark = ' ';
	
		if (all == 2) {
			if (displayed >= changed) {
				fclose(file);
				return(0);
			}
			if (let[ln].change == ' ') continue;
		}
		printf("%c %3d  %c  %-5d  %-10s  %s\n", mark, ln + 1, let[ln].change, size, userval, wline);
		displayed++;
	}
	fclose(file);
	return(0);
}

void tmperr()
{
	fclose(tmpf);
	errmsg(E_TMP,"");
	return;
}

/*
	Write a string out to tmp file, with error checking.
	Return 1 on success, else 0
*/
wtmpf(str,length)
char	*str;
{
	if (fwrite(str,1,length,tmpf) != length) {
		tmperr();
		return(0);
	}
	return(1);
}

/*
	Read a line from stdin, assign it to line and
	return number of bytes in length
*/
int getline(ptr2line, max, f)
char *ptr2line;
int max;
FILE	*f;
{
	int	i,ch;
	for (i=0; i < max-1 && (ch=getc(f)) != EOF;)
		if ((ptr2line[i++] = ch) == '\n') break;
	ptr2line[i] = '\0';
	return(i);
}

/*
	Make temporary file for letter
*/
void mktmp()
{
	mode_t	Oumask;

	lettmp = tempnam(tmpdir,"mail");

	/* Protect the temp file from prying eyes...*/
	Oumask = umask(077);
	tmpf = doopen(lettmp,"w+",E_TMP);
	(void) umask (Oumask);
}

/*
	Get a number from user's reply,
	return its value or zero if none present, -1 on error
*/
getnumbr(s)
char	*s;
{
	int	k = 0;

	while (*s == ' ' || *s == '\t') s++;

	if (*s != '\0') {
		if ((k = atoi(s)) != 0) 
			if (!validmsg(k)) return(-1);

		for (; *s >= '0' && *s <= '9';) s++;
		if (*s != '\0' && *s != '\n') {
			printf("Illegal numeric\n");
			return(-1);
		}
		return(k);
	}
	return(0);
}

/*
	If valid msgnum return 1,
		else print message and return 0
*/
validmsg(i)
{
	if ((i < 0) || (i > nlet)) {
		printf("No such message\n");
		return(0);
	}
	return(1);
}

/*
	Set letter to passed status, and adjust changed as necessary
*/
void setletr(letter,status)
int	letter;
int	status;
{
	if (status == ' ') {
		if (let[letter].change != ' ') 
			if (changed) changed--;
	}
	else if (let[letter].change == ' ') changed++;
	let[letter].change = status;
}
