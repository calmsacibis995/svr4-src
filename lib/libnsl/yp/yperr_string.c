/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libyp:yperr_string.c	1.2.1.1"

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
static  char sccsid[] = "@(#)yperr_string.c 1.9 88/02/08 Copyr 1985 Sun Micro";
#endif

#include <rpcsvc/ypclnt.h>

/*
 * This returns a pointer to an error message string appropriate to an input
 * yp error code.  An input value of zero will return a success message.
 * In all cases, the message string will start with a lower case chararacter,
 * and will be terminated neither by a period (".") nor a newline.
 */
char *
yperr_string(code)
	int code;
{
	char *pmesg;
	
	switch (code) {

	case 0:  {
		pmesg = "yp operation succeeded";
		break;
	}
		
	case YPERR_BADARGS:  {
		pmesg = "args to yp function are bad";
		break;
	}
	
	case YPERR_RPC:  {
		pmesg = "RPC failure on yp operation";
		break;
	}
	
	case YPERR_DOMAIN:  {
		pmesg = "can't bind to a server which serves domain";
		break;
	}
	
	case YPERR_MAP:  {
		pmesg = "no such map in server's domain";
		break;
	}
		
	case YPERR_KEY:  {
		pmesg = "no such key in map";
		break;
	}
	
	case YPERR_YPERR:  {
		pmesg = "internal yp server or client error";
		break;
	}
	
	case YPERR_RESRC:  {
		pmesg = "local resource allocation failure";
		break;
	}
	
	case YPERR_NOMORE:  {
		pmesg = "no more records in map database";
		break;
	}
	
	case YPERR_PMAP:  {
		pmesg = "can't communicate with rpcbind";
		break;
		}
		
	case YPERR_YPBIND:  {
		pmesg = "can't communicate with ypbind";
		break;
		}
		
	case YPERR_YPSERV:  {
		pmesg = "can't communicate with ypserv";
		break;
		}
		
	case YPERR_NODOM:  {
		pmesg = "local domain name not set";
		break;
	}

	case YPERR_BADDB:  {
		pmesg = "yp map data base is bad";
		break;
	}

	case YPERR_VERS:  {
		pmesg = "yp client/server version mismatch";
		break;
	}

	case YPERR_ACCESS: {
		pmesg = "permission denied";
		break;
	}

	case YPERR_BUSY: {
		pmesg = "database is busy";
		break;
	}
		
	default:  {
		pmesg = "unknown yp client error code";
		break;
	}
	
	}

	return(pmesg);
}
