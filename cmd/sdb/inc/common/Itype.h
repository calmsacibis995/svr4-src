/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Itype.h	1.5"
/*
 * NAME
 *	Itype.h -- internal representations of subject types
 *
 * ABSTRACT
 *	Each subject type known to the debugger is represented
 *	internally by one of the types specified in this file.
 *
 * DESCRIPTION
 *	For each subject type known to the debugger (each of the
 *	entries in the Stype enum) there is an internal
 *	representation given in the machine-dependent section
 *	below. The Stype member has a leading 'S'. The corresponding
 *	internal type has a leading 'I'. For simplicity in the
 *	interfaces to things like read routines, a union of
 *	all of the internal types, Itype, is defined.
 *	Each target type is mapped into the most appropriate internal
 *	representation available in the debugger
 *
 *	When porting the debugger, note that some of the internal
 *	types may have to be implemented as classes on your machine.
 *	Most likely candidates are the base and offset entries.
 *
 * USED BY
 *
 *	All portions of the debugger which need to deal with target
 *	data.
 */

#ifndef Itype_h
#define Itype_h

enum Stype {
	SINVALID,	/* reserve a saving value */
	Schar,
	Sint1,
	Sint2,
	Sint4,
	Suchar,
	Suint1,
	Suint2,
	Suint4,
	Ssfloat,
	Sdfloat,
	Sxfloat,
	Saddr,
	Sbase,
	Soffset
} ;

typedef char		Ichar;	/* signed target char */
typedef	char		Iint1;	/* signed 1 byte int */
typedef	short		Iint2;	/* signed 2 byte int */
typedef	long		Iint4;	/* signed 4 byte int */

typedef unsigned char	Iuchar;	/* unsigned target char */
typedef	unsigned char	Iuint1;	/* unsigned 1 byte int */
typedef	unsigned short	Iuint2;	/* unsigned 2 byte int */
typedef	unsigned long	Iuint4;	/* unsigned 4 byte int */

typedef float		Isfloat; /* ANSI single prec. floating pt */
typedef double		Idfloat; /* ANSI double precision floating pt */
typedef double		Ixfloat; /* MAU extended precision floating pt */

#ifndef utility_h
typedef unsigned long	Iaddr;	/* holds a target address */
#endif
typedef unsigned long	Ibase;	/* holds a target segment base */
typedef unsigned long	Ioffset; /* holds a target segment base */

union Itype {
	Ichar	ichar;
	Iint1	iint1;
	Iint2	iint2;
	Iint4	iint4;
	Iuchar	iuchar;
	Iuint1	iuint1;
	Iuint2	iuint2;
	Iuint4	iuint4;
	Isfloat	isfloat;
	Idfloat	idfloat;
	Ixfloat	ixfloat;
	Iaddr	iaddr;
	Ibase	ibase;
	Ioffset	ioffset;
	char	rawbytes[16];	/* for use by things which NEED TO KNOW!!!!! */
	int	rawwords[4];	/* ditto */
} ;

#endif /* Itype_h */
