/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/menuopts.c	1.9"
#include "private.h"

int
set_menu_opts (m, opt)
register MENU *m;
int opt;
{
  register ITEM **ip;

  if (m) {
    if (Posted(m)) {
      return E_POSTED;
    }
    /* Check to see if the ROWMAJOR option is changing.  If so, */
    /* set top and current to 0. */
    if ((opt & O_ROWMAJOR) != RowMajor(m)) {
      Top(m) = 0;
      Current(m) = IthItem(m, 0);
      (void)set_menu_format (m, FRows(m), FCols(m));
    }
    /* if O_NONCYCLIC option changed, set bit to re-link items */
    if ((opt & O_NONCYCLIC) != (Mopt(m) & O_NONCYCLIC)) {
    	SetLink(m);
    }
    Mopt(m) = opt;
    if (OneValue(m) && Items(m)) {
      for (ip=Items(m); *ip; ip++) {
	Value(*ip) = FALSE;	/* Unset values if selection not allowed. */
      }
    }
    _scale (m);		/* Redo sizing information */
  }
  else {
    Mopt(Dfl_Menu) = opt;
  }
  return E_OK;
}

int
menu_opts_off (m, opt)
register MENU *m;
register OPTIONS opt;
{
  return set_menu_opts (m, (Mopt(m ? m : Dfl_Menu)) & ~opt);
}

int
menu_opts_on (m, opt)
register MENU *m;
register OPTIONS opt;
{
  return set_menu_opts (m, (Mopt(m ? m : Dfl_Menu)) | opt);
}

OPTIONS
menu_opts(m)
register MENU *m;
{
  return Mopt(m ? m : Dfl_Menu);
}
