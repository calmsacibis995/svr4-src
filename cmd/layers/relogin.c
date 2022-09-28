/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xt:relogin.c	2.10.6.1"

/*
 * relogin [-s] [-t ttyname] line
 *
 * Change the /etc/utmp file (the file read by the who command) to
 * reflect proper ttyname. Mostly for use by the layers command, but
 * can also be called directly.
 *
 * Options:
 *
 *	-s or -		Suppress error messages.
 *
 *	-t ttyname	The ttyname to change the utmp entry to.
 *			If not supplied, the utmp entry is changed
 *			to the device specified by the ttyname()
 *			function. The -t option is used by the
 *			layers command to work around problems
 *			with ttyname() failing after a hangup. The
 *			-t option is only for use by the layers
 *			command and is therefore undocumented.
 *
 *	line		Current ttyname of the utmp entry we want to change.
 *			If not specified, an entry is searched for with the
 *			same as name as the current ttyname() except for
 *			the last digit. This is because it is assumed that
 *			when 'line' is not specified the intention is to
 *			change the entry to a different xt window and
 *			different xt windows have the same ttyname except
 *			for the last digit.
 *
 * The line to be changed is specified by the login associated with the real
 * UID of calling process and 'line' argument as described above.
 */

char		Usage[]		= "usage: relogin [-s] [-t ttyname] [line]\n";

#include	<sys/types.h>
#include	<utmpx.h>
#include	<stdio.h>
#include	<pwd.h>

struct utmpx	*utmpx;
char		toolong[]	= "name '%s' too long for utmp field";
char		nulls[sizeof utmpx->ut_line];
char *		name;
char		utmpxfn[]	= UTMPX_FILE;
char		silent;
char		devdir[]	= "/dev/";

typedef struct passwd *	Pwp;

void		error();

extern void	endpwent();
extern Pwp	getpwuid();
extern int	getuid();
extern int	strlen();
extern int	strncmp();
extern char *	strncpy();
extern char *	ttyname();


main(argc, argv)
int   argc;
char *argv[];
{
	register char *new_line = (char *)0;
	register int   new_llen;
	register char *utmp_line;
	register int   utmp_llen;
	register int   retval;
	register Pwp   pwp;
	struct utmpx *getutxent(), *pututxline();

	name = argv[0];

	/* should be changed to use getarg() */
	while ((argc > 1) && (argv[1][0] == '-'))
		if ( strcmp("-",argv[1])==0 || strcmp("-s",argv[1])==0 ) {
			silent++;
			argv++;
			argc--;
		}
		else if( strcmp("-t",argv[1]) == 0 ) {
			argv++;
			argc--;
			if(argc < 2) {
				fprintf(stderr, Usage);
				exit(1);
			}
			new_line = argv[1];
			argv++;
			argc--;
		}
		else {
			fprintf(stderr, Usage);
			exit(1);
		}

	if ( argc > 2 ) {
		(void)fprintf(stderr, Usage);
		exit(1);
	}

	if(new_line == (char *)0)
		if ( (new_line = ttyname(0)) == (char *)0 )
			error("stdin not a tty");

	if(strlen(new_line) < sizeof devdir)
		error("-t arg must be full path to /dev file", new_line);
	new_line += sizeof devdir - 1;
	if ((new_llen = strlen(new_line)) > sizeof(utmpx->ut_line))
		error(toolong, new_line);

	if (argc == 2) {
		/*
		** 	The user has specified which utmpx line entry
		**	to change.
		*/
		utmp_line = argv[1];

		if ( strncmp(utmp_line, devdir, sizeof devdir - 1) == 0 )
			utmp_line += sizeof devdir - 1;

		if ((utmp_llen = strlen(utmp_line)) > sizeof(utmpx->ut_line))
			error(toolong, utmp_line);
	} else {
		/*
		** 	The user has not specified which utmp line entry
		**	to change. We subtract 1 from new_llen so xt--?
		**	identifies the user's entry in /etc/utmp where
		**	"--" is the user's set of xt channels.
		*/
		utmp_line = new_line;
		utmp_llen = new_llen - 1;
	}

	if ( (pwp = getpwuid(getuid())) == (Pwp)0 )
		error("can't identify your login id");

	endpwent();

	while( (utmpx = getutxent()) != NULL )
		if ((strncmp(utmpx->ut_line, utmp_line, utmp_llen) == 0) &&
		    (strncmp(utmpx->ut_user, pwp->pw_name, sizeof utmpx->ut_user) == 0 )) {
			(void)strncpy(utmpx->ut_line, nulls, sizeof utmpx->ut_line);
			(void)strncpy(utmpx->ut_line, new_line, new_llen);
			pututxline(utmpx);
			endutxent(); /* close utmpx file */
			exit(0);
		}

	error("cannot find '%s' logged in on '%s' in '%s'",
	      pwp->pw_name, utmp_line, utmpxfn);
}



/*VARARGS1*/
void
error(s1, s2, s3, s4, s5)
	char *	s1;
	char *	s2;
	char *	s3;
	char *	s4;
	char *	s5;
{
	if ( ! silent ) {
		(void)fprintf(stderr, "%s: error - ", name);
		(void)fprintf(stderr, s1, s2, s3, s4, s5);
		(void)fprintf(stderr, ".\n"); }

	exit(1);
}
