/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/tgetch.c	1.13"
#include	"curses_inc.h"
#include	<signal.h>
#include	<errno.h>
#ifdef	DEBUG
#include	<ctype.h>
#endif	/* DEBUG */

/*
 * Read a key typed from the terminal
 *
 * interpret:	= 0 for single-char key only
 * 	 	= 1 for matching function key and macro patterns.
 * 		= 2 same as 1 but no time-out for funckey matching.
 */

static	int	_getkey(), _fpk(), _pk();

int
tgetch(interpret)
int	interpret;
{
    int			inp, i = 0, j, collapse = 1;
    register	short	*inputQ = cur_term->_input_queue;
    char		*chars_onQ = &(cur_term->_chars_on_queue);

#ifdef	SYSV
    /*
     * Register the fact that getch is being used so
     * that typeahead checking can be done.
     * This code should GO AWAY when a poll() or FIONREAD can
     * be done on the file descriptor as then the check
     * will be non-destructive.
     */
    cur_term->fl_typeahdok = TRUE;
#endif	/* SYSV */

    /* ask for input */
    if (cur_term->_ungotten > 0)
    {
	cur_term->_ungotten--;
	/* decode an ungetch()'d character */
	inp = -(inputQ[0] + 0100);
    }
    else
    {
	/* Only read a character if there is no typeahead/peekahead. */
	if (*chars_onQ == 0)
	{
	    /* (*chars_onQ)++;  MR */
#ifdef	FIONREAD
	    inp = _readchar();
#else	/* FIONREAD */
	    if ((inp = _pk()) == ERR)
	    {
		/*
		 * interpret is set to 0 so that down below we don't
		 * drop into getkey since we already know there can't be
		 * a key that starts with -1.  Also, we don't want to
		 * access funckeystarter[-1].
		 */
		interpret = FALSE;
	    }
	    (*chars_onQ)++;
#endif	/* FIONREAD */
	}
	else
	    inp = inputQ[0];

#ifdef	DEBUG
	if (outf)
	    fprintf(outf, "TGETCH read '%s'\n", unctrl(inp));
#endif	/* DEBUG */

	/* Check for arrow and function keys */
	if (interpret && cur_term->funckeystarter[inp])
	    collapse = _getkey(interpret - 1, &inp);
    }

/* Collapse the input queue to remove the escape sequence from the stack. */

    j = *chars_onQ;
    (*chars_onQ) -= collapse;
    while (collapse < j)
	inputQ[i++] = inputQ[collapse++];
    return (inp);
}

#ifdef	FIONREAD
static	int
_readchar()
{
    register	int	i;
    unsigned	char	c;

    if (cur_term->_delay == 0)
    {
	int	arg;

	(void) ioctl(cur_term->_inputfd, FIONREAD, &arg);
#ifdef	DEBUG
	if (outf) fprintf(outf, "FIONREAD returns %d\n", arg);
#endif	/* DEBUG */
	if (arg < 1)
	    return (-1);
    }
    else
	if (cur_term->_delay > 0)
	{
	    char	c;
	    int		infd;

	    infd = 1 << cur_term->_inputfd;
	    t.tv_sec = cur_term->_delay / 1000;
	    t.tv_usec = (cur_term->_delay % 1000) * 1000;
	    i = select(20, &infd, (int *)NULL, (int *)NULL, &t);
	    if (i < 0)
		return (ERR);
	    i = read(cur_term->_inputfd, &c, 1);
	}
	else
	    i = read(cur_term->_inputfd, &c, 1);

#ifdef	DEBUG
    if (outf)
	fprintf(outf, "read from %d returns %d chars, first %o\n",
	    cur_term->_inputfd, i, c);
#endif	/* DEBUG */

    if (i > 0)
	return (c);
    else
	return (ERR);
}
#endif	/* !FIONREAD */

#ifdef	DEBUG
extern	char	*_asciify();
#endif	/* DEBUG */

    /*
     * This algorithm is a "learning" algorithm. The premise is
     * that keys used once are like to be used again and again.
     * Since the time for a linear search of the table is so
     * expensive, we move keys that are found up to the top of
     * the list, making the access to a repeated key very fast and
     * keys that have been used before close to the top.
     */

