/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/5310.c	1.1.3.1"

#include <fcntl.h>

/****************************************************************************
 *
 *  Teletype Model 53 Printer Filter		Andy R. Rolfe TTY 3/18/85
 *
 *  Without the -m40 option, this filter replaces any occurrence of
 *	char-BS-char with the 5310/20 emphasize escape sequence;
 *	and any occurrence of UL-BS-char with char-BS-UL so the
 *	5310/20 can print it in one stroke.
 *
 *  With the -m40 option, this filter replaces any occurrence of the
 *	sequence "ESC \ text ESC Z" with every character of the text
 *	sent as char-BS-UL;
 *
 *	and replaces "ESC 3" & "ESC 4" with 5310/20's emphasize-on,
 *	emphasize-off escape sequences respectively.
 *
 *  If used with the -d or -dmdp option two ESC characters are sent 
 *	instead of one so that it can be used with dmdp on the 5620
 * 	which throws away all single ESC's received on-line.
 *
 ***************************************************************************/

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define UL '_'
#define ESC '\033'
#define BS '\010'

#define RD(s,c) if((t=rd(s,c))!=c){if(t<0)perror(argv[0]);goto GT3;}
#define WR(s,c) if(wr(STDOUT,(char *)s,(unsigned)c)<c){perror("STDOUT");exit(1);}
#define EMPH_ON() WR("\033[5m",4)emph_flg = 1
#define EMPH_OFF() WR("\033[0m",4)emph_flg = 0

unsigned char emph_flg, ul_flg, dflg, bufr[3];

main(argc, argv)
int argc;
char **argv;
{
	extern void perror(), exit();

	unsigned char tmp, m40 = 0;
	int t;						/* For RD() Macro */

	if(--argc > 0 && argv[1][0] == '-' && argv[1][1] == 'm'
		    && argv[1][2] == '4' && argv[1][3] == '0'
		    && argv[1][4] == 0)
	{
		m40++;
		argc--;argv++;
	}
	if(argc > 0 && argv[1][0] == '-' && argv[1][1] == 'd' &&
		(argv[1][2] == 0 || (argv[1][2] == 'm' && argv[1][3] == 'd' &&
		 argv[1][4] == 'p' && argv[1][5] == 0)))
	{
		dflg++;argc--;argv++;
	}
	do
	{
		if(argc != 0)
		{
			(void)close(STDIN);
			if(open(argv[1], O_RDONLY) < 0)
			{
				perror(argv[1]);
				argc--;argv++;
				continue;
			}
			argc--;argv++;
		}
		RD(0,2)
		if(m40)
		{
			while(1)
			{
				if((bufr[0]&0x7f) == ESC)
				{
					switch(bufr[1]&0x7f)
					{
						case '\\':
							ul_flg++;
							break;
						case 'Z':
							ul_flg = 0;
							break;
						case '3':
							if(emph_flg == 0)
							{	EMPH_ON(); }
							break;
						case '4':
							if(emph_flg == 1)
							{	EMPH_OFF(); }
							break;
						default:
							WR(&bufr[0],2)
					}
					RD(0,2)
					continue;
				}
				WR(&bufr[0],1)
				if(ul_flg && ((bufr[0]&0x7f) >= 0x20 && (bufr[0]&0x7f) <= 0x7e))
					WR("\010_",2)
				bufr[0] = bufr[1];
				RD(1,1)
			}
		}
		else
		{
			while(1)
			{
				RD(2,1)
				if((bufr[1]&0x7f) == BS)
				{
					if(bufr[0] == bufr[2] && ((bufr[0]&0x7f) > 0x20 && (bufr[0]&0x7f) <= 0x7e))
					{
						EMPH_ON();
GT1:						WR(&bufr[2],1)
						do
						{
							RD(0,2)
						}while((bufr[0]&0x7f) == BS && bufr[1] == bufr[2]);
						if((bufr[1]&0x7f) == BS)
						{
							RD(2,1)
							if(bufr[0] != bufr[2])
							{
								EMPH_OFF();
								goto GT2;
							}
							goto GT1;
						}
						EMPH_OFF();
					}
					else
GT2:						if((bufr[0]&0x7f) == UL && ((bufr[2]&0x7f) >= 0x20 && (bufr[2]&0x7f) <= 0x7e))
						{
							tmp = bufr[0];
							bufr[0] = bufr[2];
							bufr[2] = tmp;
							WR(&bufr[0],3)
							RD(0,2)
						}
						else
						{
							WR(&bufr[0],2)
							bufr[0] = bufr[2];
							RD(1,1)
						}
					continue;
				}
				WR(&bufr[0],1)
				bufr[0] = bufr[1];
				bufr[1] = bufr[2];
			}
		}
GT3:		continue;
	} while(argc > 0);
	exit(0);
}

rd(start, count)
{

	register i,j;

	if(( i = read(STDIN,(char *)&bufr[start],(unsigned)count)) != count)
	{
		if(i < 0) j = 0; else j = i;
		if(ul_flg && (bufr[0] >= 0x20 && bufr[0] <= 0x7e))
		{
			WR(&bufr[0],1)
			WR("\010_",2)
		}
		else
			WR(&bufr[0],start+j)
		if(emph_flg)
		{	EMPH_OFF(); }
	}
	return(i);
}

wr(fp, string, count)
register char *string;
unsigned count;
{

	register char *op;
	char outstr[32];
	register i;

	if(dflg)
	{
		op = &outstr[0];
		i = count;
		while(i--)
			if((*op++ = *string++) == ESC)
			{	*op++ = ESC; count++;	}
		string = &outstr[0];
	}
	return( write(fp, string, count) );

}
