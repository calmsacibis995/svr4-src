/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rtld:common/dllib.h	1.2"

/* the header "rtld.h" must be included
 * before this file
 */

/* information for dlopen, dlsym, dlclose on libraries linked by
 * rtld
 * Each shared object referred to in a dlopen call has an associated
 * dllib structure.  For each such structure there is a list of
 * the link objects dependent on that shared object.  The list
 * is created in DL_LISTSIZE object chunks.
 */

#define DL_LISTSIZE 5


struct liblist {
	struct rt_private_map *l_list[DL_LISTSIZE];
	struct liblist *l_next;
};

typedef struct liblist DLLIST;

struct dllib {
	DLLIST *dl_list;		/* objects dependent on this object */
	char *dl_name;			/* pathname of first */
	int dl_status;			/* status of first */
	struct dllib *dl_next;		/* next dllib struct */
};

typedef struct dllib DLLIB;

/* values used for dl_status */

#define DL_CLOSED	0		/* ref count 0, but not deleted */
#define DL_OPEN		1		/* ref count > 0 */
#define DL_DELETED     -1		/* files unmapped */

