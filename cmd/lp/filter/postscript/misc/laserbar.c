/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/postscript/misc/laserbar.c	1.1.2.1"
/* laserbar -- filter to print barcodes on postscript printer */

#include <stdio.h>
#include <ctype.h>

static int code39[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*	sp    !     "     #     $     %     &     '	*/
	0304, 0,    0,    0,    0250, 0052, 0,    0,   
/*	(     )     *     +     ,     -     -     /	*/
	0,    0,    0224, 0212, 0,    0205, 0604, 0242,
/*	0     1     2     3     4     5     6     7	*/
	0064, 0441, 0141, 0540, 0061, 0460, 0160, 0045,
/*	8     9     :     ;     <     =     >     ?	*/
	0444, 0144, 0,    0,    0,    0,    0,    0,   
/*	@     A     B     C     D     E     F     G	*/
	0,    0411, 0111, 0510, 0031, 0430, 0130, 0015,
/*	H     I     J     K     L     M     N     O	*/
	0414, 0114, 0034, 0403, 0103, 0502, 0023, 0422,
/*	P     Q     R     S     T     U     V     W	*/
	0122, 0007, 0406, 0106, 0026, 0601, 0301, 0700,
/*	X     Y     Z     [     \     ]     ^     _	*/
	0221, 0620, 0320, 0,    0,    0,    0,    0,
/*	`     a     b     c     d     e     f     g	*/
	0,    0411, 0111, 0510, 0031, 0430, 0130, 0015,
/*	h     i     j     k     l     m     n     o	*/
	0414, 0114, 0034, 0403, 0103, 0502, 0023, 0422,
/*	p     q     r     s     t     u     v     w	*/
	0122, 0007, 0406, 0106, 0026, 0601, 0301, 0700,
/*	x     y     z     {     |     }     ~     del	*/
	0221, 0620, 0320, 0,    0,    0,    0,    0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

main()
{
	int c;

	puts("gsave");
	barprt('*');
	while ((c = getchar()) != EOF)
		barprt(c);
	barprt('*');
	puts("grestore");
}

barprt(c)
int c;
{
	int i, mask, bar, wide;

	if (!(i = code39[c]))
		return;
	if (islower(c))
		c = toupper(c);
	if (c != '*') {
		puts("gsave 2 -88 rmoveto");
		puts("/Helvetica findfont 16 scalefont setfont");
		printf("(%c) show grestore\n", c);
	}
	for (bar = 1, mask = 0400; mask; bar = 1 - bar, mask >>= 1) {
		wide = mask & i;
		if (bar) {
			if (wide)
				puts("1 0 rmoveto");
			printf("gsave %d setlinewidth\n", wide ? 3 : 1);
			puts("0 -72 rlineto stroke grestore");
			printf("%d 0 rmoveto\n", wide ? 2 : 1);
		}
		else
			printf("%d 0 rmoveto\n", wide ? 3 : 1);
	}
	puts("1 0 rmoveto");
}



