/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sgs-inc:common/syms.h	1.14"
/*
 */
#ifndef __SYMS_H
#define __SYMS_H

/*		Storage Classes are defined in storclass.h  */
#include "storclass.h"

/*		Number of characters in a symbol name */
#define  SYMNMLEN	8
/*		Number of characters in a file name */
#define  FILNMLEN	14
/*		Number of array dimensions in auxiliary entry */
#define  DIMNUM		4


struct syment
{
	union
	{
		char		_n_name[SYMNMLEN];	/* old COFF version */
		struct
		{
			long	_n_zeroes;	/* new == 0 */
			long	_n_offset;	/* offset into string table */
		} _n_n;
		char		*_n_nptr[2];	/* allows for overlaying */
	} _n;
#ifndef pdp11
	unsigned
#endif
	long			n_value;	/* value of symbol */
	short			n_scnum;	/* section number */
	unsigned short		n_type;		/* type and derived type */
	char			n_sclass;	/* storage class */
	char			n_numaux;	/* number of aux. entries */
};

#define n_name		_n._n_name
#define n_nptr		_n._n_nptr[1]
#define n_zeroes	_n._n_n._n_zeroes
#define n_offset	_n._n_n._n_offset

/*
   Relocatable symbols have a section number of the
   section in which they are defined.  Otherwise, section
   numbers have the following meanings:
*/
        /* undefined symbol */
#define  N_UNDEF	0
        /* value of symbol is absolute */
#define  N_ABS		-1
        /* special debugging symbol -- value of symbol is meaningless */
#define  N_DEBUG	-2
	/* indicates symbol needs transfer vector (preload) */
#define  N_TV		(unsigned short)-3

	/* indicates symbol needs transfer vector (postload) */

#define  P_TV		(unsigned short)-4

/*
   The fundamental type of a symbol packed into the low 
   4 bits of the word.
*/

#define  _EF	".ef"

#define  T_VOID     0          /* void */ 
#define  T_NULL     0
#define  T_LDOUBLE  1          /* long double */	
#define  T_ARG      1          /* function argument (only used by compiler) */
#define  T_CHAR     2          /* character */
#define  T_SHORT    3          /* short integer */
#define  T_INT      4          /* integer */
#define  T_LONG     5          /* long integer */
#define  T_FLOAT    6          /* floating point */
#define  T_DOUBLE   7          /* double word */
#define  T_STRUCT   8          /* structure  */
#define  T_UNION    9          /* union  */
#define  T_ENUM     10         /* enumeration  */
#define  T_MOE      11         /* member of enumeration */
#define  T_UCHAR    12         /* unsigned character */
#define  T_USHORT   13         /* unsigned short */
#define  T_UINT     14         /* unsigned integer */
#define  T_ULONG    15         /* unsigned long */

/*
 * derived types are:
 */

#define  DT_NON      0          /* no derived type */
#define  DT_PTR      1          /* pointer */
#define  DT_FCN      2          /* function */
#define  DT_ARY      3          /* array */

/*
 *   type packing constants
 */

#define  N_BTMASK     017
#define  N_TMASK      060
#define  N_TMASK1     0300
#define  N_TMASK2     0360
#define  N_BTSHFT     4
#define  N_TSHIFT     2

/*
 *   MACROS
 */

	/*   Basic Type of  x   */

#define  BTYPE(x)  ((x) & N_BTMASK)

	/*   Is  x  a  pointer ?   */

#define  ISPTR(x)  (((x) & N_TMASK) == (DT_PTR << N_BTSHFT))

	/*   Is  x  a  function ?  */

#define  ISFCN(x)  (((x) & N_TMASK) == (DT_FCN << N_BTSHFT))

	/*   Is  x  an  array ?   */

#define  ISARY(x)  (((x) & N_TMASK) == (DT_ARY << N_BTSHFT))

	/* Is x a structure, union, or enumeration TAG? */

#define ISTAG(x)  ((x)==C_STRTAG || (x)==C_UNTAG || (x)==C_ENTAG)

#define  INCREF(x) ((((x)&~N_BTMASK)<<N_TSHIFT)|(DT_PTR<<N_BTSHFT)|(x&N_BTMASK))

#define  DECREF(x) ((((x)>>N_TSHIFT)&~N_BTMASK)|((x)&N_BTMASK))

/*
 *	AUXILIARY ENTRY FORMAT
 */

union auxent
{
	struct
	{
		long		x_tagndx;	/* str, un, or enum tag indx */
		union
		{
			struct
			{
				unsigned short	x_lnno;	/* declaration line number */
				unsigned short	x_size;	/* str, union, array size */
			} x_lnsz;
			long	x_fsize;	/* size of function */
		} x_misc;
		union
		{
			struct			/* if ISFCN, tag, or .bb */
			{
				long	x_lnnoptr;	/* ptr to fcn line # */
				long	x_endndx;	/* entry ndx past block end */
			} 	x_fcn;
			struct			/* if ISARY, up to 4 dimen. */
			{
				unsigned short	x_dimen[DIMNUM];
			} 	x_ary;
		}		x_fcnary;
		unsigned short  x_tvndx;		/* tv index */
	} 	x_sym;
	struct
	{
		char	x_fname[FILNMLEN];
	} 	x_file;
        struct
        {
                long    x_scnlen;          /* section length */
                unsigned short  x_nreloc;  /* number of relocation entries */
                unsigned short  x_nlinno;  /* number of line numbers */
        }       x_scn;

	struct
	{
		long		x_tvfill;	/* tv fill value */
		unsigned short	x_tvlen;	/* length of .tv */
		unsigned short	x_tvran[2];	/* tv range */
	}	x_tv;	/* info about .tv section (in auxent of symbol .tv)) */
	struct
	{
		unsigned short	x_type;		/* type of the aux. entry */
						/* == SHDW_AUX */
		short		x_scnum;	/* number of the shadow */
						/* section, containing the */
						/* data for the symbol */
		long		x_vaddr;	/* virtual address of symbol */
						/* within the shadow section */
		char		x_outsec[8];	/* name of the output section */
						/* the symbol is to go into */
		unsigned short	x_flags;	/* reserved */
	}	x_shdw;
};

/* auxiliary entry types (presently only 1) */
/* needed for symbols with multiple auxiliary entries */
#define	SHDW_AUX	01

#define	SYMENT	struct syment
#define	SYMESZ	18	/* sizeof(SYMENT) */

#define	AUXENT	union auxent
#define	AUXESZ	18	/* sizeof(AUXENT) */

/*	Defines for "special" symbols   */

#define _ROSTART        "_rostart"
#define _ROEND  "_roend"

#define _ETEXT	"_etext"
#define	_EDATA	"_edata"
#define	_END	"_end"

#define _START	"_start"

#define _TVORIG	"_tvorig"
#define _TORIGIN	"_torigin"
#define _DORIGIN	"_dorigin"

#define _SORIGIN	"_sorigin"
#endif
