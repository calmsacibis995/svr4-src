/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)evgainit:evgainit.c	1.4"

#include <stdio.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/kd.h>

struct evga_type {
	char		*string;
	unsigned long 	value;
} evga_types[] = {
	"vga", 		EVGA_VGA,
	"vega",		EVGA_VEGA,
	"stbga",	EVGA_STBGA,
	"sigma/h",	EVGA_SIGMAH,
	"pvga1a",	EVGA_PVGA1A,
	"dell",		EVGA_DELL,
	"vram",		EVGA_VRAM,
	"orvga",	EVGA_ORVGA,
	"orvgani",	EVGA_ORVGAni,
	"tvga",		EVGA_TVGA,
	"tvgani",	EVGA_TVGAni,
	"gvga",		EVGA_GVGA,
	"ega",		EVGA_EGA,
	"pega",		EVGA_PEGA,
	"gega",		EVGA_GEGA,
	"fastwrite",	EVGA_FASTWRITE,
	"won",		EVGA_WON,
	"pvga1024",	EVGA_PVGA1024,
	NULL,		(unsigned)-1L
};

char *EVGAINIT = "evgainit";
char *usage = "\nUSAGE:\n        evgainit type\n        evgainit -help\n";
char *head = "\ntype\t\tdescription\n-----\t\t-----------\n";
struct evga_help {
	char 	*type;
	char	*desc;
} evga_helps[] = {
	"dell     ",	"DELL VGA",
	"fastwrite",	"Video 7 FastWrite VGA",
	"gega     ",	"Genoa Super EGA Hi Res",
	"gvga     ",	"Genoa Super VGA 5200, 5300, 5400",
	"orvga    ",	"Orchid Designer, ProDesigner VGA",
	"orvgani  ",	"Orchid ProDesigner VGA (non-interlaced)",
	"pega     ",	"Paradise PEGA2",
	"pvga1024 ",	"Paradise VGA 1024",
	"pvga1a   ",	"Paradise VGA Plus, Plus 16, Professional",
	"sigma/h  ",	"SIGMA VGA/H",
	"stbga    ",	"STB VGA Extra/EM, Extra/EM-16, Extra/EM-16+",
	"tvga     ",	"Techmar VGA, VGA AD",
	"tvgani   ",	"Techmar VGA AD (non-interlaced)",
	"vega     ",	"Video 7 VEGA VGA",
	"vram     ",	"Video 7 VRAM VGA",
	"won      ",	"ATI VGA Wonder (v3)",
	NULL,		NULL
};


extern int errno;

main(argc, argv)
int 	argc;
char * 	argv[];
{
	int matched, i, rtn, fd;

	if (argc != 2) {
		fprintf(stderr, "%s", usage);
		exit(1);
	}
	if ((argv[1][0] == '-') && (argv[1][1] == 'h')) {
		fprintf(stdout, "%s", head);
		i = 0;
		while (evga_helps[i].type != NULL) {
			fprintf(stdout, "%s\t%s\n", evga_helps[i].type,
				evga_helps[i].desc);
			i++;
		}
		exit(0);
	}
	if (geteuid() != 0) {
		fprintf(stderr, "evgainit: not super user\n");
		exit(1);
	}
	matched = 0;
	i = 0;
    	while (evga_types[i].string != NULL) {
		if (strcmp(evga_types[i].string, argv[1]) == 0) {
			matched = 1;
			break;
		}
		i++;
	}

	if (matched) {
		if ((fd = open("/dev/kd/kdvm00", O_RDWR)) == -1) {
		    fprintf(stderr, "evgainit: can't open /dev/kd/kdvm00\n");
		    exit(1);
		} 
		rtn = ioctl(fd, KDEVGA, &evga_types[i].value);
		if (rtn != 0) {
			perror(EVGAINIT);
			exit(1);
		}
	}
	else	{
		fprintf(stderr, "%s", usage);
		exit(1);
	}
	exit(0);
}
