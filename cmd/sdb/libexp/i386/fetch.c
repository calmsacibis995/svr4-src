/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libexp/i386/fetch.c	1.1"
#include "Itype.h"

/* fetch_double -- extract a double from a float, double, or extended float */

int fetch_buf[3];
double fetch_dbl;

double
fetch_double( unsigned long *p, int size )
{
	switch ( size ) {
	case 4:		/* float */
		fetch_dbl = *(float *) p;
		break;
	case 8:		/* double */
		fetch_dbl = *(double *) p;
		break;
	case 12:	/* MAU extended double */
		fetch_buf[0] = *p++;
		fetch_buf[1] = *p++;
		fetch_buf[2] = *p;
		asm("	fldt	fetch_buf");
		asm("	fstpl	fetch_dbl");
		break;
	default:
		printf("bad size %d in fetch_double!\n", size);
	}

	return fetch_dbl;
}

/* convert_to_fp -- convert float or double to MAU extended float */

int
convert_to_fp( unsigned char *raw, int raw_size, enum Stype *stype, char *out )
{
	switch( raw_size ) {
	case 4:		/* float */
		fetch_dbl = *(float *) raw;
		goto convert;
	case 8:		/* double */
		fetch_dbl = *(double *) raw;
convert:	asm("	fldl	fetch_dbl");
		asm("	fstpt	fetch_buf");
		memcpy( out, fetch_buf, 12 );
		if ( stype ) *stype = Sxfloat;
		break;
	case 12:	/* MAU extended double */
		memcpy( out, raw, 12 );
		if ( stype ) *stype = Sxfloat;
		break;
	default:
		printf("bad size %d in convert_to_fp()\n", raw_size);
		return 0;
	}
	return 1;	/* success */
}
