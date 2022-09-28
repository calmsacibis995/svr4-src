/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:altconn.c	2.6.3.1"

#include "uucp.h"

EXTERN getto();

/* The call structure is also defined in cu.c.  The two definitions should
 * be kept identical.
 */

static struct call {		/* cu calling info-also in cu.c; */
				/* make changes in both places!*/
	char *speed;		/* transmission baud rate */
	char *line;		/* device name for outgoing line */
	char *telno;		/* ptr to tel-no digit string */
	char *type;		/* type of device to use for call. */
};

/*

 * altconn - place a telephone call to system 
 * from cu when telephone number or direct line used
 *
 * return codes:
 *	FAIL - connection failed
 *	>0  - file no.  -  connect ok
 * When a failure occurs, Uerror is set.
 */
GLOBAL int
altconn(call)
struct call *call;
{
	int fn = FAIL;
	char *alt[7];
	EXTERN char *Myline;

	alt[F_NAME] = "dummy";	/* to replace the Systems file fields  */
	alt[F_TIME] = "Any";	/* needed for getto(); [F_TYPE] and    */
	alt[F_TYPE] = "";	/* [F_PHONE] assignment below          */
	alt[F_CLASS] = call->speed;
	alt[F_PHONE] = "";
	alt[F_LOGIN] = "";
	alt[6] = NULL;

	CDEBUG(4,"altconn called\r\n%s", "");

	/* cu -l dev ...					*/
	/* if is "/dev/device", strip off "/dev/" because must	*/
	/* exactly match entries in Devices file, which usually	*/
	/* omit the "/dev/".  if doesn't begin with "/dev/",	*/
	/* either they've omitted the "/dev/" or it's a non-	*/
	/* standard path name.  in either case, leave it as is	*/

	if(call->line != NULL ) {
		if ( strncmp(call->line, "/dev/", 5) == 0 ) {
			Myline = (call->line + 5);
		} else {
			Myline = call->line;
		}
	}

	/* cu  ... telno */
	if(call->telno != NULL) {
		alt[F_PHONE] = call->telno;
		alt[F_TYPE] = "ACU";
	}
	/* cu direct line */
	else {
		alt[F_TYPE] = "Direct";
	}
	if (call->type != NULL)
		alt[F_TYPE] = call->type;
	fn = getto(alt);
	CDEBUG(4, "getto ret %d\n", fn);

	return(fn);

}