static	int
_getkey(blockpeek, inp)
register	int	*inp;
register	int	blockpeek;
{
    register	_KEY_MAP	**kp = cur_term->_keys;
    register	int		key, num_keys = cur_term->_ksz;
    int				i;
    short			*inputQ = cur_term->_input_queue;
    char			*chars_onQ = &(cur_term->_chars_on_queue),
				flag = cur_term->funckeystarter[*inp];
    int				first, collapse = 1;
    

#ifdef	DEBUG
    if (outf)
	fprintf(outf, "getkey(): looking in linear table, inp=%d\n", *inp);
#endif	/* DEBUG */

    if (flag & _KEY)
	key = 0;
    else
    {
	key = cur_term->_first_macro;
	blockpeek = TRUE;
    }
    first = key;

    for (; key < num_keys; key++)
    {
	if (kp[key]->_sends[0] == *inp)
	{
	    for (i = 1; i < INP_QSIZE; i++)
	    {
		/* found it? */
		if (kp[key]->_sends[i] == '\0')
		    break;
		/* partial match? peek ahead. */
		if (*chars_onQ == i)
		{
		    (*chars_onQ)++;
		    switch (inputQ[i] = (blockpeek) ? _pk() : _fpk())
		    {
			case -2:
			    /*
			     * Since -2 signifies a timeout we don't really
			     * want to put it on the queue so we decrement
			     * our counter.
			     */
			    (*chars_onQ)--;
#ifdef	DEBUG
			    if (outf)
				fprintf(outf, "Timed out\n");
#endif	/* DEBUG */
			    if (flag & _MACRO)
			    {
#ifdef	DEBUG
				if (outf)
				    fprintf(outf, "Found macro\n");
#endif	/* DEBUG */
				/*
				 * We have to decrement one because key will be
				 * incremented at the bottom of the out loop.
				 */
				key = (first = blockpeek = cur_term->_first_macro) - 1;
				goto outerloop;
			    }
			case -1:
			    goto ret;
		    }
		}

		/* not this one? */
		if (kp[key]->_sends[i] != inputQ[i])
		    goto outerloop;
	    }

/* SS-mouse */
	    if (kp[key]->_keyval == KEY_MOUSE)
	    {
		MOUSE_STATUS old_mouse;
		int rc;
		static int get_xterm_mouse();

		old_mouse = Mouse_status;

		/* read the mouse status information	*/

		if (mouse_info)
		    rc = -3;		/* NOT IMPLEMENTED */
		else
		    rc = get_xterm_mouse(blockpeek,&i);

		if (rc == -1)		/* read error */
		    goto ret;
		else if (rc == -2 || rc == -3)   /* timeout or not mouse */
		    goto outerloop;
		else if (rc == 0)		 /* report mouse pos */
		    Mouse_status.changes |= 020;
		else if (rc >= 1 && rc <= 3)	 /* mouse button event */
		    Mouse_status.changes =
			(((MOUSE_X_POS != old_mouse.x ||
			   MOUSE_Y_POS != old_mouse.y) << 3) |
			 ((Mouse_status.button[2] !=
				old_mouse.button[2]) << 2) | 
			 ((Mouse_status.button[1] !=
				old_mouse.button[1]) << 1) | 
			  (Mouse_status.button[0] !=
				old_mouse.button[0]));
	    }

	    /* We found it! Read in any chars left in _sends */

	    if ((collapse = i) == INP_QSIZE)
		for ( ; kp[key]->_sends[i]; i++)
		    (void) _fpk();

	    /* move key to top of ordered list */
	    if (key != first)
	    {
		_KEY_MAP	*savekey = kp[key];
		short		*lorder;
		int		j;

		if (key > cur_term->_first_macro)
		    lorder = &(cur_term->_lastmacro_ordered);
		else
		    lorder = &(cur_term->_lastkey_ordered);
		/*
		 * If we're below the last ordered key, swap next unordered
		 * key with this one and ripple from there.
		 */
		if (key > *lorder)
		    kp[key] = kp[(i = ++(*lorder))];
		else
		    i = key;
		/* ripple the ordered keys down */
		for (j = i--; j > first; )
		    kp[j--] = kp[i--];
		kp[first] = savekey;
	    }
	    *inp = kp[first]->_keyval;

/* SS-mouse support: if mouse button event occured on top of the soft
        label, we may have to return the function key corresponding to
        that soft label
*/

	    if (*inp == KEY_MOUSE && A_BUTTON_CHANGED &&
		(MOUSE_Y_POS == LINES) &&
                (SP->slk != (SLK_MAP *) NULL) &&
                (SP->_map_mbe_to_key  != 0))
		{
		    static void _map_button();
                    _map_button (inp);
		}

	    goto ret;
	}
outerloop:
	;
    }

ret:
    /* key not found */
#ifdef	DEBUG
    if (outf)
	if (key == num_keys)
	    fprintf(outf, "Did not match anything.\n");
#endif	/* DEBUG */
    return (collapse);
}


