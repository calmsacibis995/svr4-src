/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lp:cmd/lpadmin/do_printer.c	1.19.3.1"

#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "limits.h"
#include "sys/types.h"
#include "stdlib.h"

#include "lp.h"
#include "class.h"
#include "printers.h"
#include "msgs.h"

#define	WHO_AM_I	I_AM_LPADMIN
#include	"oam.h"

#include "lpadmin.h"

extern	void	fromallclasses();

#if     !defined(PATH_MAX)
# define PATH_MAX       1024
#endif
#if     PATH_MAX < 1024
# undef PATH_MAX
# define PATH_MAX       1024
#endif

extern char		*label;
static void		configure_printer();
static char		*fullpath();
char			*nameit();

/**
 ** do_printer() - CREATE OR CHANGE PRINTER
 **/

void			do_printer ()
{
	int			rc;

	/*
	 * Set or change the printer configuration.
	 */
	if (strlen(modifications))
		configure_printer (modifications);

	/*
	 * Allow/deny forms.
	 */
	BEGIN_CRITICAL
		if (!oldp)
			if (allow_form_printer(getlist(NAME_NONE, "", ","), p) == -1) {
				LP_ERRMSG1 (ERROR, E_ADM_ACCESSINFO, PERROR);
				done(1);
			}

		if (f_allow || f_deny) {
			if (f_allow && allow_form_printer(f_allow, p) == -1) {
				LP_ERRMSG1 (ERROR, E_ADM_ACCESSINFO, PERROR);
				done(1);
			}

			if (f_deny && deny_form_printer(f_deny, p) == -1) {
				LP_ERRMSG1 (ERROR, E_ADM_ACCESSINFO, PERROR);
				done(1);
			}
		}
	END_CRITICAL

	/*
	 * Allow/deny users.
	 */
	BEGIN_CRITICAL
		if (!oldp)
			if (allow_user_printer(getlist(NAME_ALL, "", ","), p) == -1) {
				LP_ERRMSG1 (ERROR, E_ADM_ACCESSINFO, PERROR);
				done(1);
			}

		if (u_allow || u_deny) {
			if (u_allow && allow_user_printer(u_allow, p) == -1) {
				LP_ERRMSG1 (ERROR, E_ADM_ACCESSINFO, PERROR);
				done(1);
			}

			if (u_deny && deny_user_printer(u_deny, p) == -1) {
				LP_ERRMSG1 (ERROR, E_ADM_ACCESSINFO, PERROR);
				done(1);
			}
		}
	END_CRITICAL

	/*
	 * Tell the Spooler about the printer
	 */
	send_message(S_LOAD_PRINTER, p, "", "");
	rc = output(R_LOAD_PRINTER);

	switch (rc) {
	case MOK:
		break;

	case MNODEST:
	case MERRDEST:
		LP_ERRMSG (ERROR, E_ADM_ERRDEST);
		done (1);
		/*NOTREACHED*/

	case MNOSPACE:
		LP_ERRMSG (WARNING, E_ADM_NOPSPACE);
		break;

	case MNOPERM:	/* taken care of up front */
	default:
		LP_ERRMSG1 (ERROR, E_LP_BADSTATUS, rc);
		done (1);
		/*NOTREACHED*/
	}

	/*
	 * Now that the Spooler knows about the printer,
	 * we can do the balance of the changes.
	 */

	/*
	 * Mount or unmount form, print-wheel.
	 */
	if (M) 
		do_mount(p, (f? f : (char *)0), (S? *S : (char *)0));

	/*
	 * Display the alert type.
	 */
	if (A && STREQU(A, NAME_LIST)) {
		if (label)
			(void) printf("Printer %s: ", label);
		printalert (stdout, &(oldp->fault_alert), 1);
	}

	/*
	 * -A quiet.
	 */
	if (A && STREQU(A, NAME_QUIET)) {

		send_message(S_QUIET_ALERT, p, (char *)QA_PRINTER, "");
		rc = output(R_QUIET_ALERT);

		switch(rc) {
		case MOK:
			break;

		case MNODEST:	/* not quite, but not a lie either */
		case MERRDEST:
			LP_ERRMSG1 (WARNING, E_LP_NOQUIET, p);
			break;

		case MNOPERM:	/* taken care of up front */
		default:
			LP_ERRMSG1 (ERROR, E_LP_BADSTATUS, rc);
			done (1);
			/*NOTREACHED*/
		}
	}

	/*
	 * Add printer p to class c
	 */
	if (c)  {
		CLASS			*pc,
					clsbuf;

		if (STREQU(c, NAME_ANY))
			c = NAME_ALL;

Loop:		if (!(pc = getclass(c))) {
			if (STREQU(c, NAME_ALL))
				goto Done;

			if (errno != ENOENT) {
				LP_ERRMSG2 (
					ERROR,
					E_LP_GETCLASS,
					c,
					PERROR
				);
				done (1);
			}

			/*
			 * Create the class
			 */
			clsbuf.name = strdup(c);
			clsbuf.members = 0;
			if (addlist(&clsbuf.members, p) == -1) {
				LP_ERRMSG (ERROR, E_LP_MALLOC);
				done (1);
			}
			pc = &clsbuf;

		} else if (searchlist(p, pc->members))
			LP_ERRMSG2 (WARNING, E_ADM_INCLASS, p, pc->name);

		else if (addlist(&pc->members, p) == -1) {
			LP_ERRMSG (ERROR, E_LP_MALLOC);
			done (1);
		}

		BEGIN_CRITICAL
			if (putclass(pc->name, pc) == -1) {
				LP_ERRMSG2 (
					ERROR,
					E_LP_PUTCLASS,
					pc->name,
					PERROR
				);
				done(1);
			}
		END_CRITICAL

		send_message (S_LOAD_CLASS, pc->name);
		rc = output(R_LOAD_CLASS);

		switch(rc) {
		case MOK:
			break;

		case MNODEST:
		case MERRDEST:
			LP_ERRMSG (ERROR, E_ADM_ERRDEST);
			done (1);
			/*NOTREACHED*/

		case MNOSPACE:
			LP_ERRMSG (WARNING, E_ADM_NOCSPACE);
			break;

		case MNOPERM:	/* taken care of up front */
		default:
			LP_ERRMSG1 (ERROR, E_LP_BADSTATUS, rc);
			done (1);
			/*NOTREACHED*/
		}

		if (STREQU(c, NAME_ALL))
			goto Loop;
	}
Done:
	/*
	 * Remove printer p from class r
	 */
	if (r) {
		if (STREQU(r, NAME_ALL) || STREQU(r, NAME_ANY))
			fromallclasses(p);
		else 
			fromclass(p, r);
	}

	return;
}

