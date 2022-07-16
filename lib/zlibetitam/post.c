/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:post.c	1.1"
#include "cvttam.h"

/* Update the command and prompt lines, but only if either the old and */
/* new strings are nonblank. */

static void
updatenoise (w, str, blank)
WINDOW *w;
char *str;
int *blank;
{
  int changed = FALSE;

  if (!(*blank)) {
    (void)werase (w);
    changed = TRUE;
    *blank = TRUE;
  }
  if (str) {
    (void)mvwaddstr (w, 0, 0, str);
    changed = TRUE;
    *blank = FALSE;
  }
  if (changed) {
    (void)wnoutrefresh (w);
  }
}

/* Before displaying the labels check if they fit within the */
/* given space.  If not, remove lead blanks before cutting the */
/* size of the label. */

static char *
fitlabel (w, s)
WINDOW *w;
char *s;
{
  int keylen;		/* Length of soft label keys. */
  int h;		/* Dummy height for getmaxyx */
  register j;

  getmaxyx (w, h, keylen);
  for (j=LNKEYS+1-keylen; j--;) {
    if (*s != ' ') {
      break;
    }
    s += 1;
  }
  return (s);
}

/* This routine draws the TAMWIN on top of all others and upates the */
/* prompt, cmd and slk windows. */

void
_post (tw)
TAMWIN *tw;
{
  register i;
  register changed;

  /* If this TAMWIN has a cmd, prompt or slk window associated */
  /* with it then display them. */

  if (tw && CurrentWin == tw) {
    updatenoise (PromptWindow.window, Prompt(tw), &BlankPrompt);
    updatenoise (CmdWindow.window, Cmd(tw), &BlankCmd);
    for (i=NFKEYS; i--;) {
      changed = FALSE;
      if (!BlankSlk(i)) {
	if (!Slk0(tw,i) ||  Slk0Char(tw,i,0) == '\0') {
	  (void)werase (SlkWin(i));
	  changed = TRUE;
	}
	BlankSlk(i) = TRUE;
      }
      wattron (SlkWin(i), A_REVERSE);
      if (Slk0(tw,i) && Slk0Char(tw,i,0) != '\0') {
	(void)mvwaddstr (SlkWin(i),0,0, fitlabel(SlkWin(i), Slk0(tw, i)));
	changed = TRUE;
	BlankSlk(i) = FALSE;
      }
      if (Slk1(tw,i) && Slk0Char(tw,i,0) != '\0') {
	(void)mvwaddstr (SlkWin(i),1,0, fitlabel(SlkWin(i), Slk1(tw, i)));
	changed = TRUE;
	BlankSlk(i) = FALSE;
      }
      wattroff (SlkWin(i), A_REVERSE);
      if (changed) {
	(void)wnoutrefresh (SlkWin(i));
      }
    }
  }
}
