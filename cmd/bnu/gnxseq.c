/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:gnxseq.c	2.5.3.1"

#include "uucp.h"

/*
 * get next conversation sequence number
 *	rmtname	-> name of remote system
 * returns:
 *	0	-> no entery
 *	1	-> 0 sequence number
 */
int
gnxseq(rmtname)
char *rmtname;
{
	register FILE *fp0, *fp1;
	register struct tm *tp;
	int count = 0, ct, ret;
	char buf[BUFSIZ], name[NAMESIZE];
	time_t clock;

	if (access(SQFILE, 0) != 0)
		return(0);

	{
		register int i;
	for (i = 0; i < 5; i++) 
		if ( (ret = mklock(SQLOCK)) == SUCCESS )
			break;
		sleep(5);
	}
	if (ret != SUCCESS) {
		logent("CAN'T LOCK", SQLOCK);
		DEBUG(4, "can't lock %s\n", SQLOCK);
		return(0);
	}
	if ((fp0 = fopen(SQFILE, "r")) == NULL)
		return(0);
	if ((fp1 = fopen(SQTMP, "w")) == NULL) {
		fclose(fp0);
		return(0);
	}
	chmod(SQTMP, DFILEMODE);

	while (fgets(buf, BUFSIZ, fp0) != NULL) {
		ret = sscanf(buf, "%s%d", name, &ct);
		if (ret < 2)
			ct = 0;
		name[7] = '\0';
		if (ct > 9998)
			ct = 0;
		if (strncmp(rmtname, name, SYSNSIZE) != SAME) {
			fputs(buf, fp1);
			continue;
		}

		/*
		 * found name
		 */
		count = ++ct;
		time(&clock);
		tp = localtime(&clock);
		fprintf(fp1, "%s %d %d/%d-%d:%2.2d\n", name, ct,
		tp->tm_mon + 1, tp->tm_mday, tp->tm_hour,
		tp->tm_min);

		/*
		 * write should be checked
		 */
		while (fgets(buf, BUFSIZ, fp0) != NULL)
			fputs(buf, fp1);
	}
	fclose(fp0);
	fclose(fp1);
	if (count == 0) {
		rmlock(SQLOCK);
		unlink(SQTMP);
	}
	return(count);
}

/*
 * commit sequence update
 * returns:
 *	0	-> ok
 *	other	-> link failed
 */
int
cmtseq()
{
	register int ret;

	if ((ret = access(SQTMP, 0)) != 0) {
		rmlock(SQLOCK);
		return(0);
	}
	unlink(SQFILE);
	ret = link(SQTMP, SQFILE);
	unlink(SQTMP);
	rmlock(SQLOCK);
	return(ret);
}

/*
 * unlock sequence file
 */
void
ulkseq()
{
	unlink(SQTMP);
	rmlock(SQLOCK);
	return;
}
