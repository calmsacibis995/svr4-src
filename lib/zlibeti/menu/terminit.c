/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/terminit.c	1.1"
#include "private.h"

int
set_menu_init (m, mi)
register MENU *m;
PTF_void mi;
{
  if (m) {
    SMinit(m) = mi;
  }
  else {
    SMinit(Dfl_Menu) = mi;
  }
  return E_OK;
}

PTF_void
menu_init (m)
register MENU *m;
{
  return SMinit(m ? m : Dfl_Menu);
}

int
set_menu_term (m, mt)
register MENU *m;
PTF_void mt;
{
  if (m) {
    SMterm(m) = mt;
  }
  else {
    SMterm(Dfl_Menu) = mt;
  }
  return E_OK;
}

PTF_void
menu_term (m)
register MENU *m;
{
  return SMterm(m ? m : Dfl_Menu);
}

int
set_item_init (m, ii)
register MENU *m;
PTF_void ii;
{
  if (m) {
    SIinit(m) = ii;
  }
  else {
    SIinit(Dfl_Menu) = ii;
  }
  return E_OK;
}

PTF_void
item_init (m)
register MENU *m;
{
  return SIinit(m ? m : Dfl_Menu);
}

int
set_item_term (m, it)
register MENU *m;
PTF_void it;
{
  if (m) {
    SIterm(m) = it;
  }
  else {
    SIterm(Dfl_Menu) = it;
  }
  return E_OK;
}

PTF_void
item_term (m)
register MENU *m;
{
  return SIterm(m ? m : Dfl_Menu);
}
