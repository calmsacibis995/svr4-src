/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpusers.c	1.10.3.1"

/* lpusers [-q priority-level] -u (user-list | "")
   lpusers -d priority-level
   lpusers -l
*/
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>

#include "lp.h"
#include "users.h"
#include "msgs.h"

#define	WHO_AM_I	I_AM_LPUSERS
#include "oam.h"

char message[100],
     reply[100];

char	*PRIORITY;

int add_user(), del_user();

main(argc, argv)
int argc;
char *argv[];
{
    int mtype, size, c,
	list = FALSE, limit = -1, deflt = -1;
    FILE *f, *open_lpfile();
    char *userlist = 0, *user, **users, *p;
    short status;
    struct user_priority *ppri_tbl, *ld_priority_file();
    extern char *optarg;
    extern int optind, opterr, optopt, errno;

    if(argc == 1) {
usage:
#define	P	(void) printf ("%s\n",
#define	X	);
P "usage:"								X
P ""									X
P "  (assign priority limit to users)"					X
P "	lpusers -q priority -u user-list"				X
P ""									X
P "  (assign default priority limit for balance of users)"		X
P "	lpusers -q priority"						X
P ""									X
P "  (put users back to default priority limit)"			X
P "	lpusers -u user-list"						X
P ""									X
P "  (assign default priority)"						X
P "	lpusers -d priority"						X
P ""									X
P "  (examine priority limits, defaults)"				X
P "	lpusers -l"							X
	exit(argc == 1);
    }

    opterr = 0; /* disable printing of errors by getopt */
    while ((c = getopt(argc, argv, "ld:q:u:")) != -1)
	switch(c) {
	case 'l':
	    if (list)
		LP_ERRMSG1(WARNING, E_LP_2MANY, 'l');
	    list = TRUE;
	    break;
	case 'd':
	    if (deflt != -1)
		LP_ERRMSG1(WARNING, E_LP_2MANY, 'd');
	    deflt = (int)strtol(optarg,&p,10);
	    if (*p || deflt<PRI_MIN || deflt>PRI_MAX) {
		LP_ERRMSG1(ERROR, E_LP_BADPRI, optarg);
		exit(1);
	    }
	    break;
	case 'q':
	    if (limit != -1)
		LP_ERRMSG1(WARNING, E_LP_2MANY, 'q');
	    limit = (int)strtol(optarg,&p,10);
	    if (*p || limit<PRI_MIN || limit>PRI_MAX) {
		LP_ERRMSG1(ERROR, E_LP_BADPRI, optarg);
		exit(1);
	    }
	    break;
	case 'u':
	    if (userlist)
		LP_ERRMSG1(WARNING, E_LP_2MANY, 'u');
	    userlist = optarg;
	    break;
	case '?':
	    if (optopt == '?') goto usage;
	    (p = "-X")[1] = optopt;
	    if (strchr("ldqu", optopt))
		LP_ERRMSG1(ERROR, E_LP_OPTARG, p);
	    else
		LP_ERRMSG1(ERROR, E_LP_OPTION, p);
	    exit(1);
	}

    if (optind < argc) {
	LP_ERRMSG1(ERROR, E_LP_EXTRA, argv[optind]);
	exit(1);
    }

    if (((list || deflt != -1) && (limit != -1 || userlist))
	|| (list && deflt != -1)) {
	LP_ERRMSG(ERROR, E_LP_OPTCOMB);
	/* invalid combination of options */
	exit(1);
    }

    PRIORITY = Lp_Users;

    /* load existing priorities from file */
    if (!(ppri_tbl = ld_priority_file(PRIORITY))) {
	switch (errno) {
	case EBADF:
	    LP_ERRMSG1(ERROR, E_LPU_BADFORM, PRIORITY);
	    break;
	default:
	    LP_ERRMSG2(ERROR, E_LPU_BADFILE, PRIORITY, errno);
	}
	exit(1);
    }

    if (list) {
	print_tbl(ppri_tbl);
	exit (0);
    } else {
	if (userlist) {
	    users = getlist(userlist, " \t", ",");
	    if (users)
		while (user = *users++) {
		    if (del_user(ppri_tbl, user) && (limit == -1))
			LP_ERRMSG1(WARNING, E_LPU_NOUSER, user);
		    if (limit != -1) {
			if (add_user(ppri_tbl, user, limit))
			    LP_ERRMSG1(WARNING, E_LPU_BADU, user);
		    }
		}
	} else if (deflt != -1)
	    ppri_tbl->deflt = deflt;
	else
	    ppri_tbl->deflt_limit = limit;

	if (!(f = open_lpfile(PRIORITY, "w", LPU_MODE))) {
	    LP_ERRMSG1(ERROR, E_LP_ACCESS, PRIORITY);
	    exit(1);
	}
	output_tbl(f, ppri_tbl);
	fclose(f);
    }

    if (mopen()) /* error on mopen == no spooler, exit quietly */
	exit(0);

    (void)putmessage (message, S_LOAD_USER_FILE);

    if (msend(message))
	goto Error;
    if (mrecv(reply, sizeof(reply)) == -1)
	goto Error;
    mtype = getmessage(reply, R_LOAD_USER_FILE, &status);
    if (mtype != R_LOAD_USER_FILE) {
	LP_ERRMSG1 (ERROR, E_LP_BADREPLY, mtype);
	goto NoError;
    }

    if (status == 0)
	goto NoError;

Error:	LP_ERRMSG (ERROR, E_LPU_NOLOAD);

NoError:(void)mclose ();
    exit(0);
}
