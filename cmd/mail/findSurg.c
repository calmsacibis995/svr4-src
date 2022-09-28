/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:findSurg.c	1.8.3.1"
#include "mail.h"

static void dumplbra();

int findSurg (letnum, execbuf, flag, psurr_num, paccept, lorig, lrecipname)
int	letnum;
string	*execbuf;
int	flag;	/* DELIVER or POSTDELIVER */
int	*psurr_num;
int	*paccept;
string	*lorig;
string	*lrecipname;
{
	static char	pn[] = "findSurg";
	int		rc = NOMATCH;
	char		*lbraslist[20], *lbraelist[20];
	register int	i, j;

	for ( ; *psurr_num < surr_len; (*psurr_num)++) {
		Tout(pn,"---------- Surrogate entry '%d' ----------\n",
		    *psurr_num);
		Tout(pn, "\toriginator field = '%s'\n",
		    s_to_c(surrfile[*psurr_num].orig_pattern));
		Tout("", "\trecipient field = '%s'\n",
		    s_to_c(surrfile[*psurr_num].recip_pattern));
		Tout("", "\tcommand type = '%c'\n",
		    (char)surrfile[*psurr_num].surr_type);

		/*
		 * Check for appropriate command type before doing any
		 * unnecessary work.....
		 */
		switch (surrfile[*psurr_num].surr_type) {
		case t_postprocess:
			if (flag != POSTDELIVER)
			    continue;
			break;

		case t_transport:
		case t_translate:
		case t_deny:
		case t_accept:
			if (flag != DELIVER)
			    continue;
			break;
		}

		if (!step(s_to_c(lorig), surrfile[*psurr_num].orig_regex)) {
			/* Didn't match originator */
			Tout(pn,"no match on originator\n");
			continue;
		}
		Tout(pn,"matched originator\n");
		for (i = j = 0; i < surrfile[*psurr_num].orig_nbra; i++, j++) {
			lbraslist[j] = Rpath + (braslist[j] - s_to_c(lorig));
			lbraelist[j] = Rpath + (braelist[j] - s_to_c(lorig));
			dumplbra(" bra", braslist[i], braelist[i], i);
			dumplbra("lbra", lbraslist[j], lbraelist[j], j);
		}

		if (!step(s_to_c(lrecipname), surrfile[*psurr_num].recip_regex)) {
			/* Didn't match recipient */
			Tout(pn,"no match on recipient\n");
			continue;
		}
		Tout(pn,"matched recipient\n");
		for (i = 0; i < surrfile[*psurr_num].recip_nbra; i++, j++) {
			lbraslist[j] = recipname +
				(braslist[i] - s_to_c(lrecipname));
			lbraelist[j] = recipname +
				(braelist[i] - s_to_c(lrecipname));
			dumplbra(" bra", braslist[i], braelist[i], i);
			dumplbra("lbra", lbraslist[j], lbraelist[j], j);
		}
		for ( ; j < 20; j++)
			lbraslist[j] = lbraelist[j] = 0;

		if (flag == DELIVER) {
			switch (surrfile[*psurr_num].surr_type) {
			case t_accept:
				Tout(pn, "Accept <- TRUE\n");
				*paccept = TRUE;
				continue;

			case t_deny:
				Tout(pn, "Deny?\n");
				if (*paccept) {
				    Tout(pn,
					"'Deny' ignored due to prior 'Accept'\n");
				    continue;
				}
				Tout("", "\tDenied!\n");
				return DENY;

			case t_postprocess:
				Tout(pn, "Skipping\n");
				continue;
			}
		} else /* (flag == POSTDELIVER) */ {
			switch (surrfile[*psurr_num].surr_type) {
			case t_accept:
			case t_deny:
			case t_translate:
			case t_transport:
				Tout(pn, "Skipping\n");
				continue;

			case t_postprocess:
				break;
			}
		}

		Tout(pn, "command field = '%s'\n",
		    s_to_c(surrfile[*psurr_num].cmd_left));

		/* Matched both originator AND recipient. Expand the
		   \N and %keywords in the command string */
		s_restart(execbuf);
		cmdexpand(letnum, surrfile[*psurr_num].cmd_left,
		    execbuf, lbraslist, lbraelist);

		Tout(pn, "expanded command field = '%s'\n", s_to_c(execbuf));

		/* At this point, execbuf contains the expanded cmd field */
		/* entry. Now must determine what type of command it is and */
		/* take any required actions... */
		switch (surrfile[*psurr_num].surr_type) {
		case t_postprocess:
			return POSTDELIVER;

		case t_translate:
			return TRANSLATE;

		case t_transport:
			return DELIVER;
		}
	}

	return (rc);
}

static void dumplbra(name, a, b, j)
char *name, *a, *b;
int j;
{
	static char	pn[] = "dumplbra";
	if (debug > 5) {
		Dout(pn, 5, "%s[%d] = %#lx-%#lx = '", name, j, (long)a, (long)b);
		for ( ; a < b; a++)
			Dout("", 5, "%c", *a);
		Dout("", 5, "'\n");
	}
}
