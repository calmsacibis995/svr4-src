/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:copylet.c	1.7.3.1"
#include "mail.h"
/*
    NAME
	copylet - copy a given letter to a file pointer

    SYNOPSIS
	int copylet(int letnum, FILE *f, int type)

    DESCRIPTION
	Copylet() will copy the letter "letnum" to the
	given file pointer.

		letnum	-> index into: letter table
		f	-> file pointer to copy file to
		type	-> copy type

	Returns TRUE on a completely successful copy.
*/
copylet(letnum, f, type) 
register FILE *f;
{
	static char	pn[] = "copylet";
	char	buf[LSIZE], lastc;
	char	wbuf[LSIZE];
	int	n;
	long	i, k;
	int	num;
	int	rtrncont = 1;	/* True: nondelivery&content included, or regular mail */
	int	suppress = FALSE;
	int	sav_suppress = FALSE; /* Did we suppress previous hdr line? */
	int	print_from_struct = FALSE; /* print from hdrlines struct */
					   /* rather than fgets() buffer */
	int	pushrest = FALSE;
	int	ctf = FALSE;
	int	didafflines = FALSE;	/* Did we already put out any */
					/* H_AFWDFROM lines? */
	int	didrcvlines = FALSE;	/* Did we already put out any */
					/* H_RECEIVED lines? */
	long	clen = -1L;
	int	htype;			/* header type */
	int	sav_htype;	/* Header type of last non-H_CONT header line */
	struct hdrs *hptr;

	if (!sending) {
		/* Clear out any saved header info from previous message */
		clr_hinfo();
	}

	fseek(tmpf, let[letnum].adr, 0);
	/* Get size of message as stored into tempfile by copymt() */
	k = let[letnum+1].adr - let[letnum].adr;
	Dout(pn, 1, "(letnum = %d, type = %d), k = %ld\n", letnum, type, k);
	while (k>0) {	/* process header */
		num = ((k < sizeof(buf)) ? k+1 : sizeof(buf));
		if (fgets (buf, num, tmpf) == NULL) {
			return (FALSE);
		}
		if ((n = strlen (buf)) == 0) {
			k = 0;
			break;
		}
		k -= n;
		lastc = buf[n-1];
		if (pushrest) {
			pushrest = (lastc != '\n');
			continue;
		}
		htype = isheader (buf, &ctf);
		Dout(pn, 5, "loop 1: buf = %s, htype= %d/%s\n", buf, htype, header[htype].tag);
		if (htype == H_CLEN) {
			if (!sending) {
				savehdrs(buf,htype);
			}
			if ((hptr = hdrlines[H_CLEN].head) !=
			    (struct hdrs *)NULL) {
				clen = atol (hptr->value);
			}
		}
		if (type == ZAP) {
			if (htype != FALSE) {
				pushrest = (lastc != '\n');
				continue;
			}
			/* end of header.  Print non-blank line and bail. */
			Dout(pn, 5, "ZAP end header; n=%d, buf[0] = %d\n", n, buf[0]);
			if (buf[0] != '\n') {
				if (fwrite(buf,1,n,f) != n) {
					sav_errno = errno;
					return(FALSE);
				}
			} else {
				n = 0;
			}
			break;
		}
		/* Copy From line appropriately */
		if (fwrite(buf,1,n-1,f) != n-1)  {
			sav_errno = errno;
			return(FALSE);
		}
		if (lastc != '\n') {
			if (fwrite(&lastc,1,1,f) != 1) {
				sav_errno = errno;
				return(FALSE);
			}
			continue;
		}
		switch(type) {
			case REMOTE:
				fprintf(f, rmtmsg, thissys);
				break;

			case TTY:
			case ORDINARY:
			default:
				fprintf(f, "\n");
				break;
		}
		if ((error > 0) && (dflag == 1)) {
			Dout(pn, 3, "before gendeliv(), uval = '%s'\n", uval);
			gendeliv(f, dflag, uval);
			if (!(ckdlivopts(H_TCOPY, (int*)0) & RETURN)) {
				rtrncont = 0;
			} else {
				/* Account for content-type info */
				/* of returned msg */
				fprintf(f, "%s %s\n", header[H_CTYPE].tag,
				    (let[letnum].text == TRUE ? "text" : "binary"));

				/* Compute Content-Length of what's being */
				/* returned... */
				i = k;
				/* Account for H_AFWDFROM, H_AFWDCNT, */
				/* H_TCOPY, or H_RECEIVED lines which may */
				/* be added later */
				if (affcnt > 0) {
					sprintf(wbuf, "%d", affcnt);
					i += (affbytecnt
						+ strlen(header[H_AFWDCNT].tag)
						+ strlen(wbuf) + 2);
				}
				if (orig_tcopy) {
				    if ((hptr = hdrlines[H_TCOPY].head) !=
							(struct hdrs *)NULL) {
				        i +=
					  strlen(hdrlines[H_TCOPY].head->value);
				    }
				}
				if ((hptr = hdrlines[H_RECEIVED].head) !=
							(struct hdrs *)NULL) {
				    i += rcvbytecnt;
				}
				/* Add in strlen of Content-Length: and */
				/* Content-Type: values for msg being */
				/* returned... */
				if ((hptr = hdrlines[H_CTYPE].head) !=
							(struct hdrs *)NULL) {
				    i += strlen(hdrlines[H_CTYPE].head->value);
				}
				if ((hptr = hdrlines[H_CLEN].head) !=
							(struct hdrs *)NULL) {
				    i += strlen(hdrlines[H_CLEN].head->value);
				}
				fprintf(f, "%s %ld\n", header[H_CLEN].tag, i);
			}
			fprintf(f, "\n");
		}
		fflush(f);
		break;
	}
	/* if not ZAP, copy balance of header */
	n = 0;
	if ((type != ZAP) && rtrncont)
		while (k>0 || n>0) {
			if ((n > 0) && !suppress) {
				if (print_from_struct == TRUE) {
					if (printhdr (type, htype, hptr, f) < 0) {
						return (FALSE);
					}
				} else {
				    if (sel_disp(type, htype, buf) >= 0) {
					if (fwrite(buf,1,n,f) != n)  {
						sav_errno = errno;
						return(FALSE);
					}
				    }
				}
				if (htype == H_DATE) {
					dumprcv(type, htype,&didrcvlines,&suppress,f);
					dumpaff(type, htype,&didafflines,&suppress,f);
				}
			}
			if (k <= 0) {
				/* Can only get here if k=0 && n>0, which occurs */
				/* in a message with header lines but no content. */
				/* If we haven't already done it, force out any */
				/* H_AFWDFROM or H_RECEIVED lines */
				dumprcv(type, -1,&didrcvlines,&suppress,f);
				dumpaff(type, -1,&didafflines,&suppress,f);
				break;
			}
			num = ((k < sizeof(buf)) ? k+1 : sizeof(buf));
			if (fgets (buf, num, tmpf) == NULL) {
				return (FALSE);
			}
			n = strlen (buf);
			k -= n;
			lastc = buf[n-1];

			if (pushrest) {
				pushrest = (lastc != '\n');
				continue;
			}
			sav_suppress = suppress;
			suppress = FALSE;
			print_from_struct = FALSE;
			sav_htype = htype;
			htype = isheader (buf, &ctf);
			Dout(pn, 5, "loop 2: buf = %s, htype= %d/%s\n", buf, htype, header[htype].tag);
			/* The following order is defined in the MTA documents. */
			switch (htype) {
			case H_CONT:
			    if (sending) {
				/* If we suppressed output of previous non-H_CONT */
				/* line then we should suppress this line also... */
				suppress = sav_suppress;

				/* The following header types will be dumped, with */
				/* continuation lines, by printhdr(), so suppress */
				/* output here... */
				switch (sav_htype) {
				case H_TCOPY:
				case H_CTYPE:
				case H_CLEN:
					suppress = TRUE;
				}
			    }
			    continue;
			case H_TCOPY:
			case H_CTYPE:
			case H_CLEN:
				if (!sending) {
					savehdrs(buf,htype);
				}
				hptr = hdrlines[htype].head;
				if (htype == H_CLEN) {
					clen = atol (hptr->value);
				}
				/*
				 * Use values saved in hdrlines[] structure
				 * rather than what was read from tmp file.
				 */
				print_from_struct = TRUE;
				/* FALLTHROUGH */
			case H_EOH:
			case H_AFWDFROM:
			case H_AFWDCNT:
			case H_RECEIVED:
				dumprcv(type, htype,&didrcvlines,&suppress,f);
				dumpaff(type, htype,&didafflines,&suppress,f);
				continue;	/* next header line */
			default:
				pushrest = (lastc != '\n');
				continue;	/* next header line */
			case FALSE:	/* end of header */
				break;
			}

			/* Found the blank line after the headers. */
			if (n > 0) {
				if (fwrite(buf,1,n,f) != n)  {
					return(FALSE);
				}
			}

			Dout(pn, 3,", let[%d].text = %s\n",
				letnum, (let[letnum].text ? "TRUE" : "FALSE"));

			if ((type == TTY) && (let[letnum].text == FALSE) && !pflg) {
				fprintf (f, "\n%s\n", binmsg);
				return (TRUE);
			}

			if (n == 1 && buf[0] == '\n') {
				n = 0;
			}
			break;
		}

	Dout(pn, 1, "header processed, clen/k/n = %ld/%ld/%d\n", clen, k, n);

	if (clen >= 0) {
		if (((clen - n) == k) || ((clen - n) == (k - 1))) {
			k = clen - n;
		} else {
			/* probable content-length mismatch. show it ALL! */
			Dout(pn, 1, "clen conflict. using k = %ld\n", k);
		}
	}

	/* copy balance of message */
	if (rtrncont)
		while (k > 0) {
			num = ((k < sizeof(buf)) ? k : sizeof(buf));
			if ((n = fread (buf, 1, num, tmpf)) <= 0) {
				Dout(pn, 1, "content-length mismatch. return(FALSE)\n");
				return(FALSE);
			}
			k -= n;
			if (fwrite(buf,1,n,f) != n)  {
				sav_errno = errno;
				return(FALSE);
			}
		}

	Dout(pn, 3, "body processed, k=%ld\n", k);

	if (rtrncont && type != ZAP && type != REMOTE) {
		if (fwrite("\n",1,1,f) != 1)  {
			sav_errno = errno;
			return(FALSE);
		}
	}

	return(TRUE);
}
