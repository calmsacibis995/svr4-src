/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tsort:errtext.c	1.1"
/*	Routines to print and adjust options on
	error messages.
*/

#include	"errmsg.h"
#include	<stdio.h>
#include	<varargs.h>


extern	char    *getenv();
extern  int	errno;

/*	Internal form, to handle both errtext() and _errmsg()
*/
void
__errtext( severity, format, ap )
int	severity;
char	*format;
va_list	ap;
{
	int	puterrno = 0;		/* true if an errno msg was printed */

	Err.severity = severity;
	errverb( getenv( "ERRVERB" ) );

	errbefore( Err.severity, format, ap );

	if( Err.severity == EIGNORE )
		goto after;

	if( Err.vbell )
		fputc( '\07', stderr );
	if( Err.vprefix  &&  Err.prefix ) {
		fputs( Err.prefix, stderr );
		fputc( ' ', stderr );
	}
	if( Err.vsource ) {
		if( Err.envsource  ||
		   (Err.envsource = getenv( "ERRSOURCE" )) ) {
			fprintf( stderr, "%s: ", Err.envsource );
		}
	}
	if( Err.vsource  &&  Err.source ) {
		fprintf( stderr, "%s: ", Err.source );
	}

	if( Err.vsevmsg ) {
		char	**e;

		for( e = Err.sevmsg;  *e;  e++ )
			;
		if( Err.severity < ( e - Err.sevmsg ) )
			fputs( Err.sevmsg[ Err.severity ], stderr );
		else
			fputs( "<UNKNOWN>", stderr );
	}

	if( Err.vtext ){
		if( Err.vsyserr && ((int) format == EERRNO) ) {
			fflush( stderr );
			perror("");
			puterrno = 1;
		}
		else {
			vfprintf( stderr, format, ap);
			fputs( "\n", stderr );
		}
	}

	if( ( errno && ( (int) format != EERRNO) )  &&
	    ( Err.vsyserr == EYES  ||  ( Err.vsyserr ==  EDEF  &&
	    ( Err.severity == EHALT  ||  Err.severity == EERROR ))) ){
		fputc('\t', stderr );
		fflush( stderr );
		perror("");
		puterrno = 1;
	}

	if( Err.vtag ) {
		if( Err.tagnum )
			fputc( '\t', stderr );
		else
			fputs( "HELP FACILITY KEY: ", stderr );
		if( Err.tagstr )
			fputs( Err.tagstr, stderr );
		if( Err.tagnum )
			fprintf( stderr, ", line %d", Err.tagnum );
		if( puterrno )
			fprintf( stderr, "\tUXerrno%d", errno );
		fputc( '\n', stderr );
	}

	if( ( Err.vtext || Err.vtag )  &&
	    Err.vfix  &&  Err.tofix  &&  !Err.tagnum )
		fprintf( stderr, "To Fix:\t%s\n", Err.tofix );

   after:
	erraction( errafter(Err.severity, format, ap), Err.severity );
	return;
}


/*	external form, used by errmsg() macro, when tag is not permanently
	assigned.
*/
void
errtext( va_alist )
va_dcl
{
	int	severity;
	va_list ap;
	char    *format;

	va_start( ap );
	severity = va_arg(ap, int);
	format = va_arg(ap, char *);
	va_end( ap );
	__errtext( severity, format, ap);
	return;
}


/*	external form, used when tag is permanently assigned.
*/
void
_errmsg( va_alist )
va_dcl
{
	va_list ap;
	int	severity;
	char    *format;

	va_start( ap );
	Err.tagstr = va_arg(ap, char *);
	Err.tagnum = 0;
	severity = va_arg(ap, int);
	format = va_arg(ap, char *);
	va_end( ap );
	__errtext( severity, format, ap );
	return;
}


void
errverb( s )
register  char *s;
{
	char	*errstrtok();
	char	buf[ BUFSIZ ];
	register
	 char   *token;
	static
	 char   space[] = ",\t\n";

	if( !s )
		return;
	strcpy( buf, s );
	for( token = errstrtok( buf, space);  token;
		token = errstrtok( (char*)0, space ) ) {

		if( !strcmp(token,  "nochange" ) ) {
			Err.vbell   =  ENO;
			Err.vtext   =  EYES;
			Err.vsource =  EYES;
			Err.vsyserr =  EYES;
			Err.vtag    =  ENO;
			Err.vsevmsg =  ENO;
			Err.vfix	 =  ENO;
		} else   if( !strcmp(token,  "silent" ) ) {
			Err.vbell   =  ENO;
			Err.vprefix =  ENO;
			Err.vtext   =  ENO;
			Err.vsource =  ENO;
			Err.vsyserr =  ENO;
			Err.vtag    =  ENO;
			Err.vsevmsg =  ENO;
			Err.vfix	 =  ENO;
		} else   if( !strcmp(token,  "verbose" ) ) {
			Err.vbell   =  EYES;
			Err.vprefix =  EYES;
			Err.vtext   =  EYES;
			Err.vsource =  EYES;
			Err.vsyserr =  EYES;
			Err.vtag    =  EYES;
			Err.vsevmsg =  EYES;
			Err.vfix	 =  EYES;
		} else   if( !strcmp(token,  "expert" ) ) {
			Err.vbell   =  ENO;
			Err.vprefix =  ENO;
			Err.vtext   =  EYES;
			Err.vsource =  EYES;
			Err.vsyserr =  EYES;
			Err.vtag    =  ENO;
			Err.vsevmsg =  EYES;
			Err.vfix	 =  ENO;
		} else   if( !strcmp(token,  "bell" ) ) {
			Err.vbell = EYES;

		} else   if( !strcmp(token,  "nobell" ) ) {
			Err.vbell = ENO;

		} else   if( !strcmp(token,  "tag" ) ) {
			Err.vtag = EYES;

		} else   if( !strcmp(token,  "notag" ) ) {
			Err.vtag = ENO;

		} else   if( !strcmp(token,  "text" ) ) {
			Err.vtext = EYES;

		} else   if( !strcmp(token,  "notext" ) ) {
			Err.vtext = ENO;

		} else   if( !strcmp(token,  "tofix" ) ) {
			Err.vfix = EYES;

		} else   if( !strcmp(token,  "notofix" ) ) {
			Err.vfix = ENO;

		} else   if( !strcmp(token,  "syserr"  ) ) {
			Err.vsyserr = EYES;

		} else   if( !strcmp(token,  "nosyserr" ) ) {
			Err.vsyserr = ENO;

		} else   if( !strcmp(token,  "defsyserr" ) ) {
			Err.vsyserr = EDEF;

		} else   if( !strcmp(token,  "source" ) ) {
			Err.vsource = EYES;

		} else   if( !strcmp(token,  "nosource" ) ) {
			Err.vsource = ENO;

		} else   if( !strcmp(token,  "sevmsg" ) ) {
			Err.vsevmsg = EYES;

		} else   if( !strcmp(token,  "nosevmsg" ) ) {
			Err.vsevmsg = ENO;

		} else   if( !strcmp(token,  "prefix" ) ) {
			Err.vprefix = EYES;

		} else   if( !strcmp(token,  "noprefix" ) ) {
			Err.vprefix = ENO;

		}

	}
}
