/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/logname.c	6.6"
# include	<pwd.h>
# include	<sys/types.h>
# include	<macros.h>
# include	<ccstypes.h>


char	*logname()
{
	struct passwd *getpwuid();
	struct passwd *log_name;
	uid_t	getuid();
	uid_t uid;

	uid = getuid();
	log_name = getpwuid(uid);
	endpwent();
	if (! log_name)
		return(0);
	else
		return(log_name->pw_name);
}
