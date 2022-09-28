/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:pipletr.c	1.10.3.1"
#include "mail.h"
/*
 * Returns exit code of surrogate process
 */
pipletr(letter, command, cltype)
int	letter, cltype;
char	*command;
{
	static char pn[] = "pipletr";
	char	**argvec;
	pid_t	pid;
	int	i, pfd[2];
	FILE	*sfp;
	void	(*savsig)();

	/*
	 * fork/exec the surrogate. Note that it is BY DESIGN that the
	 * surrogates are invoked with the gid of mail. This is so that
	 * they can check, if they care, that it it rmail(1) that called
	 * them and not some arbitrary user.
	 */
	/*
	 * dflag == 1 ==> sending non-delivery notification. If this fails
	 *                we are going to drop it on the floor anyway, so
	 *                don't save any particulars.....
	 */
	if ((dflag != 1) && SURRcmdstr != (char *)NULL) {
		free (SURRcmdstr);
	}
	/* Save command string in case of surrogate FAILURE so we can */
	/* include it with the non-delivery notification */
	if (dflag != 1) {
	    if ((SURRcmdstr = malloc (strlen(command) + 1)) == (char *)NULL) {
		Dout(pn, 0, "malloc for SURRcmdstr failed\n");
		return(-1);
	    }
	    strcpy (SURRcmdstr, command);
	}
	if ((argvec = setup_exec (command)) == (char **)NULL) {
		return(-1);
	}
	Dout(pn, 0,"arg vec to exec =\n");
	if (debug) {
		for (i= 0; argvec[i] != (char *)NULL; i++) {
			fprintf(dbgfp,"\targvec[%d] = '%s'\n", i, argvec[i]);
		}
	}
	if (pipe(pfd) < 0) {
		Dout(pn, 0, "cannot pipe. errno = %d\n", errno);
		return (-1);
	}
	if (dflag != 1) {
	    if (SURRerrfile != (FILE*)NULL) fclose(SURRerrfile);
	    if ((SURRerrfile = tmpfile()) == (FILE *)NULL) {
		Dout(pn, 0, "no tmp file. errno = %d\n", errno);
		return (-1);
	    }
	}
	if ((pid = fork()) < 0) {
		Dout(pn, 0, "cannot fork, errno = %d\n", errno);
		return(-1);
	}
	if (pid == CHILD) {
		for (i = SIGHUP; i < SIGTERM; i++) {
			setsig(i, exit);
		}
		/* redirect stdin from pipe */
		close (0); dup (pfd[0]);
		/*
		 * Redirect stdout & stderr output of surrogate command to file
		 * in case needed later...
		 */
		close (1); dup(fileno(SURRerrfile));
		close (2); dup(1);

		/* close all unnecessary file descriptors */
		for (i=3; i<_NFILE; i++) {
			close (i);
		}

		execvp (*argvec, argvec);
		Dout(pn, 0, "execvp failed. errno = %d\n", errno);
		_exit (-1);
	}
	/* parent */
	close (pfd[0]);
	sfp = fdopen (pfd[1], "w");
	/* In case surrogate doesn't want to see the message, must ignore */
	/* pipe errors */
	savsig = signal (SIGPIPE, SIG_IGN);
	copylet(letter, sfp, cltype);
	fclose (sfp);
	signal (SIGPIPE, savsig);
	return (dowait(pid));
}

dowait(pidval)
pid_t	pidval;
{
	register pid_t w;
	int status;
	void (*istat)(), (*qstat)();

	/*
		Parent temporarily ignores signals so it will remain 
		around for command to finish
	*/
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);

	while ((w = wait(&status)) != pidval && w != CERROR);
	if (w == CERROR) {
		status = -errno;
		signal(SIGINT, istat);
		signal(SIGQUIT, qstat);
		return (status);
	}

	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);
	status = ((status>>8)&0xFF);  		/* extract 8 high order bits */
	return (status);
}

/*
	invoke shell to execute command waiting for command to terminate
		s	-> command string
	return:
		status	-> command exit status
*/
systm(s)
char *s;
{
	pid_t	pid;

	/*
		Spawn the shell to execute command, however, since the 
		mail command runs setgid mode reset the effective group 
		id to the real group id so that the command does not
		acquire any special privileges
	*/
	if ((pid = fork()) == CHILD) {
		setuid(my_uid);
		setgid(my_gid);
#ifdef SVR3
		execl("/bin/sh", "sh", "-c", s, (char*)NULL);
#else
		execl("/usr/bin/sh", "sh", "-c", s, (char*)NULL);
#endif
		exit(127);
	}
	return (dowait(pid));
}
