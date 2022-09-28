/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:gendeliv.c	1.8.3.1"
#include "mail.h"
/*
 * generate delivery notification if required.
 */
void gendeliv(fp, rc, name)
FILE	*fp;
int	rc;
char	*name;
{
	static char pn[] = "gendeliv";
	register char	*p;
	char		buf[1024], cbuf[256], ybuf[10];
	register int	i;
	int		didafflines = 0, didrcvlines = 0, suppress = 0, svopts = 0;
	long		ltmp;
	register struct hdrs	*hptr;
	FILE		*outfile;

	Dout(pn, 0, "at entry, fp = o%lo, rc = %d,name = '%s'\n", (long)fp, rc, name);
	if (fp == (FILE *)NULL) {
		/* Want to send Positive delivery notification. Need to */
		/* put selected header info from orig. msg aside to */
		/* avoid confusion with header info in Delivery Rpt. */
		Daffbytecnt = affbytecnt; affbytecnt = 0;
		Daffcnt = affcnt; affcnt = 0;
		Drcvbytecnt = rcvbytecnt; rcvbytecnt = 0;

		hdrlines[H_DAFWDFROM].head = hdrlines[H_AFWDFROM].head;
		hdrlines[H_DAFWDFROM].tail = hdrlines[H_AFWDFROM].tail;
		hdrlines[H_AFWDFROM].head = (struct hdrs *)NULL;
		hdrlines[H_AFWDFROM].tail = (struct hdrs *)NULL;
		hdrlines[H_DRECEIVED].head = hdrlines[H_RECEIVED].head;
		hdrlines[H_DRECEIVED].tail = hdrlines[H_RECEIVED].tail;
		hdrlines[H_RECEIVED].head = (struct hdrs *)NULL;
		hdrlines[H_RECEIVED].tail = (struct hdrs *)NULL;
		hdrlines[H_DTCOPY].head = hdrlines[H_TCOPY].head;
		hdrlines[H_DTCOPY].tail = hdrlines[H_TCOPY].tail;
		hdrlines[H_TCOPY].head = (struct hdrs *)NULL;
		hdrlines[H_TCOPY].tail = (struct hdrs *)NULL;

		pushlist (H_TCOPY, HEAD, Rpath, FALSE);
	}
	if (rc == 0) {
		/* Verify that positive delivery notification requested */
		if (ckdlivopts(H_DTCOPY, &svopts) & NODELIVERY) {
			Dout(pn, 0, "pos. notif. not requested\n");
			goto rtrn;
		}
	} else {
		/* Verify that negative delivery notification requested */
		if (ckdlivopts(H_DTCOPY, &svopts) & IGNORE) {
			Dout(pn, 0, "neg. notif. not requested\n");
			goto rtrn;
		}
	}
	if (fp == (FILE *)NULL) {
		char *pargs[3];
		pargs[0] = "mail";
		pargs[1] = Rpath;
		pargs[2] = 0;
		if ((outfile = popenvp(pargs[0], pargs, "w", 1)) == (FILE *)NULL) {
			/* Can't get pipe to mail. Just forget it..... */
			Dout(pn, 0,"popenvp() failed\n");
			goto rtrn;
		}
	} else {
		outfile = fp;
	}

	/* get date string into buf for later...*/
	ltmp = time((long)0);
	strcpy(buf, asctime(gmtime(&ltmp)));
	/* strip year out of date string, insert 'GMT', and put year back... */
	p = strrchr(buf,' ');
	strcpy(ybuf,++p);
	*p = '\0';
	strcat(buf,"GMT ");
	strcat(buf, ybuf);
	trimnl(buf);

	fprintf(outfile,"%s 2\n", header[H_RVERS].tag);
	fprintf(outfile,"%s %s\n", header[H_TCOPY].tag,
					hdrlines[H_TCOPY].head->value);
	fprintf(outfile,"%s %s\n", header[H_DATE].tag, buf);
	dumprcv(ORDINARY, -1,&didrcvlines,&suppress,outfile);
	dumpaff(ORDINARY, -1,&didafflines,&suppress,outfile);
	fprintf(outfile,"Original-%s ", header[H_DATE].tag);
	if ((hptr = hdrlines[H_DATE].head) != (struct hdrs *)NULL) {
		Dout(pn, 0,"date from H_DATE = '%s'\n", hptr->value);
		fprintf(outfile,"%s\n", hptr->value);
	} else {
		/* If no H_DATE line in original message, use date */
		/* in last UNIX H_FROM1 or H_FROM line */
		if ((hptr = hdrlines[H_FROM1].tail) == (struct hdrs *)NULL) {
			hptr = hdrlines[H_FROM].tail;
		}
		Dout(pn, 0,"date from H_FROM = '%s'\n", hptr->value);
		strcpy(buf, hptr->value);
		/* Find date portion of line. */
		/* Assumes line is of form - */
		/*       'name_date_[remote_from_sys|forwarded_by_name]' */
		if ((p = strchr(buf,' ')) == (char *)NULL) {
			strcpy(buf, "No valid datestamp in original.");
		} else {
			strcpy(buf,p++);
			/* Walk backwards from end of string to 3rd blank, */
			/* and then check for 'remote from' or 'forwarded by' */
			/* If either found, truncate there, else use entire */
			/* string. */
			p = buf + strlen(buf) - 1;
			i = 0;
			while (p > buf) {
				if (*p == ' ') {
					if (++i == 3) {
						break;
					}
				}
				p--;
			}
			if ((i != 3) || (p <= buf)) {
				strcpy(buf, "No valid datestamp in original.");
			} else {
				if ((strncmp((p+1),"remote from", 11) == 0) ||
				    (strncmp((p+1),"forwarded by", 12) == 0)) {
					*p = '\0';
				}
			}
		}
		fprintf(outfile,"%s\n", buf);
	}
	if ((hptr = hdrlines[H_SUBJ].head) != (struct hdrs *)NULL) {
		fprintf(outfile,"Original-%s %s\n",
				header[H_SUBJ].tag, hptr->value);
	}
	if ((hptr = hdrlines[H_MSVC].head) != (struct hdrs *)NULL) {
		if ((strlen(hptr->value) != 4) ||
		    (casncmp("mail", hptr->value, 4) != 0)) {
			fprintf(outfile,"Original-%s %s\n", 
					header[H_MSVC].tag, hptr->value);
		}
	}
	if ((hptr = hdrlines[H_MTSID].head) != (struct hdrs *)NULL) {
		fprintf(outfile,"Confirming-%s <%s>\n", 
				header[H_MTSID].tag, hptr->value);
	}
	if ((hptr = hdrlines[H_UAID].head) != (struct hdrs *)NULL) {
		fprintf(outfile,"Confirming-%s <%s>\n",
				header[H_UAID].tag, hptr->value);
	}
	cbuf[0] = '\0';
	if ((hptr = hdrlines[H_DTCOPY].head) != (struct hdrs *)NULL) {
		/* Pick comment field off of ">To:" line and put into cbuf */
		getcomment(hptr->value, cbuf);
	}
	if (rc == 0) {
		fprintf(outfile,"Delivered-To: %s!%s %s on %s\n",
						thissys, name, cbuf, buf);
	} else {
		strcpy (buf, name);
		if ((p = strchr(buf,'!')) != (char *)NULL) {
			*p = '\0';
		}
		fprintf(outfile,"Not-Delivered-To: %s!%s %s due to ",
			thissys, buf,
			/* if en-route-to, put comment there, else put it here*/
			((p == (char *)NULL) ? cbuf : ""));
		mta_ercode(outfile);
		if (ckdlivopts(H_DTCOPY, &svopts) & RETURN) {
			fprintf(outfile,"     ORIGINAL MESSAGE ATTACHED\n");
		}

		if (error == E_FRWL) {
			fprintf(outfile, frwlmsg, program, uval);
		} else {
			fprintf(outfile, "     (%s: Error # %d '%s'",
					program,error,errlist[error]);
			if (error == E_SURG) {
				fprintf(outfile,", rc = %d)\n",surg_rc);
				fprintf(outfile,
				    "     ======= Surrogate command =======\n");
				fprintf(outfile,"     %s\n",
					((SURRcmdstr == (char *)NULL) ?
							      "" : SURRcmdstr));
				/* Include stderr from surrogate, if any */
				if (SURRerrfile) {
					fprintf(outfile,
					    "     ==== Start of stdout & stderr ===\n");
					rewind (SURRerrfile);
					while (fgets(buf, sizeof(buf), SURRerrfile) !=
									(char *)NULL) {
						fprintf(outfile,"     %s", buf);
					}
					if (buf[strlen(buf)-1] != '\n') {
						fprintf(outfile,"\n");
					}
					fprintf(outfile,
					    "     ====  End of stdout & stderr  ===\n");
				} else
					fprintf(outfile,
					    "     ==== stdout & stderr unavailable ===\n");
			} else {
				fprintf(outfile,")\n");
			}
		}
		if (p != (char *)NULL) {
			fprintf(outfile, "En-Route-To: %s %s\n", name, cbuf);
		}
	}
	if ((hptr = hdrlines[H_DAFWDFROM].head) != (struct hdrs *)NULL) {
		while (hptr != (struct hdrs *)NULL) {
			fprintf(outfile,"Original-%s %s\n",
				header[H_DAFWDFROM].tag, hptr->value);
			hptr = hptr->next;
		}
	}
	fprintf(outfile,"%s\n", header[H_EOH].tag);
	if (fp == (FILE *)NULL) {
		pclosevp(outfile);
	}
	Dout(pn, 5, "notification sent.\n");

    rtrn:
	/* Restore header info from original message. (see above and also */
	/* goback()). */
	clrhdr(H_TCOPY);
	clrhdr(H_AFWDFROM);
	clrhdr(H_RECEIVED);
	affbytecnt = Daffbytecnt; Daffbytecnt = 0;
	affcnt = Daffcnt; Daffcnt = 0;
	rcvbytecnt = Drcvbytecnt; Drcvbytecnt = 0;

	hdrlines[H_AFWDFROM].head = hdrlines[H_DAFWDFROM].head;
	hdrlines[H_AFWDFROM].tail = hdrlines[H_DAFWDFROM].tail;
	hdrlines[H_DAFWDFROM].head = (struct hdrs *)NULL;
	hdrlines[H_DAFWDFROM].tail = (struct hdrs *)NULL;
	hdrlines[H_RECEIVED].head = hdrlines[H_DRECEIVED].head;
	hdrlines[H_RECEIVED].tail = hdrlines[H_DRECEIVED].tail;
	hdrlines[H_DRECEIVED].head = (struct hdrs *)NULL;
	hdrlines[H_DRECEIVED].tail = (struct hdrs *)NULL;
	hdrlines[H_TCOPY].head = hdrlines[H_DTCOPY].head;
	hdrlines[H_TCOPY].tail = hdrlines[H_DTCOPY].tail;
	hdrlines[H_DTCOPY].head = (struct hdrs *)NULL;
	hdrlines[H_DTCOPY].tail = (struct hdrs *)NULL;

	return;
}
