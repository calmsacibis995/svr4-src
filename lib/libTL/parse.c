/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libTL:parse.c	1.8.3.1"

#include <ctype.h>
#include <table.h>
#include <internal.h>

#ifdef WHITESP
/* I don't think WHITESPACE should be skipped */
#define SKIPWS(p,d)	\
	while( p && *p \
		&& *p != e->td_eoe && *p != e->td_comment && *p != td_fs \
		&& (*p == ' ' || *p == '\t') ) \
				p++
#else
#define	SKIPWS(p,d)
#endif

#define FSMSG	"FIELD SEPARATOR="
#define	ENTMSG	"ENTRY FORMAT="

#define	isoctal(c)	(isdigit((int)c) && c != '8' && c != '9')
#define ISWSPACE(c)	(c == ' ' || c == '\t')

#define	FOUND 1
#define	NOT_FOUND 0
#define P_ERROR -1

static unsigned char *inptr;
static int p_errno;
extern tbl_t TLtables[];
extern unsigned char TLinbuffer[];
char *strpbrk();

/* 
	Find either \\octal number or a char
*/
static
TLp_char()
{
	register tmp = 0, i;
	if( *inptr == '\\' ) {
		inptr++;
		if( isoctal( *inptr ) ) {
			for( i = 0; i < 3; i++ )
				if( *inptr && isoctal( *inptr ) ) {
					tmp = tmp * 8 + (int)*inptr;
					inptr++;
				} else break;
		} else return( *inptr++ );
	} else tmp = *inptr++;
	return( (char) tmp );
}

/*
	Like strpbrk(3C) but also returns ptr to NULL
*/
unsigned char *
TLstrpbrk( s1, s2 )
unsigned char *s1, *s2;
{
	register unsigned char *ptr;
	if( ptr = (unsigned char *)strpbrk( (char *)s1, (char *)s2 ) )
		return( ptr );
	return( s1 + Strlen(s1) );
}

/*
	get the next field, handling escapes. Return ptr to
	spot after field.
*/
static
unsigned char *
getfield( ptr, size, terminators )
unsigned char *ptr, *terminators;
int *size;
{
	register unsigned char *to = ptr, *t;
	register sz = 0, nescapes = 0;

	while( *ptr ) {

		/* skip escapes except for <esc><null> */
		if( *ptr == '\\' && *(++ptr) ) {

			/* Found an escape - skip it */
			nescapes++;

		} else for( t = terminators; *t; t++ ) {

			if( *t == *ptr )
				/* Found a terminator */
				goto gf_done;

		}

		/*
			If some escapes have been found,
			then squeeze the effective field
			string together.
		*/
		if( nescapes > 0 ) *to = *ptr;

		ptr++;
		to++;
		sz++;
	}

gf_done:
	*size = sz;
	return( ptr );
}

/*
	Parse:
		[ <field> <sep> ]* <sep>
*/
static
TLp_fields( tptr, pstruct, count_wspace )
tbl_t *tptr;
parse_t *pstruct;
int count_wspace;
{
	register unsigned char *tsave = inptr, *ptr;
	register field_t *fieldp = &(pstruct->fields);
	unsigned char terminators[ 10 ];
	int size, saw_fs = FALSE;

	fieldp->count = 0;
	(void)sprintf( (char *)terminators, "%c%c%s", tptr->description.td_fs,
		tptr->description.td_comment, (count_wspace? " \t": "") );

	while( TRUE ) {
		/*
			Look for next field - field is terminated by one of the 
			terminator characters. Handle escapes.
		*/
		ptr = (unsigned char *)getfield( inptr, &size, terminators );

		if( size || saw_fs || *ptr == tptr->description.td_fs ) {
			/* Found a field */
			if( fieldp->count == TL_MAXFIELDS ) {
				/* Too many fields */
				p_errno = TLBADFORMAT;
				return( P_ERROR );
			}
			/* Found a field - get a buffer for it */
			if( !(fieldp->values[ fieldp->count ] = 
				(unsigned char *)Malloc( size + 1 ) ) ) {
				inptr = tsave;
				p_errno = TLNOMEMORY;
				return( P_ERROR );
			}
			Strncpy( fieldp->values[ fieldp->count ], inptr, size );
			fieldp->values[ fieldp->count ][size] = '\0';
			fieldp->count++;
		} 
		if( !*ptr 
			|| *ptr == tptr->description.td_comment 
			|| isspace((int)*ptr) && !count_wspace ) {

			/* Field was not terminated by a field separator; ergo we're done */
			if( !fieldp->count ) {
				inptr = tsave;
				return( NOT_FOUND );
			} 
			inptr = ptr;
			return( FOUND );
		}
		/* Skip the separator */
		if( *ptr == tptr->description.td_fs ) {
			saw_fs = TRUE;
			ptr++;
		}
		inptr = ptr;
	}
}

/*
	parse "FIELD SEPARATOR=x"
	where:
		x is some character, or \\octal number
*/
/*ARGSUSED*/
static
TLp_fs( tptr, pstruct )
tbl_t *tptr;
parse_t *pstruct;
{
	register unsigned char *tsave = inptr;
	if( !Strncmp( inptr, FSMSG, Strlen( FSMSG ) ) ) {
		inptr += Strlen( FSMSG );
		if( pstruct->descr.td_fs = TLp_char() ) {
			pstruct->type |= (PT_COMMENT|PT_SPECIAL);
			return( FOUND );
		}
	}
	inptr = tsave;
	return( NOT_FOUND );
}

