/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libcmd:sum.c	1.1.1.1"

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

/*
 *	$Header: RCS/sum.c,v 1.4 88/04/26 05:56:26 root Exp $
 */
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <std.h>
#include <sum.h>
#define	MSW( l )	( ((l) >> 16) & 0x0000ffffL )
#define	LSW( l )	( (l) & 0x0000ffffL )

/***	sumpro -- prolog
 *
 */
sumpro( sip )
register struct suminfo *sip;
{
    sip->si_sum = sip->si_nbytes = 0L;
    return;
}

/***	sumupd -- update
 *
 */
sumupd( sip, buf, cnt )
struct suminfo *sip;
register char *buf;
register int cnt;
{
    register long sum;

    if ( cnt <= 0 )
	return;
    sip->si_nbytes += cnt;
    sum = sip->si_sum;
    while ( cnt-- > 0 )
	sum += *buf++ & 0x00ff;
    sip->si_sum = sum;
    return;
}

/***	sumepi -- epilog
 *
 */
sumepi( sip )
register struct suminfo *sip;
{
    register long sum;

    sum = sip->si_sum;
    sum = LSW( sum ) + MSW( sum );
    sip->si_sum = (ushort) (LSW( sum ) + MSW( sum ));
    return;
}

/***	sumout -- output
 *
 */
sumout( fp, sip )
FILE *fp;
struct suminfo *sip;
{
#ifdef	M_V7
#define	FMT	"%u\t%D"
#else
#define	FMT	"%u\t%ld"
#endif
    fprintf(
	fp, FMT,
	(unsigned) sip->si_sum,
	(sip->si_nbytes + MULBSIZE - 1) / MULBSIZE
    );
#undef	FMT
    return;
}

#ifdef	BLACKBOX
char	*Pgm		= "tstsum";
char	Enoopen[]	= "cannot open %s\n";
char	Ebadread[]	= "read error on %s\n";

int
main( argc, argv )
int argc;
char **argv;
{
    char *pn;
    FILE *fp;
    int cnt;
    struct suminfo si;
    char buf[BUFSIZ];

    --argc; ++argv;
    for ( ; argc > 0; --argc, ++argv ) {
	pn = *argv;
	if ( (fp = fopen( pn, "r" )) == NULL ) {
	    error( Enoopen, pn );
	    continue;
	}
	sumpro( &si );
	while ( (cnt = fread( buf, sizeof(char), BUFSIZ, fp )) != 0 && cnt != EOF )
	    sumupd( &si, buf, cnt );
	if ( cnt == EOF && ferror( fp ) )
	    error( Ebadread, pn );
	sumepi( &si );
	sumout( stdout, &si );
	printf( "\t%s\n", pn );
	fclose( fp );
    }
    exit( 0 );
}

error( fmt, a1, a2, a3, a4, a5 )
char *fmt;
{
    fprintf( stderr, "%s: ", Pgm );
    fprintf( stderr, fmt, a1, a2, a3, a4, a5 );
    return;
}
#endif
