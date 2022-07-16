/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:menu.c	1.1"
#include "tam.h"
#include "menu.h"
#include "wind.h"
#include "message.h"

/******************************************************************************

  int menu(menu, op)
  menu_t *menu;
  int op;
  
  Perform operation "op" on the specified menu.  See the include file
  for a list of the operations and the structure of the menu.  menu()
  returns 8-bit (positive) keystroke codes when the operation was
  terminated due to a typed character and negative MERR codes (see
  menu.h) when the operation is terminated for some other reason.

******************************************************************************/

int
menu(m, op)
register menu_t		*m;
int			op;
{
  int ret = MERR_OK;
  mitem_t *mi;


  if (op > M_POPUP) {
    message (MT_ERROR,0,0,"Menu - Invalid operation: %d", op);
    return (MERR_ARGS);
  }

  /* Dispatch on the operation. */

  for (mi=m->m_items ; mi->mi_name ; mi++) {
    if (strlen(mi->mi_name) > 69) {	/* to prevent refreshing*/
      *(mi->mi_name+69) = 0;		/* truncate if too wide */
    }

    if ((op & M_DESEL) || (m->m_flags & M_SINGLE)) {
      mi->mi_flags &= ~M_MARKED;
    }
  }
  if (op & M_BEGIN) {
    if ((ret = mbegin(m)) < 0) {
      return(ret);
    }
  }
  if (op & M_INPUT) {
    ret = minput(m);
  }
  if (op & M_END) {
    mend(m);
  }


  /*	Finally, exit menu	*/

  return(ret);
}
