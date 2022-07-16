/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

# ifndef _SAC_H
# define _SAC_H

# ident	"@(#)/usr/src/head/sac.h.sl 1.1 4.0 03/08/90 49959 AT&T-USL"

# include <sys/types.h>

# define IDLEN		4	/* length in bytes of a utmp id */
# define SC_WILDC	0xff	/* wild character for utmp ids */
# define PMTAGSIZE	14	/* maximum length in bytes for a port monitor tag */

/*
 * values for rflag in doconfig()
 */

# define NOASSIGN	0x1	/* don't allow assign operations */
# define NORUN		0x2	/* don't allow run or runwait operations */


/*
 * message to SAC (header only).  This header is forever fixed.  The
 * size field (pm_size) defines the size of the data portion of the
 * message, which follows the header.  The form of this optional
 * data portion is defined strictly by the message type (pm_type).
 */

struct	pmmsg {
	char	pm_type;		/* type of message */
	unchar	pm_state;		/* current state of port monitor */
	char	pm_maxclass;		/* max message class this PM understands */
	char	pm_tag[PMTAGSIZE + 1];	/* port monitor's tag */
	int	pm_size;		/* size of optional data portion */
};



/*
 * pm_type values
 */

# define PM_STATUS	1		/* status response */
# define PM_UNKNOWN	2		/* an unknown message was received */

/*
 * pm_state values
 */

/*
 * Class 1 responses
 */

# define PM_STARTING	1		/* port monitor in starting state */
# define PM_ENABLED	2		/* port monitor in enabled state */
# define PM_DISABLED	3		/* port monitor in disabled state */
# define PM_STOPPING	4		/* port monitor in stopping state */

/*
 * message to port monitor
 */

struct	sacmsg {
	int	sc_size;		/* size of optional data portion */
	char	sc_type;		/* type of message */
};



/*
 * sc_type values
 * These represent commands that the SAC sends to a port monitor.  These
 * commands are divided into "classes" for extensibility.  Each subsequent
 * "class" is a superset of the previous "classes" plus the new commands
 * defined within that "class".  The header for all commands is identical;
 * however, a command may be defined such that an optional data portion may
 * be sent in addition to the header.  The format of this optional data piece
 * is self-defining based on the command.  Important note:  the first message
 * sent by the SAC will always be a class 1 message.  The port monitor
 * response will indicate the maximum class that it is able to understand.
 * Another note is that port monitors should only respond to a message with
 * an equivalent class response (i.e. a class 1 command causes a class 1
 * response).
 */

/*
 * Class 1 commands (currently, there are only class 1 commands)
 */

# define SC_STATUS	1		/* status request */
# define SC_ENABLE	2		/* enable request */
# define SC_DISABLE	3		/* disable request */
# define SC_READDB	4		/* read pmtab request */

/*
 * `errno' values for Saferrno, note that Saferrno is used by
 * both pmadm and sacadm and these values are shared between
 * them
 */

# define E_BADARGS	1	/* bad args or ill-formed command line */
# define E_NOPRIV	2	/* user not privileged for operation */
# define E_SAFERR	3	/* generic SAF error */
# define E_SYSERR	4	/* system error */
# define E_NOEXIST	5	/* invalid specification */
# define E_DUP		6	/* entry already exists */
# define E_PMRUN	7	/* port monitor is running */
# define E_PMNOTRUN	8	/* port monitor is not running */
# define E_RECOVER	9	/* in recovery */

# endif /* _SAC_H */
