/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/enet/enetload.c	1.3.3.1"

static char *rcsid = "@(#)enetload.c  $SV_enet SV-Eval01 - 06/25/90$";

static char enetload_copyright[] = "Copyright 1987, 1988, 1989 Intel Corp. 464226";

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FALSE 0
#define TRUE  1
#define MAXENTRIES 80

#ifdef DEBUG
#define BININA "/tmp/ina"
#else
#define BININA "/usr/sbin/ina"
#endif

struct idf
{
	char	i_valid;
	char	i_file[15];	/* iNA961 filename */
	char	i_bus[10];	/* Board type */
	char	i_r1r3[4];	/* Release 1 to Release 3 support */
	char	i_vc[4];	/* Number of VCs */
	char	i_nmf[4];	/* NMF Support */
	char	i_nl[10];	/* Network Layer */
	char	i_edl[4];	/* EDL */
	char	i_comm[80];	/* Comments */
} idf[MAXENTRIES];

char		inafile[20];
int		maxentries;
int		index;

struct stat	statbuf;

extern char	*optarg;
extern int	optind;

main(argc, argv)
int	argc;
char	*argv[];
{
	FILE	*fd;
	int	i,
		vflag = 0,
		bflag = 0;
	char	c;

	/*
	 * initialize
	 */
	for(i=0; i<MAXENTRIES; i++)
		idf[i].i_valid = FALSE;

	if( argc > 1 )
	{
		while( (c=getopt(argc, argv, "v:b:")) != EOF )
		switch(c)
		{
			case 'v':
				vflag++;
				strcpy(idf[0].i_file, "ina961.");
				strcat(idf[0].i_file, optarg);
				continue;
			case 'b':
				bflag++;
				strcat(idf[0].i_bus, optarg);
				continue;
			case '?':
				printf("usage: enetload [-v version -b bus_type]\n");
				exit(1);
		}

		if(  (vflag == 0) || (bflag == 0) )
		{
			printf("usage: netload [-v version -b bus_type]\n");
			exit(1);
		}

		if( (strcmp(idf[0].i_bus, "MB1") == 0) ||
		    (strcmp(idf[0].i_bus, "mb1") == 0) ||
		    (strcmp(idf[0].i_bus, "MB2") == 0) ||
		    (strcmp(idf[0].i_bus, "mb2") == 0) ||
		    (strcmp(idf[0].i_bus, "AT")  == 0) ||
		    (strcmp(idf[0].i_bus, "at")  == 0) )
		{
			index = 0;
			idf[0].i_bus[0] &= 0xDF;
			idf[0].i_bus[1] &= 0xDF;
		}
		else
		{
			printf("enetload: invalid bus type.\n");
			exit(1);
		}
#ifdef DEBUG
		printf("inafile: %s\n", idf[0].i_file);
		printf("    bus: %s\n", idf[0].i_bus);
#endif
	}
	else
	{
		if( (fd = fopen("/etc/ina961.data", "r")) == NULL )
		{
			perror("/etc/ina961.data");
			exit(-1);
		}

		getdata(fd);
		fclose(fd);
		menu();
	}

	/*
	 * Save old "/usr/sbin/ina" as "/usr/sbin/ina.old"
	 */
#ifdef DEBUG
	system("mv /tmp/ina /tmp/ina.old > /dev/null 2>&1");
#else
	system("mv /usr/sbin/ina /usr/sbin/ina.old > /dev/null 2>&1");
#endif

	/*
	 * Build new "/usr/sbin/ina"
	 */
	if( (fd = fopen(BININA, "w")) == NULL )
	{
		perror(BININA);
		exit(-1);
	}

	header(fd);
	if( strcmp(idf[index].i_bus, "AT") == 0 )
	{
		fprintf(fd, "set `/usr/sbin/enetinfo 2> /dev/null`X\n");
		fprintf(fd, "if [ \"$1\" = \"PCL2NIA\" ]; then\n");
		fprintf(fd, "	/usr/sbin/enetpump /dev/tp4-00 /etc/%s\n", idf[index].i_file);
		fprintf(fd, "elif [ \"$1\" = \"PCL2ANIA\" ]; then\n");
		fprintf(fd, "	/usr/sbin/enetpump /dev/tp4-00 /etc/%s.2A\n", idf[index].i_file);
		fprintf(fd, "fi\n");
	}
	else if( strcmp(idf[index].i_bus, "MB2") == 0 )
	{
		fprintf(fd, "PATH=$PATH:/usr/lib/cci:/etc/conf/bin:/sbin\n");
		fprintf(fd, "if [ \"$2\" = \"186/530\" ]; then\n");
		fprintf(fd, "	lckld /etc/%s $1\n", idf[index].i_file);
		fprintf(fd, "elif [ \"$2\" = \"MIX386/560\" ]; then\n");
		fprintf(fd, "	lckld /etc/%s.M $1\n", idf[index].i_file);
		fprintf(fd, "fi\n");
	}
	else if( strcmp(idf[index].i_bus, "MB1") == 0 )
	{
		fprintf(fd, "/usr/sbin/enetpump /dev/tp4-00 /etc/%s\n", idf[index].i_file);
	}
	fclose(fd);
	chmod(BININA, 0700);
	system("rm -f /usr/bin/ina > /dev/null 2>&1");
	system("ln -s /usr/sbin/ina /usr/bin/ina > /dev/null 2>&1");

	/*
	 * Perform download
	 */
	if( stat("/usr/sbin/enetreset", &statbuf) == 0 )
		system("/usr/sbin/enetreset");

	if( idf[index].i_file[7] == '1' )
		system("modddf -u -p /dev/iso-tp4 -t ina1");
	else if( idf[index].i_file[7] == '2' )
		system("modddf -u -p /dev/iso-tp4 -t ina2");
	else if( idf[index].i_file[7] == '3' )
		system("modddf -u -p /dev/iso-tp4 -t ina3");

	if( (strcmp(idf[index].i_bus, "MB1") == 0) ||
	    (strcmp(idf[index].i_bus, "AT") == 0) )
	{
		system("/usr/sbin/ina");
	}
	else if( strcmp(idf[index].i_bus, "MB2") == 0 )
	{
		char	command[80];
		int	slot;
		char	board_type[11];

		if( (slot = getslot(board_type)) != -1 )
		{
			sprintf(command, "/usr/sbin/ina %d %s", slot,
								board_type);
			system(command);
		}
	}
}

