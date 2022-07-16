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

#ident	"@(#)librpc:clnt_perror.c	1.3.1.1"

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
static char sccsid[] = "@(#)clnt_perror.c 1.31 89/03/31 Copyr 1984 Sun Micro";
#endif

/*
 * clnt_perror.c
 *
 */
#ifndef KERNEL
#include <stdio.h>
#include <string.h>
#endif

#include <rpc/types.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>

#ifndef KERNEL
extern char *t_errlist[];
extern char *malloc();
extern int t_nerr;
#endif

extern char *strcpy();
extern char *strcat();
extern char *netdir_sperror();

#ifndef KERNEL
static char *buf;

static char *
_buf()
{

	if (buf == NULL)
		buf = (char *)malloc(256);
	return (buf);
}

static char *
auth_errmsg(stat)
	enum auth_stat stat;
{
	switch (stat) {
	case AUTH_OK:
		return ("Authentication OK");
	case AUTH_BADCRED:
		return ("Invalid client credential");
	case AUTH_REJECTEDCRED:
		return ("Server rejected credential");
	case AUTH_BADVERF:
		return ("Invalid client verifier");
	case AUTH_REJECTEDVERF:
		return ("Server rejected verifier");
	case AUTH_TOOWEAK:
		return ("Client credential too weak");
	case AUTH_INVALIDRESP:
		return ("Invalid server verifier");
	case AUTH_FAILED:
		return ("Failed (unspecified error)");
	}
	return ("Unknown authentication error");
}

/*
 * Return string reply error info. For use after clnt_call()
 */
char *
clnt_sperror(cl, s)
	CLIENT *cl;
	char *s;
{
	struct rpc_err e;
	void clnt_perrno();
	char *err;
	char *str = _buf();
	char *strstart = str;

	if (str == NULL)
		return (NULL);
	CLNT_GETERR(cl, &e);

	(void) sprintf(str, "%s: ", s);  
	str += strlen(str);

	(void) strcpy(str, clnt_sperrno(e.re_status));  
	str += strlen(str);

	switch (e.re_status) {
	case RPC_SUCCESS:
	case RPC_CANTENCODEARGS:
	case RPC_CANTDECODERES:
	case RPC_TIMEDOUT:     
	case RPC_PROGUNAVAIL:
	case RPC_PROCUNAVAIL:
	case RPC_CANTDECODEARGS:
	case RPC_SYSTEMERROR:
	case RPC_UNKNOWNHOST:
	case RPC_UNKNOWNPROTO:
	case RPC_UNKNOWNADDR:
	case RPC_NOBROADCAST:
	case RPC_RPCBFAILURE:
	case RPC_PROGNOTREGISTERED:
	case RPC_FAILED:
		break;

	case RPC_N2AXLATEFAILURE:
		(void) sprintf(str, "; %s", netdir_sperror());
		str += strlen(str);
		break;

	case RPC_TLIERROR:
		(void) sprintf(str, "; %s", t_errlist[e.re_terrno]); 
		str += strlen(str);
		if (e.re_errno) {
			(void) sprintf(str, "; %s", strerror(e.re_errno)); 
			str += strlen(str);
		}
		break;

	case RPC_CANTSEND:
	case RPC_CANTRECV:
		if (e.re_errno) {
			(void) sprintf(str, "; errno = %s", strerror(e.re_errno)); 
			str += strlen(str);
		}
		if (e.re_terrno) {
			(void) sprintf(str, "; %s", t_errlist[e.re_terrno]); 
			str += strlen(str);
		}
		break;

	case RPC_VERSMISMATCH:
		(void) sprintf(str, "; low version = %lu, high version = %lu",
				e.re_vers.low, e.re_vers.high);
		str += strlen(str);
		break;

	case RPC_AUTHERROR:
		err = auth_errmsg(e.re_why);
		(void) sprintf(str, "; why = ");
		str += strlen(str);
		if (err != NULL) {
			(void) sprintf(str, "%s", err);
		} else {
			(void) sprintf(str,
				"(unknown authentication error - %d)",
				(int) e.re_why);
		}
		str += strlen(str);
		break;

	case RPC_PROGVERSMISMATCH:
		(void) sprintf(str, "; low version = %lu, high version = %lu",
				e.re_vers.low, e.re_vers.high);
		str += strlen(str);
		break;

	default:	/* unknown */
		(void) sprintf(str, "; s1 = %lu, s2 = %lu",
				e.re_lb.s1, e.re_lb.s2);
		str += strlen(str);
		break;
	}
	return (strstart);
}

