/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:i386/instr.c	1.2"


#include "symbols.h"
#include "symbols2.h"
#include "instr.h"
#include <stdio.h>

instr *insthashtab[NINSTRS];	/* hash table for instructions */

	static unsigned long
hashfn(p,nhash)
	register char *p;
	unsigned long nhash;
{
	register unsigned long ihash = 0;
	while (*p)
		ihash = ihash*4 + *p++;
	ihash += *--p * 32;
	return(ihash % nhash);
}
		
	instr *
instlookup(sptr)
/* search for instruction name in insthashtab[] */
register char *sptr;
{
  register instr *p;
  for (p = insthashtab[hashfn(sptr,(unsigned long)NINSTRS)]; p!=NULL; p=p->next)
    if (strcmp(p,sptr)==0)
      return(p);
  return(NULL);
}

	void
mk_hashtab()
{
  extern instr instab[];
  register instr *ip, **hp;
  
  /* First insert mnemonics into instruction hash table. */
  for (ip = instab; ip->name[0] != '\0'; ++ip)
    {
      hp = &insthashtab[hashfn(ip->name,(unsigned long)NINSTRS)];
      ip->next = *hp;	
      *hp = ip;
    }
}
