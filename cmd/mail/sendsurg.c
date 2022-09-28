/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:sendsurg.c	1.7.3.1"
#include "mail.h"

int sendsurg (plist, letnum, flag, local)
reciplist *plist;
int  letnum;
int flag;	/* DELIVER or POSTDELIVER */
int local;
{
	char		buf[512];
	string		*cmdstr = s_new();
	string		*lorig = s_copy(Rpath);
	string		*lrecipname = s_copy(recipname);
	static char	pn[] = "sendsurg";
	int		rc, cmdrc, n, surr_num, accept;

	Tout(pn, "============= function = '%s' ===============\n",
				((flag == DELIVER) ? "Deliver" : "Postprocess"));
	Tout("", "\toriginator = '%s', recipient = '%s'\n", Rpath, recipname);

	s_restart(lorig);
	s_tolower(lorig);
	s_restart(lrecipname);
	s_tolower(lrecipname);
	Tout("", "lower\toriginator = '%s', recipient = '%s'\n",
	    s_to_c(lorig), s_to_c(lrecipname));

	for (accept = surr_num = 0; ; surr_num++) {
		switch (rc = findSurg(letnum, cmdstr, flag, &surr_num,
				      &accept, lorig, lrecipname)) {
		default:
			Dout(pn, 0,
			    "Bad return code from findSurg(), rc = %#o\n",
			    rc);
			continue;

		case NOMATCH:
			Dout(pn, 0, "no match in mailsurr file\n");
			rc = CONTINUE;
			goto rtrn;

		case DENY:
			error = E_DENY;
			Tout(pn, "mailsurr Denial\n");
			Dout("", 0, "\terror set to %d\n", error);
			rc = FAILURE;
			goto rtrn;

		case TRANSLATE:
			Tout(pn,"'Translate' entry = '%s'\n", s_to_c(cmdstr));
			if (translate (plist, s_to_c(cmdstr), recipname) < 0) {
				continue;
			}
			goto rtrn;

		case DELIVER:
		case POSTDELIVER:
			Tout(pn,"'%c' entry = '%s'\n",
			    (rc == DELIVER ? '<' : '>'), s_to_c(cmdstr));
			if (flgT) {
				Tout(pn, "Suppressing execution phase; assuming CONTINUE\n");
				continue;
			}
			cmdrc = pipletr(letnum, s_to_c(cmdstr), local ? ORDINARY : REMOTE);
			Dout(pn, 0, "surrogate complete, result %d\n", cmdrc);
			if (debug)
			    if (SURRerrfile) {
				fprintf(dbgfp,
				   "=============== Start of stderr from surrogate ============\n");
				rewind (SURRerrfile);
				(void) copystream(SURRerrfile, dbgfp);
				fprintf(dbgfp,
				   "\n=============== End of stderr from surrogate ============\n");
			    } else
			        fprintf(dbgfp, "=============== Surrogate output unavailable ============\n");
			if (flag == POSTDELIVER) {
				continue;
			}

			switch (rc = cksurg_rc(surr_num, cmdrc)) {
			case SUCCESS:
				goto rtrn;

			case CONTINUE:
				continue;

			case FAILURE:
				error = E_SURG;
				Dout(pn, 0, "surrogate command failed, error set to %d\n", error);
				surg_rc = cmdrc;
				goto rtrn;
			}
			rc = FAILURE;
			goto rtrn;
		}
	}

    rtrn:
	s_free(cmdstr); s_free(lorig); s_free(lrecipname);
	return rc;
}
