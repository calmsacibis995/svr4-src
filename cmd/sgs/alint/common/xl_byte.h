/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/xl_byte.h	1.1"

extern void xl_init();
extern unsigned long xl_xlate();
extern FLENS * xl_t_flens();
extern ATYPE * xl_t_atype();
extern LINE * xl_t_line();
extern union rec * xl_t_rec();

/* The following bit values tell the xl_[wr] routines what type to translate
** when it is faced with a union.  The problem arises when trying to transpose
** fields in a union such as union rec:  
**
**	union rec {
**		LINE l;
**		struct {
**			short decflag;
**			char *fn;
**		} f;
**	}
**
** Within the LINE structure, we have further clashes.  Or'ing together
** the following bits can tell any of the xl_t functions which type of
** a union to translate.
*/

#define XL_TY	01	/* ATYPE.extra.ty */
#define XL_POS	02	/* ATYPE.extra.pos */
#define XL_LINE 04	/* rec.l */
#define XL_F	010	/* rec.f */

extern int xl_swap;	/* true if host and target have different byte orders */

#define XL_XLATE(var) xl_swap ? xl_xlate((unsigned long) (var), sizeof(var)) : var
