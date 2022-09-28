/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkgscripts/cmdexec.c	1.1.3.1"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

extern void	progerr(), free(), exit();
extern	int	unlink();

#define COMMAND '!'
#define LSIZE 256

char *prog;

static void	usage();
static int	docmd();

main(argc, argv)
int argc;
char *argv[];
{
	FILE	*fpout, *fp;
	char	line[LSIZE],
		*pt,
		*keyword,	/*keyword = install || remove*/
		*input,	/*sed input file*/
		*cmd,
		*srcfile,	/*sed data file*/
		*destfile;	/*target file to be updated*/
	int	flag;
	
	prog = argv[0];
	if (argc != 5)
		usage();

	cmd = argv[1];
	keyword = argv[2];
	srcfile = argv[3];
	destfile = argv[4];

	srcfile = argv[3];
	if ((fp = fopen(srcfile, "r")) == NULL) {
		progerr("unable to open %s", srcfile);
		exit(1);
	}

	input = tempnam(NULL, "sedinp");	
	if((fpout = fopen(input, "w")) == NULL) {
		progerr("unable to open %s", input);
		exit(2);
	}

	flag = (-1);
	while(fgets(line, LSIZE, fp)) {
		for(pt=line; isspace(*pt);)
			++pt;
		if(*pt == '#')
			continue;
		if(*pt == COMMAND) {
			if(flag > 0)
				break; /* no more lines to read */
			pt = strtok(pt+1, " \t\n");
			if(!pt) {
				progerr("null token after '!'");
				exit(1);
			}
			flag = (strcmp(pt, keyword) ? 0 : 1);
		} else if(flag) {
			(void) fputs(line, fpout);
		}
	}
	(void) fclose(fpout);
	if(flag > 0) {
		if(docmd(cmd, destfile, input)) {
			progerr("command failed <%s>", cmd);
			exit(1);
		}
	}
	(void) unlink(input);
	exit(0);
	/*NOTREACHED*/
}

static int
docmd(cmd, file, input)
char	*cmd, *file, *input;
{
	char *tempout;
	char command[256];

	tempout = tempnam(NULL, "temp1");	
	if(!tempout)
		return(-1);

	(void) sprintf(command, "%s -f %s <%s >%s", cmd, input, file, tempout);
	if(system(command))
		return(-1);

	(void) sprintf(command, "cp %s %s", tempout, file);
	if(system(command))
		return(-1);

	(void) unlink(tempout);
	free(tempout);
	return(0);
}

static void
usage()
{
	(void) fprintf(stderr,"usage: %s cmd keyword src dest\n", prog);
	exit(2);
}
