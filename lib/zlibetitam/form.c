/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:form.c	1.2"
#include "message.h"
#include "tam.h"
#include "tamwin.h"
#include "form.h"
#include "wind.h"
#include "kcodes.h"
#include <string.h>

char insert_mode = 1;
int f_begx;
int f_begy;

int form_page;
int max_row;

/****************************************************************************

  ret = form(form, op)
  form_t *form;
  int op;

  Perform operation "op" on the specified form.  See the include file
  for a list of the operations and the structure of the form.  form()
  returns 8-bit (positive) keystroke codes when the operation was
  terminated due to a typed character and negative FERR codes (see
  form.h) when the operation is terminated for some other reason.

****************************************************************************/

field_t *(fleft()), *(fright()), *(fup()), *(fdown());

int
form(f, op)
register form_t *f;
int op;
{
  register field_t *fl;
  static	short width;
  static	short height;
  short 	i;
  int 	err = FERR_OK;

  /* check for valid op code */

  if (op > F_POPUP) {
    (void)message(MT_ERROR,0,0,"Form - Invalid operation: %d", op);
    return (FERR_ARGS);
  }

  if (op == F_END)
    goto fleave;


  /* First compute the maximum width and height of the form. */
  /* The width is computed from the column + length of the right-most */
  /* field in the form.  The height is computed from the highest row */
  /* number.  */

  height = width = 0;

  for (fl=f->f_fields ; fl->fl_name ; fl++) {
    i = fl->fl_fcol + fl->fl_len;
    if (i > width) {
      width = i;
    }

    if (fl->fl_row > height) {
      height = fl->fl_row;
    }
  }

  max_row = height;

  /*  Adjust for margins and titles.  If there is a title on this form,
    increase the height by 2 to account for it.  Also, since rows are
    0-relative, increase height by 1 to get a count, not an offset.  */

  width += 2*F_LRMARGIN;
  height += 2*F_TBMARGIN + 1;
  if (f->f_name) {
    height += 2;
  }

  /*  Increase height by 2 to accomodate OK patch */

  height += 2;

  /*  Make sure the title fits.. */

  if (f->f_name && strlen(f->f_name) > width) {
    err = FERR_ARGS;
    goto fleave;
  }

  /*  Make sure form is not too big  */

  if (width > COLS) {
    (void)message(MT_ERROR,0,0,"Form too wide, width= %d screen max= %d",
	    width, COLS);
    err = FERR_TOOBIG;
    goto fleave;
  }

  if (height > LINES) {
    (void)message(MT_ERROR,0,0,"Form too high, height= %d screen max= %d",
	    height, LINES);
    err = FERR_TOOBIG;
    goto fleave;
  }

  err = form_2(f, op);

fleave:

  if ((op & F_END) && (f->f_win >= 0)) {
    if (!(f->f_flags & F_USEWIN)) {
      (void)wdelete(f->f_win);
    }
  }

  return(err);
}

