/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)saf:adm.h	1.3.3.1"

/*
 * REQUIRES: sac.h
 */


struct	admcmd {
	char	ac_mtype;		/* type of message */
	char	ac_tag[PMTAGSIZE + 1];	/* PM tag */
	pid_t	ac_pid;			/* pid for id purposes (seq #) */
};


/*
 * ac_mtype values
 */

# define AC_START	1		/* start PM */
# define AC_KILL	2		/* kill PM */
# define AC_ENABLE	3		/* enable PM */
# define AC_DISABLE	4		/* disable PM */
# define AC_STATUS	5		/* return PM status info - ac_tag
					   is unused with this command */
# define AC_SACREAD	6		/* read _sactab - ac_tag is unused
					   with this command */
# define AC_PMREAD	7		/* tell PM to read _pmtab */


/*
 * the following structure defines the header on messages from
 * the SAC back to sacadm.  The size field (ak_size) defines the
 * size of the data portion of * the message, which follows the header.
 * The form of this optional data portion is defined strictly by the
 * request message type that caused the data to be returned (ac_mtype).
 */

struct	admack {
	char	ak_resp;		/* response code - 0 for ack, non
					   zero indicates reason for failure */
	pid_t	ak_pid;			/* pid for id purposes (seq #) */
	long	ak_size;		/* if true, indicates size of next msg */
};


/*
 * ak_resp values
 */

# define AK_ACK		0		/* requested command succeeded */
# define AK_PMRUN	1		/* PM was already running */
# define AK_PMNOTRUN	2		/* PM was not running */
# define AK_NOPM	3		/* PM does not exist */
# define AK_UNKNOWN	4		/* unknown command */
# define AK_NOCONTACT	5		/* could not contact PM */
# define AK_PMLOCK	6		/* _pid file locked on start */
# define AK_RECOVER	7		/* PM in recovery */
# define AK_REQFAIL	8		/* the request failed for some reason */
