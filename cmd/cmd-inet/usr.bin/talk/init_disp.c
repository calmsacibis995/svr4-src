/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.bin/talk/init_disp.c	1.4.3.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */


/*
 * init_disp contains the initialization code for the display package,
 * as well as the signal handling routines
 */

#include "talk.h"
#include <signal.h>

#ifdef SYSV
#define	signal(s,f)	sigset(s,f)
#endif /* SYSV */

/* 
 * set up curses, catch the appropriate signals, and build the
 * various windows
 */

init_display()
{
    void sig_sent();

    initscr();
    curses_initialized = 1;

    clear();
    refresh();

    noecho();
    crmode();

    signal(SIGINT, sig_sent);
    signal(SIGPIPE, sig_sent);

	/* curses takes care of ^Z */

    my_win.x_nlines = LINES / 2;
    my_win.x_ncols = COLS;
    my_win.x_win = newwin(my_win.x_nlines, my_win.x_ncols, 0, 0);
    scrollok(my_win.x_win, FALSE);
    wclear(my_win.x_win);

    rem_win.x_nlines = LINES / 2 - 1;
    rem_win.x_ncols = COLS;
    rem_win.x_win = newwin(rem_win.x_nlines, rem_win.x_ncols,
					     my_win.x_nlines+1, 0);
    scrollok(rem_win.x_win, FALSE);
    wclear(rem_win.x_win);

    line_win = newwin(1, COLS, my_win.x_nlines, 0);
    box(line_win, '-', '-');
    wrefresh(line_win);

	/* let them know we are working on it */

    current_state = "No connection yet";
}

    /* trade edit characters with the other talk. By agreement
     * the first three characters each talk transmits after
     * connection are the three edit characters
     */

set_edit_chars()
{
    char buf[3];
    int cc;
#ifdef SYSV
    struct termios tty;

    ioctl(0, TCGETS, (struct termios *) &tty);
	
    buf[0] = my_win.cerase = tty.c_cc[VERASE]; /* for SVID should be VERSE */
    buf[1] = my_win.kill = tty.c_cc[VKILL];
    buf[2] = my_win.werase = tty.c_cc[VWERASE];/* for SVID should be VWERSE */
#else /* ! SYSV */
    struct sgttyb tty;
    struct ltchars ltc;

    gtty(0, &tty);

    ioctl(0, TIOCGLTC, (struct sgttyb *) &ltc);
	
    my_win.cerase = tty.sg_erase;
    my_win.kill = tty.sg_kill;

    if (ltc.t_werasc == (char) -1) {
	my_win.werase = '\027';	 /* control W */
    } else {
	my_win.werase = ltc.t_werasc;
    }

    buf[0] = my_win.cerase;
    buf[1] = my_win.kill;
    buf[2] = my_win.werase;
#endif /* SYSV */

    cc = write(sockt, buf, sizeof(buf));

    if (cc != sizeof(buf) ) {
	p_error("Lost the connection");
    }

    cc = read(sockt, buf, sizeof(buf));

    if (cc != sizeof(buf) ) {
	p_error("Lost the connection");
    }

    rem_win.cerase = buf[0];
    rem_win.kill = buf[1];
    rem_win.werase = buf[2];
}

void sig_sent()
{
    message("Connection closing. Exiting");
    quit();
}

/*
 * All done talking...hang up the phone and reset terminal thingy's
 */

quit()
{
	if (curses_initialized) {
	    wmove(rem_win.x_win, rem_win.x_nlines-1, 0);
	    wclrtoeol(rem_win.x_win);
	    wrefresh(rem_win.x_win);
	    endwin();
	}

	if (invitation_waiting) {
	    send_delete();
	}

	exit(0);
}
