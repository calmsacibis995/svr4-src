/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)misc:jwin.c	2.4.2.1"

#include <stdio.h>
#include <sys/jioctl.h>

#define BYTE(x)		(x & 0xff)

struct jwinsize win;
main(argc,argv)
char *argv[];
{
	if (argc != 1) {
		fprintf(stderr,"usage: jwin\n");
		exit(1);
	}

	if (ioctl(0, JMPX, 0) == -1) {
		fprintf(stderr,"jwin: runs only under layers\n");
		exit(1);
	} else {

		/* The size in bits indicates the size in pixels 	     */
		/* If the window is m rows x n cols, the size in pixels is : */ 
		/* m * font_height + 34	pixels high and 		     */
		/* n * font_width + 29 pixels wide 			     */

		/* The font sizes for the various fonts are as follows :     */
		/* Large font  : 11 pixels wide x 16 pixels high	     */
		/* Medium font : 9 pixels wide x 14 pixels high		     */
		/* Small font  : 7 pixels wide x 14 pixels high		     */

		/* Currently an extra 8 pixels are getting added for pixel sz*/

 
		ioctl(0, JWINSIZE, &win);
		printf("bytes:\t%d %d\n", BYTE(win.bytesx), BYTE(win.bytesy));
		if (win.bitsx != 0 || win.bitsy != 0)
			printf("bits:\t%d %d\n", win.bitsx, win.bitsy);
		exit(0);
	}
}
