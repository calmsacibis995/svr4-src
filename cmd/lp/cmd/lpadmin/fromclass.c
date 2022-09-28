/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lp:cmd/lpadmin/fromclass.c	1.5.2.1"

#include "stdio.h"
#include "errno.h"

#include "lp.h"
#include "class.h"
#include "msgs.h"

#define	WHO_AM_I	I_AM_LPADMIN
#include "oam.h"

#include "lpadmin.h"


static void		_fromclass();

/**
 ** fromclass() - REMOVE PRINTER FROM A CLASS
 **/

void			fromclass (printer, class)
	char			*printer,
				*class;
{
	CLASS			*pc;

	if (!(pc = getclass(class))) {
		LP_ERRMSG1 (ERROR, E_LP_NOCLASS, class);
		done (1);
	}

	if (!searchlist(printer, pc->members)) {
		LP_ERRMSG2 (ERROR, E_ADM_NOTMEM, printer, class);
		done (1);
	}

	_fromclass (printer, class, pc);

	return;
}

/**
 ** fromallclasses() - DELETE A PRINTER FROM ALL CLASSES
 **/

void			fromallclasses (printer)
	char			*printer;
{
	register CLASS		*pc;


	while ((pc = getclass(NAME_ALL)))
		if (searchlist(printer, pc->members))
			_fromclass (printer, pc->name, pc);

	if (errno != ENOENT) {
		LP_ERRMSG1 (ERROR, E_ADM_GETCLASSES, PERROR);
		done (1);
	}

	return;
}

/**
 ** _fromclass() - REALLY DELETE PRINTER FROM CLASS
 **/

static void		_fromclass (printer, class, pc)
	char			*printer,
				*class;
	CLASS			*pc;
{
	int			rc;


	if (dellist(&pc->members, printer) == -1) {
		LP_ERRMSG (ERROR, E_LP_MALLOC);
		done(1);
	}

 	if (!pc->members)
		rmdest (1, class);

	else {
		BEGIN_CRITICAL
			if (putclass(class, pc) == -1) {
				LP_ERRMSG2 (
					ERROR,
					E_LP_PUTCLASS,
					class,
					PERROR
				);
				done(1);
			}
		END_CRITICAL

		send_message(S_LOAD_CLASS, class, "", "");
		rc = output(R_LOAD_CLASS);

		switch(rc) {
		case MOK:
			break;

		case MNODEST:
		case MERRDEST:
			LP_ERRMSG (ERROR, E_ADM_ERRDEST);
			done (1);
			/*NOTREACHED*/

		case MNOSPACE:
			LP_ERRMSG (WARNING, E_ADM_NOCSPACE);
			break;

		case MNOPERM:	/* taken care of up front */
		default:
			LP_ERRMSG1 (ERROR, E_LP_BADSTATUS, rc);
			done (1);
			/*NOTREACHED*/
		}

	}
	return;
}
