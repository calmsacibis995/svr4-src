/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:setcolor.c	1.3"

/*
**	setcolor -  This is AT&T version of SCO's setcolor using 
**		     the AT&T KD drivers functiona;ity.
*/

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/at_ansi.h>
#include <sys/kd.h>

#define	STIME	3
#define	NUMCOLOR	8
#define NUMMODES	5

struct attr_tst {
	char *seq;
	char *desc;
	char *desc2;
	int attr[2];
};

struct attr_tst foreground[NUMCOLOR] = {
	"[30m[[[[[[[",	"black", "gray", 0x00, 0x08,
	"[31m[[[[[[[",	"red", "lt_red", 0x04, 0x0c,
	"[32m[[[[[[[",	"green", "lt_green", 0x02, 0x0a,
	"[33m[[[[[[[",	"brown", "yellow", 0x06, 0x0e,
	"[34m[[[[[[[",	"blue", "lt_blue", 0x01, 0x09,
	"[35m[[[[[[[",	"magenta", "lt_magenta", 0x05, 0x0d,
	"[36m[[[[[[[",	"cyan", "lt_cyan", 0x03, 0x0b,
	"[37m[[[[[[[",	"white", "lt_white", 0x07, 0x0f,
	};

struct attr_tst background[NUMCOLOR] = {
	"[40m",	"black", "gray", 0x00, 0x80,
	"[41m",	"red", "lt_red", 0x40, 0xc0,
	"[42m",	"green", "lt_green", 0x20, 0xa0, 
	"[43m",	"brown", "yellow", 0x60, 0xe0,
	"[44m",	"blue", "lt_blue", 0x10, 0x90,
	"[45m",	"magenta", "lt_magenta", 0x50, 0xd0,
	"[46m",	"cyan", "lt_cyan", 0x30, 0xb0,
	"[47m",	"white", "lt_white", 0x70, 0xf0,
	};

char scan_attr[2*NUMCOLOR] = {
	0x00,
	0x04,
	0x02,
	0x14,
	0x01,
	0x05,
	0x03,
	0x07,
	0x38,
	0x3C,
	0x3A,
	0x36,
	0x39,
	0x3D,
	0x3B,
	0x3F
	};

char *normreset = "[0m";
char *normmode = "[39m";
char *boldreset = "[1m";
char *blinkreset = "[5m";
char *in_extend = "[12m";
char *out_extend = "[10m";

char escseq[2];
FILE *kdfd;

char *Usage="setcolor -[nbrgopc] [argument] [argument]";

