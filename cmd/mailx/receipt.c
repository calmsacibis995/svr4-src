/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:receipt.c	1.2.4.1"
#include "rcv.h"

static int		icsubstr();

void
receipt(mp)
struct message *mp;
{
	struct headline	hl;
	char	head[LINESIZE];
	char	buf[BUFSIZ];
	FILE	*pp, *fp;
	char	*mail, *s;

	if ((mail = value("sendmail")) == 0)
		mail = MAIL;
	if (icsubstr(hfield("default-options", mp, addone), "/receipt")
	 || icsubstr(hfield(">to", mp, addto), "/receipt")) {
		sprintf(buf, "%s %s", mail, skin(nameof(mp)));
		if (pp = npopen(buf, "w")) {
			fp = setinput(mp);
			readline(fp, head);
			parse(head, &hl, buf);
			fprintf(pp, "Original-Date: %s\n", hl.l_date);
			if (s = hfield("message-id", mp, addone))
				fprintf(pp, "Original-Message-ID: %s\n", s);
			s = hfield("subject", mp, addone);
			fprintf(pp, "Subject: RR: %s\n", s ? s : "(none)");
			npclose(pp);
		}
	}
}

static int
icsubstr(s1, s2)
char	*s1, *s2;
{
	char	buf[LINESIZE];

	if (s1 && s2) {
		istrcpy(buf, s1);
		return substr(buf, s2) != -1;
	} else
		return 0;
}
