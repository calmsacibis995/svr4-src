/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:doscp.c	1.3.2.2"

/* #define		DEBUG		1	/* */

/*
		doscp file file

		doscp file ... directory
*/

#include	"MS-DOS.h"

#include	<stdio.h>

#include	<sys/stat.h>

#include	<fcntl.h>

char	doscpUsage[] = "Usage: doscp from_file to_file\n\tdoscp from_file ... to_directory\n";

#define	UPDATE_DIR \
	if (read_sector(output_handle, output_dir_sector) == -1) \
		return(-1); \
 \
	sector_buffer[FILE_SIZE + 3 + output_dir_disp] = (file_size >> 24) & 0xFF; \
	sector_buffer[FILE_SIZE + 2 + output_dir_disp] = (file_size >> 16) & 0xFF; \
	sector_buffer[FILE_SIZE + 1 + output_dir_disp] = (file_size >> 8) & 0xFF; \
	sector_buffer[FILE_SIZE + output_dir_disp] = file_size & 0xFF; \
 \
	if (write_sector(output_handle, output_dir_sector) == -1) \
		return(-1);

main(argc, argv)
int	argc;
char	**argv;
{
	int	c;			/* Index for argv loop */
	int	conversion;		/* CR/LF Translation switch */
	long	disp;			/* sector index variable */
	long	file_size;
	int	input_handle, ret;		/* UNIX input file descriptor */
	int	index;
	long	output_dir_disp;	/* Sector for new dir entry */
	long	output_dir_sector;	/* Sector for new dir entry */
	FILE	*output_fptr;		/* For UNIX output files */
	int	output_handle;		/* UNIX output file descriptor */
	long	starting_output_cluster; /* Starting output cluster */
	struct	stat	stat_buf;	/* For UNIX input files */
	int	target_dir;		/* Is our target a directory? */
	long	unix_file_size;		/* UNIX input file size */
	int	unix_in;		/* Have we UNIX input? */
	int	unix_in_inode;
	int	unix_out;		/* Have we UNIX output? */
	int	unix_out_inode;
#ifdef  DOSCP_BE_BROKEN
	char	p[100], q[100];
	char	*c_ptr1, *c_ptr2, *r, *s, *t, *m;
#endif


	if (argc < 3) {
		printf("doscp: Insufficient arguments, 2 arguments are required.\n");
		exit(1);
	}

	/*
		Parse command line arguments 
	*/
	while ((c = getopt(argc, argv, "rm")) != EOF)
		switch(c) {
		case 'm':
			conversion = 1;
			break;

		case 'r':
			conversion = 0;
			break;

		default:
			(void) fprintf(stderr, doscpUsage);
			exit(1);
		}

	/*
		When all done with command line options, we must
		have AT LEAST:

			doscp source target

		remaining.
	*/
#ifdef DEBUG
	(void) fprintf(stderr, "doscp: DEBUG - optind = %d argc = %d after getopt\n", optind, argc);
	(void)fprintf(stderr, "doscp: %s %s \n", argv[1], argv[2]);
#endif


		if (optind > (argc - 1)) {
		(void) fprintf(stderr, doscpUsage);
		exit(1);
		}

	if (argv[argc - 1][strlen(argv[argc - 1]) - 1] == '/')
		argv[argc - 1][strlen(argv[argc - 1]) - 1] = '\0';

	/*
		We only check our output file once for UNIX or MS-DOS
	*/
	if (strchr(argv[argc - 1], ':') == NULL)
		unix_out = 1;
	else
		unix_out = 0;

#ifdef DEBUG
	(void) fprintf(stderr, "doscp: DEBUG - unix_out = %d\n", unix_out);
#endif

	/*
		Is our target file a directory?
	*/
	if (unix_out) {
		/*
			Check UNIX output file
		*/
		if (stat(argv[argc - 1], &stat_buf) == -1)
			target_dir = 0;
		else {
			if (stat_buf.st_mode & S_IFDIR)
				target_dir = 1;
			else {
				target_dir = 0;
			}

			unix_out_inode = stat_buf.st_ino;
		}
	} else {
		/*
			Check our MS-DOS output file

			Need Read/Write permissions here as we 
			will remove the target file, if it is not
			a directory
		*/
		if ((output_handle = open_device(argv[argc - 1], O_RDWR)) == -1)
			exit(1);
	
		/*
			Make sure we are all setup properly
		*/
		if ((index = lookup_device(output_handle)) == -1) {
			(void) fprintf(stderr, "doscp: Failed to locate output_handle %d in device_table\n", output_handle);
			exit(1);
		}

		/*
			If *filename == '\0' then we have the case 
			where the filename is "[ABCD...]:" with no
			filename (implying current directory).
		*/
		if (*filename == '\0') {
			target_dir = 1;
			dir_sector = TABLE.root_base_sector;
		}
		else {
			char	*w_ptr;

			/*
				If target file is in a subdirectory,
				make sure that directory exists.
			*/
			if (strlen(filename) > 1 && (w_ptr = strrchr(filename + 1, '/')) != NULL) {
				*w_ptr = '\0';


				if (locate(output_handle, filename) == -1) {
					(void) fprintf(stderr, "doscp: Error - Directory: \"%s\" not found\n", filename);
					exit(1);
				}


				*w_ptr = '/';
			}


			/*
				Does the MS-DOS target file exist?
			*/
			if ((disp = locate(output_handle, filename)) != -1) {
				if (sector_buffer[FILE_ATTRIBUTE + disp] == SUB_DIRECTORY)
					target_dir = 1;
				else {
					/*
						Target MS-DOS file does exist.
						It is not a directory.
					*/
					target_dir = 0;
					dir_sector = last_sector_read;
				}
			}
			else
				target_dir = 0;
		}

		(void) close_device(output_handle);
	}

	/*
		Sanity check. If our target IS NOT a directory,
		then we must have ONLY 2 filenames!
	*/
	if (target_dir == 0 && argc != optind + 2) {
		(void) fprintf(stderr, doscpUsage);
		exit(1);
	}

	if (argv[argc - 1][strlen(argv[argc - 1]) - 1] == ':')
		target_dir = 1;

#ifdef DEBUG
	(void) fprintf(stderr, "doscp: DEBUG - target_dir = %d\n", target_dir);
#endif

	/*
		At this point we know what type of target we have.
		UNIX vs MS-DOS and directory or file.

		We now traverse our input files, and do the actual copy.
	*/
	for (c = optind; c < argc - 1; c++) {
		char	target_filename[MAX_FILENAME];
		char	*w_ptr;

		/*
			Is our input UNIX?
		*/
		if ((w_ptr = strchr(argv[c], ':')) == NULL)
			unix_in = 1;
		else
			unix_in = 0;

#ifdef DEBUG
		(void) fprintf(stderr, "doscp: DEBUG - unix_in = %d\n", unix_in);
#endif

		/*
			Construct target_filename
		*/
		if (target_dir)
			(void) sprintf(target_filename, "%s/%s", argv[argc - 1], unix_in ? basename(argv[c]) : basename(w_ptr + 1));
		else
			(void) strcpy(target_filename, argv[argc - 1]);

		/*
			If we are writing an MS-DOS file, and the 
			target dir ends in a ':' - then add the
			filename of the source file to the end of
			the target_filename
		*/
/*
		if (unix_out == 0 && argv[argc - 1][strlen(argv[argc - 1]) - 1] == ':') 
			(void) strcat(target_filename, basename(argv[c]));
		

*/
#ifdef DEBUG
		(void) fprintf(stderr, "doscp: DEBUG - target_filename = \"%s\" argv[%d] = \"%s\"\n", target_filename, c, argv[c]);
#endif

		/*
			Open the input file
		*/
		if (unix_in) {
			if (stat(argv[c], &stat_buf) == -1) {
				(void) fprintf(stderr,"doscp: Error - UNIX file \"%s\" does not exist\n", argv[c]);
				exit(1);
			}
			else {
				if(stat_buf.st_mode & S_IFDIR){
						printf("doscp: The input is a UNIX dorectory . Can't make a copy\n");
						exit(1);
					}
				else{
				unix_file_size = stat_buf.st_size;
				unix_in_inode = stat_buf.st_ino;
				}

			}

			if ((input_handle = open(argv[c], O_RDONLY)) == -1) {
				(void) fprintf(stderr, "doscp: Error - Failed to open UNIX input file \"%s\"\n", argv[c]);
				perror("	Reason");
				exit(1);
			}
#ifdef DEBUG
			(void) printf("doscp: DEBUG - UNIX input handle: %d\n", input_handle);
#endif
		}
		else if (unix_out) {
			/*
				MS-DOS input with UNIX output.
				Open our MS-DOS input file
			*/
			if ((input_handle = open_device(argv[c], O_RDONLY)) == -1)
				exit(1);
			
			/*
				Make sure we are all setup properly
			*/
			if (lookup_device(input_handle) == -1) {
				(void) fprintf(stderr, "doscp: Failed to locate input_handle %d in device_table\n", input_handle);
				exit(1);
			}
	
			if ((disp = locate(input_handle, filename)) == -1) {
				(void) fprintf(stderr, "doscp: Error - Failed to open MS-DOS input file \"%s\"\n", argv[c]); 
				exit(1); 
			} 
#ifdef	DOSCP_BE_BROKEN
			else {
				if (sector_buffer[FILE_ATTRIBUTE + disp] == SUB_DIRECTORY)
				printf("doscp: The input is a DOS directory. Can't make a copy\n");
				exit(1);
			}	
#endif
		}

		/*
			Does the MS-DOS target file exist?
		*/
		if (unix_out == 0) {
			if ((output_handle = open_device(target_filename, O_RDWR)) == -1)
				exit(1);
#ifdef  DOSCP_BE_BROKEN
				if ((r = strchr(basename(argv[c]), '.')) != NULL){
				if (strlen(r + 1) > 3) {

					c_ptr1 = strchr(basename(argv[c]), ':');
					c_ptr2 = strchr(target_filename, ':');
					if (*(c_ptr1 + 1) == '/')
						c_ptr1++;

					if (*(c_ptr2 + 1) == '/')
						c_ptr2++;
						r = strchr(target_filename, '.');
						strcpy (p, r);
						strcpy(q, c_ptr2 + 1);
						r = strchr(q, '.');
						*r= NULL;
					
						(void) fprintf(stderr, "Warning: renaming filename %s to %.8s%.4s\n", argv[c], q, p);
				}
				strncpy(filename, q, 8);
				strcat(filename, '.');
				strncat(filename, p, 4);
				strupr(filename);
			}
			else {
						(void) fprintf(stderr, "Warning: renaming filename %s to %s\n", argv[c], filename + 1);
				
			}
#endif
			if ((disp = locate(output_handle, filename)) != -1) {
				/*
					Target MS-DOS file does exist.
					It is not a directory. Delete
					it.
				*/
				dir_sector = last_sector_read;

				if (rm_file(output_handle, disp) == -1)
					exit(1);
			}

			(void) close_device(output_handle);
		}

		/*
			Open output file
		*/
		if (unix_out) {
			/*
				We do not allow UNIX to UNIX copies
				where the source and target files are
				the same file. We ensure difference by
				comparing inodes.
			*/
			if (unix_in && unix_in_inode == unix_out_inode) {
				(void) fprintf(stderr, "doscp: Error - UNIX copy with non-unique source and target\n\tfiles would truncate file. Terminating.\n");
				exit(1);
			}

			/*
				Open UNIX output file
			*/
			if ((output_fptr = fopen(target_filename, "w")) == NULL) {
				(void) fprintf(stderr, "doscp: Error - Failed to open \"%s\"\n", target_filename);
				perror("	Reason");
				exit(1);
			}
		}
		else if (unix_in) {
			/*
				MS-DOS output UNIX input
				Ensure that UNIX file exists
			*/
			/*
				Open MS-DOS output file

				Opening an MS-DOS output file puts
				us in critical code until MS-DOS
				file is closed - No signals.
			*/
			critical(1);

			if ((output_handle = open_device(target_filename, O_RDWR)) == -1)
				exit(1);
		
			/*
				Make sure we are all setup properly
			*/
			if (lookup_device(output_handle) == -1) {
				(void) fprintf(stderr, "doscp: Failed to locate output_handle %d in device_table\n", output_handle);
				exit(1);
			}

			if ((output_dir_disp = Mkdir(output_handle, filename, ARCHIVE)) == -1) {
				(void) fprintf(stderr, "doscp: Error - Failed to create an MS-DOS directory entry for file \"%s\"\n\tDisk may be unusable\n", target_filename);
				exit(1);
			}

#ifdef DEBUG
			(void) fprintf(stderr, "doscp: DEBUG - mkdir returned output_dir_disp = %ld\n\toutput_dir_sector = %ld\n", output_dir_disp, last_sector_read);
#endif

			output_dir_sector = last_sector_read;
			starting_output_cluster = GET_CLUS(output_dir_disp);
		}


		/*
			Here we do the actual copy
		*/
		if (unix_in) {
			if (unix_out) {
				/* Copy UNIX to UNIX */
				uu_copy(input_handle, conversion, output_fptr);
			} else {
				/* Copy UNIX to MS-DOS */
					
				if ((file_size = ud_copy(input_handle, conversion, output_handle, starting_output_cluster, unix_file_size)) == -1) {

/*
	Copy failed. Remove the partial file.
*/
#ifdef DEBUG
	(void) fprintf(stderr, "doscp: DEBUG - output_dir_sector = %ld\n", output_dir_sector);
#endif

				UPDATE_DIR

				if (read_sector(output_handle, output_dir_sector) == -1) {
					(void) fprintf(stderr, "doscp: Error - Failed to re-read directopry sector\n\tDisk may be unusable\n");
						exit(1);
					}

					dir_sector = output_dir_sector;

					if (rm_file(output_handle, output_dir_disp) == -1)
						(void) 	fprintf(stderr, "doscp: Error - Failed to remove partial file \"%s\"\n", target_filename);

					
					/*
						Update FAT
					*/
					if (write_fat(output_handle) == -1)
						(void) 	fprintf(stderr, "doscp: Error - Failed to update FATs after write failure\n");
					exit(1);
				}

				/*
					Data has been copied, now update the 
					directory for correct file size
				*/
				UPDATE_DIR
			}
		} else {
			if (unix_out) {
				/* Copy MS-DOS to UNIX */
				(void) read_file(input_handle, (int) disp, conversion, output_fptr);
			} else {
				/* 
					Copy MS-DOS to MS-DOS.
					Must go throught he UNIX system for now.
				*/
				char	cmd_buffer[MAX_FILENAME * 4];

				if (parse_name(argv[c]) == -1)
					return(-1);

				(void) sprintf(cmd_buffer, "%s %s /tmp/%s && %s /tmp/%s %s && /bin/rm -f /tmp/%s", argv[0], argv[c], basename(filename), argv[0], basename(filename), argv[argc - 1], filename);
#ifdef DEBUG
				(void) fprintf(stderr, "%s\n", cmd_buffer);
#endif
				(void) system(cmd_buffer);
#ifdef DEBUG
				(void) fprintf(stderr, "doscp: DEBUG - system(\"%s\")\n", cmd_buffer);
#endif
			}
		}
		
		/*
			Clean up input files
		*/
		if (unix_in) 
			(void) close(input_handle);
		else {
			if (unix_out)
				(void) close_device(input_handle);
		}

		/*
			Clean up output files
		*/
		if (unix_out) {
			(void) fflush(output_fptr);
			(void) fclose(output_fptr);
		}
		else {
			if (unix_in) {
				/*
					If output is an MS-DOS file, we need
					to write out our upated FATs
				*/
				if (write_fat(output_handle) == -1) {
					(void) fprintf(stderr, "doscp: Error: Failed to write_fat\n");
					exit(1);
				}
	
				(void) close_device(output_handle);
	
				/*
					Once we are closed - no longer
					ciritcal.
				*/
				critical(0);
			}
		}
	}

	exit(0);	/* NOTREACHED */
 }

