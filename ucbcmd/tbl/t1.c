/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbtbl:t1.c	1.2.3.1"

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

 /* t1.c: main control and input switching */
#
# include "t..c"
#include <signal.h>
# ifdef gcos
/* required by GCOS because file is passed to "tbl" by troff preprocessor */
# define _f1 _f
extern FILE *_f[];
# endif

# ifdef unix
# define MACROS "/usr/ucblib/doctools/tmac/s"
# define PYMACS "/usr/ucblib/doctools/tmac/m"
# endif

# ifdef gcos
# define MACROS "cc/troff/smac"
# define PYMACS "cc/troff/mmac"
# endif

# define ever (;;)

main(argc,argv)
	char *argv[];
{
# ifdef unix
void badsig();
signal(SIGPIPE, badsig);
# endif
# ifdef gcos
if(!intss()) tabout = fopen("qq", "w"); /* default media code is type 5 */
# endif
exit(tbl(argc,argv));
}


tbl(argc,argv)
	char *argv[];
{
char line[BUFSIZ];
/* required by GCOS because "stdout" is set by troff preprocessor */
tabin=stdin; tabout=stdout;
setinp(argc,argv);
while (gets1(line, sizeof line))
	{
	fprintf(tabout, "%s\n",line);
	if (prefix(".TS", line))
		tableput();
	}
fclose(tabin);
return(0);
}
int sargc;
char **sargv;
setinp(argc,argv)
	char **argv;
{
	sargc = argc;
	sargv = argv;
	sargc--; sargv++;
	if (sargc>0)
		swapin();
}
swapin()
{
	while (sargc>0 && **sargv=='-') /* Mem fault if no test on sargc */
		{
		if (sargc<=0) return(0);
		if (match("-me", *sargv))
			{
			*sargv = "/usr/ucblib/doctools/tmac/e";
			break;
			}
		if (match("-ms", *sargv))
			{
			*sargv = MACROS;
			break;
			}
		if (match("-mm", *sargv))
			{
			*sargv = PYMACS;
			break;
			}
		if (match("-TX", *sargv))
			pr1403=1;
		else if (sargv[0][1] != '\0') {
			fprintf(stderr,"Usage: tbl [-me|-ms|-mm] [-TX] [filename]\n");
			exit(1);
		}
		sargc--; sargv++;
		}
	if (sargc<=0) return(0);
# ifdef unix
/* file closing is done by GCOS troff preprocessor */
	if (tabin!=stdin) fclose(tabin);
# endif
	tabin = fopen(ifile= *sargv, "r");
	iline=1;
# ifdef unix
/* file names are all put into f. by the GCOS troff preprocessor */
	fprintf(tabout, ".ds f. %s\n",ifile);
# endif
	if (tabin==NULL)
		error("Can't open file");
	sargc--;
	sargv++;
	return(1);
}
# ifdef unix
void badsig()
{
signal(SIGPIPE, SIG_IGN);
 exit(0);
}
# endif
