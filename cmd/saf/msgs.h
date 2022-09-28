/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)saf:msgs.h	1.5.3.1"

# define CONT		0	/* continue after logging message */
# define EXIT		1	/* exit after logging message */

/*
 * message ids for logging
 */

# define E_SACOPEN	0	/* could not open _sactab */
# define E_MALLOC	1	/* malloc failed */
# define E_BADFILE	2	/* _sactab corrupt */
# define E_BADVER	3	/* version mismatch on _sactab */
# define E_CHDIR	4	/* couldn't chdir */
# define E_NOPIPE	5	/* could not open _sacpipe */
# define E_BADSTATE	6	/* internal error - bad state */
# define E_BADREAD	7	/* _sacpipe read failed */
# define E_FATTACH	8	/* fattach failed */
# define E_SETSIG	9	/* I_SETSIG failed */
# define E_READ		10	/* read failed */
# define E_POLL		11	/* poll failed */
# define E_SYSCONF	12	/* system error in _sysconfig */
# define E_BADSYSCONF	13	/* interpretation error in _sysconfig */
# define E_PIPE		14	/* pipe failed */
# define E_CMDPIPE	15	/* could not create _cmdpipe */
