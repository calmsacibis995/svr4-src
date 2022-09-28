/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:vidi.c	1.1"
/**
 *  SCO Xenix compatible vidi command - used to remap the font and to
 *  change the display type.
 **/

#include <stdio.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <sys/kd.h>

#define CONSOLE "/dev/console"
#define VIDEO	"/dev/video"
#define SUCCESS 0
#define ERROR 1
#define GEN_LEN 40
#define MAX_FONT_SIZE 4096
#define SIZE_8x8 2048
#define SIZE_8x14 3584
#define SIZE_8x16 4096
#define DEFLT_8x8 "/usr/lib/vidi/font8x8"
#define DEFLT_8x14 "/usr/lib/vidi/font8x14"
#define DEFLT_8x16 "/usr/lib/vidi/font8x16"

#define	DFLAG	0x01
#define	FFLAG	0x02

extern int errno;

void usage();

/**
*  Main program
**/
main(argc, argv)
int argc;
char *argv[];
{
	int f_size;        /*  Size in bytes of the font file to read  */
	int rb;            /*  Number of bytes read from font file  */
	int font_fd;    /*  Font file descriptor  */
	int vid_fd;        /*  Console file descriptor  */
	int cmd;        /*  ioctl command value  */

	char cmd_flag;                /*  Command type font remap/other  */
	char ext_flag;                /*  Extract the font */
	char *file_name,*deflt;    /*  Font file name  */
	int ropt = 0;

	/*  Process the argument list  */

	if (argc < 2) {
		usage();
		exit(ERROR);
	}

	/*  Initialise variables  */
	ext_flag = 0;
	file_name = (char *) NULL;

	*argv++;    /*  Skip command name  */
	cmd = 0;

	/*  Loop until no more args  */
	while (*argv != NULL) {
		/*  Check for command, if we've got one then any more input
		 *  is an error.
		 */
		if (cmd != 0) {
			usage(); /* will exit */
		}

		/*  Trap the -d which means get the font from the kernel  */
		if (strcmp(*argv, "-d") == 0) {
			if (cmd != 0) 
				usage(); /* will exit */

			ext_flag |= DFLAG;
			*argv++;
			continue;

		}

		/*  Trap the -r which means reset font to ROM default */
		if (strcmp(*argv, "-r") == 0) {
			if (cmd != 0) 
				usage(); /* will exit */

			ropt = 1;
			*argv++;
			continue;

		}
		/*  Trap the -f for user filename  */
		if (strcmp(*argv, "-f") == 0) {
			/*  No file name is an error  */
			if (*(++argv) == (char *)NULL)
				usage(); /* will exit */

			if (file_name != (char *) NULL) {
				fprintf(stderr,"vidi: multiple option not allowed\n");
				usage(); /* only one -f option allowed */
			} /* usage exits */

			/*  Filenames starting with a minus are assumed to be an
			 *  argument, and thus an error. */
			if (**argv == '-')
				usage(); /* will exit */

			file_name = malloc(strlen(*argv)+1);
			if (file_name == (char *) NULL) {
				fprintf(stderr,"vidi: could not malloc(3c) space for file name");
				exit(ERROR);
			}

			/*  Record the file name  */
			(void) strcpy(file_name, *argv++);
			ext_flag |= FFLAG;
			continue;
		}

		/*  Assume that any other input is a command  */
		if (strcmp(*argv, "mono") == 0) {
			cmd = SWAPMONO;        /*  Select monochrome controller  */
		}
		else if (strcmp(*argv, "cga") == 0) {
			cmd = SWAPCGA;        /*  Select cga controller  */
		}
		else if (strcmp(*argv, "vga") == 0) {
			cmd = SWAPVGA;        /*  Select vga controller  */
		}
		else if (strcmp(*argv, "ega") == 0) {
			cmd = SWAPEGA;        /*  Select ega controller  */
		}
		else if (strcmp(*argv, "c40x25") == 0) {
			cmd = SW_C40x25;        /*  CGA 40x25 text */
		}
		else if (strcmp(*argv, "e40x25") == 0) {
			cmd = SW_ENHC40x25;        /*  EGA 40x25 text */
		}
		else if (strcmp(*argv, "v40x25") == 0) {
			cmd = SW_VGAC40x25;        /*  VGA 40x25 text */
		}
		else if (strcmp(*argv, "m80x25") == 0) {
			cmd = SW_B80x25;
		}
		else if (strcmp(*argv, "c80x25") == 0) {
			cmd = SW_C80x25;
		}
		else if (strcmp(*argv, "em80x25") == 0) {
			cmd = SW_ENHB80x25;
		}
		else if (strcmp(*argv, "e80x25") == 0) {
			cmd = SW_ENHC80x25;
		}
		else if (strcmp(*argv, "vm80x25") == 0) {
			cmd = SW_VGAMONO80x25;
		}
		else if (strcmp(*argv, "v80x25") == 0) {
			cmd = SW_VGAC80x25; 
		}
		else if (strcmp(*argv, "e80x43") == 0) {
			cmd = SW_ENHC80x43; 
		}
		else if (strcmp(*argv, "mode5") == 0) {
			cmd = SW_CG320;    
		}
		else if (strcmp(*argv, "mode6") == 0) {
			cmd = SW_BG640;
		}
		else if (strcmp(*argv, "modeD") == 0) {
			cmd = SW_CG320_D; }
		else if (strcmp(*argv, "modeE") == 0) {
			cmd = SW_CG640_E;
		}
		else if (strcmp(*argv, "modeF") == 0) {
			cmd = SW_EGAMONOAPA;
		}
		else if (strcmp(*argv, "mode10") == 0) {
			cmd = SW_CG640x350;
		}
		else if (strcmp(*argv, "mode11") == 0) {
			cmd = SW_VGA640x480C;
		}
		else if (strcmp(*argv, "mode12") == 0) {
			cmd = SW_VGA640x480E;
		}
		else if (strcmp(*argv, "mode13") == 0) {
			cmd = SW_VGA320x200;
		}
		else if (strcmp(*argv, "att640") == 0) {
			cmd = SW_ATT640;
		}
		else if (strcmp(*argv, "att800x600") == 0) {
			cmd = SW_VDC800x600E;
		}
		else if (strcmp(*argv, "att640x400") == 0) {
			cmd = SW_VDC640x400V;
		}
		else if (strcmp(*argv, "cpq0") == 0) {
			cmd = -1;        /*  do not support this */
		}
		else if (strcmp(*argv, "cpq1") == 0) {
			cmd = -1;        /*  do not support this */
		}
		else if (strcmp(*argv, "font8x8") == 0) {
			/*  Select font command according to extract flag  */
			if (ext_flag & DFLAG)
				cmd = GIO_FONT8x8;
			else 
				cmd = PIO_FONT8x8;

			f_size = SIZE_8x8;    /*  Set the size of the font map  */
			deflt = DEFLT_8x8; 
		}
		else if (strcmp(*argv, "font8x14") == 0) {
			/*  As above but for 8x14 font file  */
			if (ext_flag & DFLAG)
				cmd = GIO_FONT8x14;
			else
				cmd = PIO_FONT8x14;
			deflt = DEFLT_8x14; 
			f_size = SIZE_8x14;
		}
		else if (strcmp(*argv, "font8x16") == 0) {
			/*  As above but for 8x16 font  */
			if (ext_flag & DFLAG)
				cmd = GIO_FONT8x16;
			else
				cmd = PIO_FONT8x16;

			deflt = DEFLT_8x16; 
			f_size = SIZE_8x16;
		}
		else /*  Invalid command  */
			usage();

		/*  Move to next argument  */
		*(++argv);
	}


	/*  Open the console for ioctl calls  */
	if ((vid_fd = open(VIDEO, O_RDONLY)) < 0) {
		fprintf(stderr, "vidi: Not on a virtual terminal. Unable to access %s\n",VIDEO);
		exit(ERROR);
	}

	/* do command */

	switch (cmd) {

	case PIO_FONT8x8:
	case PIO_FONT8x14:
	case PIO_FONT8x16:
		if (ropt && ext_flag) {
			fprintf(stderr,"-r option not compatible with -f or -d\n");
			usage();
		}
		pio_font(vid_fd,cmd,file_name,f_size,deflt,ropt);
		exit(SUCCESS);

	case GIO_FONT8x8:
	case GIO_FONT8x14:
	case GIO_FONT8x16:
		if (ropt) {
			fprintf(stderr,"-r option not compatible with -f or -d\n");
			usage();
		}
		gio_font(vid_fd,cmd,file_name,f_size);
		exit(SUCCESS);

	case -1:
		fprintf(stderr,"vidi: invalid argument\n");
		exit(ERROR);

	default:
		if (ropt) {
			fprintf(stderr,"-r option only for resetting fonts\n");
			usage();
		}
		if (ioctl(vid_fd,cmd,0) < 0) {
			perror("vidi: ");
			exit(ERROR);
		}
		exit(SUCCESS);
	}
}

