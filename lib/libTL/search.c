/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libTL:search.c	1.6.3.1"
#include <setjmp.h>
#include <table.h>
#include <internal.h>

extern jmp_buf TLenv;

#define INIT	register char *sp = instring; extern char *TLenvp;
#define GETC()	(*sp++)
#define	PEEKC()	(*sp)
#define	UNGETC(c)	(--sp)
#define	RETURN(c)	return;
#define	ERROR(c)	longjmp( TLenv, 0 )

#include <regexp.h>

#define	ATOI(s,r)	{ \
	unsigned char *ptr;	\
	r = (long)strtol( (char *)s, (char **)&ptr, 0L ); \
	if( ptr == s ) return( FALSE ); \
	}

extern tbl_t TLtables[];
extern unsigned char	TLgenbuf[];
long	strtol();

static int
TLs_strmatch( string, pattern )
unsigned char *string, *pattern;
{
	unsigned char	TLexpbuf[ TL_MAXLINESZ ];

	/* Come back to this point if regexp error */
	if( setjmp( TLenv ) ) return( FALSE );

	(void) compile( (char *)pattern, (char *)TLexpbuf,
		(char *)&TLexpbuf[TL_MAXLINESZ], '\0' );

	return( TLsearch( string, TLexpbuf ) );
}

/*ARGSUSED*/
static int
TLs_op( tid, fieldvalue, sptr )
int tid;
unsigned char *fieldvalue;
TLsearch_t *sptr;
{
	register rc, p_value, f_value, op;
	op = (int)(sptr->ts_operation);
	switch( op ) {
	case TLEQ:
		rc = !Strcmp( fieldvalue, sptr->ts_pattern );
		break;

	case TLNE:
		rc = Strcmp( fieldvalue, sptr->ts_pattern );
		break;

	case TLLT:
		rc = (Strcmp( fieldvalue, sptr->ts_pattern ) < 0);
		break;

	case TLGT:
		rc = (Strcmp( fieldvalue, sptr->ts_pattern ) > 0);
		break;

	case TLLE:
		rc = (Strcmp( fieldvalue, sptr->ts_pattern ) <= 0 );
		break;

	case TLGE:
		rc = (Strcmp( fieldvalue, sptr->ts_pattern ) >= 0 );
		break;

	case TLMATCH:
		rc = TLs_strmatch( fieldvalue, sptr->ts_pattern );
		break;

	case TLNMATCH:
		rc = !TLs_strmatch( fieldvalue, sptr->ts_pattern );
		break;

	case TLNUMEQ:
		ATOI( fieldvalue, f_value )
		ATOI( sptr->ts_pattern, p_value )
		rc = (f_value == p_value);
		break;

	case TLNUMNE:
		ATOI( fieldvalue, f_value )
		ATOI( sptr->ts_pattern, p_value )
		rc = (f_value != p_value);
		break;

	case TLNUMLT:
		ATOI( fieldvalue, f_value )
		ATOI( sptr->ts_pattern, p_value )
		rc = (f_value < p_value);
		break;

	case TLNUMGT:
		ATOI( fieldvalue, f_value )
		ATOI( sptr->ts_pattern, p_value )
		rc = (f_value > p_value);
		break;

	case TLNUMLE:
		ATOI( fieldvalue, f_value )
		ATOI( sptr->ts_pattern, p_value )
		rc = (f_value <= p_value);
		break;

	case TLNUMGE:
		ATOI( fieldvalue, f_value )
		ATOI( sptr->ts_pattern, p_value )
		rc = (f_value >= p_value);
		break;

	default:
		rc = (*(sptr->ts_operation))( sptr->ts_fieldname, fieldvalue,
			sptr->ts_pattern );
		break;
	}
	return( rc );
}

static int
TLs_match( tid, entry, sarray, how_to_match )
int tid, how_to_match;
entry_t *entry;
TLsearch_t *sarray;
{
	register tbl_t	*tptr = TLtables + tid;
	register TLsearch_t *sptr;
	register unsigned char *fptr;
	register accum = (how_to_match == TL_AND || how_to_match == TL_NOR ), rc;
	register offset;

	for( sptr = sarray; sptr->ts_fieldname; sptr++ ) {
		/* Get pointer to string to match against */
		if( !Strcmp( sptr->ts_fieldname, TLCOMMENT ) ) {
			if( !E_NFIELDS(entry) ) fptr = E_COMMENT(entry);
			else return( FALSE );
		} else if( !Strcmp( sptr->ts_fieldname, TLTRAILING ) )
			fptr = E_COMMENT(entry);
		else if( (offset = TLf_find( &(tptr->fieldnames), sptr->ts_fieldname ))
			== -1 )
			return( FALSE );
		else fptr = *E_GETFIELD(entry, offset);
		
		rc = TLs_op( tid, fptr, sptr );
		switch( how_to_match ) {
		case TL_AND:
			accum = accum && rc;
			if( !accum ) return( FALSE );
			break;

		case TL_OR:
			accum = accum || rc;
			if( accum ) return( TRUE );
			break;

		case TL_NAND:
			accum = accum || !rc;
			if( accum ) return( TRUE );
			break;

		case TL_NOR:
			accum = accum && !rc;
			if( !accum ) return( FALSE );
			break;
		}
	}
	return( accum );
}

