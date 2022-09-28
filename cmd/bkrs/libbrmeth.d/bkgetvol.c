/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/bkgetvol.c	1.2.4.1"

#include <stdio.h>
#include <limits.h>
#include <fcntl.h>
#include <string.h>
#include <devmgmt.h>
#include <varargs.h>
#include <bktypes.h>	/* for argv_t */
/* bkrs hdrs */
#include <bkrs.h>
#include <backup.h>
#include <brarc.h>
#include <libadmIO.h>

/* PATH_MAX should be in limits.h, but if not ... */
#ifndef	PATH_MAX
#define	PATH_MAX	1024
#endif

#define	PROMPT_SZ	512
#define	KEYWORDS	5

#define	GET_LABEL( dev, dchar, res, size ) bkgetlabel( dev, dchar, res, size )

#define	IS_ALIAS(d)	(d && *d != '/')

#define	L_COPY( r, sz, l ) \
	if( r ) { \
		if( l ) { \
			strncpy( r, l, sz ); \
			r[ sz ] = '\0'; \
		} else *r = '\0'; \
	}
#define	L_ZERO( r )	if( r ) *r = '\0'

#define ELBL_TXT "  The following external label  should appear on the %s:\\n\\t%s"
#define ILBL_TXT "  The %s should be internally labeled as follows: \\n\\t%s\\n"

#define FORMFS_MSG "\\n\\ \\ or [f] to format %s and place a filesystem on it,"
#define FORMAT_MSG "\\n\\ \\ or [f] to format the %s,"
#define WLABEL_MSG "\\n\\ \\ or [w] to write a new label on the %s,"
#define OLABEL_MSG "\\n\\ \\ or [o] to use the current label anyway,"
#define QUIT_MSG   "\\n\\ \\ or [q] to quit:"

#define ERR_ACCESS	"\n%s (%s) cannot be accessed.\n"
#define ERR_FMT		"\nAttempt to format %s failed.\n"
#define ERR_MKFS	"\nAttempt to place filesystem on %s failed.\n"

extern int	puttext(), ckstr(), ckkeywd();
extern argv_t *s_to_argv();

static char prompt_txt[ PROMPT_SZ ], *keyword[ KEYWORDS ], *sec_prompt;
static char	*fmtcmd = (char *)0, *mkfscmd = (char *)0, *voltxt;
static argv_t	*attrs;

static void	labelerr(), doformat(), new_dchar(), new_prompt();
static char	insert(), *getattr();
int chkdevice();
static char	*pname; 	/* device presentation name */
static char	*volume; 	/* volume name */

/* Return:
 *	0 - okay, label matches
 *	1 - device not accessable
 *	2 - unknown device (devattr failed)
 *	3 - user selected quit
 *	4 - label does not match
 */

