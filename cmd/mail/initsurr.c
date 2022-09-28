/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:initsurr.c	1.6.3.1"
#include "mail.h"

/*
    NAME
	initsurrfile - initialize the surrogate file array

    SYNOPSIS
	void initsurrfile()

    DESCRIPTION
	initsurrfile() reads in the surrogate file, compiles the information
	found there, and stores everything into the surrfile array.
*/

#define SURRSIZE 50
static t_surrfile _surrfile[SURRSIZE];
t_surrfile *surrfile = &_surrfile[0];
static int surrsize = SURRSIZE;

void initsurrfile()
{
    static char pn[] = "initsurrfile";
    string *cbuf = 0;
    string *trfield = 0;
    register int i = 0;

    FILE *sfp = fopen(mailsurr, "r");

    if (!sfp)
	{
	Tout(pn,"cannot open '%s'\n", mailsurr);
	return;
	}

    (void) setsurg_rc((string*)0, DEFAULT, (int*)0);

    for (;;)
	{
	t_surrfile nsurr;
	int rc;

	nsurr.orig_pattern = s_new();

#ifdef i386
	nsurr.batchsize = -1;
#endif

	cbuf = s_reset(cbuf);

	/* Get the first pattern */
	s_putc(nsurr.orig_pattern, '^');
	if ((rc = getsurr(sfp, nsurr.orig_pattern, TRUE)) == 0)
	    {
	    /* Natural end of file in mailsurr */
	    Tout(pn,"---------- End of '%s' ----------\n", mailsurr);
	    s_free(nsurr.orig_pattern);
	    break;
	    }

	Tout(pn,"---------- Next '%s' entry ----------\n", mailsurr);

	/* Get the second pattern and the command entry */
	nsurr.recip_pattern = s_new();
	s_putc(nsurr.recip_pattern, '^');
	if ((rc < 0) ||
	    (getsurr(sfp, nsurr.recip_pattern, FALSE) < 0) ||
	    (getsurr(sfp, cbuf, FALSE) < 0))
		{
		s_terminate(nsurr.recip_pattern);
		s_terminate(cbuf);
		Tout(pn, "badly formed mailsurr entry.\n");
		Tout("", "\toriginator field = '%s'\n", s_to_c(nsurr.orig_pattern));
		Tout("", "\trecipient field = '%s'\n", s_to_c(nsurr.recip_pattern));
		Tout("", "\tcommand field = '%s'\n", s_to_c(cbuf));
		s_free(nsurr.orig_pattern);
		s_free(nsurr.recip_pattern);
		break;
		}

	/* Got one! */
	/* Anchor patterns to the ends of the string */
	s_putc(nsurr.orig_pattern, '$');
	s_terminate(nsurr.orig_pattern);
	s_putc(nsurr.recip_pattern, '$');
	s_terminate(nsurr.recip_pattern);
	Tout(pn, "\toriginator field = '%s'\n", s_to_c(nsurr.orig_pattern));
	Tout("", "\trecipient field = '%s'\n", s_to_c(nsurr.recip_pattern));
	Tout("", "\tcommand field = '%s'\n", s_to_c(cbuf));

	/* Check for appropriate command type */
	s_restart(cbuf);
	switch (s_ptr_to_c(cbuf)[0])
	    {
	    case '<':			/* Delivery */
		Tout(pn, "Found delivery command\n");
		s_skipc(cbuf);
		/* split off any S=C=F=B= */
		s_skipwhite(cbuf);
		switch (s_ptr_to_c(cbuf)[0])
		    {
		    case 'S': case 's':
		    case 'C': case 'c':
		    case 'F': case 'f':
		    case 'B': case 'b':
			if (s_ptr_to_c(cbuf)[1] == '=')
			    {
			    string *statstr = s_tok(cbuf, " \t");
			    nsurr.statlist = setsurg_rc(statstr, REAL, &nsurr.batchsize);
			    s_free(statstr);
			    break;
			    }
			/* FALLTHROUGH */

		    default:
			nsurr.statlist = setsurg_rc((string*)0, REAL, &nsurr.batchsize);
			break;
		    }

		/* store rest of line */
		nsurr.cmd_left = s_clone(cbuf);
		Tout(pn, "Command = '%s'\n", s_to_c(nsurr.cmd_left));
		nsurr.surr_type = t_transport;
		break;

	    case '>':			/* Postprocessing */
		Tout(pn, "Found postprocessing command\n");
		s_skipc(cbuf);
		/* split off any B= */
		s_skipwhite(cbuf);
		switch (s_ptr_to_c(cbuf)[0])
		    {
		    case 'B': case 'b':
			if (s_ptr_to_c(cbuf)[1] == '=')
			    {
			    string *statstr = s_tok(cbuf, " \t");
			    nsurr.batchsize = atoi(s_to_c(statstr)+2);
			    s_free(statstr);
			    break;
			    }
			/* FALLTHROUGH */

		    default:
			nsurr.batchsize = -1;
			break;
		    }

		/* store rest of line */
		nsurr.cmd_left = s_clone(cbuf);
		Tout(pn, "Command = '%s'\n", s_to_c(nsurr.cmd_left));
		nsurr.surr_type = t_postprocess;
		break;

	    case 'T': case 't':		/* Translation */
		Tout(pn, "Found translation command\n");
		/* verify spelling of translate */
		trfield = s_tok(cbuf, " \t");
		if (casncmp(s_to_c(trfield), "translate", strlen(s_to_c(trfield))))
		    {
		    Tout(pn, "Unknown command field type SKIPPED!\n");
		    s_free(trfield);
		    continue;
		    }
		s_free(trfield);

		/* split off any B=T= */
		s_skipwhite(cbuf);
		switch (s_ptr_to_c(cbuf)[0])
		    {
		    case 'B': case 'b':
		    case 'T': case 't':
			if (s_ptr_to_c(cbuf)[1] == '=')
			    {
			    string *statstr = s_tok(cbuf, " \t");
			    setsurg_bt(statstr, &nsurr.batchsize, &nsurr.fullyresolved);
			    s_free(statstr);
			    break;
			    }
			/* FALLTHROUGH */

		    default:
			nsurr.batchsize = -1;
			nsurr.fullyresolved = 0;
			break;
		    }

		/* store rest of line */
		nsurr.cmd_left = s_clone(cbuf);
		Tout(pn, "Command = '%s'\n", s_to_c(nsurr.cmd_left));
		nsurr.surr_type = t_translate;
		break;

	    case 'A': case 'a':		/* Accept */
		Tout(pn, "Found accept command\n");
		/* verify spelling of accept */
		if (casncmp(s_to_c(cbuf), "accept", strlen(s_to_c(cbuf))))
		    {
		    Tout(pn, "Unknown command field type SKIPPED!\n");
		    s_free(nsurr.orig_pattern);
		    s_free(nsurr.recip_pattern);
		    continue;
		    }

		nsurr.surr_type = t_accept;
		nsurr.batchsize = -1;
		nsurr.cmd_left = 0;
		break;

	    case 'D': case 'd':		/* Deny */
		Tout(pn, "Found deny command\n");
		/* verify spelling of deny */
		if (casncmp(s_to_c(cbuf), "deny", strlen(s_to_c(cbuf))))
		    {
		    Tout(pn, "Unknown command field type SKIPPED!\n");
		    s_free(nsurr.orig_pattern);
		    s_free(nsurr.recip_pattern);
		    continue;
		    }

		nsurr.surr_type = t_deny;
		nsurr.batchsize = -1;
		nsurr.cmd_left = 0;
		break;

	    default:
		Tout(pn, "Unknown command field type SKIPPED!\n");
		s_free(nsurr.orig_pattern);
		s_free(nsurr.recip_pattern);
		continue;
	    }

	if (nsurr.batchsize >= 0)
	    {
	    nsurr.cmd_right = s_new();
	    if (getsurr(sfp, nsurr.cmd_right, FALSE) < 0)
		{
		Tout(pn, "badly formed mailsurr entry SKIPPED.\n");
		Tout("", "\toriginator field = '%s'\n",
		    s_to_c(nsurr.orig_pattern));
		Tout("", "\trecipient field = '%s'\n",
		    s_to_c(nsurr.recip_pattern));
		Tout("", "\tcommand field = '%s'\n", s_to_c(nsurr.cmd_left));
		Tout("", "\tbatch field = '%s'\n", s_to_c(nsurr.cmd_right));
		s_free(nsurr.orig_pattern);
		s_free(nsurr.recip_pattern);
		s_free(nsurr.cmd_left);
		s_free(nsurr.cmd_right);
		break;
		}

	    else
		Tout("", "Batch field = '%s'\n", s_to_c(nsurr.cmd_right));
	    }

	else
	    nsurr.cmd_right = 0;

	/*
	 * Check for leading /.
	 */
	switch (nsurr.surr_type)
	    {
	    case t_transport:
	    case t_postprocess:
		if (s_to_c(nsurr.cmd_left)[0] != '/')
		    {
		    Tout(pn, "Must use full path name for commands. Entry SKIPPED!\n");
		    s_free(nsurr.cmd_left);
		    s_free(nsurr.cmd_right);
		    s_free(nsurr.orig_pattern);
		    s_free(nsurr.recip_pattern);
		    continue;
		    }
		break;

	    case t_translate:
		if ((s_to_c(nsurr.cmd_left)[2] == '|') &&
		    (s_to_c(nsurr.cmd_left)[3] != '/'))
		    {
		    Tout(pn, "Must use full path name for commands. Entry SKIPPED!\n");
		    s_free(nsurr.cmd_left);
		    s_free(nsurr.cmd_right);
		    s_free(nsurr.orig_pattern);
		    s_free(nsurr.recip_pattern);
		    continue;
		    }
	    }

	/*
	 * Compile the patterns.
	 */
	nsurr.orig_regex =
	    mailcompile(nsurr.orig_pattern, &nsurr.orig_reglen, &nsurr.orig_nbra);
	if (!nsurr.orig_regex)
	    {
	    Tout(pn, "originator pattern compilation failed! regerrno=%d\n",
		regerrno);
	    s_free(nsurr.orig_pattern);
	    s_free(nsurr.recip_pattern);
	    s_free(nsurr.cmd_left);
	    s_free(nsurr.cmd_right);
	    continue;
	    }
	Dout(pn, 3, "orig_reglen = '%d'\n", nsurr.orig_reglen);
	Dout(pn, 3, "orig_nbra = '%d'\n", nsurr.orig_nbra);

	nsurr.recip_regex =
	    mailcompile(nsurr.recip_pattern, &nsurr.recip_reglen, &nsurr.recip_nbra);
	if (!nsurr.recip_regex)
	    {
	    Tout(pn, "recipient pattern compilation failed! regerrno=%d\n",
		regerrno);
	    s_free(nsurr.orig_pattern);
	    s_free(nsurr.recip_pattern);
	    s_free(nsurr.cmd_left);
	    s_free(nsurr.cmd_right);
	    continue;
	    }
	Dout(pn, 3, "recip_reglen = '%d'\n", nsurr.recip_reglen);
	Dout(pn, 3, "recip_nbra = '%d'\n", nsurr.recip_nbra);

	/* save the info into the surrfile array */
	surrfile[i] = nsurr;

	/* test for a full surrfile array */
	if (++i == surrsize)
	    {
	    surrsize += 10;
	    surrfile = (surrfile == _surrfile) ?
		(t_surrfile*) malloc(surrsize * sizeof(t_surrfile)) :
		(t_surrfile*) realloc((char*)surrfile, surrsize * sizeof(t_surrfile));
	    if (!surrfile)
		{
		Tout(pn, "Cannot reallocate space for surrogate file, further entries SKIPPED!\n");
		surrsize = SURRSIZE;
		s_free(nsurr.orig_pattern);
		s_free(nsurr.recip_pattern);
		s_free(nsurr.cmd_left);
		s_free(nsurr.cmd_right);
		i--;
		break;
		}
	    }
	}

    surrfile[i].surr_type = t_eof;
    surr_len = i;
    s_free(cbuf);
}
