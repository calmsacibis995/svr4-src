/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:mta_ercode.c	1.4.3.1"
#include "mail.h"
/*
 * Map mail(1) error into MTA reason-codes for negative delivery notification.
 */
static char *MTAerrors[] = {
		"",
/*  1 */	"Invalid Address Specification",
/*  2 */	"Ambiguous Originator/Recipient Name",
/*  3 */	"Message Transfer Agent Congestion",
/*  4 */	"Loop Detection",
/*  5 */	"Unavailable User Agent",
/*  6 */	"Expired Maximum Time",
/*  7 */	"Unsupported Encoded Information Types",
/*  8 */	"Prohibited Conversion",
/*  9 */	"Impractical Conversion",
/* 10 */	"Invalid Parameters",
/* 11 */	"Transfer Failure",
/* 12 */	"Inability To Transfer",
/* 13 */	"Conversion Not Performed",
/* 14 */	"Deferred Delivery Not Available",
/* 15 */	"Too many Recipients",
/* 16 */	"Mail Too Large For Destination To Receive"
};

void mta_ercode(outfile)
FILE *outfile;
{
	register int mtacode;
	switch (error) {
	case E_FROM:	/* too many From lines */
		mtacode = 1;
		break;

	case E_SNDR:	/* invalid sender */
	case E_USER:	/* invalid user */
		mtacode = 2;
		break;

	case E_FRWL:	/* forwarding loop */
	case E_UNBND:	/* Unbounded forwarding */
		mtacode = 4;
		break;

	case 23:	/* disallowed from sending binary to remote */
		mtacode = 7;
		break;

	case E_SYNTAX:	/* syntax error */
	default:
		mtacode = 10;
		break;

	case E_SURG:	/* surrogate command failed - rc != 0 || 99 */
		mtacode = 11;
		break;

	case E_REMOTE:	/* unknown remote */
	case E_FILE:	/* file error */
	case E_FRWD:	/* cannot forward */
	case E_PERM:	/* bad permissions */
	case E_TMP:	/* temporary file problem */
	case E_DEAD:	/* Cannot create dead.letter */
	case E_LOCK:	/* cannot create lock file */
	case E_GROUP:	/* no group id of 'mail' */
	case E_MEM:	/* malloc failure */
	case E_FORK:	/* could not fork */
	case E_PIPE:	/* could not pipe */
	case E_OWNR:	/* invoker does not own mailfile */
	case E_DENY:	/* permission denied by mailsurr file */
		mtacode = 12;
		break;

	case E_MBOX:	/* mbox problem */
		mtacode = 12;
		if (sav_errno != EFBIG) {
			break;
		}
		/* Note drop-thru... */
	case E_SPACE:	/* no space */
		mtacode = 16;
		break;
	}
	fprintf(outfile, "%.2d  %s\n", mtacode, MTAerrors[mtacode]);
}
