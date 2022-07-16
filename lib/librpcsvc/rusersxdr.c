/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)librpcsvc:rusersxdr.c	1.1.3.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)rusersxdr.c 1.7 89/03/24 Copyr 1985 Sun Micro";
#endif

/*
 * rusersxdr.c
 * Besides just the XDR routines, this also has a few other user
 * routines
 */

#include <rpc/rpc.h>
#include <rpcsvc/rusers.h>

rusers(host, up)
	char *host;
	struct utmpidlearr *up;
{
	return (rpc_call(host, RUSERSPROG, RUSERSVERS_IDLE, RUSERSPROC_NAMES,
			xdr_void, (char *) NULL,
			xdr_utmpidlearr, (char *) up, (char *) NULL));
}

rnusers(host)
	char *host;
{
	int nusers;
	
	if (rpc_call(host, RUSERSPROG, RUSERSVERS_ORIG, RUSERSPROC_NUM,
			xdr_void, (char *) NULL,
			xdr_u_long, (char *) &nusers, (char *) NULL) != 0)
		return (-1);
	else
		return (nusers);
}

xdr_ru_utmp(xdrsp, up)
	XDR *xdrsp;
	struct ru_utmp *up;
{
	int len;
	char *p;

	len = sizeof(up->ut_line);
	p = up->ut_line;
	if (xdr_bytes(xdrsp, &p, &len, len) == FALSE)
		return (0);
	len = sizeof(up->ut_name);
	p = up->ut_name;
	if (xdr_bytes(xdrsp, &p, &len, len) == FALSE)
		return (0);
	len = sizeof(up->ut_host);
	p = up->ut_host;
	if (xdr_bytes(xdrsp, &p, &len, len) == FALSE)
		return (0);
	if (xdr_long(xdrsp, &up->ut_time) == FALSE)
		return (0);
	return (1);
}

xdr_utmpidle(xdrsp, ui)
	XDR *xdrsp;
	struct utmpidle *ui;
{
	if (xdr_ru_utmp(xdrsp, &ui->ui_utmp) == FALSE)
		return (0);
	if (xdr_u_int(xdrsp, &ui->ui_idle) == FALSE)
		return (0);
	return (1);
}

xdr_utmpptr(xdrsp, up)
	XDR *xdrsp;
	struct ru_utmp **up;
{
	if (xdr_reference(xdrsp, (char **) up, sizeof (struct ru_utmp), 
			xdr_ru_utmp) == FALSE)
		return (0);
	return (1);
}

xdr_utmpidleptr(xdrsp, up)
	XDR *xdrsp;
	struct utmpidle **up;
{
	if (xdr_reference(xdrsp, (char **) up, sizeof (struct utmpidle), 
			xdr_utmpidle) == FALSE)
		return (0);
	return (1);
}

xdr_utmparr(xdrsp, up)
	XDR *xdrsp;
	struct utmparr *up;
{
	return (xdr_array(xdrsp, (char **) &up->uta_arr, &(up->uta_cnt),
		MAXUSERS, sizeof(struct ru_utmp *), xdr_utmpptr));
}

xdr_utmpidlearr(xdrsp, up)
	XDR *xdrsp;
	struct utmpidlearr *up;
{
	return (xdr_array(xdrsp, (char **) &up->uia_arr, &(up->uia_cnt),
		MAXUSERS, sizeof(struct utmpidle *), xdr_utmpidleptr));
}
