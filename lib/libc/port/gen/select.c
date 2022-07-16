/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident  "@(#)libc-port:gen/select.c	1.4.2.1"

/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

/*
 * Emulation of select() system call using poll() system call.
 *
 * Assumptions:
 *      polling for input only is most common.
 *      polling for exceptional conditions is very rare.
 *
 * Note that is it not feasible to emulate all error conditions,
 * in particular conditions that would return EFAULT are far too
 * difficult to check for in a library routine.
 *
 */

#ifndef DSHLIB
#ifdef __STDC__
        #pragma weak select = _select
#endif
#endif
#include "synonyms.h"
#include <values.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/poll.h>

#ifndef NULL
#define NULL    0
#endif

extern  int errno;

int
select(nfds, in0, out0, ex0, tv)
        int nfds;
        fd_set *in0, *out0, *ex0;
        struct timeval *tv;
{
        /* register declarations ordered by expected frequency of use */
        register long *in, *out, *ex;
        register u_long m;      /* bit mask */
        register int j;         /* loop counter */
        register u_long b;      /* bits to test */
        register int n, rv, ms;
        struct pollfd pfd[FD_SETSIZE];
        register struct pollfd *p = pfd;
        int lastj = -1;
        /* "zero" is read-only, it could go in the text segment */
#ifndef DSHLIB
        static fd_set zero = { 0 };
#else
        fd_set zero;
        memset(&zero,0,sizeof(fd_set));
#endif

        if (nfds < 0) {
                errno = EINVAL;
                return (-1);
        }

        /*
         * If any input args are null, point them at the null array.
         */
        if (in0 == NULL)
                in0 = &zero;
        if (out0 == NULL)
                out0 = &zero;
        if (ex0 == NULL)
                ex0 = &zero;

        /*
         * For each fd, if any bits are set convert them into
         * the appropriate pollfd struct.
         */
        in = (long *)in0->fds_bits;
        out = (long *)out0->fds_bits;
        ex = (long *)ex0->fds_bits;
        for (n = 0; n < nfds; n += NFDBITS) {
                b = (u_long)(*in | *out | *ex);
                for (j = 0, m = 1; b != 0; j++, b >>= 1, m <<= 1) {
                        if (b & 1) {
                                p->fd = n + j;
                                if (p->fd >= nfds)
                                        goto done;
                                p->events = 0;
                                if (*in & m)
                                        p->events |= POLLRDNORM;
                                if (*out & m)
                                        p->events |= POLLWRNORM;
                                if (*ex & m)
                                        p->events |= POLLRDBAND;
                                p++;
                        }
                }
                in++;
                out++;
                ex++;
        }
done:
        /*
         * Convert timeval to a number of millseconds.
         * Test for zero cases to avoid doing arithmetic.
         * XXX - this is very complex, is it worth it?
         */
        if (tv == NULL) {
                ms = -1;
        } else if (tv->tv_sec == 0) {
                if (tv->tv_usec == 0) {
                        ms = 0;
                } else if (tv->tv_usec < 0 || tv->tv_usec > 1000000) {
                        errno = EINVAL;
                        return (-1);
                } else {
                        /*
                         * lint complains about losing accuracy,
                         * but I know otherwise.  Unfortunately,
                         * I can't make lint shut up about this.
                         */
                        ms = (int)(tv->tv_usec / 1000);
                }
        } else if (tv->tv_sec > (MAXINT) / 1000) {
                if (tv->tv_sec > 100000000) {
                        errno = EINVAL;
                        return (-1);
                } else {
                        ms = MAXINT;
                }
        } else if (tv->tv_sec > 0) {
                /*
                 * lint complains about losing accuracy,
                 * but I know otherwise.  Unfortunately,
                 * I can't make lint shut up about this.
                 */
                ms = (int)((tv->tv_sec * 1000) + (tv->tv_usec / 1000));
        } else {        /* tv_sec < 0 */
                errno = EINVAL;
                return (-1);
        }

        /*
         * Now do the poll.
         */
        n = p - pfd;            /* number of pollfd's */
        rv = poll(pfd, (u_long)n, ms);
        if (rv < 0)             /* no need to set bit masks */
                return (rv);
        else if (rv == 0) {
                /*
                 * Clear out bit masks, just in case.
                 * On the assumption that usually only
                 * one bit mask is set, use three loops.
                 */
                if (in0 != &zero) {
                        in = (long *)in0->fds_bits;
                        for (n = 0; n < nfds; n += NFDBITS)
                                *in++ = 0;
                }
                if (out0 != &zero) {
                        out = (long *)out0->fds_bits;
                        for (n = 0; n < nfds; n += NFDBITS)
                                *out++ = 0;
                }
                if (ex0 != &zero) {
                        ex = (long *)ex0->fds_bits;
                        for (n = 0; n < nfds; n += NFDBITS)
                                *ex++ = 0;
                }
                return (0);
        }

        /*
         * Check for EINVAL error case first to avoid changing any bits
         * if we're going to return an error.
         */
        for (p = pfd, j = n; j-- > 0; p++) {
                /*
                 * select will return EBADF immediately if any fd's
                 * are bad.  poll will complete the poll on the
                 * rest of the fd's and include the error indication
                 * in the returned bits.  This is a rare case so we
                 * accept this difference and return the error after
                 * doing more work than select would've done.
                 */
                if (p->revents & POLLNVAL) {
                        errno = EBADF;
                        return (-1);
                }
        }

        /*
         * Convert results of poll back into bits
         * in the argument arrays.
         *
         * We assume POLLRDNORM, POLLWRNORM, and POLLRDBAND will only be set
         * on return from poll if they were set on input, thus we don't
         * worry about accidentally setting the corresponding bits in the
         * zero array if the input bit masks were null.
         *
         * Must return number of bits set, not number of ready descriptors
         * (as the man page says, and as poll() does).
         */
        rv = 0;
        for (p = pfd; n-- > 0; p++) {
                j = p->fd / NFDBITS;
                /* have we moved into another word of the bit mask yet? */
                if (j != lastj) {
                        /* clear all output bits to start with */
                        in = (long *)&in0->fds_bits[j];
                        out = (long *)&out0->fds_bits[j];
                        ex = (long *)&ex0->fds_bits[j];
                        /*
                         * In case we made "zero" read-only (e.g., with
                         * cc -R), avoid actually storing into it.
                         */
                        if (in0 != &zero)
                                *in = 0;
                        if (out0 != &zero)
                                *out = 0;
                        if (ex0 != &zero)
                                *ex = 0;
                        lastj = j;
                }
                if (p->revents) {
                        m = 1 << (p->fd % NFDBITS);
                        if (p->revents & POLLRDNORM) {
                                *in |= m;
                                rv++;
                        }
                        if (p->revents & POLLWRNORM) {
                                *out |= m;
                                rv++;
                        }
                        if (p->revents & POLLRDBAND) {
                                *ex |= m;
                                rv++;
                        }
                        /*
                         * Only set this bit on return if we asked about
                         * input conditions.
                         */
                        if ((p->revents & (POLLHUP|POLLERR)) &&
                            (p->events & POLLRDNORM)) {
                                if ((*in & m) == 0)
                                        rv++;   /* wasn't already set */
                                *in |= m;
                        }
                        /*
                         * Only set this bit on return if we asked about
                         * output conditions.
                         */
                        if ((p->revents & (POLLHUP|POLLERR)) &&
                            (p->events & POLLWRNORM)) {
                                if ((*out & m) == 0)
                                        rv++;   /* wasn't already set */
                                *out |= m;
                        }
                }
        }
        return (rv);
}
