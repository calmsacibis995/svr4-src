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
#ident	"@(#)mbus:cmd/enet/enetinfo.c	1.3.3.1"

static char *rcsid ="@(#)enetinfo  $SV_enet SV-Eval01 - 06/25/90$";

static char enetinfo_copyright[] = "Copyright 1987, 1988, 1989 Intel Corp. 464225";

#include <stdio.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/stropts.h>
#include <sys/stream.h>

struct i552info {			/* I000 */
	ulong	base;
	ulong	port;
	ushort  slot;
	char	board_type[11];
	unsigned char	fwv;
	unsigned char	inav;		/* ina961 release version */
	unsigned char	numvc;		/* number of vc's configured */
	unsigned char	eaddr[6];
} *info_p;

#define	MAXNBRDS	4
#define	BS		(sizeof (struct i552info)) * MAXNBRDS
#define	I552INFO	5;

/* i552info options */
#define	NUMBRD		1
#define LOADADDR	2
#define	NUMVCS		4
#define	PORTNUM		8
#define	BRDTYPE		0x10
#define	VERNUM		0x20
#define	RELNUM		0x40
#define	EADDR		0x80
#define	ALL		0xff

unsigned short	options;
char	version;

extern	int		errno;
extern	int		optind;

char			i552dev[] = "/dev/iso-tp4";
char			strbuf[BS];
struct	strioctl	strioctl;


/*
 *
 */
main(argc,argv)
int	argc;
char	*argv[];
{
	int		strfd, err;
	int		num_boards, i;
	char		o;
	if ((strfd = open (i552dev, O_RDWR)) == -1)
	{
		perror("enetinfo device open error");
		exit(errno);
	}
	strioctl.ic_cmd = I552INFO;
	strioctl.ic_timout =  4;
	strioctl.ic_len = sizeof(struct strioctl) + sizeof(strbuf);
	strioctl.ic_dp = strbuf;

	if ((ioctl(strfd, I_STR, &strioctl)) == -1)
	{
		perror("enetinfo ioctl error");
		exit(errno);
	}

	options = EADDR;	/* default option */
	optind = 1;
	while ((o = getopt(argc,argv,"aeblitnhvr")) != EOF)
		switch(o)
		{
		case 'a':
			options = ALL;
			break;

		case 'b':
			options |= NUMBRD;	/* number of boards */
			options &= ~(EADDR);	/* reset EADDR option */
			break;

		case 'l':
			options |= LOADADDR;	/* load address */
			options &= ~(EADDR);	/* reset EADDR option */
			break;

		case 'i':
			options |= PORTNUM;	/* port number */
			options &= ~(EADDR);	/* reset EADDR option */
			break;

		case 't':
			options |= BRDTYPE;	/* board type */
			options &= ~(EADDR);	/* reset EADDR option */
			break;

		case 'n':
			options |= NUMVCS;	/* number of vcs */

			options &= ~(EADDR);	/* reset EADDR option */
			break;

		case 'v':
			options |= VERNUM;	/* 552 firmware version number */
			options &= ~(EADDR);	/* reset EADDR option */
			break;

		case 'r':
			options |= RELNUM;	/* iNA release number */
			options &= ~(EADDR);	/* reset EADDR option */
			break;

		case '?':
		case 'h':
			usage();
			break;

		case 'e':
		default:
			options |= EADDR;	/* Ethernet address */
		}
	num_boards = strioctl.ic_len / (sizeof(struct i552info));
	info_p = (struct i552info *)strbuf;


	printf("\n");
	if ((options & NUMBRD) != 0)
		printf ("Number of COMM boards configured is . . . . . . . %d\n",
		    num_boards);

	for (i = 0; i < num_boards; i++)
	{
		/*		printf("\n");
		printf("COMM board #%d :\n", i);
		printf("\n");
		*/
		disp_cfg(info_p + i);
	}

}



/*
* etheraddr - print ETHERNET address for local 552 board
*/
etheraddr(dr_addr)
char	*dr_addr;
{
	int i, valid_flag, fid, err;
	int c;
	unsigned utemp1;
	char Ether_addr[13], final_addr[13];


	for (i=0; i < sizeof(Ether_addr); i++)
		Ether_addr[i] = '\0';

	utemp1 = (dr_addr[0] & 0377) << 8 | (dr_addr[1] & 0377);
	utos(utemp1, (char *)&(Ether_addr[0]));
	utemp1 = (dr_addr[2] & 0377) << 8 | (dr_addr[3] & 0377);
	utos(utemp1, (char *)&(Ether_addr[4]));
	utemp1 = (dr_addr[4] & 0377) << 8 | (dr_addr[5] & 0377);
	utos(utemp1, (char *)&(Ether_addr[8]));
	for (i=0; Ether_addr[i] != '\0'; final_addr[i] = toupper(Ether_addr[i]), i++)
		;
	final_addr[i] = 0;
	printf (" %s\n\n",final_addr);
}


/*
* routine to convert an unsigned integer to a character
* called by utos
*/

