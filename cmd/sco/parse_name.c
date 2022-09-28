/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:parse_name.c	1.3.2.1"

/* #define	DEBUG		1	/* */

#include	"MS-DOS.h"

#include	<stdio.h>

parse_name(dosfilename)
char	*dosfilename;
{
	int	colon_count;
	int	dot_count;
	int	index;
	char	*c_ptr;
	int	j;
	int	name_changed = 0;
	char	original_filename[MAX_FILENAME];
	int	remove_this_char;
	char	*c_ptr1;
	char	*c_ptr2;
	char 	p[100];
		char 	q[100];
		char 	*r, *s;

	/*
		Stash original filename away
	*/
	(void) strcpy(original_filename, dosfilename);

	/*
		Is it a valid MS-DOS filename?
	*/
	for (colon_count = 0, dot_count = 0, j = 0; *(dosfilename + j) != '\0'; j++) {
		char	character = *(dosfilename + j);
#ifdef DEBUG
		(void) fprintf(stderr, "parse_name(): DEBUG - Checking validity of character '%c' octal: %o\n", character, character);
#endif

		remove_this_char = 0;

		if (character < '\040' || character > '\176') {
			remove_this_char = 1;
		}
		else switch(character) {
			case ' ':
			case '*':
			case '+':
			case ',':
			case ';':
			case '<':
			case '=':
			case '>':
			case '?':
			case '@':
			case '[':
			case '\\':
			case ']':
			case '^':
				remove_this_char = 1;
				break;

			case ':':
				colon_count++;
				break;

			case '.':
				dot_count++;
	
				if (dot_count > 1)  
					remove_this_char = 1; 

				break;
		}

		if (remove_this_char) {
			/*
				Remove this character
				from the name
			*/
			name_changed++;
			*(dosfilename + j) = '\0';
			(void) strcat(dosfilename, (dosfilename + j + 1));
			j--;
		}
	}

	/*
		If not exactly one colon - NOT VALID
	*/
	if (colon_count != 1) {
		(void) printf("parse_name(): Error - File \"%s\" has %d colons - Invalid\n", dosfilename, colon_count);
		return(-1);
	}

	/*
		If the colon is in the second space, 
		then the first character must be in our 
		hardware_table array.
	*/
	if (*(dosfilename + 1) == ':') {
#ifdef DEBUG
		(void) fprintf(stderr, "parse_name(): DEBUG - \"%s\" is an MS-DOS filename DRIVE:FILE\n", dosfilename);
#endif
		/*
			Format is:

				[ABCD...]:FILENAME

			For this to be valid it must be found
			in our msdos configuration file (See
			MSDOSENV and ASSIGNMENTS in MS-DOS.h)
		*/
		strupr(dosfilename);
		fix_slash(dosfilename);

		drive = *dosfilename;

		if ((index = lookup_drive(drive)) == -1) {
			(void) fprintf(stderr, "parse_name(): Error - Drive '%c' not found in assignments file\n", drive);
			return(-1);
		}

		(void) strcpy(device_pathname, hardware_table[index].device_path);
		(void) strcpy(filename, dosfilename + 2);
	} else {
#ifdef DEBUG
		(void) fprintf(stderr, "parse_name(): DEBUG - \"%s\" is an MS-DOS filename - PATH:FILE\n", dosfilename);
#endif
		/*
			Otherwise we have the format of:

				DEVICE_PATH:FILENAME
		*/
		if ((c_ptr = strchr(dosfilename, ':')) == NULL) {
			(void) fprintf(stderr, "parse_name(): Internal error - expected colon not found\n");
			exit(1);
		}

		*c_ptr = '\0'; 

		(void) strcpy(device_pathname, dosfilename); 

		(void) strcpy(filename, c_ptr + 1);

		strupr(filename);
		fix_slash(filename);

		*c_ptr = ':';
	}


	/*
		If we had to modify the dosfilename, because of either 
		invalid characters, or multiple dots ('.') then notify
		the user here.
	*/

#ifdef DEBUG
	(void) fprintf(stderr, "parse_name(): DEBUG - Device: \"%s\" Filename: \"%s\"\n", device_pathname, filename);
#endif

	return(0);
}
