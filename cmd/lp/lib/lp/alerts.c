/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/alerts.c	1.17.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "limits.h"
#include "unistd.h"

#include "lp.h"

extern char		**environ;

#if	defined(__STDC__)
static void		envlist ( FILE * , char ** );
#else
static void		envlist();
#endif

/*
 * We recognize the following key phrases in the alert prototype
 * file, and replace them with appropriate values.
 */
#define NALRT_KEYS	7
# define ALRT_ENV		0
# define ALRT_PWD		1
# define ALRT_ULIMIT		2
# define ALRT_UMASK		3
# define ALRT_INTERVAL		4
# define ALRT_CMD		5
# define ALRT_USER		6

static struct {
	char			*v;
	short			len;
}			shell_keys[NALRT_KEYS] = {
#define	ENTRY(X)	X, sizeof(X)-1
	ENTRY("-ENVIRONMENT-"),
	ENTRY("-PWD-"),
	ENTRY("-ULIMIT-"),
	ENTRY("-UMASK-"),
	ENTRY("-INTERVAL-"),
	ENTRY("-CMD-"),
	ENTRY("-USER-"),
};

/*
 * These are used to bracket the administrator's command, so that
 * we can find it easily. We're out of luck if the administrator
 * includes an identical phrase in his or her command.
 */
#define ALRT_CMDSTART "## YOUR COMMAND STARTS HERE -- DON'T TOUCH ABOVE!!"
#define ALRT_CMDEND   "## YOUR COMMAND ENDS HERE -- DON'T TOUCH BELOW!!"

/**
 ** putalert() - WRITE ALERT TO FILES
 **/

int
#if	defined(__STDC__)
putalert (
	char *			parent,
	char *			name,
	FALERT *		alertp
)
#else
putalert (parent, name, alertp)
	char			*parent,
				*name;
	FALERT			*alertp;