int
form_2(f, op)
register form_t *f;
int op;
{
  register field_t *fl;
  register field_t *curfl;
  short	i;
  int	err = FERR_OK;
  int	w = -1;
  int	key;
  int	tkval;

  static	short nfields;
  static	int okrow, okcol;
  static	short width, height;

  if (op == F_END) {
    goto f_2leave;
  }

  /*  First compute the maximum width and height of the form.  The width is
    computed from the column + length of the right-most field in the form.
    The height is computed from the highest row number.  */

  height = width = 0;

  for (nfields=0, fl=f->f_fields ; fl->fl_name ; fl++, nfields++) {
    i = fl->fl_fcol + fl->fl_len;
    if (i > width) {
      width = i;
    }

    if (fl->fl_row > height) {
      height = fl->fl_row;
    }
  }

  /*  Adjust for margins and titles.  If there is a title on this form,
    increase the height by 2 to account for it.  Also, since rows are
    0-relative, increase height by 1 to get a count, not an offset.  */

  width += 2*F_LRMARGIN;
  height += 2*F_TBMARGIN + 1;
  if (f->f_name) {
    height += 2;
  }

  /*  Increase height by 2 to accomodate OK patch, remember it in okrow  */

  okrow = height;
  height += 2;
  okcol = (width - 4) /2;		/* 4 is len of [OK]  */

  /*  Make sure the title fits.. */

  if (f->f_name && strlen(f->f_name) > width) {
    err = FERR_ARGS;
    goto f_2leave;
  }

  if (op & F_BEGIN) {
    /*  If USEWIN is set, just alter it. */

    if (f->f_flags & F_USEWIN) {
      WSTAT ws;
      w = f->f_win;
      f->f_track = 0;		/* force a new track to be computed */
      if (wgetstat((short)w,&ws) < 0) {
        return(FERR_GETSTAT);
      }
      ws.uflags = F_BORDFLAGS;

      if (ws.width != width || ws.height != height) {

	/*  Now that we have a guess at ws.begx and ws.begy, try to fit it.
	If we cannot, slide towards the upper left until we hit 1,0 in case of
	bit-map and 0,0 in case of remote terminal.
	The '1' is to keep off the status line.  */

        while (ws.begx >= 0 && ws.begy >= 0) {
          if (ws.begy + height > F_MAXHEIGHT) {
            ws.begy--;
	  }
          else if (ws.begx + width > F_MAXWIDTH) {
            ws.begx--;
	  }
          else {
	    goto haveit;
	  }
        }
        return(FERR_BIG);

haveit:
        ws.width = width;
        ws.height = height;

        if (wsetstat((short)w,(WSTAT *)&ws) < 0) {
          return(FERR_SETSTAT);
        }
      }
    }
    else {
      if (f->f_flags & F_WINSON) {
	i = W_SON;
      }
      else if (f->f_flags & F_WINNEW) {
	i = W_NEW;
      }
      else {
	i = W_POPUP;
      }

      w = wind(i, height, width, 0, 0);
      f->f_win = w;
    }
  }
  else {
    w = f->f_win;
  }

  if (w < 0) {
    err = FERR_NOWIN;
    goto f_2leave;
  }
  else {
    WSTAT ws;
    if (wgetstat((short)w, &ws) < 0) {
      return(FERR_GETSTAT);
    }
    else {		/* save to use in procedure field */
      f_begy = ws.begy;
      f_begx = ws.begx;
    }
  }

  /*  Display the title, if present, centered.  Also display the label.  */

  if (f->f_name && (op & F_BEGIN)) {
    (void)wgoto(w,F_TBMARGIN,0);
    for (i=1; i<=width; i++) {
      (void)wputc(w,' ');	/* clear our space */
    }
    (void)wgoto(w,F_TBMARGIN, (width-strlen(f->f_name))/2);
    (void)attron (A_DIM);
    (void)wputs(w, f->f_name);
    (void)attroff (A_DIM);
  }

  if (f->f_label && (op & F_BEGIN)) {
    (void)wlabel(w,f->f_label);
  }

  /*  Display the OK patch at bottom center in reverse video           */

  if (op & F_BEGIN) {
    (void)wgoto(w, okrow, okcol);
    (void)attron (A_REVERSE);  /* set reverse video */
    (void)wputs(w, "[OK]");
    (void)attroff (A_REVERSE);/* normal attribute  */
  }

  /*  Allocate space for the track list if we don't have one already.  */

    /*  Now go through each field, calling field with the no flret to display
    each field of the form without doing any input.  */

    /* Draw all field names outside procedure field for faster response	*/

    for (fl=f->f_fields ; fl->fl_name ; fl++) {
      (void)wgoto(w,
	    (fl->fl_row) + (f->f_name==0 ? F_TBMARGIN : F_TBMARGIN+2),
	    fl->fl_ncol + F_LRMARGIN);
      (void)wputs(w, fl->fl_name);

      if (fl->fl_flags & F_MONLY) {
	(void)strcpy(fl->fl_value, fl->fl_menu->m_curi->mi_name);
      }
    }
    for (fl=f->f_fields ; fl->fl_name ; fl++) {
      err = field(w, f, fl, (f->f_name!=0), 0, 0, 0, okrow, okcol, width);
      if (err) {
        goto f_2leave;
      }
    }

  if (!(op & F_INPUT)) {
    goto f_2leave;
  }

  /* This is the main loop of form. Call field on the current field and parse
   the return value and keystroke. The fourth arg to field is the back flag
   which determines whether the Back key terminates the field (true) or is
   ignored (false).  We set the back flag only if there is a field to the
   left of the current one.  */

  while (1) {
    curfl = f->f_curfl;
    tkval= curfl - f->f_fields;
    key = field(w, f, curfl, (f->f_name!=0),(fleft(f,curfl)!=curfl),
                1, &tkval, okrow, okcol, width);

    if (key < 0) {
      err = key;
      goto f_2leave;
    }

    /*  Dispatch on the key.  */

    if ((f->f_flags & F_RDONLY) &&
	(key != Page && key != s_Page && key != Mouse)) {
      err = key;
      goto f_2leave;
    }

    switch (key) {
      case FERR_OK: {
        continue;
      }

      case Return:
      case Tab:
      case Next: {
        if (curfl < &f->f_fields[nfields-1]) {
          curfl++;
	}
        else {
          curfl = f->f_fields;
	}
        break;
      }

      case BackTab:
      case Prev: {
        if (curfl > f->f_fields) {
          curfl--;
	}
        else {
          curfl = &f->f_fields[nfields-1];
	}
        break;
      }

      case Beg:
      case Home: {
        curfl = f->f_fields;
        break;
      }

      case End: {
        curfl = &f->f_fields[nfields-1];
        break;
      }

      case Forward:	curfl = fright(f, curfl); break;
      case Back:	curfl = fleft(f, curfl); break;
      case Up: {
        curfl = fup(f, curfl);
	break;
      }
      case Down: {
        curfl = fdown(f, curfl);
	break;
      }
      case Mouse: {
        if (f->f_flags & F_RDONLY) {	/* read_only protected form */
          continue;
	}
        else {
          curfl = &f->f_fields[tkval];
          break;
        }
      }
      case Page:
      case s_Page: {
        if (form_page > 1) {	/* multi-part */
          err = key;
          goto f_2leave;
        }
        else {
          break;
	}
      }
      case Enter:
      case s_Exit:
      default: {
        err = key;
        goto f_2leave;
      }
    }
    f->f_curfl = curfl;
  }

f_2leave:

  return(err);
}

