/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/getpaths.c	1.15.3.1"
/* LINTLIBRARY */

#include "stdlib.h"

#include "lp.h"

#if	defined(__STDC__)

char	Lp_Spooldir[]		= SPOOLDIR;
char	Lp_Admins[]		= SPOOLDIR "/admins";
char	Lp_FIFO[]		= SPOOLDIR "/fifos/FIFO";
char	Lp_Private_FIFOs[]	= SPOOLDIR "/fifos/private";
char	Lp_Public_FIFOs[]	= SPOOLDIR "/fifos/public";
char	Lp_Requests[]		= SPOOLDIR "/requests";
char	Lp_Schedlock[]		= SPOOLDIR "/SCHEDLOCK";
char	Lp_System[]		= SPOOLDIR "/system";
char	Lp_Temp[]		= SPOOLDIR "/temp";
char	Lp_Tmp[]		= SPOOLDIR "/tmp";
char	Lp_NetTmp[]		= SPOOLDIR "/tmp/.net";

char	Lp_Bin[]		= LPDIR "/bin";
char	Lp_Model[]		= LPDIR "/model";
char	Lp_Slow_Filter[]	= LPDIR "/bin/slow.filter";

char	Lp_A_Logs[]		= LOGDIR;
char	Lp_Logs[]		= LOGDIR;
char	Lp_ReqLog[]		= LOGDIR "/requests";

char	Lp_A[]			= ETCDIR;
char	Lp_NetData[]		= ETCDIR "/Systems";
char	Lp_Users[]		= ETCDIR "/users";
char	Lp_A_Classes[]		= ETCDIR "/classes";
char	Lp_A_Forms[]		= ETCDIR "/forms";
char	Lp_A_Interfaces[]	= ETCDIR "/interfaces";
char	Lp_A_Printers[]		= ETCDIR "/printers";
char	Lp_A_PrintWheels[]	= ETCDIR "/pwheels";
char	Lp_A_Systems[]		= ETCDIR "/systems";
char	Lp_A_Filters[]		= ETCDIR "/filter.table";
char	Lp_Default[]		= ETCDIR "/default";

#else

char	Lp_Spooldir[]		= "/var/spool/lp",
	Lp_Admins[]		= "/var/spool/lp/admins",
	Lp_FIFO[]		= "/var/spool/lp/fifos/FIFO",
	Lp_Private_FIFOs[]	= "/var/spool/lp/fifos/private",
	Lp_Public_FIFOs[]	= "/var/spool/lp/fifos/public",
	Lp_Requests[]		= "/var/spool/lp/requests",
	Lp_Schedlock[]		= "/var/spool/lp/SCHEDLOCK",
	Lp_System[]		= "/var/spool/lp/system",
	Lp_Temp[]		= "/var/spool/lp/temp",
	Lp_Tmp[]		= "/var/spool/lp/tmp",
	Lp_NetTmp[]		= "/var/spool/lp/tmp/.net",

	Lp_Bin[]		= "/usr/lib/lp/bin",
	Lp_Model[]		= "/usr/lib/lp/model",
	Lp_Slow_Filter[]	= "/usr/lib/lp/bin/slow.filter",

	Lp_A_Logs[]		= "/var/lp/logs",
	Lp_Logs[]		= "/var/lp/logs",
	Lp_ReqLog[]		= "/var/lp/logs/requests",

	Lp_A[]			= "/etc/lp",
	Lp_NetData[]		= "/etc/lp/Systems",
	Lp_Users[]		= "/etc/lp/users",
	Lp_A_Classes[]		= "/etc/lp/classes",
	Lp_A_Forms[]		= "/etc/lp/forms",
	Lp_A_Interfaces[]	= "/etc/lp/interfaces",
	Lp_A_Printers[]		= "/etc/lp/printers",
	Lp_A_PrintWheels[]	= "/etc/lp/printwheels",
	Lp_A_Systems[]		= "/etc/lp/systems",
	Lp_A_Filters[]		= "/etc/lp/filter.table",
	Lp_Default[]		= "/etc/lp/default";

#endif

int	Lp_NTBase		= sizeof(Lp_NetTmp);

/*
**	Sorry about these nonfunctional functions.  The data is
**	static now.  These exist for historical reasons.
*/

#undef	getpaths
#undef	getadminpaths

#if	defined(__STDC__)
void		getpaths ( void ) { return; }
void		getadminpaths ( char * admin) { return; }
#else
void		getpaths();
void		getadminpaths();
#endif

/**
 ** getprinterfile() - BUILD NAME OF PRINTER FILE
 **/

#if	defined(__STDC__)
char * getprinterfile ( char * name, char * component )
#else
char * getprinterfile ( name, component )
char	*name;
char	*component;
#endif
{
    char	*path;

    if (!name)
	return (0);

    path = makepath(Lp_A_Printers, name, component, NULL);

    return (path);
}

/**
 ** getsystemfile() - BUILD NAME OF SYSTEM FILE
 **/

#if	defined(__STDC__)
char * getsystemfile ( char * name, char * component )
#else
char * getsystemfile ( name, component )
char	*name;
char	*component;
#endif
{
    char	*path;

    if (!name)
	return (0);

    path = makepath(Lp_A_Systems, name, component, NULL);

    return (path);
}

/**
 ** getclassfile() - BUILD NAME OF CLASS FILE
 **/

#if	defined(__STDC__)
char * getclassfile ( char * name )
#else
char * getclassfile ( name )
char	*name;
#endif
{
    char	*path;

    if (!name)
	return (0);

    path = makepath(Lp_A_Classes, name, NULL);

    return (path);
}

/**
 ** getfilterfile() - BUILD NAME OF FILTER TABLE FILE
 **/

#if	defined(__STDC__)
char * getfilterfile ( char * table )
#else
char * getfilterfile ( table )
char	*table;
#endif
{
    char	*path;

    if (!table)
	table = FILTERTABLE;

    path = makepath(ETCDIR, table, NULL);

    return (path);
}

/**
 ** getformfile() - BUILD NAME OF PRINTER FILE
 **/

#if	defined(__STDC__)
char * getformfile ( char * name, char * component )
#else
char * getformfile ( name, component )
char	*name;
char	*component;
#endif
{
    char	*path;

    if (!name)
	return (0);

    path = makepath(Lp_A_Forms, name, component, NULL);

    return (path);
}