getvol( device, dchar, label, result, result_sz, options, prompt )
char	*device, *dchar, *label, *result, *prompt;
int	options, result_sz;
{
	FILE	*tmp;
	char	*advice, *ptr, *type, *buffer;
	long capacity = 0;

	if ( (buffer=(char *)malloc(PATH_MAX + 1)) == NULL )
		return( DMR_QUIT);

	/* record the new dchar */
	if( dchar )
		new_dchar( dchar );

	if( !(pname = getattr( device, "desc"))
		&& !(pname = getattr( device, "alias" )) )
		pname = device;

	/* Can't check external labels */
	if( (options & (DM_ELABEL|DM_CHKLBL)) == (DM_ELABEL|DM_CHKLBL) )
		options |= ~DM_CHKLBL;

	/* No label to check */
	if( (options & DM_CHKLBL) && (!label || !result) )
		options |= ~DM_CHKLBL;

	volume = getattr( device, "volume" );
	type = getattr( device, "type" );

	if( !strncmp(type, "dpart", 5)
		&& (ptr = getattr( device, "capacity" ) ) ) {
		capacity = atol( ptr );

		if( capacity > 0 ) capacity = (capacity << 9);
 	}

	strcpy(buffer, device);

	if( !chkdevice( device, label, type, buffer ) )
		return( DMR_UNKDEVICE );

	/* Build the new prompt */
	new_prompt( buffer, label, prompt, options );

	/* Is the right volume already available? */
	if( (options & DM_AUTO) && result ) {
		switch( GET_LABEL( buffer, dchar, result, result_sz ) ) {
		case -1:
			break;

		case 1:
			if( label && *label && strcmp( label, result ) )
				break;

			L_COPY( result, result_sz, label );
			return( DMR_SUCCESS );
		}
	}
	
	for( ptr = prompt_txt; ; ptr = sec_prompt ) {
		if( !(options & DM_BATCH) && volume ) {
			switch( insert( keyword, ptr ) ) {
			case '\0':
				/* no input available */
				L_ZERO( result );
				return( DMR_BADDEVICE );

			case 'f':

				doformat( voltxt, fmtcmd, mkfscmd );
				continue;

			case 'g':
				/* GO */
				if( !(options & DM_CHKLBL) ) {
					L_COPY( result, result_sz, label );
					break;
				}

				switch( GET_LABEL( buffer, dchar, result, result_sz ) ) {
				case -1:
					L_ZERO( result );
					continue;

				case 0:
					L_COPY( result, result_sz, label );
					break;

				case 1:
					if( strcmp( label, result ) ) {
						labelerr( result );
						continue;
					}
				}

				break;

			case 'o':
				/* OVERRIDE */
				switch( GET_LABEL( buffer, dchar, result, result_sz ) ) {
				case -1:
					L_ZERO( result );
					continue;

				case 0:
					L_ZERO( result );
					break;

				case 1:
					if( !(options & DM_OLABEL) && strcmp( label, result ) ) {
						labelerr( result );
						continue;
					}
					break;
				}
				break;

			case 'q':
				/* QUIT */
				L_ZERO( result );
				return( DMR_QUIT );

			}
		}

		/* Assert: the 'correct' or 'acceptable' volume is available */
		if((tmp = fopen( buffer, "r")) == NULL) {
			/* device was not accessible */
			if( !volume || (options & DM_BATCH) )
				return( DMR_BADDEVICE );

			(void) fprintf(stderr, ERR_ACCESS, pname, buffer);
			if(advice = getattr( buffer, "advice"))
				(void) puttext(stderr, advice, 0, 0);

			continue;
		}
		(void) fclose(tmp);
		break;
	}

	return( DMR_SUCCESS );
}

static char
insert( keyword, prompt )
char **keyword, *prompt;
{
	char	strval[16], *ptr;

	switch( ckkeywd( strval, keyword, NULL, NULL, NULL, prompt ) ) {
	case 1:
		return( '\0' );
	case 3:
		return( 'q' );
	}

	return( *strval );
}

static void
doformat( voltxt, fmtcmd, mkfscmd )
char	*voltxt, *fmtcmd, *mkfscmd;
{
	char	buffer[512];

	fprintf(stderr, "\t[%s]\n", fmtcmd);
	(void) sprintf(buffer, "(%s) 1>&2", fmtcmd);
	if(system(buffer)) {
		(void) fprintf(stderr, ERR_FMT, voltxt);
		return;
	}
	if(mkfscmd) {
		fprintf(stderr, "\t[%s]\n", mkfscmd);
		(void) sprintf(buffer, "(%s) 1>&2", mkfscmd);
		if(system(buffer)) {
			(void) fprintf(stderr, ERR_MKFS, voltxt);
			return;
		}
	}
}


static void
labelerr( label )
char *label;
{
	(void) fprintf(stderr, "\nLabel incorrect.\n");
	if(volume)
		(void) fprintf( stderr, 
			"The internal label on the inserted %s is\n", volume);
	else
		(void) fprintf(stderr, "The internal label for %s is", pname);
	(void) fprintf(stderr, "\t%s\n", label );
}

/* 
	If dchar is given, look for the attribute in there first.
	Since these are stored as <name>=<value>, return a pointer
	to the <value> part only.
*/
static
char *
getattr( device, attribute )
char *device, *attribute;
{
	register i, size;

	if( attrs != (argv_t *)0 ) {
		size = strlen( attribute );
		for( i = 0; (*attrs)[i]; i++ ) 
			if( !strncmp( (*attrs)[i], attribute, size ) 
				&& *((*attrs)[i] + size) == '=' )
				return( (*attrs)[i] + size );
	}
	return( devattr( device, attribute ) );
}

static
void
new_dchar( dchar )
char *dchar;
{
	if( attrs != (argv_t *)0 )
		argv_free( attrs );

	attrs = s_to_argv( dchar, ":" );
}