/* SS-mouse */
/* this function tries to read in information that follows KEY_MOUSE: */
/* the first character identifies what button is involved (1,2,or 3)  */
/* if the first character is 0, we are dealing with report_mouse_pos  */
/* The routine returns the following:
		-3:	not a mouse button event
		-2:	read timed out
		-1:	the read failed	
		[0,1,2,3] - the first character in the mouse event
*/
static int
get_xterm_mouse(blockpeek, i)
register  int	blockpeek, *i;
{
    short	*inputQ = cur_term->_input_queue;
    char	*chars_onQ = &(cur_term->_chars_on_queue);
    register int j, mx, my;
    register short  char1, char2, c1, c2;

    /* the first character should be 0, 1, 2, or 4	*/

    char1 = (inputQ[(*i)++] = (blockpeek) ? _pk() : _fpk());

    /* read error or timeout	*/

    if (char1 < 0)	
	return (char1);
    (*chars_onQ)++;

    if (char1 < '0' || char1 > '3')
	return (-3);

    /* if the character is 1, 2, or 3 it must be followed by 	*/
    /* P, R, C, D, or T						*/

    if (char1 != '0')
    {
        char2 = (inputQ[(*i)++] = (blockpeek) ? _pk() : _fpk());

	if (char2 < 0)
	    return (char2);

        (*chars_onQ)++;
	if (char2 != 'P' && char2 != 'R' && char2 != 'C' &&
	    char2 != 'D' && char2 != 'T')
	    return (-3);
    }

    /* read X  and Y coordinates of the mouse	*/

    for (j=0; j<2; j++)
    {
	c1 = (inputQ[(*i)++] = (blockpeek) ? _pk() : _fpk());
	if (c1 < 0)
	    return (c1);
        (*chars_onQ)++;
	if (c1 >= ' ' && c1 <= '~')	/* ascii char */
	{
	    if (j == 0)
        	mx = c1 - ' ';
    	    else
        	my = c1 - ' ';
	}
        else if (char1 == 01 || char1 == 02)   /* ^A || ^B */
        {
            c2 = (inputQ[(*i)++] = (blockpeek) ? _pk() : _fpk());
	    if (c2 < 0)
		return (c2);
            (*chars_onQ)++;
    	    if (c2 >= ' ' && c2 <= '~')
	    {
	        if (j == 0)
	    	    mx = c1 * (c2 - ' ');
		else
		    my = c1 * (c2 - ' ');
    	    }
    	    else
		return (-3);
	}
	else
	    return (-3);
    }

    /* read complete mouse event: update the Mouse_status structure */

    MOUSE_X_POS = mx;
    MOUSE_Y_POS = my;
    j = char1 - '0';
    if (j != 0)
    {
	switch (char2)
	{
	   case 'P':	BUTTON_STATUS(j) = BUTTON_PRESSED;	break;
	   case 'R':	BUTTON_STATUS(j) = BUTTON_RELEASED;	break;
	   case 'C':	BUTTON_STATUS(j) = BUTTON_CLICKED;	break;
	   case 'D':	BUTTON_STATUS(j) = BUTTON_DOUBLE_CLICKED;	break;
	   case 'T':	BUTTON_STATUS(j) = BUTTON_TRIPLE_CLICKED;	break;
	}
    }
    return (j);
}
/* SS-mouse-end */


/*
 * Fast peek key.  Like getchar but if the right flags are set, times out
 * quickly if there is nothing waiting, returning -1.
 * f is an output stdio descriptor, we read from the fileno.
 * We wait for long enough for a terminal to send another character
 * (at 15cps repeat rate, this is 67 ms, I'm using 100ms to allow
 * a bit of a fudge factor) and time out more quickly.
 * -2 is returned if we time out, -1 is returned if interrupted, and the
 * character is returned otherwise.
 */

#ifndef	FIONREAD

