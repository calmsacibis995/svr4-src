/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:echo.c	1.6.3.1"

/*	Portions Copyright (c) 1988, Sun Microsystems, Inc.	*/
/*	All Rights Reserved.					*/

/*
 *	UNIX shell
 */
#include	"defs.h"

#define	exit(a)	flushb();return(a)

extern int exitval;

echo(argc, argv)
unsigned char **argv;
{
	register unsigned char	*cp;
	register int	i, wd;
	int	j;

#ifdef i386
	int	nonl = 0;
#endif
	
	if (ucb_builtins) {
		register int nflg;

		nflg = 0;
                if(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'n') {
                        nflg++;
                        argc--;
                        argv++;
                }
                for(i=1; i<argc; i++) {
                        sigchk();
                        prs_buff(argv[i]);
                        if (i < argc-1)
                                prc_buff(' ');
                }
                if(nflg == 0)
                        prc_buff('\n');
                exit(0);
        }
	else {	
		if(--argc == 0) {
			prc_buff('\n');
			exit(0);
		}

#ifdef i386
		if (!strncmp(argv[1], "-n", 3)) {
			nonl++;
			*++argv;
			argc--;
		}
#endif
	
		for(i = 1; i <= argc; i++) 
		{
			sigchk();
			for(cp = argv[i]; *cp; cp++) 
			{
				if(*cp == '\\')
				switch(*++cp) 
				{
					case 'b':
						prc_buff('\b');
						continue;
	
					case 'c':
						exit(0);
	
					case 'f':
						prc_buff('\f');
						continue;
	
					case 'n':
						prc_buff('\n');
						continue;
	
					case 'r':
						prc_buff('\r');
						continue;
	
					case 't':
						prc_buff('\t');
						continue;
	
					case 'v':
						prc_buff('\v');
						continue;
	
					case '\\':
						prc_buff('\\');
						continue;
					case '0':
						j = wd = 0;
						while ((*++cp >= '0' && *cp <= '7') && j++ < 3) {
							wd <<= 3;
							wd |= (*cp - '0');
						}
						prc_buff(wd);
						--cp;
						continue;
	
					default:
						cp--;
				}
				prc_buff(*cp);
			}

#ifdef i386
			if (!nonl || i != argc)
				prc_buff(i == argc? '\n': ' ');
#else
			prc_buff(i == argc? '\n': ' ');
#endif

		}
		exit(0);
	}
}
