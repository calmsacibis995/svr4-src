/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:message.c	1.2"
/*
*  message.c
*
*	Output help or error message in printf format to new window
*	and wait for user response.
*/

#include "tam.h"
#include "message.h"
#include <errno.h>
#include "menu.h"
#include "path.h"
#include <stdio.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <string.h>

#define  Max_lines	10
#define  Max_cols	40

extern void	exit ();

/* First, the entry point: */

int
#ifdef __STDC__
message(int mtype, char *file, char *title, char *format, ...)
#else
message(mtype, file, title, format, va_alist)

int	mtype;		/* Type of message */
char	*file;		/* Name of help file */
char	*title;		/* Title of initial help screen */
char *format;
va_dcl
#endif
{
  va_list ap;
  char buf[512];

#ifdef __STDC__
  va_start(ap, format);
#else
  va_start(ap);
#endif
  (void)vsprintf (buf, format, ap);
  va_end(ap);
  return _domsg(mtype, file, title, buf);
}

/*
 *  _domsg  -  Do the real work of message, now that printf has been
 *		performed
 */

_domsg (mtype, file, title, ptr)
int	mtype;			/* Type of message */
char	*file;			/* Name of help file */
char	*title;			/* Title of initial help screen */
register char	*ptr;		/* Initial message to display */
{
  char	lines [Max_lines] [Max_cols+1];
  int	lin;
  register	col;		/* Current column number */
  register char	*dptr;		/* Destination pointer */
  char	*sv_ptr;		/* Input pointer at last break */
  char	*sv_dptr;		/* Destination pointer at last break */

  char	top_ju;			/* If window should be top justified */
  int	height;			/* Height of message window */
  int	w_id;			/* ID of message window */
  int	i;
  int	chr;

  char	looping;


  /* First wrap the message to fit */

  lin = col = 0;
  sv_ptr = NULL;
  dptr = lines [lin];
  while (*ptr) {
    *dptr++ = *ptr;
    if (*ptr == '\n') {
      *--dptr = 0;
      ptr++;
      if (++lin >= Max_lines) {
        break;		/* Lines buffer full */
      }
      col = 0;
      sv_ptr = NULL;
      dptr = lines [lin];
      continue;
    }
    if (*ptr++ == ' ') {			/* Remember line breaks */
      sv_ptr = ptr;
      sv_dptr = dptr - 1;
    }
    if (++col >= Max_cols) {
      if (sv_ptr != NULL) {			/* Backup to last break */
        ptr = sv_ptr;
        dptr = sv_dptr;
      }
      *dptr = 0;
      if (++lin >= Max_lines) {
	break;		/* Lines buffer full */
      }
      col = 0;
      sv_ptr = NULL;
      dptr = lines [lin];
    }
  }
  *dptr = 0;
  if (++lin > Max_lines) {
    lin = Max_lines;
  }

  /*
   *	Now display the message
   */

  top_ju = 0;
  switch (mtype) {
    case MT_HELP: {
      height = lin + 5;
      if (file == 0) {
        height--;
      }
      w_id = _mwcr (height, top_ju, (file != 0));
      if (w_id < 0) {
        return (MERR_NOWIN);
      }
      (void)wlabel (w_id, "Help");
      for (i = 0; i < lin; i++) {
        (void)wgoto (w_id, i + 1, 0);
        (void)wputs (w_id, "  ");
        (void)wputs (w_id, lines [i]);
      }
      i = lin + 2;
      (void)wgoto (w_id, i, 0);
      (void)wputs (w_id, "     Touch ENTER to continue");
      (void)wgoto (w_id, i + 1, 0);
      if (file != 0) {
        (void)wputs (w_id, "  or HELP for more information");
      }
      break;
    }

    case MT_ERROR: {
      height = lin + 5;
      if (file == 0) {
        height--;
      }
      w_id = _mwcr (height, top_ju, (file != 0));
      if (w_id < 0) {
        return (MERR_NOWIN);
      }
      (void)wlabel (w_id, "Error");
      for (i = 0; i < lin; i++) {
        (void)wgoto (w_id, i + 1, 0);
        (void)wputs (w_id, "  ");
        (void)wputs (w_id, lines [i]);
      }
      i = lin + 2;
      (void)wgoto (w_id, i, 0);
      (void)wputs (w_id, "     Touch ENTER to continue");
      (void)wgoto (w_id, i + 1, 0);
      if (file != 0) {
        (void)wputs (w_id, "  or HELP for more information");
      }
      break;
    }

    case MT_QUIT: {
      height = lin + 6;
      if (file == 0) {
        height--;
      }
      w_id = _mwcr (height, top_ju, (file != 0));
      if (w_id < 0) {
        return (MERR_NOWIN);
      }
      (void)wlabel (w_id, "Error");
      for (i = 0; i < lin; i++) {
        (void)wgoto (w_id, i + 1, 0);
        (void)wputs (w_id, "  ");
        (void)wputs (w_id, lines [i]);
      }
      i = lin + 2;
      (void)wgoto (w_id, i, 0);
      (void)wputs (w_id, "     Touch ENTER to continue");
      (void)wgoto (w_id, i + 1, 0);
      (void)wputs (w_id, "  or CANCL to stop.");
      (void)wgoto (w_id, i + 2, 0);
      if (file != 0) {
        (void)wputs (w_id, "  Touch HELP for more information");
      }
      break;
    }

    case MT_POPUP: {
      height = lin + 2;
      w_id = _mwcr (height, top_ju, (file != 0));
      if (w_id < 0) {
        return (MERR_NOWIN);
      }
      for (i = 0; i < lin; i++) {
        (void)wgoto (w_id, i + 1, 0);
        (void)wputs (w_id, "  ");
        (void)wputs (w_id, lines [i]);
      }
      i = lin + 2;
      break;
    }

    case MT_CONFIRM: {
      height = lin + 6;
      if (file == 0) {
        height--;
      }
      w_id = _mwcr (height, top_ju, (file != 0));
      if (w_id < 0) {
        return (MERR_NOWIN);
      }
      (void)wlabel (w_id, "Confirm");
      for (i = 0; i < lin; i++) {
        (void)wgoto (w_id, i + 1, 0);
        (void)wputs (w_id, "  ");
        (void)wputs (w_id, lines [i]);
      }
      i = lin + 2;
      (void)wgoto (w_id, i, 0);
      (void)wputs (w_id, "     Touch ENTER to continue");
      (void)wgoto (w_id, i + 1, 0);
      (void)wputs (w_id, "  or CANCL to stop.");
      (void)wgoto (w_id, i + 2, 0);
      if (file != 0) {
        (void)wputs (w_id, "  Touch HELP for more information");
      }
      break;
    }

    case MT_INFO: {
      height = lin + 4;
      if (file == 0) {
        height--;
      }
      w_id = _mwcr (height, top_ju, (file != 0));
      if (w_id < 0) {
        return (MERR_NOWIN);
      }
      (void)wlabel (w_id, lines [0]);
      for (i = 1; i < lin; i++) {
        (void)wgoto (w_id, i, 0);
        (void)wputs (w_id, "  ");
        (void)wputs (w_id, lines [i]);
      }
      i = lin + 1;
      (void)wgoto (w_id, i, 0);
      (void)wputs (w_id, "     Touch ENTER to continue");
      (void)wgoto (w_id, i + 1, 0);
      if (file != 0) {
        (void)wputs (w_id, "  or HELP for more information");
      }
      break;
    }
  }

  /*
   *	Now wait for user input
   */

  looping = 1;
  while (looping) {
    chr = wgetc (w_id);
    if (chr < 0) {
      if (w_id != wgetsel ()) {	/* Another window selected, leave */
        chr = MERR_SYS;
        break;
      }
      continue;
    }
    switch (chr) {
      case Enter:
      case Return:
      case Cancl:
      case s_Cancl: {
        looping = 0;
        break;
      }

      case Help: {
        if (mtype == MT_POPUP) {
          looping = 0;
          break;
	}
        else if (file != 0) {
          if (exhelp (file, title) < 0) {
            (void)wprompt (w_id, "Can't exec help process");
            break;
	  }
          if (w_id != wgetsel ()) {	/* Another window selected, leave */
            chr = MERR_SYS;
            looping = 0;
            break;
	  }
	}
        else {
	  (void)beep ();
	}
        break;
      }

      default: {
        if (mtype == MT_POPUP) {
          looping = 0;
	}
        else {
	  (void)beep ();
	}
        break;
      }
    }
  }

  (void)wdelete (w_id);
  if (chr == Return) {
    chr = Enter;
  }
  if (chr == s_Cancl) {
    chr = Cancl;
  }
  return (chr);
}


  /*
   *  _mwcr  -  Message window create algorithm
   */

