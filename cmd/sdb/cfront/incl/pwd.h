/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/pwd.h	1.1"
/*ident	"@(#)cfront:incl/pwd.h	1.7"*/

#ifndef PWDH
#define PWDH

#ifndef FILE
#       include <stdio.h>
#endif

struct passwd {
	char	*pw_name;
	char	*pw_passwd;
	int	pw_uid;
	int	pw_gid;
	char	*pw_age;
	char	*pw_comment;
	char	*pw_gecos;
	char	*pw_dir;
	char	*pw_shell;
};

struct comment {
	char	*c_dept;
	char	*c_name;
	char	*c_acct;
	char	*c_bin;
};

extern passwd *getpwent ();
extern passwd *getpwuid (int);
extern passwd *getpwnam (const char*);
extern passwd *fgetpwent (FILE*);

extern void setpwent ();
extern void endpwent ();
extern int putpwent (const passwd*, FILE*);

#endif