/****************************************************************************

  key/err = FIELD(w, f, fl, tf, bf, doin, &tkval, okr, okc, width)
    - display/accept a single field

  w is the window number, fl is a field structure, doin is true if
  the field should be accepted, false if it should just be displayed.
  Returns key (>=0) or err (<0) and a complete field structure.  If
  tf is true, the fields are displayed 2 lines down.  If bf (read back
  flag) is false, then the Back key does not return from the field.
  okr and okc are the row and column value of OK patch, respectively.
  width is the form window width (for *OT label use).

****************************************************************************/

/*ARGSUSED*/
int
field(w, f, fl, tf, bf, doin, pval, okr, okc, width)
int w;
form_t *f;
register field_t *fl;
char tf,bf;
char doin;
int *pval;
int okr, okc;
short width;
{
  register short pos;
  register short i;
  short vlen;
  short row;
  char redisp;
  char *val = fl->fl_value;
  int key;
  char monly;
  static char first = 1;
  char firstch = 1;
  char read_form;


  /* First, compute the corresponding track item, and position and */
  /* display the name.  */

  i = fl - f->f_fields;

  row = fl->fl_row + F_TBMARGIN;
  if (tf) {
    row += 2;
  }

  /* Set the initial cursor position to the end of default text and set */
  /* the redisp flag.  */

  vlen = strlen(val);
  pos = 0;
  firstch = redisp = 1;
  read_form = (f->f_flags & F_RDONLY);	/* if read-only form */

  /* Set up monly flag              */

  monly = fl->fl_flags & F_MONLY;

  /* This is the main loop of field.  If redisp is true, re-draw */
  /* the entire field (except the name, which never changes). Then */
  /* get input.  The loop is a little tricky since the doin flag */
  /* controls whether the loop returns after drawing the field, */
  /* everyone who wants to return simply clears doin and continues */
  /* the while(1).  At the top of the loop, if doin is false, the */
  /* string can be redisplayed (if redisp) and then we exit with key. */
  /* This way, on exit, the field is redrawn in normal video. */

  key = FERR_OK;

  while (1) {
    vlen = strlen(val);

    if (redisp) {
      redisp = 0;
      (void)wgoto(w, row, fl->fl_fcol + F_LRMARGIN);
      if (doin && !read_form) {
	(void)attron (A_REVERSE);
      }
      (void)wputs(w, val);
      for (i=vlen ; i<fl->fl_len ; i++) {
        (void)wputc(w, ' ');
      }
      
      if (doin) {
        (void)wgoto(w, row, fl->fl_fcol + F_LRMARGIN);
	(void)attroff (A_REVERSE);
        if (read_form) {
          (void)wprompt(w, "Read-only protected form, cannot input data");
	}
        else {
          (void)wprompt(w, fl->fl_prompt);
	}
      }
    }

    (void)wgoto(w, row,fl->fl_fcol + F_LRMARGIN + pos);

    if (!doin) {
      (void)wprompt(w,"");
      return(key);
    }
    key = wgetc (w);
    if (key < 0) {
      return(FERR_SYS);	/* SOMETHING WRONG */
    }

    /* read only form, return key. Mouse still needs to do */

    if (read_form) {
      doin = 0;
      continue;
    }

    switch (key) {
      case Backspace: {
        if (monly) {
          key = flmenu(fl);
	}
        else {
          if (pos > 0 ) {
            for (i=pos ; i<=vlen ; i++) {
              val[i-1] = val[i];
	    }
            pos--;
          }
        }
        redisp = 1;
        break;
      }
      case Back: {
	if (monly) {
	  register field_t *tempfl;
	  tempfl = fleft (f, fl);
	  /* if no field on left, display menu */
	  if (tempfl == fl) {
	    key = flmenu (fl);
	  }
	  else {
	    doin = 0;
	  }
        }
	else {
	  if (pos > 0) {
	    pos--;
	  }
	  else if (bf) {
	    doin = 0;
	  }
	}
	redisp = 1;
	break;
      }

      case Forward: {
	if (monly) {
	  register field_t *tempfl;
	  tempfl = fright (f, fl);
	  /* if no field on right, display menu */
	  if (tempfl  == fl) {
	    key = flmenu (fl);
	  }
	  else {
	    doin = 0;
	  }
	}
	else {
	  if (pos < vlen) {
	    pos++;
	  }
	  else {
	    doin = 0;
	  }
	}
	redisp = 1;
	break;
      }

      case ClearLine: {
        if (monly) {
          key = flmenu(fl);
        }
        else {
          val[ (pos=0) ] = '\0';
          for (i=pos+1; i<=fl->fl_len ; i++) {
            val[i] = '\0';
	  }
        }
        redisp = 1;
        break;
      }

      case DleteChar: {
        if (monly) {
          key = flmenu(fl);
	}
        else {
          if (pos < vlen) {
            for (i=pos ; i<vlen ; i++) {
              val[i] = val[i+1];
	    }
          }
        }
        redisp = 1;
        break;
      }

      case InputMode: {
        insert_mode = !insert_mode;
        disp_ins(w, f->f_label, width);
        break;
      }

      /* If Cmd or Opts is pressed and this is field has an associated */
      /* menu, go off and display the menu.  Note that if the menu returns */
      /* due to cancel, we do not update the field's current content and */
      /* "lie" saying the field was terminated with the inoccuous Return */
      /* key. If we returned with Cancel, the caller (form) would think */
      /* the form was cancelled.  */

      case Cmd:
      case Opts: {
        if (read_form) {	/* protected form */
          doin = 0;
          continue;
        }
        if (fl->fl_menu) {
          key = flmenu(fl);
          redisp = 1;
        }
        else {
          doin = 0;	/* return */
        }
        continue;
      }

      /* If Slect or Mark is pressed and the form has an associated menu, */
      /* step to the next choice in the menu.  */

      case Slect:
      case Mark: {
        if (read_form) {	/* protected form */
          doin = 0;
          continue;
        }
        if (fl->fl_menu) {
          register mitem_t *mi;
          mi = fl->fl_menu->m_curi;
	  if (mi->mi_name) {
	    mi++;
	    if (!mi->mi_name) {
	      mi = fl->fl_menu->m_items;
	    }
	    fl->fl_menu->m_curi = mi;
	    (void)strcpy(fl->fl_value, mi->mi_name);
	    redisp = 1;
	  }
          key = FERR_OK;
        }
        break;
      }

      case FERR_SYS:
      case FERR_WRITE: {
        doin = 0;
        continue;
      }
        
      default: {
        if (key < ' ' || key > '~') {
          redisp = 1;
          doin = 0;
          continue;
        }

        if ((fl->fl_flags & F_CLEARIT) && first) {
          first = 0;
          val[ (pos=0) ] = '\0';
          vlen = 0;
          redisp = 1;
        }

        if (monly) {
          key = flmenu(fl);
          redisp = 1;
          continue;
        }
        if (firstch) {
          val[ (pos=0) ] = key;
          pos = redisp = 1;
          val[ pos ] = '\0';
        }
        else {
	  /* do overtype or insert mode	*/
	  /* depending on insert_mode	*/
	  /* This flag toggled by Input_Mode */

	  if (insert_mode) {
	    if (vlen < fl->fl_len) {
	      for (i=vlen+1 ; i>pos ; i--) {
		val[i] = val[i-1];
	      }
	      val[pos++] = key;
	      redisp = 1;
	    }
	    else {
	      continue;
	    }
	  }
          else {	/* overtype-mode */
            if (pos < fl->fl_len) {
              val[pos++] = key;
              redisp = 1;
            }
          }
        }
      }
    }
    firstch = 0;
  }
}



