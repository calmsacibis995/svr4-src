/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:edsysadm/updt_pkgdesc.c	1.4.1.2"

#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>

/************************************************************************************
 *	Module Name: updt_pkgdesc
 *	
 *      (args)->       1     2     3    4     5    6       7       8       9    10
 *	updt_pkgdesc flag pkgdesc loc descp help action mifile prototype files olog
 *
 *	This command gets requested information from the package description
 *      file and writes to standard out.  Arguments are as follows:
 *
 *	  flag     --> chgmenu
 *		       chgtask
 *		       addmenu
 *		       addtask
 *
 *	  pkg_desc --> name of desired package description file.  It
 *		       can be a full path name to the file or just the
 *		       file name.  If just a file name is present it is
 *		       assumed to be in the current invocation directory.
 *		       If a package description file does not exist than
 *		       one is created with the name given.
 *
 * 	  loc      --> CHANGED logical location of item (includes name field)
 *
 *	  descp    --> item description
 *
 *	  help     --> item help file
 *
 * 	  action   --> task item action file (TASK ONLY)
 *
 *	  mifile   --> absolute path to menu information file
 *
 *	  prototype -> absolute path to prototype file
 *
 *	  files    --> Comma separated list of files (TASK ONLY)
 *
 *	  oloc     --> ORINGINAL location
 ************************************************************************************/

#define	SUCCESS		0
#define	OPEN_ERR	1

#define LNSZ	256
#define	IDENT	6

extern int	errno;
void perror();

static char menumark[] = "#menu#";	/* menu line prefix */
static char taskmark[] = "#task#";	/* task line prefix */
static char filemark[] = "#file#";	/* file line prefix */

/* Functions */
extern char *strtok(), *getcwd();

main(argc, argv)
int argc;
char *argv[];
{
	FILE *fp;			/* package description file pointer */
	FILE *tmpfp;			/* temporary file pointer */
	int  taskfiles = 0;		/* flag to process task files */
	char pkgdesc_line[LNSZ];	/* package description line buffer */
	char add_line[LNSZ];		/* add line buffer */
	char *tmp;			/* pointer to temporary file */
	char *cwd;			/* pointer to current directory */
	char flag[7];			/* menu/task flag */
	char *itemloc;			/* pointer to location in item */
	static char item_delim[] = "^";	/* delimiter within an item from package
					   description file */


	/* Functions */
	FILE *fopen();
	char *fgets();
	char *strchr();
	char *strcpy();
	char *tempnam();

	strcpy(flag, argv[1]);

	/***************/
	/*  ADD ENTRY  */
	/***************/

	if ((strcmp("addtask", flag) == 0) ||
	    (strcmp("addmenu", flag) == 0)) {

		/*  Flag is ADD - OPEN file create append */
		if ((fp = fopen(argv[2], "a+")) == NULL) {
			printf("Error - cannot open %s.\n", argv[2]);
			exit(OPEN_ERR);
		}

		if (strcmp("addmenu", flag) == 0) 
			/* Add menu entry to package description file */
			fprintf(fp, "%s%s^%s^^%s^%s^%s\n",menumark,argv[3],argv[4],
                        	argv[5],argv[7],argv[8]);

		else {
			/* Add task entry to package description file */
			fprintf(fp, "%s%s^%s^%s^%s^%s^%s\n",taskmark,argv[3],argv[4],
                        	argv[5],argv[6],argv[7],argv[8]);

			/* Add file enties to package description file */
			if (argv[9])
				write_files(fp,argv[9]);
		}


		fclose(fp);
	}

	/******************/
	/*  CHANGE ENTRY  */
	/******************/

	else if ((strcmp("chgtask", flag) == 0) ||
	         (strcmp("chgmenu", flag) == 0)) {
		/*  Flag is CHG - OPEN file read */
		if ((fp = fopen(argv[2], "r")) == NULL) {
			printf("Error - cannot open %s.\n", argv[2]);
			exit(OPEN_ERR);
		}

		/* Get path-name of current working directory */
		if ((cwd = getcwd((char *)NULL, 128)) == NULL) {
			puts("Error - cannot get path name of current directory.\n");
			exit(OPEN_ERR);
		}

		/* Create temporary file */
		if ((tmp=tempnam(cwd, 0)) == NULL) {
			puts("Error - cannot create temp file.\n");
			exit(OPEN_ERR);
		}

		/*  OPEN temporary file create - read/write */
		if ((tmpfp = fopen(tmp, "w+")) == NULL) {
			puts("Error - cannot open tmp file\n");
			exit(OPEN_ERR);
		}

		/* read each line in package description file */
		while (fgets(pkgdesc_line, sizeof(pkgdesc_line), fp) != NULL) {

			if ((taskfiles) &&
			   (strncmp(pkgdesc_line, filemark, IDENT) == 0))
				continue;
			else taskfiles = 0;

			strcpy(add_line, pkgdesc_line);
			itemloc = strtok(pkgdesc_line, item_delim);

			if (strcmp(itemloc + IDENT, argv[10]) == 0) {
				if (strcmp("chgmenu", flag) == 0) 
				  fprintf(tmpfp, "%s%s^%s^%s^^%s^%s\n",menumark,
					  argv[3],argv[4],argv[5],argv[7],argv[8]);

				else {
				  fprintf(tmpfp, "%s%s^%s^%s^%s^%s^%s\n",taskmark,
					  argv[3],argv[4],argv[5],argv[6],argv[7],
					  argv[8]);

					write_files(tmpfp, argv[9]);
					taskfiles++;
				}
			}
			else {
				fputs(add_line, tmpfp);
			}
		}

		fclose(fp);
		fclose(tmpfp);

		if (unlink(argv[2]) < 0)
			printf("unlinking %s file failed\n", argv[2]);
	
/*
		if (rename(tmp, argv[2]) != 0)
			printf("rename %s file to %s file failed.\n", argv[2], tmp);
*/
		if (link(tmp, argv[2]) < 0) {
			printf("linking %s file to %s file failed.\n", tmp, argv[2]);
		}


		if (unlink(tmp) < 0) 
			puts("unlinking tmp file failed\n");
	}

	/**************/
	/*  BAD FLAG  */
	/**************/

	else {
		puts("Error - Bad action flag given pdt_pkgdesc.\n");
	}

exit(SUCCESS);
}

/*******************************************************************************
*	Module Name: write_files
*
*	this routine write parses a comma separated string and writes
*	each filename to a file in the following format:
*		#file#file_name
*******************************************************************************/
write_files(fp,files)
FILE *fp;
char *files;
{
	char *file;				/* pointer to a filename */
	static char file_delim[] = ",";	 	/* delimiter within file list of
				 	   	   package description file */

	file = strtok(files, file_delim);
	fprintf(fp, "%s%s\n", filemark, file);

	while (file = strtok(NULL, file_delim))
		fprintf(fp, "%s%s\n", filemark, file);
}
