/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cg:i386/sdb.h	1.1"
/*	sdb.h -  header file for debugger.
 *	i386 CG
 *		Intel iAPX386
*/

/* values for n_dtype */
#define	S_GSYM	0x01		/* global sym: name,type,0 */
#define	S_STSYM	0x02		/* static symbol: name,type,address */
#define	S_LCSYM	0x03		/* .lcomm symbol: name,type,address */
#define	S_RSYM	0x04		/* register sym: name,register,offset */
#define	S_LSYM	0x05		/* local sym: name,type,offset */
#define	S_PSYM	0x06		/* parameter: name,type,offset */
#define	S_BSTR	0x07		/* begin structure: name,, */
#define	S_ESTR	0x08		/* end structure: name,, */
#define	S_SSYM	0x09		/* structure elt: name,type,struct_offset */
#define	S_BENUM	0x0a		/* begin enum: name,, */
#define	S_EENUM	0x0b		/* end enum: name,, */
#define	S_ENUM	0x0c		/* enum element: name,,value */
#define	S_BCOMM	0x0d		/* begin common: name,, */
#define	S_ECOMM	0x0e		/* end common: name,, */
#define	S_ECOML	0x0f		/* end common (local name): ,,address */
#define	S_LBRAC	0x10		/* left bracket: ,nesting level,address */
#define	S_RBRAC	0x11		/* right bracket: ,nesting level,address */
#define	S_BFUN	0x12		/* procedure: name,linenumber,address */
#define	S_ENTRY	0x13		/* alternate entry: name,linenumber,address */
#define	S_SLINE	0x14		/* src line: ,linenumber,address */
#define	S_SO	0x15		/* source file name: name,,address */
#define	S_SOL	0x16		/* #line source filename: name,,address */
#define	S_FNAME	0x17		/* procedure name (f77 kludge): name,,0 */
#define	S_LENG	0x18		/* additional entry with length: ,,length */
#define	S_EFUN	0x19		/* end of function: name,, */
#define S_NARGS 0x20		/* number of args in the function call */
#define S_TYID	0x21		/* follows STRTY, etc: name,, */
#define S_DIM   0x22		/* ary dimension */

#define S_MAX   S_DIM
#define STABTYPE 0xff