/**
 ** configure_printer() - SET OR CHANGE CONFIGURATION OF PRINTER
 **/

static void		configure_printer (list)
	char			*list;
{
	register PRINTER	*prbufp;

	PRINTER			printer_struct;

	char			type;


	if (oldp) {

		prbufp = oldp;

		if (!T)
			T = prbufp->printer_types;

		if (!i && !e && !m)
			/*
			 * Don't copy the original interface program
			 * again, but do keep the name of the original.
			 */
			ignprinter = BAD_INTERFACE;
		else
			ignprinter = 0;

		/*
		 * If we are making this a remote printer,
		 * make sure that local-only attributes are
		 * cleared.
		 */
		if (s) {
			prbufp->banner = 0;
			prbufp->cpi.val = 0;
			prbufp->cpi.sc = 0;
			prbufp->device = 0;
			prbufp->dial_info = 0;
			prbufp->fault_rec = 0;
			prbufp->interface = 0;
			prbufp->lpi.val = 0;
			prbufp->lpi.sc = 0;
			prbufp->plen.val = 0;
			prbufp->plen.sc = 0;
			prbufp->login = 0;
			prbufp->speed = 0;
			prbufp->stty = 0;
			prbufp->pwid.val = 0;
			prbufp->pwid.sc = 0;
			prbufp->fault_alert.shcmd = strdup(NAME_NONE);
			prbufp->fault_alert.Q = 0;
			prbufp->fault_alert.W = 0;
#if	defined(CAN_DO_MODULES)
			prbufp->modules = 0;
#endif

		/*
		 * If we are making this a local printer, make
		 * sure that some local-only attributes are set.
		 * (If the user has specified these as well, his/her
		 * values will overwrite what we set here.)
		 */
		} else if (oldp->remote) {
			prbufp->banner = BAN_ALWAYS;
			prbufp->interface = makepath(Lp_Model, STANDARD, (char *)0);
			prbufp->fault_alert.shcmd = nameit(NAME_MAIL);

			/*
			 * Being here means "!s && oldp->remote" is true,
			 * i.e. this printer never had an interface pgm
			 * before. Thus we can safely clear the following.
			 * This is needed to let "putprinter()" copy the
			 * (default) interface program.
			 */
			ignprinter = 0;
		}

	} else {
		/*
		 * The following takes care of the lion's share
		 * of the initialization of a new printer structure.
		 * However, special initialization (e.g. non-zero,
		 * or substructure members) needs to be considered
		 * for EACH NEW MEMBER added to the structure.
		 */
		(void)memset (&printer_struct, 0, sizeof(printer_struct));

		prbufp = &printer_struct;
		prbufp->banner = BAN_ALWAYS;
		prbufp->cpi.val = 0;
		prbufp->cpi.sc = 0;
		if (!s)
			prbufp->interface = makepath(Lp_Model, m, (char *)0);
		prbufp->lpi.val = 0;
		prbufp->lpi.sc = 0;
		prbufp->plen.val = 0;
		prbufp->plen.sc = 0;
		prbufp->pwid.val = 0;
		prbufp->pwid.sc = 0;
		if (!s && !A)
			prbufp->fault_alert.shcmd = nameit(NAME_MAIL);
		prbufp->fault_alert.Q = 0;
		prbufp->fault_alert.W = 0;
	}

	while ((type = *list++) != '\0')  switch(type) {

	case 'A':
		if (!s) {
			if (STREQU(A, NAME_MAIL) || STREQU(A, NAME_WRITE))
				prbufp->fault_alert.shcmd = nameit(A);
			else if (!STREQU(A, NAME_QUIET))
				prbufp->fault_alert.shcmd = A;
		}
		break;

	case 'b':
		if (!s)
			prbufp->banner = banner;
		break;

	case 'c':
		if (!s)
			prbufp->cpi = cpi_sdn;
		break;

	case 'D':
		prbufp->description = D;
		break;

	case 'e':
		if (!s) {
			prbufp->interface = makepath(
				Lp_A_Interfaces,
				e,
				(char *)0
			);
		}
		break;

	case 'F':
		if (!s)
			prbufp->fault_rec = F;
		break;

#if	defined(CAN_DO_MODULES)
	case 'H':
		if (!s)
			prbufp->modules = H;
		break;
#endif

	case 'h':
		if (!s)
			prbufp->login = 0;
		break;

	case 'i':
		if (!s)
			prbufp->interface = fullpath(i);
		break;

	case 'I':
		prbufp->input_types = I;
		break;

	case 'l':
		if (!s)
			prbufp->login = 1;
		break;

	case 'L':
		if (!s)
			prbufp->plen = length_sdn;
		break;

	case 'm':
		if (!s)
			prbufp->interface = makepath(Lp_Model, m, (char *)0);
		break;

	case 'M':
		if (!s)
			prbufp->lpi = lpi_sdn;
		break;

	case 'R':
		if (s) {
			prbufp->remote = s;
			prbufp->dial_info = 0;
			prbufp->device = 0;
		} else
			prbufp->remote = 0;
		break;
		
	case 's':
		if (!s) {
			/*
			 * lpadmin always defers to stty
			 */
			prbufp->speed = 0;
			prbufp->stty = stty;
		}
		break;

	case 'S':
		if (!M)
			if (STREQU(*S, NAME_NONE))
				prbufp->char_sets = 0;
			else
				prbufp->char_sets = S;
		break;

	case 'T':
		prbufp->printer_types = T;
		break;

	case 'U':
		if (!s) {
			prbufp->dial_info = U;
			prbufp->device = 0;
			prbufp->remote = 0;
		}
		break;

	case 'v':
		if (!s) {
			prbufp->device = v;
			prbufp->dial_info = 0;
			prbufp->remote = 0;
		}
		break;

	case 'w':
		if (!s)
			prbufp->pwid = width_sdn;
		break;

	case 'W':
		if (!s)
			prbufp->fault_alert.W = W;
		break;

	}


	BEGIN_CRITICAL
		if (putprinter(p, prbufp) == -1) {
			if (
				errno == EINVAL
			     && (badprinter & BAD_INTERFACE)
			)
				LP_ERRMSG1 (
					ERROR,
					E_ADM_BADINTF,
					prbufp->interface
				);
			else
				LP_ERRMSG2 (
					ERROR,
					E_LP_PUTPRINTER,
					p,
					PERROR
				);
			done(1);
		}
	END_CRITICAL

	return;
}

/**
 ** fullpath()
 **/

static char		*fullpath (str)
	char			*str;
{
	register char		*cur_dir,
				*path;


	while (*str && *str == ' ')
		str++;
	if (*str == '/')
		return (str);

	if (!(cur_dir = malloc(PATH_MAX + 1)))
		return (str);

	getcwd (cur_dir, PATH_MAX);
	path = makepath(cur_dir, str, (char *)0);

	/*
	 * Here we could be nice and strip out /./ and /../
	 * stuff, but it isn't necessary.
	 */

	return (path);
}

/**
 ** nameit() - ADD USER NAME TO COMMAND
 **/

char			*nameit (cmd)
	char			*cmd;
{
	register char		*nm = getname(),
				*copy = malloc(
					(unsigned) (strlen(cmd) + 1 + 
					strlen(nm) + 1)
	);

	(void) strcpy (copy, cmd);
	(void) strcat (copy, " ");
	(void) strcat (copy, nm);
	return (copy);
}
