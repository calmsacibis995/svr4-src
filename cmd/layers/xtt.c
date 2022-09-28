/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)xt:xtt.c	2.5.3.1"

/*
 *	Print out XT driver traces
 */

#ifdef SVR32
#include "sys/patch.h"
#endif /* SVR32 */
#include "sys/types.h"
#include "sys/tty.h"
#include "sys/jioctl.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/nxtproto.h"
#include "sys/nxt.h"
#include "stdio.h"

#define		Fprintf		(void)fprintf

char *name;
int display;
int traceoff;
extern int xtraces();
extern struct Tbuf Traces;
extern int errno;
extern unsigned char _sobuf[];
char usage[] = "Usage: %s [-o] [-f]\n";

main(argc, argv)
char *argv[];
{
	name = *argv++;
	for (; --argc; argv++) {
		if (argv[0][0] == '-' && argv[0][2] == 0)
			switch (argv[0][1]) {
			case 'f':
				display++;
				break;
			case 'o':
				traceoff++;
				break;
			default:
				(void)fprintf(stderr, usage, name);
				return 1;
			}
		else {
			(void)fprintf(stderr, usage, name);
			return 1;
		}
	}

	if (ioctl(0, JMPX, 0) == -1){
		Fprintf(stderr, "%s: Must be invoked with stdin attached to an xt channel.\n", name);
		exit(1);
	}

	setbuf(stdout, (char *)_sobuf);

	if (ioctl(0, XTIOCTRACE, &Traces) == -1) {
		Fprintf(stderr, "%s: XTIOCTRACE ioctl failed - errno '%d'\n", name, errno);
		exit(1);
	}
	if ( xtraces(stdout) )
		exit(1);

	if (display)
		(void)fprintf(stdout, "\f");

	if (traceoff)
		(void)ioctl(0, XTIOCNOTRACE, 0);

	return 0;
}
