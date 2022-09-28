/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpstat/printer.c	1.13.3.1"

#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "sys/types.h"
#include "stdlib.h"

#include "lp.h"
#include "printers.h"
#include "msgs.h"

#define	WHO_AM_I	I_AM_LPSTAT
#include "oam.h"

#include "lpstat.h"

#if	defined(__STDC__)
static void	figure_pitch_size ( char * , SCALED * , SCALED * , SCALED * , SCALED * );
static void	printallowdeny ( FILE * , char * , char * , char ** , char ** );
static void	printpwheels ( PRINTER * , char * );
static void	printsets ( PRINTER * );
#else
static void	figure_pitch_size();
static void	printallowdeny();
static void	printpwheels();
static void	printsets();
#endif

/**
 ** do_printer()
 **/

void
#if	defined(__STDC__)
do_printer (
	char **			list
)
#else
do_printer (list)
	char			**list;
#endif
{
	while (*list) {
		if (STREQU(*list, NAME_ALL)) {
			send_message (S_INQUIRE_PRINTER_STATUS, "");
			(void)output(R_INQUIRE_PRINTER_STATUS);

		} else {
			send_message (S_INQUIRE_PRINTER_STATUS, *list);
			switch (output(R_INQUIRE_PRINTER_STATUS)) {
			case MNODEST:
				LP_ERRMSG1 (ERROR, E_LP_NOPRINTER, *list);
				exit_rc = 1;
				break;
			}
		}
		list++;
	}
	return;
}

/**
 ** putpline() - DISPLAY STATUS OF PRINTER
 **/

void
#if	defined(__STDC__)
putpline (
	char *			printer,
	int			printer_status,
	char *			request_id,
	char *			enable_date,
	char *			disable_reason,
	char *			form,
	char *			character_set
)
#else
putpline (printer, printer_status, request_id, enable_date, disable_reason, form, character_set)
	char			*printer;
	int			printer_status;
	char			*request_id,
				*enable_date,
				*disable_reason,
				*form,
				*character_set;
