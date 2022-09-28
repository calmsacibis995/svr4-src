/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:sel_disp.c	1.5.3.1"
#include "mail.h"

/*
 * If in default display mode from printmail(), selectively output
 * header lines. Any recognized header lines will have flag stored in
 * header[] structure. Other header lines which should be displayed in
 * the default output mode will be listed in the seldisp[] array.
 * This can all be overridden via the 'P' command at the ? prompt.
 */
sel_disp (type, hdrtype, s)
int	type;
int	hdrtype;
char	*s;
{
	static char pn[] = "sel_disp";
	register char	*p;
	static	int	sav_lastrc = 0;
	int		i, rc = 0;

	if (sending || Pflg || (type != TTY)) {
		return (0);
	}

	switch (hdrtype) {
	case H_CONT:
		rc = sav_lastrc;
		break;
	case H_NAMEVALUE:
		for (i=0,p=seldisp[i]; p; p=seldisp[++i]) {
			if (casncmp(s, p, strlen(p)) == 0) {
				break;
			}
		}
		if (p == (char *)NULL) {
			rc = -1;
		}
		break;
	default:
		if (header[hdrtype].default_display == FALSE) {
			rc = -1;
			break;
		}
	}

	Dout(pn, 2, "type = %d, hdrtype = %d/'%s', rc = %d\n",
		type, hdrtype, header[hdrtype].tag, rc);
	sav_lastrc = rc;	/* In case next one is H_CONT... */
	return (rc);
}
