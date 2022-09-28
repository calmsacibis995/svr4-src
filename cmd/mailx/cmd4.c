/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:cmd4.c	1.4.5.1"

#include "rcv.h"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * More commands..
 */


/*
 * pipe messages to cmd.
 */

dopipe(str)
	char str[];
{
	register int *ip, mesg;
	register struct message *mp;
	char *cp, *cmd;
	int f, *msgvec, nowait=0;
	void (*sigint)(), (*sigpipe)();
	long lc, cc, t;
	register pid_t pid;
	int page, s, pivec[2];
	char *Shell;
	FILE *pio;

	msgvec = (int *) salloc((msgCount + 2) * sizeof *msgvec);
	if ((cmd = snarf(str, &f, 0)) == NOSTR) {
		if (f == -1) {
			printf("pipe command error\n");
			return(1);
			}
		if ( (cmd = value("cmd")) == NOSTR) {
			printf("\"cmd\" not set, ignored.\n");
			return(1);
			}
		}
	if (!f) {
		*msgvec = first(0, MMNORM);
		if (*msgvec == NULL) {
			printf("No messages to pipe.\n");
			return(1);
		}
		msgvec[1] = NULL;
	}
	if (f && getmsglist(str, msgvec, 0) < 0)
		return(1);
	if (*(cp=cmd+strlen(cmd)-1)=='&'){
		*cp=0;
		nowait++;
		}
	printf("Pipe to: \"%s\"\n", cmd);
	flush();

					/*  setup pipe */
	if (pipe(pivec) < 0) {
		perror("pipe");
		return(0);
	}

	if ((pid = vfork()) == 0) {
		close(pivec[1]);	/* child */
		fclose(stdin);
		dup(pivec[0]);
		close(pivec[0]);
		if ((Shell = value("SHELL")) == NOSTR || *Shell=='\0')
			Shell = SHELL;
		execlp(Shell, Shell, "-c", cmd, (char *)0);
		perror(Shell);
		_exit(1);
	}
	if (pid == (pid_t)-1) {		/* error */
		perror("fork");
		close(pivec[0]);
		close(pivec[1]);
		return(0);
	}

	close(pivec[0]);		/* parent */
	pio=fdopen(pivec[1],"w");
	sigint = sigset(SIGINT, SIG_IGN);
	sigpipe = sigset(SIGPIPE, SIG_IGN);

					/* send all messages to cmd */
	page = (value("page")!=NOSTR);
	lc = cc = 0;
	for (ip = msgvec; *ip && ip-msgvec < msgCount; ip++) {
		mesg = *ip;
		touch(mesg);
		mp = &message[mesg-1];
		if ((t = send(mp, pio, 0)) < 0) {
			perror(cmd);
			sigset(SIGPIPE, sigpipe);
			sigset(SIGINT, sigint);
			return(1);
		}
		lc += t;
		cc += mp->m_size;
		if (page) putc('\f', pio);
	}

	fflush(pio);
	if (ferror(pio))
	      perror(cmd);
	fclose(pio);

					/* wait */
	if (!nowait) {
		while (wait(&s) != pid);
		if (s != 0)
			printf("Pipe to \"%s\" failed\n", cmd);
	}
	if (nowait || s == 0)
		printf("\"%s\" %ld/%ld\n", cmd, lc, cc);
	sigset(SIGPIPE, sigpipe);
	sigset(SIGINT, sigint);
	return(0);
}