getdata(fd)
FILE	*fd;
{
	int	i;
	char	*bp;
	char	buffer[256];
	int	readln();

	i = 0;	
	while( readln(fd, buffer) )
	{
		if( buffer[0] == '#' )
			continue;

		/*
		 * iNA961 filename
		 */
		bp = strtok(buffer, ":");
		strcpy(idf[i].i_file, bp);

		/*
		 * Board Type
		 */
		bp = strtok(NULL, ":");
		strcpy(idf[i].i_bus, bp);
		idf[i].i_bus[0] &= 0xDF;
		idf[i].i_bus[1] &= 0xDF;

		/*
		 * Release 1 to Release 3
		 */
		bp = strtok(NULL, ":");
		strcpy(idf[i].i_r1r3, bp);

		/*
		 * Number of VCs
		 */
		bp = strtok(NULL, ":");
		strcpy(idf[i].i_vc, bp);

		/*
		 * NMF Support
		 */
		bp = strtok(NULL, ":");
		strcpy(idf[i].i_nmf, bp);

		/*
		 * Network Layer
		 */
		bp = strtok(NULL, ":");
		strcpy(idf[i].i_nl, bp);

		/*
		 * EDL
		 */
		bp = strtok(NULL, ":");
		strcpy(idf[i].i_edl, bp);

		/*
		 * Comments
		 */
		bp = strtok(NULL, ":");
		strcpy(idf[i].i_comm, bp);
		idf[i].i_comm[30] = '\0';

		idf[i++].i_valid = TRUE;
	}
	maxentries = i;
}

int
readln(fd, buf)
FILE	*fd;
char	*buf;
{
	char	ch;
	int	count;

	count = 0;
	while( ch = fgetc(fd) )
	{
		if( ch == EOF )
		{
			*buf++ = '\0';
			return(count);
		}
		else
		{
			if( ch == '\n' )
			{
				*buf++ = '\0';
				return(++count);
			}
			else
			{
				*buf++ = ch;
				count++;
			}
		}
	}
	*buf++ = '\0';
	return(++count);
}

