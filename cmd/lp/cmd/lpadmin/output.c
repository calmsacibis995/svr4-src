/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lp:cmd/lpadmin/output.c	1.7.2.1"

#include "stdio.h"
#include "string.h"
#include "sys/types.h"

#include "lp.h"
#include "printers.h"
#include "msgs.h"
#include "requests.h"

#define	WHO_AM_I	I_AM_LPADMIN
#include "oam.h"

#include "lpadmin.h"


/**
 ** output() - (MISNOMER) HANDLE MESSAGES BACK FROM SPOOLER
 **/

int			output (type)
	int			type;
{
	char			buffer[MSGMAX];

	int			rc;

	short			status;
	char			*dummy;


	if (!scheduler_active)
		switch (type) {

		case R_MOUNT:
		case R_UNMOUNT:
		case R_QUIET_ALERT:
		case R_INQUIRE_PRINTER_STATUS:
		case R_ALLOC_FILES:
		case R_PRINT_REQUEST:
		case R_REJECT_DEST:
		case R_ACCEPT_DEST:
		case R_DISABLE_DEST:
		case R_ENABLE_DEST:
		case R_CANCEL_REQUEST:
		default:
			LP_ERRMSG (ERROR, E_LP_NEEDSCHED);
			done (1);

		case R_UNLOAD_PRINTER:
		case R_UNLOAD_CLASS:
		case R_UNLOAD_PRINTWHEEL:
			if (anyrequests()) {
				LP_ERRMSG (ERROR, E_LP_HAVEREQS);
				done (1);
			}
			/* fall through */

		case R_LOAD_PRINTER:
		case R_LOAD_CLASS:
		case R_LOAD_PRINTWHEEL:
			return (MOK);

		}

	status = MOKMORE;
	while (status == MOKMORE) {

		if ((rc = mrecv(buffer, MSGMAX)) != type) {
			LP_ERRMSG (ERROR, E_LP_MRECV);
			done (1);
		}
			
		switch(type) {

		case R_MOUNT:
		case R_UNMOUNT:
		case R_LOAD_PRINTER:
		case R_UNLOAD_PRINTER:
		case R_LOAD_CLASS:
		case R_UNLOAD_CLASS:
		case R_LOAD_PRINTWHEEL:
		case R_UNLOAD_PRINTWHEEL:
		case R_QUIET_ALERT:
		case R_REJECT_DEST:
		case R_ACCEPT_DEST:
		case R_ENABLE_DEST:
		case R_CANCEL_REQUEST:
			rc = getmessage(buffer, type, &status);
			goto CheckRC;

		case R_DISABLE_DEST:
			rc = getmessage(buffer, type, &status, &dummy);
CheckRC:		if (rc != type) {
				LP_ERRMSG1 (ERROR, E_LP_BADREPLY, rc);
				done (1);
			}
			break;

		case R_INQUIRE_PRINTER_STATUS:
		case R_ALLOC_FILES:
		case R_PRINT_REQUEST:
			return (0);	/* handled by caller */
		}

	}

	return (status);
}