/******************************************************************************

  ret = FLMENU(fl)	- do the associated menu

******************************************************************************/

int
flmenu(fl)
register field_t *fl;
{
  int ret;
  int op;
  char tmpname[80];
  mitem_t *save_curi;

  /* Cannot offer help right now */

  fl->fl_menu->m_flags |= M_NOHELP;

  /* Set up the default to be the current menu's item */

  (void)strcpy(tmpname, fl->fl_value);
  (void)strcpy(fl->fl_value, fl->fl_menu->m_curi->mi_name);
  op = M_BEGIN | M_INPUT;
  save_curi = fl->fl_menu->m_curi;

  for (;;) {
    ret = menu(fl->fl_menu, op);
    op &= ~M_BEGIN;
    if ((ret < 0) ||
	(ret == Enter) ||
	(ret == Cancl) ||
	(ret == s_Cancl)  ||
	(ret == Exit)) {
      break;
    }
    fl->fl_menu->m_lbuf[0] = '\0';
  }

  if (ret == Enter) {
    if (fl->fl_menu->m_selcnt == 1) {
      (void)strcpy(fl->fl_value, fl->fl_menu->m_curi->mi_name);
    }
  }
  else {
    (void)strcpy(fl->fl_value, tmpname);
    fl->fl_menu->m_curi = save_curi;
  }

  (void)menu(fl->fl_menu, M_END);

  return(FERR_OK);
}



