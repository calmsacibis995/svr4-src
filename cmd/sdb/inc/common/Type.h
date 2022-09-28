/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Type.h	1.3"
#ifndef Type_h
#define Type_h

enum Fund_type {
	ft_none,	// not a type representation
	ft_char,	// generic character
	ft_schar,	// signed character
	ft_uchar,	// unsigned character
	ft_short,	// generic short
	ft_sshort,	// signed short
	ft_ushort,	// unsigned short
	ft_int,		// generic integer
	ft_sint,	// signed integer
	ft_uint,	// unsigned integer
	ft_long,	// generic long
	ft_slong,	// signed long
	ft_ulong,	// unsigned long
	ft_pointer,	// untyped pointer, void *
	ft_sfloat,	// short float
	ft_lfloat,	// long float (double)
	ft_xfloat,	// extra long float (long double)
	ft_scomplex,	// Fortran complex
	ft_lcomplex,	// Fortran double precision complex
	ft_set,		// Pascal set
	ft_void,	// C void
};

#endif

// end of Type.h