int
TLs_search( tid, sarray, first, last, how_to_match )
int tid, how_to_match;
TLsearch_t *sarray;
entryno_t first, last;
{
	register tbl_t *tptr = TLtables + tid;
	register TLsearch_t *sptr;
	register entryno_t dot, direction = (first < last), done = FALSE;
	register rc;
	entry_t *entry;
	
	for( sptr = sarray; sptr->ts_fieldname; sptr++ )
		if( Strcmp( sptr->ts_fieldname, TLCOMMENT )
			&& Strcmp( sptr->ts_fieldname, TLTRAILING ) 
			&& TLf_find( &(tptr->fieldnames), sptr->ts_fieldname ) == -1 )
			return( TLBADFIELD );

	dot = first;
	while( !done ) {
		/* Get the current entry */
		if( (rc = TLe_getentry( tid, dot, &entry )) != TLOK )
			return( rc );

		if( TLs_match( tid, entry, sarray, how_to_match ) )
			return( dot );
		if( direction ) {
			dot++;
			if( dot > last ) done = TRUE;
		} else {
			dot--;
			if( dot < last ) done = TRUE;
		}
	}
	return( TLFAILED );
}

static unsigned char *
TLplace( sp, l1, l2, limit )
register unsigned char *sp, *l1, *l2, *limit;
{

	while (l1 < l2) {
		*sp++ = *l1++;
		if( sp >= limit ) return( 0 );
	}
	return( (unsigned char *)sp );
}

/*
	Search for pattern in string; replace with replacetxt and leave
	the result in TLgenbuf;
*/
static int
TLsubstitute( string, pattern, replacetxt )
unsigned char *string, *pattern, *replacetxt;
{
	if( !TLs_strmatch( string, pattern ) ) return( 0 );
	return( TLreplace( string, replacetxt ) );
}

static int
TLreplace( string, text )
unsigned char *string, *text;
{
	register unsigned char *lp, *sp, *rp, *limit;
	unsigned char c, buf[2];
	register offset;

	lp = (unsigned char *)string;
	sp = TLgenbuf;
	rp = (unsigned char *)text;
	limit = (unsigned char *)(TLgenbuf + TL_MAXLINESZ);

	/* Copy text from before match (loc1 points at 1st char of matched text) */
	while (lp < (unsigned char *)loc1)
		*sp++ = *lp++;

	/* Copy Matched portion */
	while( c = *rp++ ) {
		if( c == '&' ) {
			if( !(sp = TLplace( sp, (unsigned char *)loc1, (unsigned char *)loc2,
				limit ) ) ) 
				return( 0 ); 
			continue;
		} else if( c == '\\' ) {
			buf[0] = c = *rp++;
			buf[1] = '\0';
			offset = atoi( (char *)buf );
			if( offset > 0 && offset <= nbra ) {
				sp = TLplace( sp, (unsigned char *)braslist[ offset - 1 ],
					(unsigned char *)braelist[ offset - 1 ], limit );
				if( !sp ) return( 0 );
				continue;
			}
		}
		*sp++ = c;
		if( sp >= limit )
			return( 0 );
	}

	/* Copy portion AFTER matched text */
	lp = (unsigned char *)loc2;
	while( *sp++ = *lp++ )
		if( sp >= limit )
			return( 0 );
	return( 1 );
}

static
TLsearch( string, expbuf )
unsigned char *string, *expbuf;
{
	register c;

	/* Reset Braced expression list */
	for( c = 0; c < NBRA; c++ ) {
		braslist[c] = 0;
		braelist[c] = 0;
	}
	locs = 0;
	return( step( (char *)string, (char *)expbuf ) );
}

int
TLs_replace( tid, entry, fieldname, pattern, replacetxt )
int tid;
entry_t *entry;
unsigned char *fieldname, *pattern, *replacetxt;
{
	register tbl_t *tptr = TLtables + tid;
	register offset;
	unsigned char buf[1], *fieldvalue;
	if( !Strcmp( fieldname, TLCOMMENT ) ) {
		if( !E_NFIELDS(entry) ) fieldvalue = E_COMMENT(entry);
		else return( TLFAILED );
	} else if( !Strcmp( fieldname, TLTRAILING ) )
		fieldvalue = E_COMMENT(entry);
	else if( (offset = TLf_find( &(tptr->fieldnames), fieldname ) ) == -1 )
		return( TLBADFIELD );
	else fieldvalue = *E_GETFIELD(entry, offset);

	if( !replacetxt ) {
		buf[0] = '\0';
		replacetxt = buf;
	}
	if( !TLsubstitute( fieldvalue, pattern, replacetxt ) )
		return( TLFAILED );

	/* Now 'new' field value is in TLgenbuf */
	return( TLf_assign( tid, entry, fieldname, TLgenbuf ) );
}
