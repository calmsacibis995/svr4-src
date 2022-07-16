/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/global.c	1.6"
#include "menu.h"

MENU _Default_Menu = {
  16,				/* height */
  1,				/* width */
  16,				/* rows */
  1,				/* cols */
  16,				/* frows */
  1,				/* fcols */
  0,				/* namelen */
  0,				/* desclen */
  1,				/* marklen */
  1,				/* itemlen */
  (char *) 0,			/* pattern */
  0,				/* pindex */
  (WINDOW *) 0,			/* win */
  (WINDOW *) 0,			/* sub */
  (WINDOW *) 0,			/* userwin */
  (WINDOW *) 0,			/* usersub */
  (ITEM **) 0,			/* items */
  0,				/* nitems */
  (ITEM *)0,			/* curitem */
  0,				/* toprow */
  ' ',				/* pad */
  A_STANDOUT,			/* fore */
  A_NORMAL,			/* back */
  A_UNDERLINE,			/* grey */
  (PTF_void) 0,			/* menuinit */
  (PTF_void) 0,			/* menuterm */
  (PTF_void) 0,			/* iteminit */
  (PTF_void) 0,			/* itemterm */
  (char *) 0,			/* userptr */
  "-",				/* mark */
  O_ONEVALUE|O_SHOWDESC|
  O_ROWMAJOR|O_IGNORECASE|
  O_SHOWMATCH|O_NONCYCLIC,	/* opt */
  0				/* status */
};

ITEM _Default_Item = {
  (char *)0,			/* name.str */
  0,				/* name.length */
  (char *)0,			/* description.str */
  0,				/* description.length */
  0,				/* index */
  0,				/* imenu */
  FALSE,			/* value */
  (char *)0,			/* userptr */
  O_SELECTABLE,			/* opt */
  0,				/* status */
  0,				/* y */
  0,				/* x */
  0,				/* up */
  0,				/* down */
  0,				/* left */
  0				/* right */
};