/****************************************************************************

  rightfl = FRIGHT(f, curfl)	- find field to the right
  leftfl  = FLEFT (f, curfl)	- . . . . . . . . . left
  upfl    = FUP   (f, curfl)	- . . . . .  above
  downfl  = FDOWN (f, curfl)	- . . . . .  below
  form_t *f;
  int curfl;

  These routines look in the form (f) to find a the field right, left
  above or below the one indexed as curfl.  If no field is found, they
  return curfl.

****************************************************************************/

/*ARGSUSED*/
field_t *
fright(f, cfl)
register field_t *cfl;
{
  register field_t *fl;
  register short crow;

  crow = cfl->fl_row;

  for (fl=cfl+1 ; fl->fl_name ; fl++) {
    if (fl->fl_row == crow) {
      return(fl);
    }
    else if (fl->fl_row > crow) {
      return(cfl);
    }
  }
  return (cfl);
}

field_t *
fleft(f, cfl)
register form_t *f;
register field_t *cfl;
{
  register field_t *fl;
  register short crow;

  crow = cfl->fl_row;

  if (cfl == 0) {
    return (0);
  }

  for (fl=cfl-1 ; fl>=f->f_fields ; fl--) {
    if (fl->fl_row == crow) {
      return(fl);
    }
    else if (fl->fl_row < crow) {
      return(cfl);
    }
  }
  return (cfl);
}

