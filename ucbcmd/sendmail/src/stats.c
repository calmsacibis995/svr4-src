/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbsendmail:src/stats.c	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
*/

# include "sendmail.h"
# include "mailstats.h"

SCCSID(@(#)stats.c 1.10 88/02/08 SMI); /* from UCB 5.8 5/2/86 */

struct statistics	Stat;

/*
**  KBYTES -- given a number, returns the number of Kbytes.
**
**	Used in statistics gathering of message sizes to try to avoid
**	wraparound (at least for a while.....)
**
**	Notes:
**		This function is actually a ceiling function to
**			the nearest K.
*/
#define KBYTES(bytes) (((bytes) + 1023) >> 10);

/*
**  MARKSTATS -- mark statistics
*/

markstats(e, to)
	register ENVELOPE *e;
	register ADDRESS *to;
{
	if (to == NULL)
	{
		if (e->e_from.q_mailer != NULL)
		{
			Stat.stat_nf[e->e_from.q_mailer->m_mno]++;
			Stat.stat_bf[e->e_from.q_mailer->m_mno] +=
				KBYTES(CurEnv->e_msgsize);
		}
	}
	else
	{
		Stat.stat_nt[to->q_mailer->m_mno]++;
		Stat.stat_bt[to->q_mailer->m_mno] += KBYTES(CurEnv->e_msgsize);
	}
}
/*
**  POSTSTATS -- post statistics in the statistics file
**
**	Parameters:
**		sfile -- the name of the statistics file,
**			 or a pointer to an empty string if no statistics
**			 collection is desired.
**
**	Returns:
**		none.
**
**	Side Effects:
**		merges the Stat structure with the sfile file.
*/

poststats(sfile)
	char *sfile;
{
	register int fd;
	struct statistics stat;
	extern off_t lseek();
	int n;

	if (sfile == NULL || *sfile == '\0') 
		return;

	for (n=0; n<MAXMAILERS;n++)
	     if (Mailer[n] && Mailer[n]->m_name)
		strncpy(Stat.stat_names[n], Mailer[n]->m_name, NAMELEN);
	    else
		(void) strcpy(Stat.stat_names[n], "");

	(void) time(&Stat.stat_itime);
	Stat.stat_size = sizeof Stat;

	fd = open(sfile, 2);
	if (fd < 0)
	{
		errno = 0;
		return;
	}
	if (read(fd, (char *) &stat, sizeof stat) == sizeof stat &&
	    stat.stat_size == sizeof stat)
	{
		/* merge current statistics into statfile */
		register int i;

		for (i = 0; i < MAXMAILERS; i++)
		{
			stat.stat_nf[i] += Stat.stat_nf[i];
			stat.stat_bf[i] += Stat.stat_bf[i];
			stat.stat_nt[i] += Stat.stat_nt[i];
			stat.stat_bt[i] += Stat.stat_bt[i];
		}
	}
	else
		bcopy((char *) &Stat, (char *) &stat, sizeof stat);

	/* write out results */
	(void) lseek(fd, (off_t) 0, 0);
	(void) write(fd, (char *) &stat, sizeof stat);
	(void) close(fd);
}