#endif
{
	register PRINTER	*prbufp;

	char			**u_allow	= 0,
				**u_deny	= 0,
				**f_allow	= 0,
				**f_deny	= 0;

	char **			pt;

	int			multi_type;


	if (!(prbufp = getprinter(printer))) {
		LP_ERRMSG2 (ERROR, E_LP_GETPRINTER, printer, PERROR);
		done(1);
	}

	printf ("printer %s", printer);

	if (prbufp->login)
		printf (" (login terminal)");

	if (!(printer_status & (PS_DISABLED|PS_LATER))) {
		if (printer_status & PS_FAULTED) {
			printf (" faulted");
			if (printer_status & PS_BUSY)
				printf (" printing %s", request_id);
			printf (".");
		} else if (printer_status & PS_BUSY)
			printf (" now printing %s.", request_id);
		else
			printf (" is idle.");

		printf (" enabled since %s.", enable_date);

	} else if (printer_status & PS_DISABLED)
		printf (" disabled since %s.", enable_date);

	else if (printer_status & PS_LATER)
		printf (" waiting for auto-retry.");

	(void)load_userprinter_access (printer, &u_allow, &u_deny);
	printf (
		" %s.\n",
		is_user_allowed(getname(), u_allow, u_deny)?
			  "available"
			: "not available"
	);

	if (printer_status & (PS_DISABLED|PS_LATER))
		printf ("\t%s\n", disable_reason);

	if (D && !(verbosity & (V_LONG|V_BITS)))
		printf ("\tDescription: %s\n", NB(prbufp->description));

	else if (verbosity & V_BITS) {
		register char		*sep	= "	";

		BITPRINT (printer_status, PS_REJECTED);
		BITPRINT (printer_status, PS_DISABLED);
		BITPRINT (printer_status, PS_FAULTED);
		BITPRINT (printer_status, PS_BUSY);
		BITPRINT (printer_status, PS_LATER);
		BITPRINT (printer_status, PS_REMOTE);
		if (sep[0] == '|')
			printf ("\n");

	} else if (verbosity & V_LONG) {

	    if (!prbufp->remote)
		printf ("\tForm mounted: %s\n", NB(form));

		printf ("\tContent types:");
		if (prbufp->input_types) {
			printlist_setup (" ", 0, ",", "");
			printlist (stdout, prbufp->input_types);
			printlist_unsetup ();
		}
		printf ("\n");

		printf ("\tPrinter types:");
		if (prbufp->printer_types) {
			printlist_setup (" ", 0, ",", "");
			printlist (stdout, prbufp->printer_types);
			printlist_unsetup ();
		} else
			printf (" (%s)", NAME_UNKNOWN);
		printf ("\n");

		printf ("\tDescription: %s\n", NB(prbufp->description));

	    if (!prbufp->remote)
		printf (
			"\tConnection: %s\n",
			(prbufp->dial_info? prbufp->dial_info : NAME_DIRECT)
		);

	    if (!prbufp->remote)
		printf ("\tInterface: %s\n", NB(prbufp->interface));

	    if (!prbufp->remote) {
		if (is_user_admin()) {
			printf ("\t");
			printalert (stdout, &(prbufp->fault_alert), 1);
		}
		printf (
			"\tAfter fault: %s\n",
		(prbufp->fault_rec? prbufp->fault_rec : NAME_CONTINUE)
		);
	    }

		(void)load_formprinter_access (printer, &f_allow, &f_deny);
		printallowdeny (stdout, "\tUsers ", 0, u_allow, u_deny);
		printallowdeny (stdout, "\tForms ", 0, f_allow, f_deny);

		printf (
			"\tBanner %srequired\n",
			(prbufp->banner & BAN_ALWAYS ? "" : "not ")
		);
	
		if (prbufp->daisy) {
			printf ("\tPrint wheels:\n");
			printpwheels (prbufp, character_set);
		} else {
			printf ("\tCharacter sets:\n");
			printsets (prbufp);
		}

	    multi_type = (lenlist(prbufp->printer_types) > 1);
	    for (pt = prbufp->printer_types; *pt; pt++) {

		SCALED			cpi;
		SCALED			lpi;
		SCALED			pwid;
		SCALED			plen;

		cpi = prbufp->cpi;
		lpi = prbufp->lpi;
		pwid = prbufp->pwid;
		plen = prbufp->plen;

		figure_pitch_size (*pt, &cpi, &lpi, &pwid, &plen);

		printf (
			"\tDefault pitch%s%s%s:",
			(multi_type? " (" : ""),
			(multi_type?  *pt : ""),
			(multi_type?  ")" : "")
		);
		if (cpi.val == N_COMPRESSED)
			printf (" %s CPI", NAME_COMPRESSED);
		else {
			printsdn_setup (" ", " CPI", "");
			printsdn (stdout, cpi);
		}
		printsdn_setup (" ", " LPI", "");
		printsdn (stdout, lpi);
		printf ("\n");

		printf (
			"\tDefault page size%s%s%s:",
			(multi_type? " (" : ""),
			(multi_type?  *pt : ""),
			(multi_type?  ")" : "")
		);
		printsdn_setup (" ", " wide", "");
		printsdn (stdout, pwid);
		printsdn_setup (" ", " long", "");
		printsdn (stdout, plen);
		printf ("\n");

		printsdn_unsetup ();
	    }

	    if (!prbufp->remote)
		printf ("\tDefault port settings: %s ", NB(prbufp->stty)); 
	    if (!prbufp->remote)
		if (prbufp->speed && prbufp->dial_info)
			if (!STREQU(prbufp->dial_info, NAME_DIRECT))
				printf ("%s\n", NB(prbufp->speed));

		printf ("\n");

	}


	return;
}

/**
 ** figure_pitch_size() - CALCULATE *REAL* DEFAULT PITCH, PAGE SIZE
 **/

static void
#if	defined(__STDC__)
figure_pitch_size (
	char *			type,
	SCALED *		cpi,
	SCALED *		lpi,
	SCALED *		pwid,
	SCALED *		plen
)
#else
figure_pitch_size (type, cpi, lpi, pwid, plen)
	char *			type;
	SCALED			*cpi;
	SCALED			*lpi;
	SCALED			*pwid;
	SCALED			*plen;
