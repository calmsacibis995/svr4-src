/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:mend.c	1.1"
#include "tam.h"
#include "menu.h"

/****************************************************************************

  mend(m)			- end a menu

****************************************************************************/

mend(m)
register menu_t *m;
{
  if (!(m->m_flags & M_USEWIN)) {
    (void)wdelete(m->m_win);
  }
}
