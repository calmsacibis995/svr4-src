/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:demo/pacman/movie.c	1.1"
/*
 * "Movie" routine for Mark Horton's version of PACMAN which uses the
 *  curses library stuff.  You could probably diddle with it a little
 *  and make it shorter/smaller, however, I haven't seen any document-
 *  ation on the "new" curses stuff (although we have it), so I don't
 *  know what I can get away with.
 *
 *  NOTE: This routine was written for a 24 x 80 screen (adm3a, adm5).
 *        If your screen is different (columns matter, lines don't),
 *	  you will have to change some things.  It shouldn't be very
 *	  difficult, but be warned.
 *
 *  Dave Curry (pur-ee!davy)
 *
 */
#include "pacdefs.h"
#include <curses.h>

/* This is the monster we use.  Don't take out the extra spaces here, 
 * they are there so that the thing "cleans up" after itself, since 
 * I'm too lazy to move all over the place writing out spaces.
 */
char *bigmonster[] = {		
	"   _____    ",	
	"  /     \\   ",
	"  | O O |   ",
	"  v^v^v^v   ",
};

movie()
{
	int i, j, k;

	clear();
	syncscreen();

	/*
	 * this loop moves the monster and the small pacman from right to 
	 * left, until it's time to start printing "fractional" monsters
	 */
	for (i=70; i > (-1); i--) {
		for (j=0; j < 4; j++) {
			SPLOT((j+10), i, bigmonster[j]);
		}
		if (i > 20) {
			SPLOT(13, (i-20), "> ");
		}
		else {
			SPLOT(13, 1, " ");
		}
		syncscreen();
	}

	/*
	 * left1-left7 are "partial" monster routines, they are also called when
	 * the monster comes back on from the left (called in opposite order)
	 */
	left1(); left2(); 
	left3(); left4(); 
	left5(); left6(); 
	left7(); 

	sleep(1);

	/* Now we come back on...  */
	left7(); left6(); 
	left5(); left4(); 
	left3(); left2(); 
	left1(); 

	/*
	 * Now we start moving the big pacman across.  monst1-monst3 are 
	 * routines for drawing pieces of the pacman, until the whole thing
	 * is on the screen.
	 */
	for (i=0; i < 70; i++) {
		for(j=0; j < 4; j++) {
			SPLOT((j+10), i, bigmonster[j]);
		}
		if (i > 20) {
			switch(i) {
			case 21:
				monst1((i-20));
				break;
			case 22:
				monst2((i-20));
				break;
			case 23:
				monst3((i-20));
				break;
			default:
				monst4((i-20));
				break;
			}
		}
		syncscreen();
	}
	/*
	 * right1-right9 are partial monster routines, for moving him off to 
	 * the right of the screen.  monst4 prints the whole pacman.
	 */
	right1(); monst4(50); syncscreen();
	right2(); monst4(51); syncscreen();
	right3(); monst4(52); syncscreen();
	right4(); monst4(53); syncscreen();
	right5(); monst4(54); syncscreen();
	right6(); monst4(55); syncscreen();
	right7(); monst4(56); syncscreen();
	right8(); monst4(57); syncscreen();
	right9(); monst4(58); syncscreen();

	/* Now finish moving the pacman to the end of the screen.  */
	for (i=59; i < 74; i++) {
		monst4(i);
		syncscreen();
	}

	/* monst5-monst7 print pieces of pacman as he moves off the screen */
	monst5(74); syncscreen();
	monst6(75); syncscreen();
	monst7(76); syncscreen();

	/* clean up a little bit */
	clear();
	syncscreen();
}
left1()
{
	SPLOT(10, 0, "_____    \n"); 
	SPLOT(11, 0, "     \\   \n");
	SPLOT(12, 0, " O O |   \n"); 
	SPLOT(13, 0, "^v^v^v   \n");
	syncscreen();
}
left2()
{
	SPLOT(10, 0, "____    \n"); 
	SPLOT(11, 0, "    \\   \n");
	SPLOT(12, 0, "O O |   \n"); 
 	SPLOT(13, 0, "v^v^v   \n");
	syncscreen();
}
left3()
{
	SPLOT(10, 0, "___    \n"); 
	SPLOT(11, 0, "   \\   \n");
	SPLOT(12, 0, " O |   \n"); 
	SPLOT(13, 0, "^v^v   \n");
	syncscreen();
}
left4()
{
	SPLOT(10, 0, "__    \n"); 
	SPLOT(11, 0, "  \\   \n");
	SPLOT(12, 0, "O |   \n"); 
	SPLOT(13, 0, "v^v   \n");
	syncscreen();
}
left5()
{
	SPLOT(10, 0, "_    \n"); 
	SPLOT(11, 0, " \\   \n");
	SPLOT(12, 0, " |   \n"); 
	SPLOT(13, 0, "^v   \n");
	syncscreen();
}
left6()
{
	SPLOT(10, 0, "    \n"); 
	SPLOT(11, 0, "\\   \n");
	SPLOT(12, 0, "|   \n"); 
	SPLOT(13, 0, "v   \n");
	syncscreen();
}
left7()
{
	SPLOT(10, 0, "   \n"); 
	SPLOT(11, 0, "   \n");
	SPLOT(12, 0, "   \n"); 
	SPLOT(13, 0, "   \n");
	syncscreen();
}
right1()
{
	SPLOT(10, 69, "   _____ "); 
	SPLOT(11, 69, "  /     \\");
	SPLOT(12, 69, "  | O O |"); 
	SPLOT(13, 69, "  v^v^v^v");
}
right2()
{
	SPLOT(10, 70, "   _____"); 
	SPLOT(11, 70, "  /     ");
	SPLOT(12, 70, "  | O O "); 
	SPLOT(13, 70, "  v^v^v^");
}
right3()
{
	SPLOT(10, 71, "   ____"); 
	SPLOT(11, 71, "  /    ");
	SPLOT(12, 71, "  | O O"); 
	SPLOT(13, 71, "  v^v^v");
}
right4()
{
	SPLOT(10, 72, "   ___"); 
	SPLOT(11, 72, "  /   ");
	SPLOT(12, 72, "  | O "); 
	SPLOT(13, 72, "  v^v^");
}
right5()
{
	SPLOT(10, 73, "   __"); 
	SPLOT(11, 73, "  /  ");
	SPLOT(12, 73, "  | O"); 
	SPLOT(13, 73, "  v^v");
}
right6()
{
	SPLOT(10, 74, "   _"); 
	SPLOT(11, 74, "  / ");
	SPLOT(12, 74, "  | "); 
	SPLOT(13, 74, "  v^");
}
right7()
{
	SPLOT(10, 75, "   "); 
	SPLOT(11, 75, "  /");
	SPLOT(12, 75, "  |"); 
	SPLOT(13, 75, "  v");
}
right8()
{
	SPLOT(10, 76, " ");
	SPLOT(11, 76, " ");
	SPLOT(12, 76, " ");
	SPLOT(13, 76, " ");
}
right9()
{
	SPLOT(10, 77, " ");
	SPLOT(11, 77, " ");
	SPLOT(12, 77, " ");
	SPLOT(13, 77, " ");
}
monst7(xxx)
int xxx;
{
	SPLOT(10, xxx, "     <");
}
monst6(xxx)
int xxx;
{
	SPLOT( 9, xxx, "     /");
	SPLOT(10, xxx, "    < ");
	SPLOT(11, xxx, "     \\");
}
monst5(xxx)
int xxx;
{
	SPLOT( 8, xxx, "     /");
	SPLOT( 9, xxx, "    / ");
	SPLOT(10, xxx, "   <  ");
	SPLOT(11, xxx, "    \\ ");
	SPLOT(12, xxx, "     \\");
}
monst4(xxx)
int xxx;
{
	SPLOT( 7, xxx, "     /");
	SPLOT( 8, xxx, "    / ");
	SPLOT( 9, xxx, "   /  ");
	SPLOT(10, xxx, "  <   ");
	SPLOT(11, xxx, "   \\   ");
	SPLOT(12, xxx, "    \\  ");
	SPLOT(13, xxx, "     \\ ");
}
monst3(xxx)
int xxx;
{
	SPLOT(7 , xxx, "    / ");
	SPLOT(8 , xxx, "   /  ");
	SPLOT(9 , xxx, "  /   ");
	SPLOT(10, xxx, "      ");
	SPLOT(11, xxx, "  \\   ");
	SPLOT(12, xxx, "   \\  ");
	SPLOT(13, xxx, "    \\ ");
}
monst2(xxx)
int xxx;
{
	SPLOT( 7, xxx, "   / ");
	SPLOT( 8, xxx, "  /  ");
	SPLOT( 9, xxx, "     ");
	SPLOT(10, xxx, "     ");
	SPLOT(11, xxx, "     ");
	SPLOT(12, xxx, "  \\  ");
	SPLOT(13, xxx, "   \\ ");
}
monst1(xxx)
int xxx;
{
	SPLOT( 7, xxx, "  / ");
	SPLOT( 8, xxx, "    ");
	SPLOT( 9, xxx, "    ");
	SPLOT(10, xxx, "    ");
	SPLOT(11, xxx, "    ");
	SPLOT(12, xxx, "    ");
	SPLOT(13, xxx, "  \\ ");
}
