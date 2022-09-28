/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)acomp:common/tblmgr.h	52.3"
/* tblmgr.h */

/* Definitions for table management code. */

#if 0				/* Use CG definitions. */
struct td {
    SIZE td_allo;		/* array elements allocated */
    SIZE td_used;		/* array elements in use */
    SIZE td_size;		/* sizeof(array element) */
    I16 td_flags;		/* flags word */
    myVOID * td_start;		/*  pointer to array start */
    char * td_name;		/* table name for error */
#ifdef STATSOUT
    SIZE td_max;		/* maximum value reached */
#endif
};

/* flags for td_flags */
#define TD_MALLOC 	1	/* array was malloc'ed */
#define TD_ZERO		2	/* zero expansion area */

SIZE td_enlarge();		/* enlarges a table so described, returns
				** old size
				*/

#endif /* 0 */

/* These macros handle dynamic tables, as follows:
**	TABLE		defines a table with these parameters:
**		tab	name of table (descriptor)
**		class	storage class for the initially allocated table
**		intab	name of initial table
**		type	type of each table element
**		allo	initial allocation of table elements
**		flags	initial flags
**		name	name string for table (for errors)
**
**	TD_NEED1	declares a need for one more table element
**	TD_NEED		declares a need for n table elements
**	TD_USED		returns l-value for number of elements used
**	TD_ELEM		returns an l-value for the selected table element
**			number
**	TD_CHKMAX	compare used against max, update max
*/
#define	TD_USED(tbl) (tbl.td_used)
/* extra cast to myVOID * quiets lint */
#define	TD_ELEM(tbl,type,elem) (((type *)(myVOID *)(tbl.td_start))[elem])
#define	TD_NEED1(tbl)   if (tbl.td_used >= tbl.td_allo) td_enlarge(&tbl,0)
#define	TD_NEED(tbl, n) if (tbl.td_used + (n) > tbl.td_allo) \
				td_enlarge(&tbl, n)
#ifdef	STATSOUT
#define	TD_CHKMAX(tbl)	if (tbl.td_used > tbl.td_max) tbl.td_max = tbl.td_used
#else
#define	TD_CHKMAX(tbl)	/* empty */
#endif

/* Want to be able to get to table descriptors if generating
** statistics.  Otherwise they're usually static.
*/
#ifdef	STATSOUT
#define	Static
#else
#define Static static
#endif

#define TABLE(tdclass, tab, iclass, intab, type, allo, flags, name) \
iclass type intab[allo]; \
tdclass struct td tab = { \
	allo,	/* allocation */ \
	0,	/* used always 0 */ \
	sizeof(type),	/* entry size */ \
	flags,	/* flags */ \
	(myVOID *)intab, /* pointer */ \
	name	/* table name */ \
}
