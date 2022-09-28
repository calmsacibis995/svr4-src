/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)fumount:fumount.c	1.9.9.1"



#include <stdio.h>
#include <sys/types.h>
#include <sys/vnode.h>
#include <sys/rf_messg.h>
#include <sys/idtab.h>
#include <sys/nserve.h>
#include <sys/rf_sys.h>
#include <nserve.h>

#define MAXGRACE 3600

extern char *malloc();

char usage[] = 
    "Usage: fumount [-w seconds(0-%d)] resource [[-w seconds] resource]...\n";

main(argc, argv)
	int argc;
	char **argv;
{
	char dom_resrc[(2 * MAXDNAME) + 2];
	char cmd[24+SZ_RES];
	char *grace_str;
	int grace_num = 0;
	int i, advflg = 0;
	int maxclients;		/* max possible mounts of this resource */
	int numclients;		/* actual number of mounts */
	struct client *clientp;
	int exit_status = 0;	/* reset to 1 if an error is encountered */
	int argpos = 0;		/* cursor for argument list */

	if (geteuid() != 0) {
		fprintf(stderr,"fumount: must be super user\n");
		exit(1);
	}
	maxclients = rfsys(RF_TUNEABLE, T_NSRMOUNT);
	if ((clientp = (struct client *)malloc(maxclients * 
	    (sizeof(struct client)))) == NULL) {
		fprintf(stderr,"fumount: can't get enough memory\n");
		exit(1);
	}
	for (argpos = 1; argpos < argc; argpos++) {
		/*
		 * -w seconds specifies that a "grace period" of "seconds"
		 * should transpire after an initial warning before actually
		 * fumounting the resource(s).
		 */
		if (strncmp(argv[argpos], "-w", 2) == 0){
			if (*(argv[argpos] + 2) != '\0') {
				/*
				 * Handle a -wxxx here, -w xxx below.
				 */
				if (((grace_num = atoi((argv[argpos] + 2))) < 0)
				    || (grace_num > MAXGRACE)) {
					/* "seconds" invalid */
					fprintf(stderr, usage, MAXGRACE);
					exit(1);
				}
				grace_str = argv[argpos] + 2;
			} else {
				if (++argpos == argc) {
					/* trailing -w */
					fprintf(stderr, usage, MAXGRACE);
					exit(1);
				}
				if (((grace_num = atoi(argv[argpos])) < 0) || 
			    	    (grace_num > MAXGRACE)) {
					/* "seconds" invalid */
					fprintf(stderr, usage, MAXGRACE);
					exit(1);
				}
				grace_str = argv[argpos];
			}
			if (++argpos == argc) {
				/* trailing -w seconds */
				fprintf(stderr, usage, MAXGRACE);
				exit(1);
			}
		}

		if ((numclients = 
		    rfsys(RF_CLIENTS, argv[argpos], clientp)) < 0) {
			fprintf(stderr,"fumount: %s not known\n", argv[argpos]);
			exit_status = 1;
			continue;
		}
		sprintf(cmd, "unadv %s >/dev/null 2>&1\n", argv[argpos]);
		(void)system(cmd);

		if (numclients == 0) {
			/* resource is gone now, assuming unadv worked */
			continue;
		}

		/* execute remote warning script only if -w option was used */
		if (grace_num) {
			if (rfsys(RF_GETDNAME, dom_resrc, MAXDNAME) < 0) {
				fprintf(stderr,
				    "fumount: can't get domain name.\n");
				perror("fumount");
				exit_status = 1;
				continue;
			}
			strcat(dom_resrc, ".");
			strcat(dom_resrc, argv[argpos]);
			for (i = 0; i < numclients; i++) {
				sndmes(clientp[i].cl_sysid, grace_str,
				    dom_resrc);
			}
			sleep(grace_num);
		}
	
		/* blow them away */
		if (rfsys(RF_FUMOUNT, argv[argpos]) == -1) {
			perror("fumount: failure due to internal error");
			exit_status = 1;
			continue;
		}
	}
	exit(exit_status);
}