static
void
new_prompt( device, label, prompt, options )
char *device, *label, *prompt;
int options;
{
	register	n = 0;
	register char	*ptr;

	voltxt = (volume ? volume : "volume");

	if( prompt ) {
		(void) strcpy( prompt_txt, prompt );

		for( ptr = prompt_txt; *prompt; ) {
			if(( *prompt == '\\') && (prompt[1] == '%'))
				prompt++;
			else if(*prompt == '%') {
				switch(prompt[1]) {
				  case 'v':
					strcpy( ptr, voltxt );
					break;

				  case 'p':
					(void) strcpy(ptr, pname);
					break;

				  default:
					*ptr = '\0';
					break;
				}
				ptr += strlen( ptr );
				prompt += 2;
				continue;
			}
			*ptr++ = *prompt++;
		}
		*ptr = '\0';
		ptr = prompt_txt + strlen( prompt_txt );

	} else {
		ptr = prompt_txt + sprintf( prompt_txt, "Insert a %s into %s.", voltxt, pname );

		if( label ) 
			ptr += sprintf( ptr, ((options & DM_ELABEL)? ELBL_TXT: ILBL_TXT),
				voltxt, label );
	}

	sec_prompt = ptr;

	ptr += sprintf( ptr, "\\nType [go] when ready," );
	keyword[n++] = "go";

	if(options & DM_FORMFS) {
		if((fmtcmd = getattr( device, "fmtcmd")) && *fmtcmd
			&& (mkfscmd = getattr( device, "mkfscmd")) && *mkfscmd) {
			ptr += sprintf( ptr, FORMFS_MSG, voltxt );
			keyword[n++] = "f";
		}
	} else if(options & DM_FORMAT) {
		if((fmtcmd = getattr( device, "fmtcmd")) && *fmtcmd) {
			ptr += sprintf( ptr, FORMAT_MSG, voltxt );
			keyword[n++] = "f";
		}
	}
	/* Not supported 
	if(options & DM_WLABEL) {
		ptr += sprintf( ptr, WLABEL_MSG, voltxt );
		keyword[n++] = "w";
	}
	*/
	if(options & DM_OLABEL) {
		ptr += sprintf( ptr, OLABEL_MSG );
		keyword[n++] = "o";
	}
	ptr += sprintf( ptr, QUIT_MSG );
	keyword[n++] = NULL;

}

/* expand aliases and "dir" devices */
chkdevice( device, volname, type, buffer )
char *device, *volname, *type, *buffer;
{
	register char *ptr;
	register vsize = (volname? strlen( volname ): 0 );

	if( type && !(strncmp(type, "dir", 3)) ) {
		if( !vsize ) return( 0 );

		if( IS_ALIAS( device ) ) {

			if( ptr = getattr( device, "pathname" ) )
				if( strlen( ptr ) + strlen( volname ) < PATH_MAX - 1 )
					(void) sprintf( buffer, "%s/%s", ptr, volname );
				else return( 0 );
		} else
			(void) sprintf( buffer, "%s/%s", device, volname );

		return( 1 );
	}

	if( IS_ALIAS( device ) ) {
		if( (ptr = getattr( device, "cdevice" ) )
			|| (ptr = getattr( device, "bdevice" ) )
			|| (ptr = getattr( device, "pathname" ) ) ) {
			
			strcpy( buffer, ptr );
			return( 1 );
		}
			
		return( 0 );
	} 
	return( 1 );
}

extern GFILE *g_open();

bkgetlabel( device, dchar, result, result_sz )
char *device, *dchar, *result;
int result_sz;
{
	char *type, *ptr;
	long capacity = 0L;
	struct archive_info ai;
	GFILE	*f;
	register rc = 0;

	type = getattr( device, "type" );

	if( !(strncmp(type, "dpart", 5)) ) {

		if( ptr = getattr( device, "capacity" ) )
			capacity = atol( ptr );

		if( capacity > 0 )
			capacity = (capacity << 9);
	}

	if( (f = g_open( device, O_RDONLY, 0)) != NULL ) {

		if( br_read_hdr( &f, &ai, capacity, device ) > 0 ) {
			strncpy( result, ai.br_mname, result_sz );
			result[ (result_sz-1) ] = '\0';
			rc = 1;
		}

		(void) g_close( f );

	} else return( -1 );

	return( rc );
}
