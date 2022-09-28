/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpstat/accept.c	1.6.3.1"

#include "stdio.h"

#include "lp.h"
#include "class.h"
#include "msgs.h"

#define	WHO_AM_I	I_AM_LPSTAT
#include "oam.h"

#include "lpstat.h"

/**
 ** do_accept()
 **/

void
#if	defined(__STDC__)
do_accept (
	char **			list
)
#else
do_accept (list)
	char			**list;
#endif
{
	while (*list) {
		if (STREQU(*list, NAME_ALL)) {
			send_message (S_INQUIRE_CLASS, "");
			(void)output (R_INQUIRE_CLASS);
			send_message (S_INQUIRE_PRINTER_STATUS, "");
			(void)output (R_INQUIRE_PRINTER_STATUS);

		} else if (isclass(*list)) {
			send_message (S_INQUIRE_CLASS, *list);
			(void)output (R_INQUIRE_CLASS);

		} else {
			send_message (S_INQUIRE_PRINTER_STATUS, *list);
			switch (output(R_INQUIRE_PRINTER_STATUS)) {
			case MNODEST:
				LP_ERRMSG1 (ERROR, E_LP_BADDEST, *list);
				exit_rc = 1;
				break;
			}

		}
		list++;
	}
	return;
}

/**
 ** putqline()
 **/

void
#if	defined(__STDC__)
putqline (
	char *			dest,
	int			rejecting,
	char *			reject_date,
	char *			reject_reason
)
#else
putqline (dest, rejecting, reject_date, reject_reason)
	char			*dest;
	int			rejecting;
	char			*reject_date,
				*reject_reason;
#endif
{
	(void) printf("%s ", dest);
	if (!rejecting)
		(void) printf("accepting requests since %s\n", reject_date);
	else
		(void) printf(
			"not accepting requests since %s -\n\t%s\n",
			reject_date,
			reject_reason
		);
	return;
}
