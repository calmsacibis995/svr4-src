/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpadmin/usage.c	1.9.3.1"

#include "lp.h"
#include "printers.h"

/**
 ** usage() - PRINT COMMAND USAGE
 **/

void			usage ()
{
#define	P	(void) printf ("%s\n",
#define X	);

P "usage:"								X
P ""									X
P "  (add printer)"							X
P "    lpadmin -p printer {-v device | -U dial-info | -s system[!printer]} [options]"X
P "	[-s system[!printer]]			(remote system/printer name)"X
P "	[-v device]				(printer port name)"	X
P "	[-U dial-info]				(phone # or sys. name)"	X
P "	[-T type-list]				(printer types)"	X
P "	[-c class | -r class]			(add to/del from class)"X
P "	[-A mail|write|quiet|cmd [-W interval]]	(alert definition)"	X
P "	[-A none]				(no alerts)"		X
P "	[-A list]				(examine alert)"	X
P "	[-D comment]				(printer description)"	X
P "	[-e printer | -i interface | -m model]	(interface program)"	X
P "	[-l | -h]				(is/isn't login tty)"	X
P "	[-f allow:forms-list | deny:forms-list]	(forms allowed)"	X
P "	[-u allow:user-list | deny:user-list]	(who's allowed to use)"	X
P "	[-S char-set-maps | print-wheels]	(list of avail. fonts)"	X
P "	[-I content-type-list]			(file types accepted"	X
P "	[-F beginning|continue|wait]		(fault recovery)"	X
#if	defined(CAN_DO_MODULES)
P "	[-H module,...|keep|default|none]	(STREAMS modules to push)"X
#endif
P "	[-o stty='stty-options']		(port characteristics)"	X
P "	[-o cpi=scaled-number]			(character pitch)"	X
P "	[-o lpi=scaled-number]			(line pitch)"		X
P "	[-o width=scaled-number]		(page width)"		X
P "	[-o length=scaled-number]		(page length)"		X
P "	[-o nobanner]				(allow no banner)"	X
P ""									X
P "  (delete printer or class)"						X
P "    lpadmin -x printer-or-class"					X
P ""									X
P "  (define default destination)"					X
P "    lpadmin -d printer-or-class"					X
P ""									X
P "  (mount form, printwheel)"						X
P "    lpadmin -p printer -M {options}"					X
P "	[-f form [-a [-o filebreak]]]		(mount (align) form)"	X
P "	[-S print-wheel]			(mount print wheel)"	X
P ""									X
P "  (define print-wheel mount alert)"					X
P "    lpadmin -S print-wheel {options}"				X
P "	[-A mail|write|quiet|cmd [-W interval] [-Q queue-size]]"	X
P "	[-A none]				(no alerts)"		X
P "	[-A list]				(examine alert)"	X

	return;
}