main(argc, argv)
int	argc;
char	**argv;
{
	int i, err_flag, implemented, type;
	int index, index1, attri, attri1, bld_attr, pres_attr, cnt;
	int row, col;
	struct attr_tst p_attr, p_attr1;
	char *nullptr;
	extern char *optarg;
	extern int optind;   

	implemented = err_flag = 0;

	if ( ((type = ioctl(0, KIOCINFO, 0)) != (('k' << 8) | 'd')) &&
		(( type < (('s'<< 8) | 0x00)) && ( type > (('s' << 8) | 0x20)))) {
		printf(stderr,"setcolor: Not Supported this Device\n");
		exit(1);
	}

	kdfd = stdout;

	escseq[0] = '\033';
	escseq[1] = '\0';

	nullptr = 0;
	if( argv[1] == nullptr ) {
		Us_age22();
		exit(0);
	}

	pres_attr = ioctl(0, GIO_ATTR, 0);

	if( *argv[1] == '-' ) {

	while(( i = getopt(argc, argv, "nb:o:r:c:p:" )) != EOF )
		switch(i) {
			/* Set screen to Normal white char/Black back */
			case 'n':  
				implemented++;
				fprintf(kdfd,"%s%s",escseq,normmode);
				fprintf(kdfd,"%s%s",escseq,normreset);
				break;
			case 'o':
				implemented++;
				clk_pt(optarg, &attri, 1, &index);
				if ( index < 0 ) {
					fprintf(stderr,"setcolors: Invalid color name\n");
					err_flag++;
					break;
				}
				if ( attri )
					index += 8;
				cnt = scan_attr[index];
				if( ioctl(1, KDSBORDER, cnt) >= 0 )
					fflush(stdout);
				break;
			case 'b':
				implemented++;
				clk_pt(optarg, &attri, 1, &index);
				if ( index < 0 ) {
					fprintf(stderr,"setcolors: Invalid color name\n");
					err_flag++;
					break;
				}
				clk_clr(pres_attr, foreground, 0, &row, &col);
				if( ( row == index ) && ( col == attri )) {
				  printf("Same foreground and back colors\n");
					exit(1);
				}
				fprintf(kdfd,"%s%s",escseq,normmode);
				fprintf(kdfd,"%s%s",escseq,normreset);
				if ( col )
					fprintf(kdfd,"%s%s",escseq,boldreset);
				p_attr = foreground[ row ];
				fprintf(kdfd,"%s%-4.4s",escseq,p_attr.seq);
				p_attr = background[index];
				if ( attri )
					fprintf(kdfd,"%s%s",escseq,blinkreset);
				fprintf(kdfd,"%s%s",escseq,p_attr.seq);

				break;
			/*
			 These  SCO  Options are not presently supported  i
			 in this command but should report sucessful completion
			 anyway.  We just return 0.
			*/
			case 'c':
			case 'p':
			case 'g':
			case 'r':
				clk_pt(optarg, &attri, 1, &index);
				if ( index < 0 ) {
					fprintf(stderr,"setcolors: Invalid color name\n");
					err_flag++;
				}
				break;
			case '?':
				err_flag++;
		}
		if(err_flag) {
			fprintf(stderr,"setcolors: %s\n",Usage);
			exit(1);
		}

		exit(0);
	
		} 
	else {
		clk_pt(argv[1], &attri, 0, &index);
		if ( index < 0 ) {
			fprintf(stderr,"setcolors: Invalid color name\n");
			exit(1);
		}
		p_attr = foreground[index];
		if( argv[2] != nullptr ) {
			clk_pt(argv[2], &attri1, 1, &index1);
			if ( index < 0 ) {
				printf("%s: Invalid color name\n",argv[0]);
				exit(1);
			}
			p_attr1 = background[index1];
		} 
		else 
		{
			clk_clr(pres_attr, background, 1, &index1, &attri1);
			p_attr1 = background [ index1 ];
		}
		if ( (index == index1 ) && ( attri == attri1 )) {
			printf("Same foreground and back colors\n");
			exit(1);
		}
		fprintf(kdfd,"%s%s",escseq,normmode);
		fprintf(kdfd,"%s%s",escseq,normreset);
		if ( attri )
			fprintf(kdfd,"%s%s",escseq,boldreset);
		fprintf(kdfd,"%s%-4.4s",escseq,p_attr.seq);
		if ( attri1 )
			fprintf(kdfd,"%s%s",escseq,blinkreset);
		fprintf(kdfd,"%s%s",escseq,p_attr1.seq);
	}
}

/* This routine takes an present attribute the foreground or background 
   arrray and a forground or backgreound flag and reutrns the roe &
   col of the color.
 */

clk_clr(prt_attr, array, flag, row, col)
char prt_attr;
struct attr_tst *array;
int flag, *row, *col;
{
	int attr, i, j;

	*row = *col = -1;

	if(flag == 0 )
	    attr = prt_attr & 0x0f;
	else
	    attr = prt_attr & 0xf0;

	for ( i = 0; i < 8; i++ )
		for( j = 0; j < 2 ; j++)
			if ( array[i].attr[j] == attr )
			{
				*row = i;
				*col = j;
				return;
			}
}

/* This routine will search the foreground structure for valid colors.
   It will return the approiat foreground/backgroud seq ptr and att ptr
*/


clk_pt(poptopt, attri, fb, index)
char *poptopt;
int fb, *index, *attri;
{
	int i, j;
	struct attr_tst *attrtpt;

	i = *index = *attri = -1;

	for( j=0; j < NUMCOLOR; j++) {
		if( !( strncmp(poptopt,foreground[j].desc,strlen(foreground[j].desc)) ) ) {
			i++;
			*index = j;
			*attri = 0;
			break;
		}
	}
	for( j=0; i < 0 && j < NUMCOLOR; j++) {
		if( !(strncmp(poptopt,foreground[j].desc2,strlen(foreground[j].desc2)) ) ) {
			*index = j;
			*attri = 1;
			break;
		}
	}
}

