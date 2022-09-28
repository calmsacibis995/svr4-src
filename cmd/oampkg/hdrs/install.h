/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:hdrs/install.h	1.3.3.2"

#define MAILCMD	"/usr/bin/mail"
#define DATSTRM	"datastream"
#define SHELL	"/sbin/sh"
#define PKGINFO	"pkginfo"
#define PKGMAP	"pkgmap"
#define isdot(x)	((x[0]=='.')&&(!x[1]||(x[1]=='/')))
#define isdotdot(x)	((x[0]=='.')&&(x[1]=='.')&&(!x[2]||(x[2]=='/')))

struct mergstat {
	char	*setuid;
	char	*setgid;
	char	contchg;
	char	attrchg;
	char	shared;
};

struct admin {
	char	*mail;
	char	*instance;
	char	*partial;
	char	*runlevel;
	char	*idepend;
	char	*rdepend;
	char	*space;
	char	*setuid;
	char	*conflict;
	char	*action;
	char	*basedir;
	char	*list_files;
};

#define ADM(x, y)	!strcmp(adm.x, y)
