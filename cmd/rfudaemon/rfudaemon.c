/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rfudaemon:rfudaemon.c	1.4.7.1"


/*
 *	User-level daemon for file sharing.
 *	Do syscall to wait for messages.
 *	When we get one, exec admin program
 *	and go back for more.
 */
#include <sys/types.h>
#include <sys/nserve.h>
#include <sys/rf_sys.h>
#include <sys/signal.h>
#include <stdio.h>
#include <errno.h>
#include <sys/resource.h>

#define DATASIZE 512
extern int errno;

main(argc,argv)
int argc;
char **argv;
{
	char	cmd[DATASIZE];
	char	buf[DATASIZE];
	int	i;
	char	*tp;
	int	fd;
	int	sig;
	struct rlimit rlimit;

	if (getrlimit(RLIMIT_NOFILE, &rlimit) == 0) {
		for (fd = 0; fd < rlimit.rlim_cur; fd++)
			(void)close(fd);
	} else {
		fprintf(stderr, "%s: getrlimit failed\n", argv[0]);
		exit(1);
	}
	for (sig = 1; sig <= MAXSIG; sig++)
		(void)signal(sig, SIG_IGN);	

	for (;;) {
		/* clear buffer */
		for (tp = buf, i = 0; i < DATASIZE; i++)
			*tp++ = '\0';

		strcpy(cmd, "/etc/rfs/rfuadmin ");

		switch (rfsys(RF_GETUMSG, buf, DATASIZE)) {
		case RFUD_GETUMSG:
			break;
		case RFUD_FUMOUNT:
			strcat(cmd, "fumount ");
			break;
		case RFUD_DISCONN:
			strcat(cmd, "disconnect ");
			break;
		case RFUD_LASTUMSG:
			exit(0);
		default:
			strcat(cmd, "error : rfsys in rfudaemon failed");
			break;
		}
		strcat(cmd, buf);
		system(cmd);
	}
}