gio_font(fd,cmd,fname,size)
int fd,cmd,size;
char *fname;
{
	int font_fd;
	unchar fontbuf[MAX_FONT_SIZE];

	/*  If no file name has been supplied use stdout  */
	if (fname == (char *)NULL)
		font_fd = fileno(stdout);
	/*  Otherwise use the filename supplied  */
	else {
		if ((font_fd = open(fname, O_WRONLY| O_CREAT, 0666)) < 0) {
			fprintf(stderr, "vidi: cannot write to %s\n", fname);
			exit(ERROR);
		}
		free(fname);
	}
	/* upload font */
	if (ioctl(fd,cmd,&fontbuf[0]) < 0) {
		perror("vidi: font read ioctl failed: ");	
		exit (ERROR);
	}
	/* write font to file */
	if (write(font_fd,&fontbuf[0],size) != size) {
		fprintf(stderr,"vidi: attempt to write font output failed\n");
		exit(ERROR);
	}
}

pio_font(fd,cmd,fname,size,deflt,resetflg)
int fd,cmd;
char *fname,*deflt;
int size;
int resetflg;
{
	int font_fd;
	unchar fontbuf[MAX_FONT_SIZE];

	if (!resetflg) {
	   /*  If no file name has been supplied use deflt */
	   if (fname == (char *)NULL) {
		if ((font_fd = open(deflt, O_RDONLY)) < 0) {
			fprintf(stderr, "vidi: cannot open %s\n", deflt);
			exit(ERROR);
		}
	   }
	   /*  Otherwise use the filename supplied  */
	   else {
		if ((font_fd = open(fname, O_RDONLY)) < 0) {
			fprintf(stderr, "vidi: cannot open %s\n", fname);
			exit(ERROR);
		}
		free(fname);
	   }
	   /* read in font */
	   if (read(font_fd,&fontbuf[0],size) != size) {
		fprintf(stderr,"vidi: attempt to read font input failed\n");
		exit(ERROR);
	   }
	   /* download font */
	   if (ioctl(fd,cmd,&fontbuf[0]) < 0) {
		perror("vidi: font change ioctl failed: ");	
		exit (ERROR);
	   }
	}
	else /* reset font to default */
	   if (ioctl(fd,cmd,(char *) NULL) < 0) {
		perror("vidi: font change ioctl failed: ");	
		exit (ERROR);
	   }
}

/*
 *  Display the usage of the vidi command
 **/
void
usage()
{
	fprintf(stderr,"Usage: vidi [-d] [-f fontfile] [mode or font]\n");
	fprintf(stderr,"modes:\n");
	fprintf(stderr, "mono\t\tcga\t\tega\t\tvga\t\tc40x25\n");
	fprintf(stderr, "e40x25\t\tv40x25\t\tm80x25\t\tc80x25\t\tem80x25\n");
	fprintf(stderr, "e80x25\t\tvm80x25\t\tv80x25\t\te80x43\t\tmode5\n");
	fprintf(stderr, "mode6\t\tmodeD\t\tmodeE\t\tmodeF\t\tmode10\n");
	fprintf(stderr, "mode11\t\tmode12\t\tmode13\t\tcpq0\t\tcpq1\n");
	fprintf(stderr, "att640\t\tatt800x600\tatt640x400\n\n");
	fprintf(stderr, "fonts:\n");
	fprintf(stderr, "font8x8\t\tfont8x14\tfont8x16\n");
	exit(ERROR);
}