Us_age22()
{
	struct attr_tst att;
		
	printf("%s%s",escseq,normmode);
	printf("%s%s",escseq,normreset);
	fflush(stdout);

	printf("Usage  :   %s\n",Usage);
	printf("Options: -n\t\t  Normal white foreground and black background\n");
	printf("\t color [color]\t  Set foreground and optional background \n");
	printf("\t -b color\t  Set background \n");
	printf("\t -r color color\t  Set foreground & background reverse video colors\n");
	printf("\t -g color color\t  Set foreground & background graphic box colors\n");
	printf("\t -o color\t  Set border color\n");
	printf("\t -c first last\t  Set cursor size\n");
	printf("\t -p pitch dur\t  Set pitch and duration of bell\n\n\n");
	printf("Colors and their names used for [color] options:\n\n");

	printf("%s%s%s%-10.10s%s%s%s%s %-11.10s",escseq,in_extend,escseq,
	foreground[4].seq,escseq,out_extend,escseq,normreset,foreground[4].desc);
	printf("  %s%s%s%-10.10s%s%s%s%s %-11.10s",escseq,in_extend,escseq,
	foreground[5].seq,escseq,out_extend,escseq,normreset,foreground[5].desc);
	printf("  %s%s%s%-10.10s%s%s%s%s %-11.10s",escseq,in_extend,escseq,
	foreground[3].seq,escseq,out_extend,escseq,normreset,foreground[3].desc);
	printf("  [%s%s%s%-8.8s%s%s%s%s] %-11.10s\n\n",escseq,in_extend,escseq,
	foreground[0].seq,escseq,out_extend,escseq,normreset,foreground[0].desc);
	printf("%s%s",escseq,normreset);

	printf("%s[1m%s%s%s%-10.10s%s%s%s%s %-11.10s",escseq,escseq,in_extend,escseq,
	foreground[4].seq,escseq,out_extend,escseq,normreset,foreground[4].desc2);
	printf("  %s[1m%s%s%s%-10.10s%s%s%s%s %-11.10s",escseq,escseq,in_extend,escseq,
	foreground[5].seq,escseq,out_extend,escseq,normreset,foreground[5].desc2);
	printf("  %s[1m%s%s%s%-10.10s%s%s%s%s %-11.10s",escseq,escseq,in_extend,escseq,
	foreground[3].seq,escseq,out_extend,escseq,normreset,foreground[3].desc2);
	printf("  %s[1m%s%s%s%-10.10s%s%s%s%s %-11.10s\n\n",escseq,escseq,in_extend,escseq,
	foreground[0].seq,escseq,out_extend,escseq,normreset,foreground[0].desc2);
	printf(kdfd,"%s%s",escseq,normreset);
	printf("%s%s%s%-10.10s%s%s%s%s %-11.10s",escseq,in_extend,escseq,
	foreground[6].seq,escseq,out_extend,escseq,normreset,foreground[6].desc);
	printf("  %s%s%s%-10.10s%s%s%s%s %-11.10s",escseq,in_extend,escseq,
	foreground[7].seq,escseq,out_extend,escseq,normreset,foreground[7].desc);
	printf("  %s%s%s%-10.10s%s%s%s%s %-11.10s",escseq,in_extend,escseq,
	foreground[2].seq,escseq,out_extend,escseq,normreset,foreground[2].desc);
	printf("  %s%s%s%-10.10s%s%s%s%s %-11.10s\n\n",escseq,in_extend,escseq,
	foreground[1].seq,escseq,out_extend,escseq,normreset,foreground[1].desc);
	printf(kdfd,"  %s%s",escseq,normreset);
	printf("%s[1m%s%s%s%-10.10s%s%s%s%s %-11.10s",escseq,escseq,in_extend,escseq,
	foreground[6].seq,escseq,out_extend,escseq,normreset,foreground[6].desc2);
	printf("  %s[1m%s%s%s%-10.10s%s%s%s%s %-11.10s",escseq,escseq,in_extend,escseq,
	foreground[7].seq,escseq,out_extend,escseq,normreset,foreground[7].desc2);
	printf("  %s[1m%s%s%s%-10.10s%s%s%s%s %-11.10s",escseq,escseq,in_extend,escseq,
	foreground[2].seq,escseq,out_extend,escseq,normreset,foreground[2].desc2);
	printf("  %s[1m%s%s%s%-10.10s%s%s%s%s %-11.10s\n\n",escseq,escseq,in_extend,escseq,
	foreground[1].seq,escseq,out_extend,escseq,normreset,foreground[1].desc2);
}