/*  FDOWN and FUP are more complicated.  They work in two passes: first, they
    try to find a field which is directly underneath (above) the current one.
    If that fails, they try to find any field which has a greater (lesser)
    row.  */

/*ARGSUSED*/
field_t *
fdown(f, cfl)
register field_t *cfl;
{
  register field_t *fl;
  register short crow;

  crow = cfl->fl_row;
  
/*  Pass 2: try for one which is below  */

  for (fl=cfl+1 ; fl->fl_name ; fl++) {
    if (fl->fl_row > crow) {
      return(fl);
    }
  }

  return(cfl);
}

field_t *
fup(f, cfl)
register form_t *f;
register field_t *cfl;
{
  register field_t *fl;
  register short crow, col1, col2;
  short ccol1, ccol2;

  if (cfl == 0) {
    return (0);
  }

  crow = cfl->fl_row;
  ccol1 = cfl->fl_ncol;
  ccol2 = cfl->fl_fcol + strlen(cfl->fl_value);

  /*  Pass 1: try for one which is directly above */

  for (fl=cfl-1 ; fl>=f->f_fields ; fl--) {
    col1 = fl->fl_ncol;
    col2 = fl->fl_fcol + strlen(fl->fl_value);
    if (ccol1 < col2 && col1 < ccol2) {
      return (fl);
    }
  }

  /*  Pass 2: try for one which is above  */

  for (fl=cfl-1 ; fl>=f->f_fields ; fl--) {
    if (fl->fl_row < crow) {
      return(fl);
    }
  }

  return(cfl);
}

/*************************************************************************

  disp_ins(w, f->f_label, width);

  w is the form window no.
  f->f_label is the form label.
  width is the width of the form window.

  global insert_mode is true for insert, false for overtype.

**************************************************************************/

disp_ins(w, label, width)
char *label;
short width;
{
  char template[80];
  int i;

  for (i = 0 ; i < strlen(label) ; i++) {
    template[i] = label[i];
  }
  for (i = strlen(label) ; i <= width - 2 ; i++) {
    template[i] = ' ';
  }

  if (insert_mode) {
    (void)strncpy(&template[width-7], "   \0", 4);
  }
  else {
    (void)strncpy(&template[width-7], "*OT\0", 4);
  }

  (void)wlabel(w, template);
}
