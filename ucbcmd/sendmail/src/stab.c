/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbsendmail:src/stab.c	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
*/

# include "sendmail.h"

SCCSID(@(#)stab.c 1.7 88/02/08 SMI); /* from UCB 5.2 6/7/85 */

/*
**  STAB -- manage the symbol table
**
**	Parameters:
**		name -- the name to be looked up or inserted.
**		type -- the type of symbol.
**		op -- what to do:
**			ST_ENTER -- enter the name if not
**				already present.
**			ST_FIND -- find it only.
**
**	Returns:
**		pointer to a STAB entry for this name.
**		NULL if not found and not entered.
**
**	Side Effects:
**		can update the symbol table.
*/

# define STABSIZE	400

static STAB	*SymTab[STABSIZE];

STAB *
stab(name, type, op)
	char *name;
	int type;
	int op;
{
	register STAB *s;
	register STAB **ps;
	extern bool sameword();
	register int hfunc;
	register char *p;
	extern char lower();

# ifdef DEBUG
	if (tTd(36, 5))
		printf("STAB: %s %d ", name, type);
# endif DEBUG

	/*
	**  Compute the hashing function
	**
	**	We could probably do better....
	*/

	hfunc = type;
	for (p = name; *p != '\0'; p++)
		hfunc = (((hfunc << 7) | lower(*p)) & 077777) % STABSIZE;

# ifdef DEBUG
	if (tTd(36, 9))
		printf("(hfunc=%d) ", hfunc);
# endif DEBUG

	ps = &SymTab[hfunc];
	while ((s = *ps) != NULL && (!sameword(name, s->s_name) || s->s_type != type))
		ps = &s->s_next;

	/*
	**  Dispose of the entry.
	*/

	if (s != NULL || op == ST_FIND)
	{
# ifdef DEBUG
		if (tTd(36, 5))
		{
			if (s == NULL)
				printf("not found\n");
			else
			{
				long *lp = (long *) s->s_class;

				printf("type %d val %lx %lx %lx %lx\n",
					s->s_type, lp[0], lp[1], lp[2], lp[3]);
			}
		}
# endif DEBUG
		return (s);
	}

	/*
	**  Make a new entry and link it in.
	*/

# ifdef DEBUG
	if (tTd(36, 5))
		printf("entered\n");
# endif DEBUG

	/* make new entry */
	s = (STAB *) xalloc(sizeof *s);
	bzero((char *) s, sizeof *s);
	s->s_name = newstr(name);
	makelower(s->s_name);
	s->s_type = type;

	/* link it in */
	*ps = s;

	return (s);
}
