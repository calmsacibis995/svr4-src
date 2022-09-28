/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/bkrotate.c	1.4.2.1"

#include <stdio.h>
#include <backup.h>
#include <table.h>

extern char *strcpy();

static unsigned char *d_names[] = {
	(unsigned char *) "S",
	(unsigned char *) "M",
	(unsigned char *) "T",
	(unsigned char *) "W",
	(unsigned char *) "Th",
	(unsigned char *) "F",
	(unsigned char *) "S"
};
	
/* Operations on bkrotate_t's */
/* bkr_and(): c = a & b */
int
bkr_and( c, a, b )
bkrotate_t a, b, c;
{
	register accum = 0, i;
	for( i = 0; i < WK_PER_YR; i++ ) {
		c[ i ] = a[ i ] & b[ i ];
		accum |= c[ i ];
	}
	return( accum );
}
		
/* bkr_or(): c = a | b */
int
bkr_or( c, a, b )
bkrotate_t a, b, c;
{
	register accum = 0, i;
	for( i = 0; i < WK_PER_YR; i++ ) {
		c[ i ] = a[ i ] | b[ i ];
		accum |= c[ i ];
	}
	return( accum );
}
		
int
bkr_equal( a, b )
bkrotate_t a, b;
{
	register i;
	for( i = 0; i < WK_PER_YR; i++ ) {
		if( a[ i ] != b[ i ] ) return( FALSE );
	}
	return( TRUE );
}

/* set all bits in a that are found in b */
void
bkr_set( a, b )
bkrotate_t a, b;
{
	register i;
	for( i = 0; i <= WK_PER_YR; i++ ) 
		a[ i ] |= b[ i ];
}

/* unset all bits in a that are found in b */
void
bkr_unset( a, b )
bkrotate_t a, b;
{
	register i;
	for( i = 0; i <= WK_PER_YR; i++ ) 
		a[ i ] &= ~(b[ i ]);
}

static 
unsigned char *
pr_one( ptr, w1, w2, day )
unsigned char *ptr, day;
int w1, w2;
{
	register i, d1, d2, rc, first = TRUE;
	unsigned char sep = ':';

	if( !day ) return( ptr );

	/* Print weeks */
	if( (rc = sprintf( (char *)ptr, "%d", w1 + 1 )) < 0 ) return( ptr );
	else ptr += rc;
	if( w1 != w2 )
		if( (rc = sprintf( (char *)ptr, "-%d", w2 + 1 )) < 0 ) return( ptr );
		else ptr += rc;

	for( i = 0; i < 7; ) {
		if( !(day & (1<<i)) ) {
			i++;
			continue;
		}
		
		/* Print days */
		for( d2 = d1 = i; i < 7; i++ )
			if( !(day & (1<<i)) ) {
				d2 = i - 1;
				break;
			}
		if( i == 7 ) d2 = 6;
		if( first ) first = FALSE;
		else sep = ',';
		if( (rc = sprintf( (char *)ptr, "%c%s", sep, d_names[ d1 ] )) < 0 )
			return( ptr );
		else ptr += rc;
		if( d1 != d2 )
			if( (rc = sprintf( (char *)ptr, "-%s", d_names[ d2 ])) < 0 )
				return( ptr );
			else ptr += rc;
	}
	return( ptr );
}

/* Print out a bkrotate_t into:
	<range>|<range> ... <range>
	where:
		<range> :== <week> '-' <week> ':' <day> '-' <day>
*/
unsigned char *
bkr_print( a )
bkrotate_t a;
{
	register i, wk1, wk2, first = 1;
	static unsigned char buffer[ 512 ];
	unsigned char *ptr = buffer;

	if( IS_DEMAND( a ) ) {
		(void) strcpy( (char *) buffer, "demand" );
		return( buffer );
	}

	*ptr = '\0';
	i = 0;
	while( i < WK_PER_YR ) {
		if( !a[ i ] ) {
			i++;
			continue;
		}
		for( wk2 = wk1 = i; i < WK_PER_YR; i++ ) {
			if( a[ wk1 ] != a[ i ] ) {
				wk2 = i - 1;
				break;
			}
		}
		if( i >= WK_PER_YR ) wk2 = WK_PER_YR - 1;
		if( first ) first = 0;
		else *ptr++ = '|';
		ptr = pr_one( ptr, wk1, wk2, a[ wk1 ] );
	}
	return( buffer );
}

int
bkr_empty( a )
bkrotate_t a;
{
	register i;
	for( i = 0; i <= WK_PER_YR; i++ )
		if( a[ i ] ) return( FALSE );
	return( TRUE );
}

void
bkr_copy( to, from )
bkrotate_t to, from;
{
	register i;
	for( i = 0; i <= WK_PER_YR; i++ )
		to[ i ] = from[ i ];
}

void
bkr_init( a )
bkrotate_t a;
{
	register i;
	for( i = 0; i <= WK_PER_YR; i++ )
		a[ i ] = 0;
}

/* Set bits for whole period */
void
bkr_setall( a )
bkrotate_t a;
{
	register i;
	for( i = 0; i < WK_PER_YR; i++ )
		a[ i ] = ( SUNDAY_F | MONDAY_F | TUESDAY_F | WEDNESDAY_F
			| THURSDAY_F | FRIDAY_F | SATURDAY_F );
}

/* Set the bits to indicate a DEMAND backup */
void
bkr_setdemand( a )
bkrotate_t a;
{
	a[ WK_PER_YR ] = 1;
}

/* Set a particular week/day */
void
bkr_setdate( a, w, d )
bkrotate_t a;
int w, d;
{
	if( w > WK_PER_YR || w < 0 || d < 0 || d > 6 ) return;
	a[ w ] |= 1<<d;
}
