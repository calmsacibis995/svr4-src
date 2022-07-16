/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbplot:libplot/hp7221/subr.c	1.1.3.1"

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved. The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */


#include "hp7221.h"

putMBP( x, y )
    int		x,	y;
{
    int		chr;

    chr = ( x >> 10 ) & 017;
    chr|= 0140;
    putchar( chr );
    chr = ( x >> 4 ) & 077;
    if ( chr < 32 ) {
	chr += 64;
    }
    putchar( chr );
    chr = ( y >> 12 ) & 03;
    chr|= ( x << 2  ) & 071;
    if ( chr < 32 ) {
	chr += 64;
    }
    putchar( chr );
    chr = ( y >> 6 ) & 077;
    if ( chr < 32 ) {
	chr += 64;
    }
    putchar( chr );
    chr = ( y ) & 077;
    if ( chr < 32 ) {
	chr += 64;
    }
    putchar( chr );
    return;
}

putMBN( i )
    int		i;
{
    int		chr;

    chr = ( i>>12 ) & 07;
    chr|= 0140;
    putchar( chr );
    chr = ( i>>6 ) & 077;
    if ( chr < 32 ) {
	chr += 64;
    }
    putchar( chr );
    chr = i & 077;
    if ( chr < 32 ) {
	chr += 64;
    }
    putchar( chr );
    return;
}

putSBN( i )
    int		i;
{
    i &= 077;
    if ( i < 32 ) {
	i += 64;
    }
    putchar( i );
    return;
}