void
clnt_perror(cl, s)
	CLIENT *cl;
	char *s;
{
	(void) fprintf(stderr,"%s\n", clnt_sperror(cl, s));
}

void
clnt_perrno(num)
	enum clnt_stat num;
{
	(void) fprintf(stderr, "%s\n", clnt_sperrno(num));
}

/*
 * Why a client handle could not be created
 */
char *
clnt_spcreateerror(s)
	char *s;
{
	extern int _sys_num_err;
	char *str = _buf();

	if (str == NULL)
		return (NULL);
	(void) sprintf(str, "%s: ", s);
	(void) strcat(str, clnt_sperrno(rpc_createerr.cf_stat));

	switch (rpc_createerr.cf_stat) {
	case RPC_N2AXLATEFAILURE:
		(void) strcat(str, " - ");
		(void) strcat(str, netdir_sperror());
		break;

	case RPC_RPCBFAILURE:
		(void) strcat(str, " - ");
		(void) strcat(str,
			clnt_sperrno(rpc_createerr.cf_error.re_status));
		break;

	case RPC_SYSTEMERROR:
		(void) strcat(str, " - ");
		if (rpc_createerr.cf_error.re_errno > 0
		    && rpc_createerr.cf_error.re_errno < _sys_num_err)
			(void) strcat(str,
			    strerror(rpc_createerr.cf_error.re_errno));
		else
			(void) sprintf(&str[strlen(str)], "Error %d",
			    rpc_createerr.cf_error.re_errno);
		break;
	case RPC_TLIERROR:
		(void) strcat(str, " - ");
		if (rpc_createerr.cf_error.re_terrno > 0
		    && rpc_createerr.cf_error.re_terrno < t_nerr)
			(void) strcat(str,
				t_errlist[rpc_createerr.cf_error.re_terrno]);
		else
			(void) sprintf(&str[strlen(str)], "TLI Error %d",
					    rpc_createerr.cf_error.re_terrno);
		if (rpc_createerr.cf_error.re_errno > 0) {
			if (rpc_createerr.cf_error.re_errno < _sys_num_err)
				(void) strcat(str,
			    strerror(rpc_createerr.cf_error.re_errno));
			else
				(void) sprintf(&str[strlen(str)], "Error %d",
					    rpc_createerr.cf_error.re_terrno);
		}
		break;
	}
	return (str);
}

void
clnt_pcreateerror(s)
	char *s;
{
	(void) fprintf(stderr, "%s\n", clnt_spcreateerror(s));
}
#endif /* ! KERNEL */

/*
 * This interface for use by rpc_call() and rpc_broadcast()
 */
char *
clnt_sperrno(stat)
	enum clnt_stat stat;
{
	switch (stat) {
	case RPC_SUCCESS:
		return ("RPC: Success"); 
	case RPC_CANTENCODEARGS:
		return ("RPC: Can't encode arguments");
	case RPC_CANTDECODERES:
		return ("RPC: Can't decode result");
	case RPC_CANTSEND:
		return ("RPC: Unable to send");
	case RPC_CANTRECV:
		return ("RPC: Unable to receive");
	case RPC_TIMEDOUT:
		return ("RPC: Timed out");
	case RPC_VERSMISMATCH:
		return ("RPC: Incompatible versions of RPC");
	case RPC_AUTHERROR:
		return ("RPC: Authentication error");
	case RPC_PROGUNAVAIL:
		return ("RPC: Program unavailable");
	case RPC_PROGVERSMISMATCH:
		return ("RPC: Program/version mismatch");
	case RPC_PROCUNAVAIL:
		return ("RPC: Procedure unavailable");
	case RPC_CANTDECODEARGS:
		return ("RPC: Server can't decode arguments");
	case RPC_SYSTEMERROR:
		return ("RPC: Remote system error");
	case RPC_UNKNOWNHOST:
		return ("RPC: Unknown host");
	case RPC_UNKNOWNPROTO:
		return ("RPC: Unknown protocol");
	case RPC_RPCBFAILURE:
		return ("RPC: Rpcbind failure");
	case RPC_N2AXLATEFAILURE:
		return ("RPC: Name to address translation failed");
	case RPC_NOBROADCAST:
		return ("RPC: Broadcast not supported");
	case RPC_PROGNOTREGISTERED:
		return ("RPC: Program not registered");
	case RPC_UNKNOWNADDR:
		return ("RPC: Remote server address unknown");
	case RPC_TLIERROR:
		return ("RPC: Miscellaneous tli error");
	case RPC_FAILED:
		return ("RPC: Failed (unspecified error)");
	}
	return ("RPC: (unknown error code)");
}
