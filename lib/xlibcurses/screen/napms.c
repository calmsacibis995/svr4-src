/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/napms.c	1.13"
#include	"curses_inc.h"
#include	<signal.h>

struct	_timeval
	{
	    long	tv_sec;
	    long	tv_usec;
	};

/*
 * napms.  Sleep for ms milliseconds.  We don't expect a particularly good
 * resolution - 60ths of a second is normal, 10ths might even be good enough,
 * but the rest of the program thinks in ms because the unit of resolution
 * varies from system to system.  (In some countries, it's 50ths, for example.)
 * Vaxen running 4.2BSD and 3B's use 100ths.
 *
 * Here are some reasonable ways to get a good nap.
 *
 * (1) Use the poll() or select() system calls in SVr3 or Berkeley 4.2BSD.
 *
 * (2) Use the 1/10th second resolution wait in the System V tty driver.
 *     It turns out this is hard to do - you need a tty line that is
 *     always unused that you have read permission on to sleep on.
 *
 * (3) Install the ft (fast timer) device in your kernel.
 *     This is a psuedo-device to which an ioctl will wait n ticks
 *     and then send you an alarm.
 *
 * (4) Install the nap system call in your kernel.
 *     This system call does a timeout for the requested number of ticks.
 *
 * (5) Write a routine that busy waits checking the time with ftime.
 *     Ftime is not present on SYSV systems, and since this busy waits,
 *     it will drag down response on your system.  But it works.
 */

#ifdef	SIGPOLL

/* on SVr3, use poll */
#include	<sys/poll.h>
napms(ms)
int ms;
{
    struct pollfd pollfd;

    if (poll(&pollfd, 0L, ms) == -1)
        perror("poll");
    return (OK);
}

#else	/* SIGPOLL */

#ifdef	TIOCREMOTE

static	struct	_timeval	tz;

#ifdef	BSD4_1C

/* Delay for us microseconds, but not more than 1 second */

static	long
_dodelay(us)
long	us;
{
    struct	_timeval	old, now, d, want;

    gettimeofday(&old, &tz);
    want = old;
    want.tv_usec += us;
    want.tv_sec += want.tv_usec / 1000000;
    want.tv_usec %= 1000000;
    now = old;
    do
    {
	d.tv_sec = 0;
	d.tv_usec = _timediff(want, now);
	select(20, 0, 0, 0, &d);
	gettimeofday(&now, &tz);
    } while (_timediff(now, want) < 0);
}

static long
_timediff(t1, t2)
struct _timeval t1, t2;
{
    return (t1.tv_usec - t2.tv_usec + 1000000 * (t1.tv_sec - t2.tv_sec));
}
#endif	/* BSD4_1C */

/* on 4.2BSD, use select */
napms(ms)
int ms;
{
    struct	_timeval	t;

    /*
     * This code has been tested on 4.1cBSD.  The 4.1c select call
     * tends to return too early from a select, so we check and try
     * again.  This will work ok, but if it waited much too long
     * we would be in trouble.
     */
    t.tv_sec = ms/1000;
    t.tv_usec = 1000 * (ms % 1000);

    /*
     * The next 3 lines are equivalent to a correctly working:
     * select(0, 0, 0, 0, &t);
     */

#ifdef	BSD4_1C
    if (t.tv_sec > 0)
	sleep(t.tv_sec);
    _dodelay(t.tv_usec);
#else	/* BSD4_1C */
    select(0, 0, 0, 0, &t);
#endif	/* BSD4_1C */
    return (OK);
}

#else	/* TIOCREMOTE */

#define	NAPINTERVAL	100
#ifdef	SYSV
#include	<sys/param.h>
#else	/* SYSV */
#define	HZ	60
#endif	/* SYSV */

/*
 * Pause for ms milliseconds.  Convert to ticks and wait that long.
 * Call nap, which is either defined below or a system call.
 */

