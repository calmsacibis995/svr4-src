/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cflow:nmf.c	1.7.1.5"

#include <stdio.h>
#include <string.h>
#ifdef __STDC__
#include <stdlib.h>
#endif

#define UNDEFINE	0
#define CI50		1
#define CI4x		2

main(argc, argv)
char	*argv[];
{
	char name[BUFSIZ], buftab[BUFSIZ], *fname = NULL, *pty;
	char *buf = buftab;
	char funty[4], shnty[5];
	register char *p;
	char *lnptr;
	int bufsiz = BUFSIZ;
	static int nmflag = UNDEFINE;
	int c, nsize;

	if (argc > 2)
		fname = argv[2];
	else
		fname = "???";

	while ((c=getchar()) != EOF)
	{
		for (p = buf; c != '\n'; c=getchar())
		{
			*p++ = (char) c;
			if (p >= buf + bufsiz)
			{
				if (buf == buftab)
				{
					buf = (char *)malloc((bufsiz += BUFSIZ));
					if (buf == NULL) {
						(void)fprintf(stderr,"out of space\n");
						exit(1);
					}
					(void)memcpy(buf, buftab, (bufsiz - BUFSIZ));
				}
				else {
					buf = (char *)realloc(buf, (bufsiz+=BUFSIZ));
					if (buf == NULL) {
						(void)fprintf(stderr,"out of space\n");
						exit(1);
					}
				}
				p = buf + (bufsiz - BUFSIZ);
			}
		}
		*p = '\0';
		p = buf;

		if (nmflag == UNDEFINE) {
			if (*p == '[') nmflag = CI50;
			else	nmflag = CI4x;
		}
		switch (nmflag) {
		case CI50:
			do ; while (*p++ != '|');		/* skip Index */
			do ; while (*p++ != '|');		/* skip Value */
			do ; while (*p++ != '|');		/* skip Size */
			(void) strncpy(funty, p, 4);		/*is it function? */
			if (!strncmp("FILE", funty, 4))
				continue;			/* skip file name */
			do ; while (*p++ != '|');		/* skip Type */
			do ; while (*p++ != '|');		/* skip Bind */
			do ; while (*p++ != '|');		/* skip Other */
			(void) strncpy(shnty, p, 5);		/*is it undefined? */
			do ; while (*p++ != '|');		/* skip Shndx */
			if (*p == '.' || !strcmp(p, "")) continue;
			(void) printf("%s = ", p);
			if (!strncmp("FUNC", funty, 4))
				(void) printf("( ), ");
			if (!strncmp("UNDEF", shnty, 5))
				(void) printf("<>\n");
			else
				(void) printf("<%s 0>\n", fname);
			break;
		case CI4x:
			while (*p != ' ' && *p != '|')
				++p;
			nsize = p - buf;
			(void) strncpy(name, buf, nsize);
			name[nsize] = '\0';
			if (name[0] == '.' || !strcmp(name, "")) continue;
			(void) printf("%s = ", name);

			do ; while (*p++ != '|');	/* skip rem of name */
			do ; while (*p++ != '|');	/* skip value */
			do ; while (*p++ != '|');	/* skip class */

			while (*p == ' ') ++p;		/* get Type */
			pty = p;
			while (*p != '|') ++p;
			*p++ = '\0';
			do ; while (*p++ != '|');	/* skip size */

			while (*p == ' ') ++p;		/* get line number */
			lnptr = p;
			while (*p != '|') ++p;
			*p++ = '\0';

			while (*p == ' ') ++p;

			if (*pty)
				(void) printf("%s, ", pty);
			else {
				if (!strncmp(p, ".text", 5))
					(void) fputs("( ), ", stdout);
			}
			if (*p == '.')
				(void) printf("<%s %d>\n", fname, atoi(lnptr));
			else
				(void) printf("<>\n");
			break;
		}
	}
	nmflag = UNDEFINE;
	exit(0);
	/* NOTREACHED */
}