_mwcr (height, top_ju, help_fl)
int	height;		/* Window height */
char	top_ju;		/* Top justify flag */
char	help_fl;	/* If help icon should be displayed */
{
  int	begy;
  int	wid;

  begy = LINES - (height + 6);
  if (top_ju || (begy < 1)) {
    begy = 1;
  }
  if (help_fl) {
    wid = wcreate (begy, 15, height, Max_cols + 4, BORDHELP | BORDCANCEL);
  }
  else {
    wid = wcreate (begy, 15, height, Max_cols + 4, BORDCANCEL);
  }
  if (wid < 0) {
    (void)wprompt (wncur, "Error - Can't create window - Touch ENTER to continue");
    (void)wgetc (wncur);
    (void)wprompt (wncur, 0);
  }
  return (wid);
}

/*
 *  exhelp  -  Execute the help process and block till return
 */

exhelp (file, title)
char	*file;
char	*title;
{
  char	harg [128];		/* Help file path argument */
  int	pid;			/* Process ID */
  int	ret;			/* Wait return code */
  int	status;

  if ((strlen (file) > 100) || (strlen (title) > 100)) {
    return (-1);
  }
  if (*file == '/') {
    (void)strcpy (harg, file);
  }
  else {
    (void)strcpy (harg, KMAPDIR);
    (void)strcat (harg, file);
  }

  if ((pid = fork ()) == 0) {			/* Child process */
    register	i;

    if (wprexec () < 0) {
      exit (-1);
    }
    for (i = 3; i < 20; i++) {
      (void)close (i);
    }
    (void)execl (TAMHELP, "tamhelp", "-h", harg, "-t", title, 0);
    exit (-1);		/* Exec failed */
  }
  else if (pid < 0) {
    return (-1);
  }
  else {			/* Parent process */
    while (1) {
      ret = wait (&status);
      if (pid == ret) {
        break;		/* Child terminated, done */
      }
      else if ((ret < 0) && (errno == EINTR)) {		/* Return on signal */
        status = 0;
        break;
      }
    }
    (void)wpostwait ();
    if ((status & 0xffff) == 0xff00) {
      return (status);		/* Exec failed */
    }
  }
  return (0);
}
