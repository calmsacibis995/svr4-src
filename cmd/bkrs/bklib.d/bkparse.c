/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/bkparse.c	1.8.2.1"
#include <ctype.h>
#include <stdio.h>
#include <backup.h>
#include <string.h>

#define min( a, b ) ((a < b)? a: b)
#define max( a, b ) ((a > b)? a: b)

extern void free();
extern long strtol();
extern void bkr_setdemand();

typedef struct token_s {
	unsigned char *string;
	int value;
} token_t;

token_t daynames[] = {
	(unsigned char *)"sa", 6,
	(unsigned char *)"6", 6,
	(unsigned char *)"f", 5,
	(unsigned char *)"5", 5,
	(unsigned char *)"th", 4,
	(unsigned char *)"4", 4,
	(unsigned char *)"w", 3,
	(unsigned char *)"3", 3,
	(unsigned char *)"t", 2,
	(unsigned char *)"2", 2,
	(unsigned char *)"m", 1,
	(unsigned char *)"1", 1,
	(unsigned char *)"s", 0,
	(unsigned char *)"0", 0,
	(unsigned char *)"", 0 
};

unsigned char *
p_integer( string, value )
unsigned char *string;
int *value;
{
	unsigned char *ptr;
	if( !value ) return( 0 );
	
	*value = strtol( (char *) string, (char **) &ptr, 10 );
	return( (ptr == string)? (unsigned char *)0: ptr );
}

static
void
strlower( string )
unsigned char *string;
{
	for( ; *string; string++ )
		if( isupper( *string ) ) *string = _tolower( *string );
}

static
unsigned char *
p_scan( string, value, toktab )
unsigned char *string;
int *value;
token_t *toktab;
{
	unsigned char *ptr = (unsigned char *)strdup( (char *)string );
	register token_t *tptr;
	register size;
	strlower( ptr );
	for( tptr = toktab; *(tptr->string); tptr++ ) {
		size = strlen( (char *)tptr->string );
		if( !strncmp( (char *)ptr, (char *)tptr->string, size ) ) {
			*value = tptr->value;
			free( ptr );
			return( string + size );
		}
	}
	free( ptr );
	return( (unsigned char *)0 );
}

unsigned char *
p_day( string, value )
unsigned char *string;
int *value;
{
	return( p_scan( string, value, daynames ) );
}

unsigned char *
p_filename( string, filename )
unsigned char *string, *filename;
{
	if( !string || !(*string) || !filename ) return( (unsigned char *)0 );
	return( filename = string );
}

unsigned char *
p_range( string, begin, end )
unsigned char *string;
int *begin, *end;
{
	unsigned char *ptr;
	if( !(ptr = (unsigned char *)p_integer( string, begin ) ) )
			return( NULL );
	if( *ptr == '-' ) {
		ptr++;
		if( !(ptr = (unsigned char *)p_integer( ptr, end ) ) )
			return( NULL );
	} else *end = *begin;
	return( ptr );
}

unsigned char *
p_weekrange( string, begin, end )
unsigned char *string;
int *begin, *end;
{
	unsigned char *ptr;
	ptr = p_range( string, begin, end );
	if( *begin > 0 && *begin <= WK_PER_YR && *end > 0 && *end <= WK_PER_YR )
		return( ptr );
	else return( NULL );
}

unsigned char *
p_dayrange( string, begin, end )
unsigned char *string;
int *begin, *end;
{
	unsigned char *ptr;
	if( !(ptr = (unsigned char *)p_day( string, begin ) ) )
			return( NULL );
	if( *ptr == '-' ) {
		ptr++;
		if( !(ptr = (unsigned char *)p_day( ptr, end ) ) )
			return( NULL );
	} else *end = *begin;
	return( ptr );
}

unsigned char *
p_weekday( wk_str, day_str, bkdate, w1, w2, d1, d2 )
unsigned char *wk_str, *day_str;
bkrotate_t bkdate;
int *w1, *w2, *d1, *d2;
{
	unsigned char *ptr, days = 0;
	int i, day1, day2 = 0, week1, week2 = 0;
	/* Parse:
		<wk> [ '-' <wk> ] ':' { <daynum> | <day> } [ '-' { <daynum> | <day> } ]

		where:
			<wk> :: 1 to 52
			<daynum> :: 1 to 7
			<day> :: "S" | "M" | "T" | "W" | "Th" | "F"
	*/
	if( !(ptr = p_weekrange( wk_str, &week1, &week2 ) ) ) return( NULL );
	if( !(ptr = p_dayrange( day_str, &day1, &day2 ) ) ) return( NULL );

	/* Now record in data structure */
	for( i = day1; i <= day2; i++ )
		days |= (1<<i);
	for( i = week1 - 1; i < week2; i++ )
		bkdate[ i ] = days;
	if( w1 ) *w1 = week1;
	if( w2 ) *w2 = week2;
	if( d1 ) *d1 = day1;
	if( d2 ) *d2 = day2;
	return( ptr );
}

