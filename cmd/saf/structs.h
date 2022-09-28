/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)saf:structs.h	1.5.3.1"

/*
 * REQUIRES: sac.h misc.h
 */


/*
 * error messages
 */

struct errmsg {
	char *e_str;			/* error string */
	int e_exitcode;			/* and associated exit status */
};


/*
 * everything we need to know about a port monitor
 */

struct	sactab {
	long	sc_flags;		/* flags */
	pid_t	sc_pid;			/* pid of PM */
	int	sc_rsmax;		/* max # of restarts */
	int	sc_rscnt;		/* # of restarts */
	int	sc_fd;			/* _pmpipe fd */
	int	sc_ok;			/* true if responded to last sanity poll */
	int	sc_valid;		/* true if entry is "current" */
	char	*sc_cmd;		/* command */
	char	*sc_comment;		/* comment associated with entry */
	struct	sactab	*sc_next;	/* next in list */
	short	sc_exit;		/* exit status */
	char	sc_maxclass;		/* largest class instruction this PM
					   understands.  This is currently
					   a place holder for future messages */
	unchar	sc_sstate;		/* SAC's idea of PM's state */
	unchar	sc_lstate;		/* SAC's idea of last valid state -
					   used for failure recovery - note:
					   SAC will set this field to ENABLED,
					   DISABLED, or NOTRUNNING as appropriate */
	unchar	sc_pstate;		/* PM's last reported state - note:
					   SAC will set this field to STARTING,
					   NOTRUNNING, or FAILED as appropriate */
	char	sc_tag[PMTAGSIZE + 1];	/* port monitor tag */
	char	sc_type[PMTYPESIZE + 1];/* port monitor type */
	char	sc_utid[IDLEN];		/* utmp id of PM */
};

/*
 * defn's for sc_sstate, sc_pstate, and sc_lstate
 */

# define NOTRUNNING	0	/* PM not running */
# define STARTING	1	/* PM is starting, must be same as PM_STARTING */
# define ENABLED	2	/* PM is enabled, must be same as PM_ENABLED */
# define DISABLED	3	/* PM is disabled, must be same as PM_DISABLED */
# define STOPPING	4	/* PM is stopping, must be same as PM_STOPPING */
# define FAILED		5	/* PM has failed */
# define UNKNOWN	6	/* in recovery, state unknown */

/*
 * defn's for sc_flags
 */

# define D_FLAG	0x1
# define X_FLAG	0x2
