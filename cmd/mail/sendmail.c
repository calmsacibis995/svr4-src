/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:sendmail.c	1.8.3.1"
#include "mail.h"
/*
	 Send mail - High level sending routine
 */
void sendmail(argc, argv)
char **argv;
{
	char		**args;
	char		*tp, *zp;
	char		buf[2048],last1c;
	int		aret;
	int		i, n;
	int		ttyf = 0;
	int		pushrest = 0;
	int		hdrtyp = 0;
	int		ctf = FALSE;
	int		binflg = 0;
	long		count = 0L;
	struct tm	*bp;
	struct hdrs	*hptr;
	static char	pn[] = "sendmail";
	reciplist	list;

	Dout(pn, 0, "entered\n");
	new_reciplist(&list);
	for (i = 1; i < argc; ++i) {
	        if (argv[i][0] == '-') {
		        if (argv[i][1] == '\0') {
				errmsg(E_SYNTAX,"Hyphens MAY NOT be followed by spaces");
			}
		        if (i > 1) {
				errmsg(E_SYNTAX,"Options MUST PRECEDE persons");
			}
		        done(0);
	        }
		/*
			Ensure no NULL names in list
		*/
	        if (argv[i][0] == '\0' || argv[i][strlen(argv[i])-1] == '!') {
			errmsg(E_SYNTAX,"Null names are not allowed");
	  	       	done(0);
		}
		add_recip(&list, argv[i], FALSE); /* Don't check for duplication */
	}

	mktmp();
	/*
		Format time
	*/
	time(&iop);
	bp = localtime(&iop);
	tp = asctime(bp);
	zp = tzname[bp->tm_isdst];
	sprintf(datestring, "%.16s %.3s %.5s", tp, zp, tp+20);
	trimnl (datestring);
	/* asctime: Fri Sep 30 00:00:00 1986\n */
	/*          0123456789012345678901234  */
	/* RFCtime: Fri, 28 Jul 89 10:30 EDT   */
	sprintf(RFC822datestring, "%.3s, %.2s %.3s %.4s %.5s %.3s",
		tp, tp+8, tp+4, tp+20, tp+11, zp);

	/*
		Write out the from line header for the letter
	*/
	sprintf(buf, "%s%s %s\n", header[H_FROM].tag, my_name, datestring);
	if (!wtmpf(buf, strlen(buf))) {
		done(0);
	}
	savehdrs(buf, H_FROM);

	/*
		Copy to list in mail entry?
	*/
	if (flgt == 1 && argc > 1) {
		aret = argc;
		args = argv;
		while (--aret > 0) {
			sprintf (buf, "%s %s\n", header[H_TO].tag, *++args);
			if (!wtmpf(buf, strlen(buf))) {
				done(0);
			}
			savehdrs(buf, H_TO);
		}
	}

	flgf = 1;	/* reset when first read of message body succeeds */
	/*
		Read mail message, allowing for lines of infinite 
		length. This is tricky, have to watch for newlines.
	*/
	saveint = setsig(SIGINT, savdead);
	last1c = ' ';	/* anything other than newline */
	ttyf = isatty (fileno(stdin));
	pushrest = 0;

	/*
	 * scan header & save relevant info.
	 */
	strcpy (fromU, my_name); fromS[0] = 0;	/* set up for >From scan */
	while ((n = getline (line, sizeof line, stdin)) > 0) {
		last1c = line[n-1];
		if (pushrest) {
			if (!wtmpf(line,n)) {
				done(0);
			}
			pushrest = (last1c != '\n');
			continue;
		}
		pushrest = (last1c != '\n');

		if ((hdrtyp = isheader (line, &ctf)) == FALSE) {
			break;
		}
		flgf = 0;
		switch (hdrtyp) {
		case H_RVERS:
			/* Are we dealing with a delivery report? */
			/* dflag = 9 ==> do not return on failure */
			dflag = 9;
			Dout(pn, 0, "dflag = 9\n");
			break;
		case H_FROM:
			if (!wtmpf(">", 1)) {
				done(0);
			}
			/* note dropthru */
			hdrtyp = H_FROM1;
		case H_FROM1:
			if (substr(line, "forwarded by") > -1) {
				break;
			}
			pickFrom (line);
			if (Rpath[0] != '\0') {
				strcat(Rpath, "!");
			}
			strcat(Rpath, fromS);
			break;
		case H_CLEN:
		case H_CTYPE:
			/* suppress it: only generated if needed */
			n = 0; /* suppress */
			break;
		case H_TCOPY:
			/* Write out placeholder for later */
			sprintf(buf, "%s \n", header[H_TCOPY].tag);
			if (!wtmpf(buf, strlen(buf))) {
				done(0);
			}
			n = 0; /* suppress */
			break;
		case H_MTYPE:
			if (flgm) {
				/* suppress if message-type argument */
				n = 0;
			}
			break;
		}
		if (n && !wtmpf(line,n)) {
			done(0);
		}
		savehdrs(line,hdrtyp);
	}
	if (Rpath[0] != '\0') {
		strcat(Rpath, "!");
	}
	strcat(Rpath, fromU);

	/* push out message type if so requested */
	if (flgm) {	/* message-type */
		sprintf (buf, "%s%s\n", header[H_MTYPE].tag, msgtype);
		if (!wtmpf(buf, strlen(buf))) {
			done(0);
		}
	}

	memcpy (buf, line, n);
	if (n == 0 || (ttyf && !strncmp (buf, ".\n", 2)) ) {
		if (flgf) {
			/* no input */
			return;
		} else {
			/*
			 * no content: put content-type and -length only if
			 * explicitly present. Write out 'place-holders'
			 * only. (see below....)
			 */
			if ((hptr = hdrlines[H_CTYPE].head) !=
						    (struct hdrs *)NULL) {
				sprintf (line, "%s \n", header[H_CTYPE].tag);
				if (!wtmpf(line, strlen(line))) {
					done(0);
				}
			}
			if ((hptr = hdrlines[H_CLEN].head) !=
						    (struct hdrs *)NULL) {
				sprintf (line, "%s \n", header[H_CLEN].tag);
				if (!wtmpf(line, strlen(line))) {
					done(0);
				}
			}
			goto wrapsend;
		}
	}

	if (n == 1 && last1c == '\n') {	/* blank line -- suppress */
		n = getline (buf, sizeof buf, stdin);
		if (n == 0 || (ttyf && !strncmp (buf, ".\n", 2)) ) {
			/*
			 * no content: put content-type and -length only if
			 * explicitly present. Write out 'place-holders'
			 * only. (see below....)
			 */
			if ((hptr = hdrlines[H_CTYPE].head) !=
						    (struct hdrs *)NULL) {
				sprintf (line, "%s \n", header[H_CTYPE].tag);
				if (!wtmpf(line, strlen(line))) {
					done(0);
				}
			}
			if ((hptr = hdrlines[H_CLEN].head) !=
						    (struct hdrs *)NULL) {
				sprintf (line, "%s \n", header[H_CLEN].tag);
				if (!wtmpf(line, strlen(line))) {
					done(0);
				}
			}
			goto wrapsend;
		}
	}

	if (debug > 0) {
		buf[n] = '\0';
		Dout(pn, 0, "header scan complete, readahead %d = \"%s\"\n", n, buf);
	}

	/* 
	 * Write out H_CTYPE & H_CLEN lines. These are used only as 
	 * placeholders in the tmp file. When the 'real' message is sent,
	 * the proper values will be put out by copylet().
	 */
	sprintf (line, "%s \n", header[H_CTYPE].tag);
	if (!wtmpf(line, strlen(line))) {
		done(0);
	}
	if (hdrlines[H_CTYPE].head == (struct hdrs *)NULL) {
		savehdrs(line,H_CTYPE);
	}
	sprintf (line, "%s \n", header[H_CLEN].tag);
	if (!wtmpf(line, strlen(line))) {
		done(0);
	}
	if (hdrlines[H_CLEN].head == (struct hdrs *)NULL) {
		savehdrs(line,H_CLEN);
	}
	/* and a blank line */
	if (!wtmpf("\n", 1)) {
		done(0);
	}
	Dout(pn, 0, "header out completed\n");

	pushrest = 0;
	count = 0L;
	/*
	 *	Are we returning mail from a delivery failure of an old-style
	 *	(SVR3.1 or SVR3.0) rmail? If so, we won't return THIS on failure
	 *	[This line should occur as the FIRST non-blank non-header line]
	 */
	if (!strncmp("***** UNDELIVERABLE MAIL sent to",buf,32)) {
	     dflag = 9; /* 9 says do not return on failure */
	     Dout(pn, 0, "found old-style UNDELIVERABLE line. dflag = 9\n");
	}

	/* scan body of message */
	while (n > 0) {
		if (ttyf && !strcmp (buf, ".\n"))
			break;
		if (!binflg) {
			binflg = !istext ((unsigned char *)buf, n);
		}

		if (!wtmpf(buf, n)) {
			done(0);
		}
		count += n;
		n = ttyf
			? getline (buf, sizeof buf, stdin)
			: fread (buf, 1, sizeof buf, stdin);
	}
	setsig(SIGINT, saveint);

wrapsend:
	/*
	 *	In order to use some of the subroutines that are used to
	 *	read mail, the let array must be set up
	 */
	nlet = 1;
	let[0].adr = 0;
	let[1].adr = ftell(tmpf);
	let[0].text = (binflg == 1 ? FALSE : TRUE);
	Dout(pn, 0, "body copy complete, count %ld\n", count);
	/*
	 * Modify value of H_CTYPE if necessary.
	 */
	if ((hptr = hdrlines[H_CTYPE].head) != (struct hdrs *)NULL) {
		if (strlen(hptr->value) == 0) {
			strcpy (hptr->value, "text");
		}
		if (binflg) {
			/*
			 * If it's anything OTHER than 'text', don't change it
			 */
			if (strcmp(hptr->value,"text") == 0) {
				strcpy(hptr->value,"binary");
			}
		}
	}
	/*
	 * Set 'place-holder' value of content length to true value
	 */
	if ((hptr = hdrlines[H_CLEN].head) != (struct hdrs *)NULL) {
		sprintf(hptr->value,"%ld", count);
	}

	if (fclose(tmpf) == EOF) {
		tmperr();
		done(0);
	}
	 
	tmpf = doopen(lettmp,"r+",E_TMP);

	sendlist(&list, 0, 0);
	done(0);
}
