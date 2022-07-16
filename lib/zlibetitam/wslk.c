/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:wslk.c	1.3"
#include "cvttam.h"

static char *
copy(from, to, ln)
register char *from, *to;
int ln;				/* Length of copy */
{
  register int i;

  if (from && *from) {
    for (i=ln; i--;) {
      if (!(*from)) {
	*to++ = ' ';
      }
      else {
	*to++ = *from++;
      }
    }
  }
  *to = '\0';
  return (from);
}

static char *
copykey (from, to, keynum)
char *from, *to;
int keynum;
{
  register i;
  char junk[5];

  i = (keynum==3 || keynum==5) ? 4 : 1;
  from = copy (from, junk, i);
  return (copy (from, to, LNKEYS));
}

TAMwslk (wn, kn, slong, sshort, sextra)	/* write slk label */
/*
 * kn=0 is a special case
 * in this case <slong,sshort> are 2 WTXTLEN lines containing long slk labels
 * and <sextra> is 1 WTXTLEN line containing short slk labels
 */
short wn;
short kn;		/* key # (1-8) */
char *slong;		/* long label (16 chars) */
char *sshort;		/* short label (8 chars) */
char *sextra;
{
  register i, j;
  register char *from, *to;
  TAMWIN *tw;

  if (tw = _validwindow(wn)) {
    if (kn >=0 && kn <= NFKEYS) {
      if (kn) {				/* single slk */
	from = NumSlkLines==1 ? sshort : slong;
	to = Slk0(tw, kn-1);
	from = copy (from, to, LNKEYS);
	if (!(NumSlkLines==1)) {
	  to = Slk1(tw, kn-1);
	  from = copy (from, to, LNKEYS);
	}
      }
      else {
	if (NumSlkLines==1) {			/* whole slk lines */
	  from = sextra;
	  for (j=0;j<NFKEYS; j++) {
	    from = copykey (from, Slk0(tw, j), j);
	  }
	}
	else {
	  from = slong;
	  for (j=0;j<NFKEYS; j++) {
	    from = copykey (from, Slk0(tw, j), j);
	  }
	  from = sshort;
	  for (j=0;j<NFKEYS; j++) {
	    from = copykey (from, Slk1(tw, j), j);
	  }
	}
      }
      _post (tw);
      (void)doupdate ();
      return (OK);
    }
  }
  return (ERR);
}