char
utoc(u)
char u;
{
	char c;

	switch(u)
	{
	case 0:
		c = '0';
		break;
	case 1:
		c = '1';
		break;
	case 2:
		c = '2';
		break;
	case 3:
		c = '3';
		break;
	case 4:
		c = '4';
		break;
	case 5:
		c = '5';
		break;
	case 6:
		c = '6';
		break;
	case 7:
		c = '7';
		break;
	case 8:
		c = '8';
		break;
	case 9:
		c = '9';
		break;
	case 10:
		c = 'a';
		break;
	case 11:
		c = 'b';
		break;
	case 12:
		c = 'c';
		break;
	case 13:
		c = 'd';
		break;
	case 14:
		c = 'e';
		break;
	case 15:
		c = 'f';
		break;
	default:
		c = 'z';
	}
	return (c);
}



/*
* routine to convert an unsigned integer to a string
*
*/

utos (u, s)
unsigned u;
char	*s;
{
	unsigned t;
	char	   c;


	t   = u / (16 * 16 * 16);
	c   = utoc(t);
	u   = u - t * (16 * 16 * 16);
	*s++ = c;


	t   = u / (16 * 16);
	c   = utoc(t);
	u   = u - t * (16 * 16);
	*s++ = c;


	t   = u / (16);
	c   = utoc(t);
	u   = u - t * 16;
	*s++ = c;

	c   = utoc(u);
	*s = c;
}

/*
 * display_cfg() 
 */
disp_cfg(ec_p)
struct	i552info	*ec_p;
{
	char		board_type[30];
	char		ver_str[4];
	char		ver_tmp[4];
	char		release[20];

	/* I001 begin */
	if (strlen (ec_p->board_type) !=0)
		sprintf (board_type, "%-10s", ec_p->board_type);
	else if ( ec_p->fwv == 0x30 )
		sprintf (board_type, "%s", "iSXM552   ");
	else if ( ec_p->fwv == 0x40 )
		sprintf (board_type, "%s", "iSXM552A  ");
	else
		sprintf (board_type, "%s", "UNKNOWN   ");
	/* I001 end */

	if ( ec_p->inav == 20 )
		sprintf (release, "%s", "R2.0 ");
	else if ( ec_p->inav == 10 )
		sprintf (release, "%s", "R1.3 ");
	else if ( ec_p->inav == 30 )
		sprintf (release, "%s", "R3.0 ");	/* I000 */
	else if ( ec_p->inav == 31 )
		sprintf (release, "%s", "R3.1 ");
	else
		sprintf (release, "%s", "iNA not running ");

	if ((options & 0x10) != 0)
		printf ("Board type is . . . . . . . . . . . . . . . . . . %s\n", board_type);

	if ((options & LOADADDR) != 0)
	{
		if (ec_p->base == 0xffffffff)
		{
			printf ("%s base load address is . . . . . . . . . N/A\n", board_type);
			printf ("%s slot number is . . . . . . . . . . . . %x\n", board_type, ec_p->slot);
		}
		else
		{
			printf ("%s base load address is . . . . . . . . . 0x%x\n",
			    board_type,  (ec_p->base & 0xffff));
			printf ("%s slot number is . . . . . . . . . . . . N/A\n", board_type);
		}
	}
	if ((options & 4) != 0)
		printf ("Number VC's configured for the %s is. . . %d\n",
		    board_type, ec_p->numvc);

	if ((options & 8) != 0)
	{
		if (ec_p->port == 0xffffffff)
			printf ("%s I/O port address is. . . . . . . . . . N/A\n", board_type);
		else
			printf ("%s I/O port address is. . . . . . . . . . 0x%x\n",
			    board_type, ec_p->port);
	}

	if ((options & 0x20) != 0)
	{
		sprintf(ver_tmp,"%x",ec_p->fwv);
		ver_str[0] = ver_tmp[0];
		ver_str[1] = '.';
		ver_str[2] = ver_tmp[1];
		ver_str[3] = '\0';
		printf ("%s firmware version number is . . . . . . V%s\n",
		    board_type, ver_str);
	}
	if ((options & 0x40) != 0)
		printf ("iNA961 release number is. . . . . . . . . . . . . %s\n", release);

	if ((options & 0x80) != 0)
	{
		printf ("%s ethernet address is. . . . . . . . . .", board_type);
		etheraddr(ec_p->eaddr);
	}
}

usage()
{
	fprintf(stderr,"enetinfo: Usage: enetinfo -[options]\n");
	fprintf(stderr,"\nWhere the options are:\n\n");
	fprintf(stderr,"-a  Prints all information.\n");
	fprintf(stderr,"-b  Prints # of Ethernet boards configured in the system.\n");
	fprintf(stderr,"-e  Prints the 12 character ethernet address.\n");
	fprintf(stderr,"-h  Prints this help message.\n");
	fprintf(stderr,"-i  Prints the I/O port used by the Ethernet board.\n");
	fprintf(stderr,"-l  Prints the load address used to load iNA961 on the Ethernet board.\n");
	fprintf(stderr,"-n  Prints the maximum number of Virtual Circuits the Ethernet driver\n");
	fprintf(stderr,"    currently supports.\n");
	fprintf(stderr,"-r  Prints the release number of the iNA961.\n");
	fprintf(stderr,"-t  Prints the type of board installed (e.g., iSXM552A, 186/530 or PCL2NIA).\n");
	fprintf(stderr,"-v  Prints the firmware version number of the Ethernet board.\n\n");
	exit(1);
}
