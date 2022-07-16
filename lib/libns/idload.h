/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libns:idload.h	1.4.4.1"

#include <sys/param.h>
#include <sys/types.h>
#include <sys/idtab.h>

#define MAX_USERNAME	64

struct idtable {
	uid_t	h_defval;
	uid_t	h_size;
	struct 	 idtab  h_idtab[1];
};

struct nm_tab {
	char	n_rem[MAX_USERNAME];
	char	n_loc[MAX_USERNAME];
};

struct nmlist {
	struct idtab	*idmap;
	struct nm_tab	*nm_map;
	struct nmlist	*next;
};

struct lname {
	char		 l_val[MAXSNAME];
	struct lname	*l_next;
};

struct mach {
	struct lname	*m_name;
	uid_t		 m_def;
	int		 m_defname[MAX_USERNAME];
	int		 m_count;
	struct nmlist	*m_nmlist;
	struct mach	*m_next;
};

#define	SUCCESS	1
#define FAILURE	0

#define MAP	0
#define EXCLUDE	1

#define	A_MAP		01
#define	A_EXCLUDE	02

#define EQ(x,y)		(!strcmp(x,y))
#define DIGIT(x)	((x >= '0') && (x <= '9'))

#define	CONTINUATION	(uid_t)-1
#define	GETID_FAIL	(uid_t)-1

#define	COMMENT_CHR	'#'

#define	DEF_PASSDIR	"/etc/rfs/auth.info"
#define	OFF_USR_RULES	"/etc/rfs/auth.info/.<uid.rules>"
#define	OFF_GRP_RULES	"/etc/rfs/auth.info/.<gid.rules>"

/*
 *	Parse errors.
 */

#define	P_NOVAL		 1
#define	P_XDEF		 2
#define	P_INVTOK	 3
#define	P_RANGE		 4
#define	P_RNONAME	 5
#define	P_LNONAME	 6
#define	P_MAPERR	 7
#define	P_EXCERR	 8
#define	P_NODOM		 9
#define	P_NUMONLY	10
#define	P_NOHOST	11
#define	P_DUPNAME	12
#define	P_XGLOB		13
#define	P_ORDER		14
#define	P_EXISTS	15
#define	P_NOROOT	16
#define	P_EXOVER	17
#define	P_DEFORD	18
#define	P_EXORD		19
#define	P_TOOBIG	20
#define	P_LINE2BIG	21