napms(ms)
int ms;
{
    int	ticks;
    int rv;

    ticks = ms / (1000 / HZ);
    if (ticks <= 0)
	ticks = 1;
    rv = nap(ticks);  /* call either the code below or nap system call */
    return (rv);
}

#if	!defined(HASNAP) && defined(FTIOCSET)
#define	HASNAP

/*
 * The following code is adapted from the sleep code in libc.
 * It uses the "fast timer" device posted to USENET in Feb 1982.
 * nap is like sleep but the units are ticks (e.g. 1/60ths of
 * seconds in the USA).
 */

#include	<setjmp.h>
static	jmp_buf	jmp;
static	int	ftfd;

/* don't call nap directly, you should call napms instead */
static	int
nap(n)
unsigned	n;
{
    int	_napx();
    unsigned	altime;
    int		(*alsig)() = SIG_DFL;
    char	*ftname;
    struct	requestbuf
		{
		    short time;
		    short signo;
		} rb;

    if (n == 0)
	return (OK);
    if (ftfd <= 0)
    {
	ftname = "/dev/ft0";
	while (ftfd <= 0 && ftname[7] <= '~')
	{
	    ftfd = open(ftname, 0);
	    if (ftfd <= 0)
		ftname[7] ++;
	}
    }
    if (ftfd <= 0)	/* Couldn't open a /dev/ft? */
    {
	_sleepnap(n);
	return (ERR);
    }
    altime = alarm(1000);	/* time to maneuver */
    if (setjmp(jmp))
    {
	(void) signal(SIGALRM, alsig);
	alarm(altime);
	return (OK);
    }
    if (altime)
    {
	if (altime > n)
	    altime -= n;
	else
	{
	    n = altime;
	    altime = 1;
	}
    }
    alsig = signal(SIGALRM, _napx);
    rb.time = n;
    rb.signo = SIGALRM;
    ioctl(ftfd, FTIOCSET, &rb);
    for(;;)
	pause();
    /*NOTREACHED*/
}

static
_napx()
{
    longjmp(jmp, 1);
}
#endif	/* !defined(HASNAP) && defined(FTIOCSET) */

#if	!defined(HASNAP) && defined(SYSV) && defined(HASIDLETTY)
#define	HASNAP
#define	IDLETTY	"/dev/idletty"

/*
 * Do it with the timer in the tty driver.  Resolution is only 1/10th
 * of a second.  Problem is, if the user types something while we're
 * sleeping, we wake up immediately, and have no way to tell how long
 * we should sleep again.  So we're sneaky and use a tty which we are
 * pretty sure nobody is using.
 *
 * Note that we should be able to do this by setting VMIN to 100 and VTIME
 * to the proper number of ticks.  But due to a bug in the SYSV tty driver
 * (this bug was still there in 5.0) this hangs until VMIN chars are typed
 * no matter how much time elapses.
 *
 * This requires some care.  If you choose a tty that is a dialup or
 * which otherwise can show carrier, it will hang and you won't get
 * any response from the keyboard.  You can use /dev/tty if you have
 * no such tty, but response will feel funny as described above.
 * To find a suitable tty, try "stty > /dev/ttyxx" for various ttyxx's
 * that look unused.  If it hangs, you can't use it.  You might try
 * connecting a cable to your port that raises carrier to keep it from hanging.
 *
 * To use this feature on SYSV, you must
 *	ln /dev/ttyxx /dev/idletty,
 * where /dev/ttyxx is one of your tty lines that is never used but
 * won't hang on open.  Otherwise we always return ERR.
 *
 * THIS SYSV CODE IS UNSUPPORTED AND ON A USE-AT-YOUR-OWN-RISK BASIS.
 */