#endif
{
	short			orc,
				orhi,
				orl,
				orvi,
				cols,
				lines;

	/*
	 * The user want's to know how the page will look if
	 * he or she uses this printer. Thus, if the administrator
	 * hasn't set any defaults, figure out what they are from
	 * the Terminfo entry.
	 */
	if (!type || STREQU(type, NAME_UNKNOWN))
		return;

	/*
	 * NOTE: We should never get a failure return unless
	 * someone has trashed the printer configuration file.
	 * Also, if we don't fail the first time, we can't fail
	 * subsequently.
	 */
	if (tidbit(type, "orc", &orc) == -1)
		return;
	(void)tidbit (type, "orhi", &orhi);
	(void)tidbit (type, "orl", &orl);
	(void)tidbit (type, "orvi", &orvi);
	(void)tidbit (type, "cols", &cols);
	(void)tidbit (type, "lines", &lines);

#define COMPUTE(ORI,OR) \
	(ORI != -1 && OR != -1? (int)((ORI / (double)OR) + .5) : 0)

	if (cpi->val <= 0) {
		cpi->val = (float)COMPUTE(orhi, orc);
		cpi->sc = 0;
	}
	if (lpi->val <= 0) {
		lpi->val = (float)COMPUTE(orvi, orl);
		lpi->sc = 0;
	}
	if (pwid->val <= 0) {
		pwid->val = (float)cols;
		pwid->sc = 0;
	}
	if (plen->val <= 0) {
		plen->val = (float)lines;
		plen->sc = 0;
	}

	return;
}

/**
 ** printallowdeny() - PRINT ALLOW/DENY LIST NICELY
 **/

static void
#if	defined(__STDC__)
printallowdeny (
	FILE *			fp,
	char *			prefix,
	char *			suffix,
	char **			allow,
	char **			deny
)
#else
printallowdeny (fp, prefix, suffix, allow, deny)
	FILE			*fp;
	char			*prefix,
				*suffix,
				**allow,
				**deny;
#endif
{
#define	PRT(X) (void) fprintf(fp, "%s%s%s:\n", NB(prefix), X, NB(suffix))

	printlist_setup ("\t\t", 0, 0, 0);

	if (allow || deny && !*deny || !deny) {
		PRT ("allowed");
		if (allow && *allow)
			printlist (fp, allow);
		else if (allow && !*allow || !deny)
			(void) fprintf(fp, "\t\t(%s)\n", NAME_NONE);
		else
			(void) fprintf(fp, "\t\t(%s)\n", NAME_ALL);

	} else {
		PRT ("denied");
		printlist (fp, deny);

	}

	printlist_unsetup ();
	return;
}

/**
 ** printpwheels() - PRINT LIST OF PRINT WHEELS
 **/

static void
#if	defined(__STDC__)
printpwheels (
	PRINTER *		prbufp,
	char *			pwheel
)
#else
printpwheels (prbufp, pwheel)
	register PRINTER	*prbufp;
	register char		*pwheel;
#endif
{
	register char		**list;

	register int		mount_in_list	= 0,
				something_shown	= 0;


	if ((list = prbufp->char_sets))
		while (*list) {
			printf ("\t\t%s", *list);
			if (pwheel && STREQU(*list, pwheel)) {
				printf (" (mounted)");
				mount_in_list = 1;
			}
			printf ("\n");
			list++;
			something_shown = 1;
		}

	if (!mount_in_list && pwheel && *pwheel) {
		printf ("\t\t%s (mounted)\n", pwheel);
		something_shown = 1;
	}

	if (!something_shown)
		printf ("\t\t(none)\n");

	return;
}

/**
 ** printsets() - PRINT LIST OF CHARACTER SETS, WITH MAPPING
 **/

static void
#if	defined(__STDC__)
printsets (
	PRINTER *		prbufp
)
#else
printsets (prbufp)
	register PRINTER	*prbufp;
#endif
{
	register char		**alist		= prbufp->char_sets,
				*cp;

	char			**tlist = 0;


	/*
	 * We'll report the administrator defined character set aliases
	 * and any OTHER character sets we find in the Terminfo database.
	 */
	tlist = get_charsets(prbufp, 0);

	if ((!alist || !*alist) && (!tlist || !*tlist)) {
		printf ("\t\t(none)\n");
		return;
	}

	if (alist)
		while (*alist) {
			cp = strchr(*alist, '=');
			if (cp)
				*cp++ = 0;

			/*
			 * Remove the alias from the Terminfo list so
			 * we don't report it twice.
			 */
			if (dellist(&tlist, *alist) == -1) {
				LP_ERRMSG (ERROR, E_LP_MALLOC);
				done (1);
			}

			if (cp)
				printf ("\t\t%s (as %s)\n", cp, *alist);
			else
				printf ("\t\t%s\n", *alist);

			alist++;
		}

	if (tlist)
		while (*tlist)
			printf ("\t\t%s\n", *tlist++);

	return;
}
