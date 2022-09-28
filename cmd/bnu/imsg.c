/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:imsg.c	2.4.3.1"

#include "uucp.h"

#define MSYNC	'\020'
/* maximum likely message - make sure you don't get run away input */
#define MAXIMSG	256

/*
 * read message routine used before a
 * protocol is agreed upon.
 *	msg	-> address of input buffer
 *	fn	-> input file descriptor 
 * returns:
 *	EOF	-> no more messages
 *	0	-> message returned
 */
int
imsg(msg, fn)
register char *msg;
register int fn;
{
	register char c;
	short fndsync;
	char *bmsg;

	fndsync = 0;
	bmsg = msg;
	CDEBUG(7, "imsg %s>", "");
	while ((*Read)(fn, msg, sizeof(char)) == sizeof(char)) {
		*msg &= 0177;
		c = *msg;
		CDEBUG(7, "%s", c < 040 ? "^" : "");
		CDEBUG(7, "%c", c < 040 ? c | 0100 : c);
		if (c == MSYNC) { /* look for sync character */
			msg = bmsg;
			fndsync = 1;
			continue;
		}
		if (!fndsync)
			continue;

		if (c == '\0' || c == '\n') {
			*msg = '\0';
			return(0);
		}
		else
			msg++;

		if (msg - bmsg > MAXIMSG)	/* unlikely */
			return(FAIL);
	}
	/* have not found sync or end of message */
	*msg = '\0';
	return(EOF);
}

/*
 * initial write message routine -
 * used before a protocol is agreed upon.
 *	type	-> message type
 *	msg	-> message body address
 *	fn	-> file descriptor
 * return: 
 *	Must always return 0 - wmesg (WMESG) looks for zero
 */
int
omsg(type, msg, fn)
register char *msg;
register char type;
int fn;
{
	char buf[BUFSIZ];

	(void) sprintf(buf, "%c%c%s", MSYNC, type, msg);
	DEBUG( 7, "omsg \"%s\"\n", &buf[1] );
	(*Write)(fn, buf, strlen(buf) + 1);
	return(0);
}