static	int
nap(ticks)
int	ticks;
{
    struct	termio	t, ot;
    static	int	ttyfd;
    int		n, tenths;
    char	c;

    if (ttyfd == 0)
	ttyfd = open(IDLETTY, 2);
    if (ttyfd < 0)
    {
	_sleepnap(ticks);
	return (ERR);
    }
    tenths = (ticks+(HZ/10)/2) / (HZ/10); /* Round to nearest 10th second */
    (void) ioctl(ttyfd, TCGETA, &t);
    ot = t;
    t.c_lflag &= ~ICANON;
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = tenths;
    (void) ioctl(ttyfd, TCSETAW, &t);
    n = read(ttyfd, &c, 1);
    (void) ioctl(ttyfd, TCSETAW, &ot);
    /*
     * Now we potentially have a character in c that somebody's going
     * to want.  We just hope and pray they use getch, because there
     * is no reasonable way to push it back onto the tty.
     */
    if (n > 0)
	cur_term->_input_queue[cur_term->_chars_on_queue++] = c;
    return (OK);
}

/*
 * Nothing better around, so we have to simulate nap with sleep.
 */
static
_sleepnap(ticks)
{
    (void) sleep((ticks+(HZ-1))/HZ);
}
#endif	/* !defined(HASNAP) && defined(SYSV) && defined(HASIDLETTY) */

/* If you have some other externally supplied nap(), add -DHASNAP to cflags */

#ifndef	HASNAP
int
nap(ms)
int	ms;
{
    (void) sleep((unsigned) (ms+999)/1000);
    return (ERR);
}
#endif	/* HASNAP */
#endif	/* TIOCREMOTE */
#endif	/* SIGPOLL */

/*
 * Decide if we can emulate select but don't have it.  This is
 * intended to be true only on 4.1BSD, not 4.2BSD or SYSV.
 */

#if	FIONREAD && !defined(TIOCREMOTE)
#define	NEEDSELECT
#endif	/* FIONREAD && !defined(TIOCREMOTE) */

#ifdef	NEEDSELECT
#ifdef	FIONREAD
/*
 * Emulation of 4.2BSD select system call.  This is somewhat crude but
 * better than nothing.  We do FIONREAD on each fd, and if we have to
 * wait we use nap to avoid a busy wait.  The resolution of the nap
 * will hurt response - so will the fact that we ignore the write fds.
 * If we are simulating nap with a 1 second sleep, this will be very poor.
 *
 * nfds is the number of fds to check - this is usually 20.
 * prfds is a pointer to a bit vector of file descriptors - in the case
 *	where nfds < 32, prfds points to an integer, where bit 1<<fd
 *	is 1 if we are supposed to check file descriptor fd.
 * pwfds is like prfds but for write checks instead of read checks.
 * ms is the max number of milliseconds to wait before returning failure.
 * The value returned is the number of file descriptors ready for input.
 * The bit vectors are updated in place.
 */

int
select(nfds, prfds, pwfds, pefds, timeout)
register	int	nfds;
int		*prfds, *pwfds, *pefds;
struct		_timeval	*timeout;
{
    register	int	fd;
    register	int	rfds = *prfds;
    register	int	n;
    int		nwaiting, rv = 0;
    long	ms = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;

    while (TRUE)
    {
	/* check the fds */
	for (fd=0; fd<nfds; fd++)
	    if (1<<fd & rfds)
	    {
		ioctl(fd, FIONREAD, &nwaiting);
		if (nwaiting > 0)
		{
		    rv++;
		}
		else
		    *prfds &= ~(1<<fd);
	    }
	if (rv)
	    return (rv);

	/* Nothing ready.  Should we give up? */
	if (ms <= 0)
	    return (0);

	*prfds = rfds;	/* we clobbered it, so restore. */

	/* Wait a bit */
	n = NAPINTERVAL;
	if (ms < NAPINTERVAL)
		n = ms;
	ms -= n;
	napms(n);
    }
}
#else	/* !FIONREAD */
/*ARGSUSED*/
int
select(nfds, prfds, pwfds, pefds, time_out)
register int nfds;
int *prfds, *pwfds, *pefds;
struct _timeval *time_out;
{
    /* Can't do it, but at least compile right */
    return (ERR);
}
#endif	/* FIONREAD */
#endif	/* NEEDSELECT */