/*
 * Traditional implementation.  The best resolution we have is 1 second,
 * so we set a 1 second alarm and try to read.  If we fail for 1 second,
 * we assume there is no key waiting.  Problem here is that 1 second is
 * too long; people can type faster than this.
 *
 * Another possible implementation of changing VMIN/VTIME before and
 * after each read does not work because the tty driver's timeout
 * mechanism is too unreliable when the timeouts are changed too quickly.
 */

static	char	sig_caught;

static
#ifdef	SIGPOLL	/* Vr3 and beyond */
void
#endif  /* SIGPOLL */
_catch_alarm(dummy)
int dummy;
{
    sig_caught = 1;
}

static
_fpk()
{
    unsigned	char	c;
    int		rc, infd = cur_term->_inputfd;
#ifdef	SIGPOLL	/* Vr3 and beyond */
    void	(*oldsig)();
#else	/* SIGPOLL */
    int		(*oldsig)();
#endif	/* SIGPOLL */
    unsigned	int	oldalarm, alarm();

    /* turn off any user alarms and set our own */
    oldalarm = alarm(0);
    sig_caught = 0;
    oldsig = signal(SIGALRM, _catch_alarm);
    (void) alarm(1);
    rc = read(cur_term->_inputfd, (char *) &c, 1);
    (void) alarm(0);

    /*
     * This code is to take care of the possibility of
     * the process getting swapped out in the middle of
     * read() call above. The interrupt will cause the
     * read() call to return, even if a character is really
     * on the clist. So we do a non-blocking read() to make
     * sure that there really isn't a character there.
     */

    if (sig_caught && rc != 1)
	if (cur_term->_check_fd != -1)
	    rc = read(cur_term->_check_fd, (char *) &c, 1);
	else
	{
#include	<fcntl.h>
	    register	int	fcflags = fcntl(infd, F_GETFL, 0);

	    (void) fcntl(infd, F_SETFL, fcflags | O_NDELAY);
	    rc = read(infd, (char *) &c, 1);
	    (void) fcntl(infd, F_SETFL, fcflags);
	}

    /* restore the user alarms */
	(void) signal(SIGALRM, oldsig);
	if (sig_caught && oldalarm > 1)
	    oldalarm--;
	(void) alarm(oldalarm);
	if (rc == 1)			/* got a character */
	    return (c);
	else
	    if (sig_caught)		/* timed out */
		return (-2);
	    else			/* EOF or got interrupted */
		return (-1);
}
#else	/* FIONREAD */
/*
 * If we have the select system call, we can do much better than the
 * traditional method. Even if we don't have the real 4.2BSD select, we
 * can emulate it with napms and FIONREAD.  napms might be done with only
 * 1 second resolution, but this is no worse than what we have in the
 * traditional implementation.
 */
static
_fpk()
{
    int		infd, rc;
    int		*outfd, *exfd;
    unsigned	char	c;
    struct	_timeval
		{
		    long tv_sec;
		    long tv_usec;
		}t;

    infd = 1 << cur_term->_inputfd;
    outfd = exfd = (int *) NULL;
    t.tv_sec = 0;
    t.tv_usec = 100000;		/* 100 milliseconds */
    rc = select(20, &infd, outfd, exfd, &t);
    if (rc < 0)
	return (-2);
    rc = read(fileno(f), &c, 1);
    return (rc == 1 ? c : -1);
}
#endif	/* FIONREAD */

/*
 * Plain peekchar function.  Nothing fancy.  This is just like _fpk
 * but will wait forever rather than time out.
 */

static
_pk()
{
    unsigned	char	c;

    return ((read(cur_term->_inputfd, (char *) &c, 1) == 1) ? c : ERR);
}


/* SS-mouse: check if this mouse button event should map into
	     function key
*/


static void
_map_button (inp)
int *inp;
{
	SLK_MAP *slk = SP->slk;
	register int num = slk->_num;
	register int len = slk->_len;
	register int i;

	/* first determine if this mouse button event should be */
	/* mapped into function key				*/

	if (!(SP->_map_mbe_to_key &
	      ((BUTTON_CHANGED(3) << (10 + BUTTON_STATUS(3))) |
	       (BUTTON_CHANGED(2) << (5 + BUTTON_STATUS(2)))  |
	       (BUTTON_CHANGED(1) << BUTTON_STATUS(1)))))
	    return;

	for (i=0; i<num; i++)
	{
	     if (MOUSE_X_POS < slk->_labx[i])
		 break;
	     if (MOUSE_X_POS > slk->_labx[i] + len)
		 continue;
	     *inp = KEY_F(1) + i;
	     break;
	}
}