/*
	Parse "ENTRY FORMAT=[[^<sep>]+<sep>]+
*/
static
TLp_format( tptr, pstruct )
tbl_t *tptr;
parse_t *pstruct;
{
	register unsigned char *tsave = inptr;
	register rc = NOT_FOUND, countwspace;
	if( !Strncmp( inptr, ENTMSG, Strlen( ENTMSG ) ) ) {
		inptr += Strlen( ENTMSG );
		countwspace = ISWSPACE(tptr->description.td_fs)
			|| ISWSPACE(tptr->description.td_comment);
		switch( rc = TLp_fields( tptr, pstruct, countwspace ) ) {
		case FOUND:
			if( pstruct->fields.count == 0 ) {
				/* Disallow empty ENTRY FORMATs */
				inptr = tsave;
				p_errno = TLBADFORMAT;
				rc = P_ERROR;
			} else {
				pstruct->type |= (PT_COMMENT|PT_SPECIAL);
				rc = FOUND;
			}
			break;
		default:
			inptr = tsave;
		}
	}
	return( rc );
}

/*
	Parse a Special Self-describing comment.
*/
static
TLp_special( tptr, pstruct )
tbl_t *tptr;
parse_t *pstruct;
{
	register unsigned char *tsave = inptr;
	if( *inptr != tptr->description.td_comment )
		return( NOT_FOUND );
	inptr++;
	/* Two types of special comments: 
		> Field separator setting,
		> Entry format 
	*/
	while( TLp_fs( tptr, pstruct ) || TLp_format( tptr, pstruct ) ) 
		SKIPWS( inptr, tptr );

	if( !(pstruct->type & PT_SPECIAL) ) {
		/* This was not a SPECIAL comment */
		inptr = tsave;
		return( NOT_FOUND );
	} else return( FOUND );
}

/*
	Parse a Comment 
*/
static
TLp_comment( tptr, pstruct )
tbl_t *tptr;
parse_t *pstruct;
{
	register unsigned char *tsave = inptr;
	register size;
	if( *inptr != tptr->description.td_comment )
		return( NOT_FOUND );
	if( TLp_special( tptr, pstruct ) == NOT_FOUND )
		pstruct->type |= PT_COMMENT;
	/* Regardless of whether or not it is a SPECIAL comment, it is a comment,
		so record it. */
	inptr = tsave + 1;
	while( *inptr && *inptr != tptr->description.td_eoe ) inptr++;
	if( inptr != tsave + 1 ) {
		size = inptr - tsave + 1;
		if( !(pstruct->comment
			= (unsigned char *)Malloc( size + 5 ))) {
			/* Extra character malloc'd for strncpy's null */
			inptr = tsave;
			p_errno = TLNOMEMORY;
			return( P_ERROR );
		}
		Strncpy( pstruct->comment, tsave + 1, size );
		pstruct->comment[ size + 1 ] = '\0';
	} else pstruct->comment = 0;
	return( FOUND );
}

/*
	Parse an entry - this is some (possibly zero) number of fields followed
	by an optional comment;
*/
	
static
TLp_entry( tptr, pstruct )
tbl_t *tptr;
parse_t *pstruct;
{
	if( TLp_fields( tptr, pstruct, FALSE ) == P_ERROR ) return( P_ERROR );
	if( TLp_comment( tptr, pstruct ) == P_ERROR ) return( P_ERROR );
	return( FOUND );
}

/*
	TLbparse() - parse an entry that is currently in a buffer.
*/
int
TLbparse( tid, pstruct, buffer )
int tid;
parse_t *pstruct;
unsigned char *buffer;
{
	register tbl_t *tptr = TLtables + tid;
	Strncpy( pstruct, "", sizeof( parse_t ) );
	/* Assume buffer is null-terminated */
	inptr = buffer;
	SKIPWS( inptr, &(tptr->description) );
	if( TLp_entry( tptr, pstruct ) == P_ERROR )
		return( p_errno );
	return( TLOK );
}

/*
	TLparse() starts at where ever the file id currently points, gets an
	entry, and parses it. The parse results are placed in pstruct;
*/
int
TLparse( tid, pstruct, size )
int tid, *size;
parse_t *pstruct;
{
	register rc;
	/* Read next entry into max-sized buffer */
	if( (rc = TLfl_getentry( tid, TLinbuffer, 2 * TL_MAXLINESZ )) > 0 ) {
		*size = rc;
		return( TLbparse( tid, pstruct, TLinbuffer ) );
	}
	if( rc < 0 ) return( rc );
	return( TLOK );
}

int
TLp_iscomment( tid, ptr )
int tid;
unsigned char *ptr;
{
	register tbl_t *tptr = TLtables + tid;
	SKIPWS(ptr, &(tptr->description) );
	return( *ptr == tptr->description.td_comment );
}

/*
	Parse and fill in field names 
*/
int
TLp_fieldnames( tid, buffer )
int tid;
unsigned char *buffer;
{
	register tbl_t *tptr = TLtables + tid;
	parse_t pstruct;
	inptr = buffer;
	if( TLp_fields( tptr, &pstruct, FALSE ) != FOUND )
		return( FALSE );
	tptr->fieldnames = pstruct.fields;
	return( TRUE );
}
