/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:get_assign.c	1.3"

/* #define		DEBUG		1	/* */

/*
			get_assignments()

	Loads our hardware_table with the devices found in the
	ASSIGNMENTS file. Comments are allowed in ASSIGNMENTS file
	and are indicated by a '#' in column one.

	Return -1 on failure;
*/

#include	"MS-DOS.h"
#include	<stdio.h>

get_assignments()
{
	char	buffer[MAX_ASSIGNMENTS];
	FILE	*handle;
	int	i;
	int	j;
	int	index;

	/*
		We determine our MS-DOS file by using the
		environment variable ${MSDOS}. If it exists,
		that is our assignments file, otherwise we
		use ASSIGNMENTS (defined in MS-DOS.h).
	*/
#ifdef MSDOSENV
	char	*c_ptr;

	if ((c_ptr = getenv(MSDOSENV)) == NULL)
		(void) strcpy(buffer, ASSIGNMENTS);
	else
		(void) strcpy(buffer, c_ptr);
#else
	(void) strcpy(buffer, ASSIGNMENTS);
#endif

	/*
		fopen() the ASSIGNMENTS file.
	*/
	if ((handle = fopen(buffer, "r")) == NULL) {
		/*
			Set up defaults
		*/
		hardware_table[0].device_letter = 'A';
		(void) strcpy(hardware_table[0].device_path, DEFAULT_A);

		hardware_table[1].device_letter = 'B';
		(void) strcpy(hardware_table[1].device_path, DEFAULT_B);
		return(0);
	}

	/*
		Read each record from the ASSIGNMENTS file.
		Ignore records which begin with '#'. Add all
		non-commentary records to our hardware_table.
	*/
	for (index = 0; index < MAX_DEVICES && my_fgets(buffer, MAX_ASSIGNMENTS, handle) != NULL; index++) {
		/*
			Skip comments
		*/
		while (buffer[0] == '#') {
#ifdef DEBUG
			(void) fprintf(stderr, "get_assignments(): DEBUG - Skip comment line \"%s\"\n", buffer);
#endif
			if (my_fgets(buffer, MAX_ASSIGNMENTS, handle) == NULL)
				return(0);
			continue;
		}

		/*
			Ignore lines of whitespace or nothing at all
		*/
		if (buffer[0] == '\n')
			continue;

		for (i = 0; buffer[i] == ' ' || buffer[i] == '	'; i++)
			if (buffer[i] == '\n')
				continue;

		/*
			If line ends in a newline - remove it
		*/
		if (buffer[strlen(buffer) - 1] == '\n')
			buffer[strlen(buffer) - 1] = '\0';

		/*
			Store MS-DOS device letter
		*/
		HARDWARE.device_letter = buffer[0];

		/*
			Scoop up the UNIX device 
		*/
		for (j = 0, i = 2; buffer[i] != '\0'; i++) {
			if (buffer[i] == ' ' || buffer[i] == '\t')
				break;

			switch(buffer[i]) {
			case ' ':	/* Stop on whitespace */
			case '\t':
				break;

			default:	/* Store this char */
				HARDWARE.device_path[j++] = buffer[i];
				break;
			}
		}

		HARDWARE.device_path[j] = '\0';

#ifdef DEBUG
		(void) fprintf(stderr, "get_assignments(): DEBUG - device_path #%d = \"%s\"\n", index,  HARDWARE.device_path);
#endif

	}

	return(0);
}
