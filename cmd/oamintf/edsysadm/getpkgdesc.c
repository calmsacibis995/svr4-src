/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:edsysadm/getpkgdesc.c	1.3.1.2"

#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>

/*******************************************************************************
 *	Module Name: getpkgdesc
 *	
 *	getpkgdesc -t | -m | -n name  pkg_desc_file
 *
 *	This command gets requested information from the package description
 *      file and writes to standard out.  Arguments are as follows:
 *
 *	  flag --> '-t' indicates task definition retreival
 *
 *		   '-m' indicates menu definition retreival.
 *
 *		   '-n' indicates the name of a task or menu to be
 *		        retreived.  It is the complete logical path
 *		 	name that defines the menu or task.  It must
 *			be in the same format (colon separated list)
 *			as it appears within the package description
 *			file and the menu information file.
 *
 *	  pkg_desc_file --> name of desired package description file.  It
 *			    can be a full path name to the file or just the
 *			    file name.  If just a file name is present it is
 *			    assumed to be in the current invocation directory.
 *
 *
 *	This command is called by Form.getpkg that passes the package
 *	description file and '-t' or '-m' flag indicating a task or
 *	menu information desired.  In the case of multiple menu or task
 *	description entries, output will consist of a list of each
 *	description entry.  This output would redirected to a temporary
 *	file and be used as input to Menu.choices.  The user would select
 *	the desired description entry and this command would be called
 *	again with the package description file.
 *	
 ******************************************************************************/

#define	DEF_OUT		0
#define	CHOICE_OUT	1
#define	DEF_NOTFND	2
#define	OPEN_ERR	3
#define	BAD_ARGCNT	4

#define LNSZ	128
#define	IDENT	6


char *itemline;			/* pointer to a line in pkg desc file */
char *itemname;			/* pointer to menu/task name */
char *itemdesc;			/* pointer to menu/task description */
char *itemloc;			/* pointer to menu/task location */
char *itemhelp;			/* pointer to menu/task help */
char *itemaction;		/* pointer to task action */
int  found=0;			/* entry found flag */


static char item_delim[] = "^";	 	/* delimiter within item in 
				 	   package description file */
static int loc_delim = ':'; 		/* delimiter within location field */

/* Functions */
char *fgets();
char *strrchr();
char *strcpy();
char *strcat();
char *strncat();
char *strtok();
void rewind();


main(argc, argv)
int argc;
char *argv[];
{
	extern char *optarg;		/* pointer to option arg used by getopt */
	extern int optind, opterr;	/* used by getopt */
	FILE *fp;			/* package description file pointer */
	FILE *fopen();
	int opt;			/* return from getopt */
	int tflag = 0;			/* task flag */
	int mflag = 0;			/* menu flag */
	int nflag = 0;			/* name flag */
	int errflag = 0;		/* error flag */
	int taskcnt = 0;		/* task lines counter */
	int menucnt = 0;		/* menu lines counter */
	int ret;			/* return code */
	int n = 0;			/* throw away variable */
	int first_file = 0;		/* first file flag */
	int taskfiles = 0;		/* task file flag */
	char pkgdesc_line[LNSZ];	/* package description line buffer */
	char ck_line[LNSZ];		/* check line buffer */
	char savitemfiles[LNSZ];	/* save task file buffer */
	char itemfiles[LNSZ];		/* task file buffer */
	char objname[LNSZ];		/* menu or task name */

	static char menumark[] = "#menu#";	/* menu line prefix */
	static char taskmark[] = "#task#";	/* task line prefix */
	static char filemark[] = "#file#";	/* file line prefix */

	/* Check arguement count */
	if ((argc < 3) || (argc > 4)) {
		printf("Usage: getpkgdesc -t | -m | -n name pkg_desc_file\n");
		exit(BAD_ARGCNT);
	}

	/* Process command line options */
	while ((opt = getopt(argc, argv, "tmn:")) != EOF)  {
		switch (opt) {
			case 't':
				if (mflag || nflag)
					errflag++;
				else
					tflag++;
				break;
			case 'm':
				if (tflag || nflag)
					errflag++;
				else
					mflag++;
				break;
			case 'n':
				if (tflag || mflag)
					errflag++;
				else {
					strcpy(objname, optarg);
					nflag++;
				}
				break;
			default:
				errflag++;
				break;
		}
		/* Bad option entered */
		if (errflag) {
			printf("Usage: getpkgdesc -t | -m | -n name pkg_desc_file");
			exit(BAD_ARGCNT);
		}
	}

		

	/*  OPEN package description file supplied by user */
	if ((fp = fopen(argv[optind], "r")) == NULL) {
		printf("Error - cannot open %s.\n", argv[optind]);
		exit(OPEN_ERR);
	}

	/* task/menu flag option given - check for mutiple task or menu lines */
	if (tflag || mflag) {

		/* read each line in package description file */
		while (fgets(pkgdesc_line, sizeof(pkgdesc_line), fp) != NULL) {
			/* task flag option given - check for mutiple task lines */
			if (tflag) {
				if (strncmp(pkgdesc_line, taskmark, IDENT) == 0)
					++taskcnt;
			}
			/* menu flag option given - check for mutiple menu lines */
			else if (mflag) {
				if (strncmp(pkgdesc_line, menumark, IDENT) == 0)
					++menucnt;
			}
		}
	}

	/* Position read pointer back to beginning of file */
	rewind(fp);

	/* read each line in package description file */
	while (fgets(pkgdesc_line, sizeof(pkgdesc_line), fp) != NULL) {

		/* process task files */
		if (taskfiles) {
			if (strncmp(pkgdesc_line, filemark, IDENT) == 0) {
			
				/* Initialize savitemfiles buffer */
				*savitemfiles = '\0';
				*itemfiles = '\0';

				if (first_file) {
					printf("FILES ");
					/* Task file name */
					strcpy(savitemfiles, (pkgdesc_line + IDENT));
					n = strlen(savitemfiles);
					strncat(itemfiles, savitemfiles, (n - 1));
					first_file = 0;
				}
				else {
					/* Task file name */
					strcat(itemfiles, ",");
					strcpy(savitemfiles, (pkgdesc_line + IDENT));
					n = strlen(savitemfiles);
					strncat(itemfiles, savitemfiles, (n - 1));
				}

				printf("%s", itemfiles);
			}

		}
		/* Search package description file for object given */
		else if (nflag) {
			strcpy(ck_line, (pkgdesc_line + IDENT));
			itemloc = strtok(ck_line, item_delim);
			
			if (strcmp(itemloc, objname) == 0) {
				if (strncmp(pkgdesc_line, taskmark, IDENT) == 0) {
					do_taskline(pkgdesc_line);
					taskfiles = 1;
					first_file = 1;
					ret = DEF_OUT;
				}
				
				if (strncmp(pkgdesc_line, menumark, IDENT) == 0) {
					do_menuline(pkgdesc_line);
					ret = DEF_OUT;
				}
			}
		}
		/* only 1 task line exist in package description file */
		else if (tflag && (taskcnt == 1)) {
			if (strncmp(pkgdesc_line, taskmark, IDENT) == 0) {
				do_taskline(pkgdesc_line);
				taskfiles = 1;
				first_file = 1;
				ret = DEF_OUT;
			}
		}
		/* only 1 menu line exist in package description file */
		else if (mflag && (menucnt == 1)) {
			if (strncmp(pkgdesc_line, menumark, IDENT) == 0) {
				do_menuline(pkgdesc_line);
				ret = DEF_OUT;
			}

		}
		/* Multiple task lines exist */
		else if (taskcnt > 1) {
			if (strncmp(pkgdesc_line, taskmark, IDENT) == 0) {
				do_choices(pkgdesc_line);
				ret = CHOICE_OUT;
			}
		}	
		/* Multiple menu lines exist */
		else if (menucnt > 1) {
			if (strncmp(pkgdesc_line, menumark, IDENT) == 0) {
				do_choices(pkgdesc_line);
				ret = CHOICE_OUT;
			}
		}	
	}
	fclose(fp);
	
	if (!found)
		ret = DEF_NOTFND;

	exit(ret);
}

