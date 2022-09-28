/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:include/access.h	1.7.2.1"

#if	!defined(_LP_ACCESS_H)
#define	_LP_ACCESS_H

#include "stdio.h"

/*
 * To speed up reading in each allow/deny file, ACC_MAX_GUESS slots
 * will be preallocated for the internal copy. If these files
 * are expected to be substantially larger than this, bump it up.
 */
#define ACC_MAX_GUESS	100

#if	defined(__STDC__)

int	allow_form_printer ( char **, char * );
int	allow_user_form ( char ** , char * );
int	allow_user_printer ( char **, char * );
int	allowed ( char *, char **, char ** );
int	deny_form_printer ( char **, char * );
int	deny_user_form ( char ** , char * );
int	deny_user_printer ( char **, char * );
int	dumpaccess ( char *, char *, char *, char ***, char *** );
int	is_form_allowed_printer ( char *, char * );
int	is_user_admin ( void );
int	is_user_allowed ( char *, char ** , char ** );
int	is_user_allowed_form ( char *, char * );
int	is_user_allowed_printer ( char *, char * );
int	load_formprinter_access ( char *, char ***, char *** );
int	load_userform_access ( char *, char ***, char *** );
int	load_userprinter_access ( char *, char ***, char *** );
int	loadaccess ( char *, char *, char *, char ***, char *** );
int	bangequ ( char * , char * );
int	bang_searchlist ( char * , char ** );
int	bang_dellist ( char *** , char * );

char *	getaccessfile ( char *, char *, char *, char * );

#else

int	allow_form_printer();
int	allow_user_form();
int	allow_user_printer();
int	allowed();
int	deny_form_printer();
int	deny_user_form();
int	deny_user_printer();
int	dumpaccess();
int	is_form_allowed_printer();
int	is_user_admin();
int	is_user_allowed();
int	is_user_allowed_form();
int	is_user_allowed_printer();
int	load_formprinter_access();
int	load_userform_access();
int	load_userprinter_access();
int	loadaccess();
int	bangequ();
int	bang_searchlist();
int	bang_dellist();

char *	getaccessfile();

#endif

#endif
