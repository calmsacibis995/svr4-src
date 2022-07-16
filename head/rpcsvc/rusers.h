/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)/usr/src/head/rpcsvc/rusers.h.sl 1.1 4.0 12/08/90 16232 AT&T-USL"

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
/*	@(#)rusers.h 1.4 89/03/24 SMI	*/

/* 
 * rusers.h
 */

#ifndef _rpcsvc_rusers_h
#define _rpcsvc_rusers_h

#define RUSERSPROC_NUM 1
#define RUSERSPROC_NAMES 2
#define RUSERSPROC_ALLNAMES 3
#define RUSERSPROG 100002
#define RUSERSVERS_ORIG 1
#define RUSERSVERS_IDLE 2
#define RUSERSVERS 2

#define MAXUSERS 100
/*
 * This is a utmp entry that does not correspond to a genuine user
 */
#define nonuser(ut) ((ut).ut_host[0] == 0 && \
	strncmp((ut).ut_line, "tty", 3) == 0 && ((ut).ut_line[3] == 'p' \
					      || (ut).ut_line[3] == 'q' \


/*
 * the structure (data object) passed between the rusers program and
 * rpc.rusersd. For historical reasons it is the utmp structure for the
 * bsd sytems. It is the data object that is included in the stuctures 
 * passed between the client and the rusers service.
 */

struct ru_utmp {
	char	ut_line[8];		/* tty name */
	char	ut_name[8];		/* user id */
	char	ut_host[16];		/* host name, if remote */
	long	ut_time;		/* time on */
};


struct utmparr {
	struct ru_utmp **uta_arr;
	int uta_cnt;
};

struct utmpidle {
	struct ru_utmp ui_utmp;
	unsigned ui_idle;
};

struct utmpidlearr {
	struct utmpidle **uia_arr;
	int uia_cnt;
};

int xdr_utmparr();
int xdr_utmpidlearr();

#endif /*!_rpcsvc_rusers_h*/
