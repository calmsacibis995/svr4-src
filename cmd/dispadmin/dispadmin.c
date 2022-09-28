/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)dispadmin:dispadmin.c	1.5.1.1"
#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<sys/priocntl.h>

/*
 * This file contains the code implementing the class independent part
 * of the dispadmin command.  Most of the functionality of the dispadmin
 * command is provided by the class specific sub-commands, the code for
 * which is elsewhere.  The class independent part of the command is
 * responsible for switching out to the appropriate class specific
 * sub-command based on the user supplied class argument.
 * Code in this file should never assume any knowledge of any specific
 * scheduler class (other than the SYS class).
 */

#define	BASENMSZ	16
#define	CLASSPATH	"/usr/lib/class"

extern char	*basename();
extern void	fatalerr();

static char usage[] =
"usage:	dispadmin -l\n\
	dispadmin -c class [c.s.o.]\n";

static char	basenm[BASENMSZ];
static char	cmdpath[256];

static void	print_classlist(), exec_cscmd();


main(argc, argv)
int	argc;
char	**argv;
{
	extern char	*optarg;
	extern int	optind, opterr;

	int		c;
	int		lflag, cflag, csoptsflag;
	char		*clname;

	strcpy(cmdpath, argv[0]);
	strcpy(basenm, basename(argv[0]));
	lflag = cflag = csoptsflag = 0;
	opterr = 0;
	while ((c = getopt(argc, argv, "lc:")) != -1) {
		switch(c) {

		case 'l':
			lflag++;
			break;

		case 'c':
			cflag++;
			clname = optarg;
			break;

		case '?':
			/*
			 * We assume for now that any option that
			 * getopt() doesn't recognize is intended for a
			 * class specific subcommand.
			 */
			csoptsflag++;
			if (argv[optind][0] != '-') {

				/*
				 * Class specific option takes an
				 * argument which we skip over for now.
				 */
				optind++;
			}
			break;

		default:
			break;
		}
	}

	if (lflag) {
		if (cflag || csoptsflag)
			fatalerr(usage);

		print_classlist();
		exit(0);

	} else if (cflag) {
		if (lflag)
			fatalerr(usage);

		exec_cscmd(clname, argv);

	} else {
		fatalerr(usage);
	}
}


/*
 * Print the heading for the class list and execute the
 * class specific sub-command with the -l option for each
 * configured class.
 */
static void
print_classlist()
{
	id_t		cid;
	int		nclass;
	pcinfo_t	pcinfo;
	static char	subcmdpath[128];
	int		status;
	pid_t		pid;


	if ((nclass = priocntl(0, 0, PC_GETCLINFO, NULL)) == -1)
		fatalerr("%s: Can't get number of configured classes\n",
		    cmdpath);

	printf("CONFIGURED CLASSES\n==================\n\n");
	printf("SYS\t(System Class)\n");
	fflush(stdout);
	for (cid = 1; cid < nclass; cid++) {
		pcinfo.pc_cid = cid;
		if (priocntl(0, 0, PC_GETCLINFO, &pcinfo) == -1)
			fatalerr("%s: Can't get class name (class ID = %d)\n",
			    cmdpath, cid);
		sprintf(subcmdpath, "%s/%s/%s%s", CLASSPATH, pcinfo.pc_clname,
		    pcinfo.pc_clname, basenm);
		if ((pid = fork()) == 0) {
			(void)execl(subcmdpath, subcmdpath, "-l", (char *)0);
			printf("%s\n", pcinfo.pc_clname);
			fatalerr("\tCan't execute %s specific subcommand\n",
			    pcinfo.pc_clname);
		} else if (pid == (pid_t)-1) {
			printf("%s\n", pcinfo.pc_clname);
			fprintf(stderr, "Can't execute %s specific subcommand)\n",
			    pcinfo.pc_clname);
		} else {
			wait(&status);
		}
	}
}


/*
 * Execute the appropriate class specific sub-command for the class
 * specified by clname, passing it the arguments in subcmdargv.
 */
static void
exec_cscmd(clname, subcmdargv)
char	*clname;
char	**subcmdargv;
{
	pcinfo_t	pcinfo;
	char		subcmdpath[128];

	/*
	 * Do a quick check to make sure clname is valid.
	 * We could just wait and see if the exec below
	 * succeeds but we wouldn't know much about the reason.
	 * This way we can give the user a more meaningful error
	 * message.
	 */
	strcpy(pcinfo.pc_clname, clname);
	if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
		fatalerr("%s: Invalid or unconfigured class %s\n", cmdpath,
		    clname);

	sprintf(subcmdpath, "%s/%s/%s%s", CLASSPATH, clname, clname, basenm);
	subcmdargv[0] = subcmdpath;

	(void)execv(subcmdpath, subcmdargv);
	fatalerr("%s: Can't execute %s sub-command\n", cmdpath, clname);
}
