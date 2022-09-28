/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)echo:echo.c	1.4"

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

main(argc, argv)
char **argv;
{
	register char	*cp;
	register int	i, wd;
	int	j;
	int fnewline = 1;

	if(--argc == 0) {
		putchar('\n');
		exit(0);
	} else if (!strcmp(argv[1], "-n")) {
		fnewline = 0;
		++argv;
		--argc;
	}
	for(i = 1; i <= argc; i++) {
		for(cp = argv[i]; *cp; cp++) {
			if(*cp == '\\')
			switch(*++cp) {
				case 'b':
					putchar('\b');
					continue;

				case 'c':
					exit(0);

				case 'f':
					putchar('\f');
					continue;

				case 'n':
					putchar('\n');
					continue;

				case 'r':
					putchar('\r');
					continue;

				case 't':
					putchar('\t');
					continue;

				case 'v':
					putchar('\v');
					continue;

				case '\\':
					putchar('\\');
					continue;
				case '0':
					j = wd = 0;
					while ((*++cp >= '0' && *cp <= '7') && j++ < 3) {
						wd <<= 3;
						wd |= (*cp - '0');
					}
					putchar(wd);
					--cp;
					continue;

				default:
					cp--;
			}
			putchar(*cp);
		}
		putchar(((i == argc) && fnewline)? '\n': ' ');
	}
	exit(0);
}
