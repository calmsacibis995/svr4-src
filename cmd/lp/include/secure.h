/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:include/secure.h	1.3.2.1"

#if	!defined(_LP_SECURE_H)
#define _LP_SECURE_H

#include "sys/types.h"

/**
 ** The disk copy of the secure request files:
 **/

/*
 * There are 7 fields in the secure request file.
 */
#define	SC_MAX  7
# define SC_REQID	0	/* Original request id */
# define SC_UID		1	/* Originator's user ID */
# define SC_USER	2	/* Originator's real login name */
# define SC_GID		3	/* Originator's group ID */
# define SC_SIZE	4	/* Total size of the request data */
# define SC_DATE	5	/* Date submitted (in seconds) */
# define SC_SYSTEM	6	/* Originating system */

/**
 ** The internal copy of a request as seen by the rest of the world:
 **/

typedef struct SECURE {
    uid_t	uid;
    gid_t	gid;
    off_t	size;
    time_t	date;
    char	*system;
    char	*user;
    char	*req_id;
}			SECURE;

/**
 ** Various routines.
 **/

#if	defined(__STDC__)

SECURE *	getsecure ( char * );
int		putsecure ( char *, SECURE * );
void		freesecure ( SECURE * );

#else

SECURE *	getsecure();
int		putsecure();
void		freesecure();

#endif

#endif