/********************************************************************************
*
*	Module Name: do_menuline
*
*	do_menuline *pkgdesc_line
*
*	This function accepts a pointer to a menu (#menu#) line in the package
*	description file.  It will parse the line and prints to stdout in
*	the following format:
*
*		NAME (object name)
*		DESC (object description)
*		LOC (object location)
*		HELP (help file name)
*
********************************************************************************/
do_menuline(menuline)
char *menuline;
{

	/* Menu logical location */
	itemloc = strtok(menuline, item_delim);
			
	/* Menu name */
	/* Put a null at the end of itemloc and increment */
        /* itemname past the location delimitor		  */
	if (itemname = strrchr(itemloc, loc_delim))
		*itemname++ = '\0';

	/* Menu description */
	itemdesc = strtok(NULL, item_delim);
		
	/* Menu should not have an action 
	itemaction = strtok(NULL, item_delim);
	*/

	/* Menu help file path */
	itemhelp = strtok(NULL, item_delim);

	printf("NAME %s\n", itemname);
	printf("DESC %s\n", itemdesc);
	printf("LOC %s\n", (itemloc + IDENT)); /* skip identification mark */
	printf("HELP %s\n", itemhelp);
	found = 1;
}

/********************************************************************************
*
*	Module Name: do_taskline
*
*	do_taskline *pkgdesc_line
*
*	This function accepts a pointer to a task (#task#) line in the package
*	description file.  It will parse the line and prints to stdout in
*	the following format:
*
*		NAME (object name)
*		DESC (object description)
*		LOC (object location)
*		HELP (help file name)
*		ACTION (task action file)
*		FILES (comma separated list of file)
*
********************************************************************************/
do_taskline(taskline)
char *taskline;
{

	/* Task logical location */
	itemloc = strtok(taskline, item_delim);
			
	/* Task name */
	if (itemname = strrchr(itemloc, loc_delim))
		*itemname++ = '\0';

	/* Task description */
	itemdesc = strtok(NULL, item_delim);
		
	/* Task action file */
	itemaction = strtok(NULL, item_delim);

	/* Task help file path */
	itemhelp = strtok(NULL, item_delim);

	printf("NAME %s\n", itemname);
	printf("DESC %s\n", itemdesc);
	/* Need to skip line identification */
	printf("LOC %s\n", (itemloc + IDENT));
	printf("HELP %s\n", itemhelp);
	printf("ACTION %s\n", itemaction);
	found = 1;
}

/********************************************************************************
*
*	Module Name: do_choices
*
*	do_choices *pkgdesc_line
*
*	This function accepts a pointer to a menu/task (#menu#/#task#) line
*	in the package description file.
*
********************************************************************************/
do_choices(choiceline)
char *choiceline;
{

	/* Menu/Task logical location */
	itemloc = strtok(choiceline, item_delim);
			
	printf("%s\n", (itemloc + IDENT));
	found = 1;
}
