/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)xt:xts.c	2.5.4.1"

/*
**	Print out XT driver statistics
*/

char		Usage[]	=	"Usage: %s [-f]\n";

#include "stdio.h"
#include "errno.h"
#ifdef SVR32
#include "sys/patch.h"
#endif /* SVR32 */
#include "sys/param.h"
#include "sys/types.h"
#include "ctype.h"
#include "sys/tty.h"
#include "sys/jioctl.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/nxtproto.h"
#include "sys/nxt.h"		/* defines XTRACES and XTSTATS to be 1       */

#define		Fprintf		(void)fprintf
#define		SYSERROR	(-1)

char		*name;		/* name of the command itself		     */
int		display = 0;	/* is formfeed to be displayed?		     */

extern int	xtstats();

extern Stats_t Stats[];    /* defined in xtstats.c */
extern unsigned char	_sobuf[];

int
main(argc, argv)
	int		argc;
	char *		argv[];
{
	name = *argv;

	if (argc != 1)
		if (!(argc == 2 && (strcmp("-f", argv[1]) == 0))) {
			Fprintf(stderr, Usage, name);
			exit(1);
		} else
			 ++display;

	if (ioctl(0, JMPX, 0) == -1){
		Fprintf(stderr, "%s: Must be invoked with stdin attached to an xt channel.\n", name);
		exit(1);
	}

	setbuf(stdout, (char *)_sobuf);

	/* Get the statistics in the Stats buffer.
	 * Each index of the Stats buffer indicates a particular type of
	 * statistics and its contents indicate the value logged for that.
	 */
	if ( ioctl(0, XTIOCSTATS, Stats) == SYSERROR ) {
		Fprintf(stderr,"%s: xt ioctl failed - errno '%d'\n",name,errno);
		return(1);
	}
	if ( xtstats(stdout) )
		exit(1);

	if ( display )
		Fprintf(stdout, "\f");

	exit(0);
}