unsigned char *
p_weekday1( wk_str, day_str, bkdate )
unsigned char *wk_str, *day_str;
bkrotate_t bkdate;
{
	unsigned char *ptr, days = 0, calender[52];
	int i, day1, day2 = 0, week1, week2 = 0, some = FALSE;
	/* Parse:
		"demand"
		<WK> { ',' <WK> }+
			and
		<DAY> { ',' <DAY> }+

		where:
			<WK> :: <wk> [ '-' <wk> ] 
			<DAY> :: { <daynum> | <day> } [ '-' { <daynum> | <day> } ]
			<wk> :: 1 to 52
			<daynum> :: 1 to 7
			<day> :: "S" | "M" | "T" | "W" | "Th" | "F"
	*/
	/* Record weeks in "calender" array */
	(void) strncpy( (char *)calender, "", 52 );

	/* First check to see if "demand" is specified */
	strlower( wk_str );
	if( !strcmp( "demand", (char *)wk_str ) ) {
		bkr_setdemand( bkdate );
		return( wk_str + strlen( "demand" ) );
	}
	ptr = wk_str;
	while( ptr = p_weekrange( ptr, &week1, &week2 ) ) {
		some = TRUE;
		for( i = min( week1, week2 ) - 1; i < max( week1, week2 ); i++ ) 
			calender[ i ] = 1;
		if( !*ptr || *ptr != (unsigned char)',' ) break;
		ptr++;
	}
	if( !some ) return( NULL );
	else some = FALSE;

	/* record days in "days" */
	ptr = day_str;
	while( ptr = p_dayrange( ptr, &day1, &day2 ) ) {
		some = TRUE;
		/* Now record in data structure */
		for( i = day1; i <= day2; i++ )
			days |= (1<<i);
		if( !*ptr || *ptr != (unsigned char)',' ) break;
		ptr++;
	}
	if( !some ) return( NULL );

	for( i = 0; i < 52; i++ )
		if( calender[ i ] ) bkdate[ i ] = days;

	return( ptr );
}

unsigned char *
p_backup( wk_str, day_str, bkdate, w1, w2, d1, d2 )
unsigned char *wk_str, *day_str;
bkrotate_t bkdate;
int *w1, *w2, *d1, *d2;
{
	unsigned char *ptr, days = 0;
	int i, day1, day2 = 0, week1, week2 = 0;
	/* Parse:
		'demand' 
			or
		<wk> [ '-' <wk> ] ':' { <daynum> | <day> } [ '-' { <daynum> | <day> } ]

		where:
			<wk> :: 1 to 52
			<daynum> :: 1 to 7
			<day> :: "S" | "M" | "T" | "W" | "Th" | "F"
	*/
	/* First check to see if "demand" is specified */
	strlower( wk_str );
	if( !strcmp( "demand", (char *)wk_str ) ) {
		*w1 = *w2 = *d1 = *d2 = 0;
		bkr_setdemand( bkdate );
		return( wk_str + strlen( "demand" ) );
	}
		
	if( !(ptr = p_weekrange( wk_str, &week1, &week2 ) ) ) return( NULL );
	if( !(ptr = p_dayrange( day_str, &day1, &day2 ) ) ) return( NULL );

	/* Now record in data structure */
	for( i = day1; i <= day2; i++ )
		days |= (1<<i);
	for( i = week1 - 1; i < week2; i++ )
		bkdate[ i ] = days;
	if( w1 ) *w1 = week1;
	if( w2 ) *w2 = week2;
	if( d1 ) *d1 = day1;
	if( d2 ) *d2 = day2;
	return( ptr );
}

/* Copy argument string, replacing blanks by commas.  Return pointer to */
/* comma-separated string. */
unsigned char *
comma_sep( string )
char *string;
{
	register unsigned char *tmpstr, *ptr;

	tmpstr = ptr = (unsigned char *)strdup( string );
	while( ptr && *ptr )
		if( *ptr == ' ' ) {
			*ptr++ = ',';
			while( *ptr && *ptr++ == ' ' )
				;
		}
		else ptr++;
	return( tmpstr );
}
