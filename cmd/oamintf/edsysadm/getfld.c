/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:edsysadm/getfld.c	1.2.1.2"

#include <stdio.h>

/*******************************************************************************
 *	Module Name: getfld
 *	
 *	getfld pkg_desc_file item
 *	
 *	Description: This function retrieves the item from the
 *		     packacge description file.  It is used in 
 *		     Form.menu and Form.task to retrieve each
 *		     item within the form.
 ******************************************************************************/

/* Functions */
char *fgets();
char *strcpy();
FILE *fopen();


main(argc, argv)
int argc;
char *argv[];
{
	FILE *fp;
	char tmp_item[1024];			/* tmp file item */
	char pr_item[1024];			/* item buffer */
	static char name[] = "NAME";		/* Name indicator */
	static char desc[] = "DESC";		/* Description indicator */
	static char loc[] = "LOC";		/* Location indicator */
	static char help[] = "HELP";		/* Help indicator */
	static char action[] = "ACTION";	/* Task Action indicator */
	static char files[] = "FILES";		/* Task Files indicator */
	int len;
	

	/*  OPEN package description file supplied by user */
	if ((fp = fopen(argv[1], "r")) == NULL) {
		exit(1);
	}

	len = strlen(argv[2]);

	/* read each line in tmp file */
	while (fgets(tmp_item, sizeof(tmp_item), fp) != NULL)
		/* Check for item */
		if (strncmp(tmp_item, argv[2], len) == 0)
			strcpy(pr_item, (tmp_item + (len+1)));

	/* Print field info to stdout */
	printf("%s", pr_item);

	fclose(fp);
}