menu()
{
	int	i;
	char	answer[10];
	char	c;

	system("tput clear");
	printf("\n\n");
	printf("     iNA961             # of      Network\n");
	printf("    Filename     R1-R3  VCs  NMF   Layer  EDL ");
	printf("Comment\n");
	printf("    --------     -----  ---- ---  ------- --- ");
	printf("---------------------------\n");
	for(i=0; i<maxentries; i++)
	{
		printf("%-2d- %-12s %4s %5s %4s %7s %4s %-30s\n",
			i+1,
			idf[i].i_file,
			idf[i].i_r1r3,
			idf[i].i_vc,
			idf[i].i_nmf,
			idf[i].i_nl,
			idf[i].i_edl,
			((idf[i].i_comm[0] == 'L') ? "\0" : idf[i].i_comm));
	}

	if( maxentries == 1 )
		printf("\n\nInput 1 or (q)uit? ");
	else
		printf("\n\nInput 1-%d or (q)uit? ", maxentries);
	gets(answer);
	if( answer[0] == 'q' )
		exit(0);
	else if( answer[0] == '\0' )
	{
		menu();
		exit(0);
	}
	else
	{
		index = ahtoi(answer);
		index--;
		if( index >= maxentries )
		{
			printf("\nInvalid selection!!!\n");
			printf("Press RETURN to continue... ");
			gets(answer);
			menu();
		}
	}
	printf("Selection is %s. OK [y/n/q]? ", idf[index].i_file);
	gets(answer);
	switch( answer[0] )
	{
	case 'q':
		exit(0);
	case 'n':
		menu();
		break;
	case 'y':
		break;
	default:
		menu();
	}
}

validate()
{
	int	i;

	for(i=0; i<maxentries; i++)
	{
		if( strcmp(idf[i].i_file, inafile) == 0 )
			return(i);
		else
			continue;
	}
	return(-1);
}

header(fd)
FILE	*fd;
{
	fprintf(fd, ":\n");
	fprintf(fd, "#\n");
	fprintf(fd, "#	Copyright 1988 Intel Corporation\n");
	fprintf(fd, "#	All Rights Reserved\n");
	fprintf(fd, "#\n");
	fprintf(fd, "# ina:	Download Ethernet Controller with iNA961 Software\n");
	fprintf(fd, "#\n");
	fprintf(fd, "#	The file \"ina\" is invoked from the S08ina961\n");
	fprintf(fd, "#	SV-OpenNET startup script in /etc/rc2.d.\n");
	fprintf(fd, "#\n");
	fprintf(fd, "#\n");
	fprintf(fd, "#ident	\"@(#)ina  $SV_enet SV-Eval01 - 06/25/90$");
	fprintf(fd, "#\n");
}

int
ahtoi(ptr)
char *ptr;
{
	unsigned i = 0;

	while (*ptr != NULL)
	{
		if (isxdigit(*ptr))
		{
			i *= 0x10;
			if( isupper(*ptr) )
				i += *ptr -0x37;
			else if( islower(*ptr) )
				i += *ptr - 0x57;
			else
				i += *ptr - 0x30;
		}
		ptr++;
	}
	return(i);
}

int
getslot(board_type)
char	*board_type;
{
	char	command[80];
	char	board[80];
	char	file[80];
	int	slot;
	FILE	*err_fd;


	sprintf(file, "/usr/tmp/slot%d", getpid());
	for(slot=0; slot<20; slot++)
	{
		sprintf(command, "/usr/sbin/icsrd -s %d 2 10 > %s 2>&1", slot, file);
		system(command);
		if( (err_fd = fopen(file, "r")) == NULL )
			continue;
		fscanf(err_fd, "%s", board);
		fclose(err_fd);
		if ( (strcmp(board, "186/530") == 0) ||
		     (strcmp(board, "MIX386/560") == 0)
		   )
		{
			printf("Found %s in slot #%d\n", board, slot);
			strcpy(board_type, board);
			unlink(file);
			return(slot);
		}
	}
	unlink(file);
	return(-1);
}
