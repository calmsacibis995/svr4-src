/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:translate.c	1.7.3.1"
#include "mail.h"

translate (plist, cmdstr, origname)
reciplist *plist;
char *cmdstr, *origname;
{
	char	*p = cmdstr;
	char	**argvec;
	char	buf[1024];
	char	sav_buf[1024];
	static char	pn[] = "translate";
	int	pfd[2];
	int	i;
	pid_t	pid;
	int	firsttime = 1;
	int	num_added = 0;
	FILE	*tfp;
	static char	execfail[] = "execvp of translate program failed.";

	Tout(pn, "translate using '%s', origname='%s'\n", cmdstr, origname);
	p = skipspace(p);

	switch (*p) {
	case 'S':
	    Tout(pn, "'S' feature not implemented. line SKIPPED\n");
	    return (-1);
	default:
    badchar:
	    Tout(pn,"unexpected char '%c' in '%s'. line SKIPPED\n", *p, cmdstr);
	    return (-1);
	case 'R':
	    if (*(++p) != '=') {
		goto badchar;
	    }
	    p = skipspace(++p);
	    switch (*p) {
	    case '\0':
		/* nothing to do! */
		Tout(pn, "found end of string, no replacement!\n");
		return (-1);

	    default:
		/* straight replacement */
		Tout(pn, "straight replacement...\n");
		if (strcmp(p, origname) == 0) {
			/* Translation returned original name. Continue...  */
			Tout("", "\nof same name!\n");
			return (-1);
		}
		Tout("", "\nadding new name '%s'\n", p);
		add_recip (plist, p, TRUE);
		return (0);

	    case '|':
		Tout(pn, "found translation pipe!\n");
		break;
	    }
	    break;
	}

	/* If we get here, then we have a translation program to exec */
	if ((argvec = setup_exec (++p)) == (char **)NULL) {
		return(-1);
	}
	Dout(pn, 0,"arg vec to exec =\n");
	if (debug > 0) {
		for (i= 0; argvec[i] != (char *)NULL; i++) {
			fprintf(dbgfp,"\targvec[%d] = '%s'\n", i, argvec[i]);
		}
	}
	if (pipe(pfd) < 0) {
		Tout(pn, "cannot setup pipe to translate pgm. errno = %d\n",
									errno);
		return (-1);
	}
#ifdef DEBUG
	else {
		Dout(pn, 0,"pfd[0] = %d, pfd[1] = %d\n", pfd[0], pfd[1]);
	}
#endif

	/* Note that we cannot use popen here because we want to */
	/* capture the error message "execfail". */
	if ((pid = fork()) < 0) {
		Tout(pn, "cannot fork translate program. errno = %d\n", errno);
		close(pfd[0]);
		close(pfd[1]);
		return(-1);
	}
	if (pid == CHILD) {
		/* redirect stdout to pipe */
		close (1); dup (pfd[1]);
		/* close all unnecessary file descriptors */
		for (i=3; i<_NFILE; i++) {
			close (i);
		}
		execvp (*argvec, argvec);
		printf("%s errno = %d\n", execfail, errno);
		exit (-1);
	}
	/* parent */
	close (pfd[1]);
	tfp = fdopen (pfd[0], "r");
	while (fgets(buf, sizeof(buf), tfp) != (char *)NULL) {
		trimnl (buf);
		Dout(pn, 9, "Translation[%d]: '%s'\n", num_added, buf);
		if (firsttime) {
			/* If exec of translate program by child fails */
			/* this will be the first (only) line read on the */
			/* pipe... */
			if (strncmp(buf, execfail, sizeof(execfail)-1) == 0) {
				Tout(pn, "%s\n", buf);
				return(-1);
			}
			firsttime = 0;
			strcpy(sav_buf, buf);
		}
		num_added++;
		madd_recip (plist, buf, TRUE);
	}
	fclose (tfp);
	dowait(pid);
	if ((num_added == 1) && (strcmp(origname, sav_buf) == 0)) {
		/* If we tried to add exactly one name to the list and */
		/* that name was identical to the original recipient */
		/* name, we can infer that no translation was performed */
		/* and that we should continue processing the current */
		/* recipient in the surrogate file..... */
		Tout(pn, "one translated name == original name\n");
		return (-1);
	}
	return (0);
}