#endif
{
	char			*path,
				cur_dir[PATH_MAX + 1],
				buf[BUFSIZ];

	int			cur_umask;

	FILE			*fpout,
				*fpin;


	if (!parent || !*parent || !name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	if (!alertp->shcmd) {
		errno = EINVAL;
		return (-1);
	}

	if (STREQU(alertp->shcmd, NAME_NONE))
		return (delalert(parent, name));

	/*
	 * See if the form/printer/print-wheel exists.
	 */

	if (!(path = makepath(parent, name, (char *)0)))
		return (-1);

	if (Access(path, F_OK) == -1) {
		if (errno == ENOENT)
			errno = ENOTDIR; /* not quite, but what else? */
		Free (path);
		return (-1);
	}
	Free (path);

	/*
	 * First, the shell command file.
	 */

	if (!(path = makepath(parent, name, ALERTSHFILE, (char *)0)))
		return (-1);

	if (!(fpout = open_lpfile(path, "w", MODE_NOEXEC))) {
		Free (path);
		return (-1);
	}
	Free (path);

	/*
	 * We use a prototype file to build the shell command,
	 * so that the alerts are easily customized. The shell
	 * is expected to handle repeat alerts and failed alerts,
	 * because the Spooler doesn't. Also, the Spooler runs
	 * each alert with the UID and GID of the administrator
	 * who defined the alert. Otherwise, anything goes.
	 */

	if (!Lp_Bin) {
		getpaths ();
		if (!Lp_Bin)
			return (-1);
	}
	if (!(path = makepath(Lp_Bin, ALERTPROTOFILE, (char *)0)))
		return (-1);

	if (!(fpin = open_lpfile(path, "r", 0))) {
		Free (path);
		return (-1);
	}
	Free (path);

	while (fgets(buf, BUFSIZ, fpin)) {
		int			key;
		char			*cp,
					*dash;

		cp = buf;
		while ((dash = strchr(cp, '-'))) {

		    *dash = 0;
		    fputs (cp, fpout);
		    *(cp = dash) = '-';

		    for (key = 0; key < NALRT_KEYS; key++)
			if (STRNEQU(
				cp,
				shell_keys[key].v,
				shell_keys[key].len
			)) {
				register char	*newline =
						(cp != buf)? "\n" : "";

				cp += shell_keys[key].len;

				switch (key) {

				case ALRT_ENV:
					fprintf (fpout, newline);
					envlist (fpout, environ);
					break;

				case ALRT_PWD:
					getcwd (cur_dir, PATH_MAX);
					fprintf (fpout, "%s", cur_dir);
					break;

				case ALRT_ULIMIT:
					fprintf (fpout, "%ld", ulimit(1, (long)0));
					break;

				case ALRT_UMASK:
					umask (cur_umask = umask(0));
					fprintf (fpout, "%03o", cur_umask);
					break;

				case ALRT_INTERVAL:
					fprintf (fpout, "%ld", (long)alertp->W);
					break;

				case ALRT_CMD:
					fprintf (fpout, newline);
					fprintf (fpout, "%s\n", ALRT_CMDSTART);
					fprintf (fpout, "%s\n", alertp->shcmd);
					fprintf (fpout, "%s\n", ALRT_CMDEND);
					break;

				case ALRT_USER:
					fprintf (fpout, "%s", getname());
					break;

				}

				break;
			}
		    if (key >= NALRT_KEYS)
			fputc (*cp++, fpout);

		}
		fputs (cp, fpout);

	}
	if (feof(fpout) || ferror(fpout) || ferror(fpin)) {
		int			save_errno = errno;

		close_lpfile (fpin);
		close_lpfile (fpout);
		errno = save_errno;
		return (-1);
	}
	close_lpfile (fpin);
	close_lpfile (fpout);

	/*
	 * Next, the variables file.
	 */

	if (!(path = makepath(parent, name, ALERTVARSFILE, (char *)0)))
		return (-1);

	if (!(fpout = open_lpfile(path, "w", MODE_NOREAD))) {
		Free (path);
		return (-1);
	}
	Free (path);

	fprintf (fpout, "%d\n", alertp->Q > 0? alertp->Q : 1);
	fprintf (fpout, "%d\n", alertp->W >= 0? alertp->W : 0);

	close_lpfile (fpout);

	return (0);
}

/**
 ** getalert() - EXTRACT ALERT FROM FILES
 **/

FALERT *
#if	defined(__STDC__)
getalert (
	char *			parent,
	char *			name
)
#else
getalert (parent, name)
	char			*parent,
				*name;
#endif
{
	static FALERT		alert;

	register char		*path;

	char			buf[BUFSIZ];

	FILE			*fp;

	int			len;


	if (!parent || !*parent || !name || !*name) {
		errno = EINVAL;
		return (0);
	}

	/*
	 * See if the form/printer/print-wheel exists.
	 */

	if (!(path = makepath(parent, name, (char *)0)))
		return (0);

	if (Access(path, F_OK) == -1) {
		if (errno == ENOENT)
			errno = ENOTDIR; /* not quite, but what else? */
		Free (path);
		return (0);
	}
	Free (path);

	/*
	 * First, the shell command file.
	 */

	if (!(path = makepath(parent, name, ALERTSHFILE, (char *)0)))
		return (0);

	if (!(fp = open_lpfile(path, "r", 0))) {
		Free (path);
		return (0);
	}
	Free (path);

	/*
	 * Skip over environment setting stuff, while loop, etc.,
	 * to find the beginning of the command.
	 */
	while (
		fgets(buf, BUFSIZ, fp)
	     && !STRNEQU(buf, ALRT_CMDSTART, sizeof(ALRT_CMDSTART)-1)
	)
		;
	if (feof(fp) || ferror(fp)) {
		int			save_errno = errno;

		close_lpfile (fp);
		errno = save_errno;
		return (0);
	}

	alert.shcmd = sop_up_rest(fp, ALRT_CMDEND);
	if (!alert.shcmd) {
		close_lpfile (fp);
		return (0);
	}

	/*
	 * Drop terminating newline.
	 */
	if (alert.shcmd[(len = strlen(alert.shcmd)) - 1] == '\n')
		alert.shcmd[len - 1] = 0;

	close_lpfile (fp);

	/*
	 * Next, the variables file.
	 */

	if (!(path = makepath(parent, name, ALERTVARSFILE, (char *)0)))
		return (0);

	if (!(fp = open_lpfile(path, "r", 0))) {
		Free (path);
		return (0);
	}
	Free (path);

	(void)fgets (buf, BUFSIZ, fp);
	if (ferror(fp)) {
		int			save_errno = errno;

		close_lpfile (fp);
		errno = save_errno;
		return (0);
	}
	alert.Q = atoi(buf);

	(void)fgets (buf, BUFSIZ, fp);
	if (ferror(fp)) {
		int			save_errno = errno;

		close_lpfile (fp);
		errno = save_errno;
		return (0);
	}
	alert.W = atoi(buf);

	close_lpfile (fp);

	return (&alert);
}

/**
 ** delalert() - DELETE ALERT FILES
 **/

int
#if	defined(__STDC__)
delalert (
	char *			parent,
	char *			name
)
#else
delalert (parent, name)
	char			*parent,
				*name;
#endif
{
	char			*path;


	if (!parent || !*parent || !name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	/*
	 * See if the form/printer/print-wheel exists.
	 */

	if (!(path = makepath(parent, name, (char *)0)))
		return (-1);

	if (Access(path, F_OK) == -1) {
		if (errno == ENOENT)
			errno = ENOTDIR; /* not quite, but what else? */
		Free (path);
		return (-1);
	}
	Free (path);

	/*
	 * Remove the two files.
	 */

	if (!(path = makepath(parent, name, ALERTSHFILE, (char *)0)))
		return (-1);
	if (rmfile(path) == -1) {
		Free (path);
		return (-1);
	}
	Free (path);

	if (!(path = makepath(parent, name, ALERTVARSFILE, (char *)0)))
		return (-1);
	if (rmfile(path) == -1) {
		Free (path);
		return (-1);
	}
	Free (path);

	return (0);
}

/**
 ** envlist() - PRINT OUT ENVIRONMENT LIST SAFELY
 **/

static void
#if	defined(__STDC__)
envlist (
	FILE *			fp,
	char **			list
)
#else
envlist (fp, list)
	register FILE		*fp;
	register char		**list;
#endif
{
	register char		*env,
				*value;

	if (!list || !*list)
		return;

	while ((env = *list++)) {
		if (!(value = strchr(env, '=')))
			continue;
		*value++ = 0;
		if (!strchr(value, '\''))
			fprintf (fp, "export %s; %s='%s'\n", env, env, value);
		*--value = '=';
	}
}

/**
 ** printalert() - PRINT ALERT DESCRIPTION
 **/

void
#if	defined(__STDC__)
printalert (
	FILE *			fp,
	FALERT *		alertp,
	int			isfault
)
#else
printalert (fp, alertp, isfault)
	register FILE		*fp;
	register FALERT		*alertp;
	int			isfault;
#endif
{
	if (!alertp->shcmd) {
		if (isfault)
			(void)fprintf (fp, "On fault: no alert\n");
		else
			(void)fprintf (fp, "No alert\n");

	} else {
		register char	*copy = Strdup(alertp->shcmd),
				*cp;

		if (isfault)
			(void)fprintf (fp, "On fault: ");
		else
			if (alertp->Q > 1)
				(void)fprintf (
					fp,
					"When %d are queued: ",
					alertp->Q
				);
			else
				(void)fprintf (fp, "Upon any being queued: ");

		if (copy && (cp = strchr(copy, ' ')))
			while (*cp == ' ')
				*cp++ = 0;

		if (
			copy
		     && syn_name(cp)
		     && (
				STREQU(copy, NAME_WRITE)
			     || STREQU(copy, NAME_MAIL)
			)
		)
			(void)fprintf (fp, "%s to %s ", copy, cp);
		else
			(void)fprintf (fp, "alert with \"%s\" ", alertp->shcmd);

		if (alertp->W > 0)
			(void)fprintf (fp, "every %d minutes\n", alertp->W);
		else
			(void)fprintf (fp, "once\n");

		Free (copy);
	}
	return;
}
