/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cflow:flip.c	1.3.1.3"
#include <stdio.h>
#include <ctype.h>
#include <memory.h>
#ifdef __STDC__
#include <stdlib.h>
#endif

extern void exit();

main()
{
	register int bufsiz = BUFSIZ;
	register int headpl = 0;
	int c;
	char linbuf[BUFSIZ];
	char *line = linbuf;
	char *ptr = linbuf;
	char *pl = linbuf;

	while ((c = getchar()) != EOF) {
		for (*ptr = (char)c, pl = line; *ptr != '\n'; *++ptr = getchar()) {
			if (ptr >= line + (bufsiz - 1)) {
				int ploffset = pl - line;
				if (line == linbuf) {
					line = (char *)malloc(bufsiz += BUFSIZ);
					if (line == NULL) {
						(void)fprintf(stderr,"out of space\n");
						exit(1);
					}
					(void)memcpy(line, linbuf, (bufsiz-BUFSIZ));
				} else {
					line =(char *)realloc(line,(bufsiz+=BUFSIZ));
					if (line == NULL) {
						(void)fprintf(stderr,"out of space\n");
						exit(1);
					}
				}
				ptr = line + (bufsiz - BUFSIZ - 1);
				pl = line + ploffset;
			}
			if (*ptr == ':') {
				*ptr = '\0';
				pl = ptr + 1;
				headpl = 1;
			} else if (isspace(*ptr) && headpl) {
				++pl;
			} else	headpl = 0;
		}
		*ptr = '\0';
		(void) printf("%s : %s\n", pl, (ptr = line));
		headpl = 0;
	}
	exit(0);
	/* NOTREACHED */
}
