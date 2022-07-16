/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:demo/pacman/pacman.c	1.2"
/*
 * PACMAN  - written by Dave Nixon, AGS Computers Inc., July, 1981.
 * Converted for curses Feb 1982 by Mark Horton.
 *
 * Terminal handling for video games taken from aliens
 *      the original version  of aliens is from 
 *      Fall 1979                      Cambridge               Jude Miller
 *
 * Score keeping modified and general terminal handling (termcap routines
 * from UCB's ex) added by Rob Coben, BTL, June, 1980.
 */
#include <stdio.h>
#include "pacdefs.h"
 
/*
 * global variables
 */

extern char
	message[];

extern char
	initbrd[BRDY][BRDX],
	display[BRDY][BRDX];

extern unsigned
	pscore;

extern struct pac
	monst[];

extern char monst_names[];

int	pacsymb = PACMAN,
	rounds,		/* time keeping mechanism */
	killflg,
	delay,
	potion,
	goldcnt,		/* no. of gold pieces remaining */
	game,
	monst_often,
	monsthere,
	boardcount = 1,
	wmonst,
	potintvl = POTINTVL,
	potioncnt,
	bcount = 0,
	/* attrs: normal monster, edible monster, pacman */
	mflash, rflash, pflash,
	showcount,
	treascnt = 0;
extern
	char *full_names[];

struct pac
	pac;

struct pac
	pacstart =
{
	PSTARTX,
	PSTARTY,
	DNULL,
	SLOW,
	FALSE
};

struct pac
	*pacptr = &pac;

#define DEFCHPERTURN 60

main(argc, argv)
char **argv;
{
	register int tmp;		/* temp variables */
	register int pac_cnt;
	register int monstcnt;	/* monster number */
	int tries;
	long denom;
	struct pac *mptr;
	char gcnt[10];
	char msgbuf[50];
	int c;
	extern char *optarg;
	int chperturn = DEFCHPERTURN;

	game = 0;

	pflash = A_BLINK | COLOR_PAIR (5);
	mflash = A_BOLD;
	rflash = A_REVERSE;

	while ((c = getopt(argc, argv, "cemhpn:")) != EOF)
		switch(c) {
		case 'c':
			showcount = 1;
			break;
		case 'e':
			game = 1;
			break;
		case 'm':
			game = 2;
			break;
		case 'h':
			game = 3;
			break;
		case 'p':
			mflash = pflash = rflash = 0;
			break;
		case 'n':
			chperturn = atoi(optarg);
			break;
		default:
			fprintf(stderr, "Usage: pacman -emh -p -n#\n");
			exit(1);
		}
	
	init();		/* global init */
	for (pac_cnt = MAXPAC; pac_cnt > 0; pac_cnt--)
	{
redraw:
		erase();
		potioncnt = 0;
		treascnt = 0;
		bcount = 0;
		potion = FALSE;
		SPLOT(0, 45, "SCORE: ");
		(void) sprintf(msgbuf, "GAME: %s",	game==1 ? "EASY" :
							game==2 ? "MEDIUM" :
								  "HARD");
		SPLOT(0, 65, msgbuf);
		SPLOT(21, 45, "food left = ");
		(void) sprintf(gcnt, "%6d", goldcnt);
		SPLOT(21, 57, gcnt);

		/*
		 * We update the monsters every monst_often turns, to keep
		 * the CRT caught up with the computer.  The fudge factor
		 * was calculated from the assumption that each full syncscreen
		 * outputs chperturn characters.  The default is pessimistic
		 * based on ANSI and HP terminals w/verbose cursor addressing.
		 */
		denom = ((long) delay) * baudrate();
		monst_often = (chperturn * 10000L + denom - 1) / denom;
		if (monst_often < 1)
			monst_often = 1;

		if (potion == TRUE)
		{
			SPLOT(3, 45, "COUNTDOWN: ");
			(void) sprintf(message, "%2d", potioncnt);
			SPLOT(3, 60, message);
		};
		pacsymb = PACMAN;
		killflg = FALSE;
		(void) sprintf(message,
			"delay = %3d, syncscreen = %3d", delay, monst_often);
		SPLOT(22, 45, message);
		/*
		 * PLOT maze
		 */
		for (tmp = 0; tmp < BRDY; tmp++)
		{
			SPLOT(tmp, 0, &(display[tmp][0]));
		};
		/* initialize a pacman */
		pac = pacstart;
		PLOT(pacptr->ypos, pacptr->xpos, pacsymb | pflash);
		/* display remaining pacmen */
		for (tmp = 0; tmp < pac_cnt - 1; tmp++)
		{
			PLOT(LINES >=24 ? 23 : 22, (MAXPAC * tmp), PACMAN);
		};
		/*
		 * Init. monsters
	 	 */
		for (mptr = &monst[0], monstcnt = 0; monstcnt < MAXMONSTER; mptr++, monstcnt++)
		{
			mptr->xpos = MSTARTX + (2 * monstcnt);
			mptr->ypos = MSTARTY;
			mptr->speed = SLOW;
			mptr->dirn = DNULL;
			mptr->danger = TRUE;
			mptr->stat = START;
			PLOT(mptr->ypos, mptr->xpos, monst_names[monstcnt] | COLOR_PAIR(monstcnt+1));
			mptr->xdpos = mptr->xpos;
			mptr->ydpos = mptr->ypos;
		};
		rounds = 0;	/* timing mechanism */
		update();
		syncscreen();
		tries = 0;
		while ((pacptr->dirn == NULL) && (tries++ < 300))
		{
			napms(100);
			poll(1);
		}

		/* main game loop */
		do
		{
			if (rounds++ % MSTARTINTVL == 0)
			{
				startmonst();
			};
			pacman();
			if (killflg == TURKEY)
				break;
			for (monstcnt = 0; monstcnt < (MAXMONSTER / 2); monstcnt++)
			{
				monster(monstcnt);	/* next monster */
			};
			if (killflg == TURKEY)
				break;
			if (pacptr->speed == FAST)
			{
				pacman();
				if (killflg == TURKEY)
					break;
			};
			for (monstcnt = (MAXMONSTER / 2); monstcnt < MAXMONSTER; monstcnt++)
			{
				monster(monstcnt);	/* next monster */
			};
			if (killflg == TURKEY)
				break;
			if (potion == TRUE)
			{
				(void) sprintf(message, "%2d", potioncnt);
				SPLOT(3, 60, message);
				if (potioncnt == 10 || potioncnt < 5)
					beep();
				if (--potioncnt <= 0)
				{
					SPLOT(3,45,"                        ");
					SPLOT(4,45,"                        ");
					SPLOT(5,45,"                        ");
					potion = FALSE;
					pacptr->speed = SLOW;
					pacptr->danger = FALSE;
					for (monstcnt = 0; monstcnt < MAXMONSTER; monstcnt++)
					{
						monst[monstcnt].danger = TRUE;
					}
				}
			}
			if (bcount && --bcount == 0) {
				SPLOT(7,45,"                   ");
			}
			if (treascnt && --treascnt == 0) {
				display[TRYPOS][TRXPOS] = VACANT;
				PLOT(TRYPOS, TRXPOS, VACANT);
			}
			if (rounds % monst_often == 0)
				update();	/* score display etc */
			syncscreen();
			if (goldcnt <= 0)
			{
				potintvl -= 5;
				if (potintvl <= 0)
					potintvl = 5;
				reinit();
				goto redraw;
			};
		} while (killflg != TURKEY);
		sprintf(msgbuf, "Oops!  %s got you!\n", full_names[wmonst]);
		SPLOT(5, 45, msgbuf);
		flushinp();
		syncscreen();
		sleep(2);
	}
	SPLOT(8, 45, "THE MONSTERS ALWAYS TRIUMPH");
	SPLOT(9, 45, "IN THE END!");
	update();
	syncscreen();
	over();
}

pacman()
{
	register int sqtype;
	register int mcnt;
	register int tmpx, tmpy;
	int deltat;
	struct pac *mptr;
	int bscore;
	char msgbuf[50];

	syncscreen();

	/* pause; this is the main delay on each turn */
	napms(delay);

	/*
	 * Wait until .1 seconds or less of output in queue.
	 * This is to make it work better on verbose terminals
	 * at 1200 baud.
	 */
	draino(100);

	/* get command from player, but don't wait */
	poll(0);

	/* remember current pacman position */
	tmpx = pacptr->xpos;
	tmpy = pacptr->ypos;

	/* "eat" any gold */
	/* update display array to reflect what is on terminal */
	display[tmpy][tmpx] = VACANT;

	/* what next? */
	switch (pacptr->dirn)
	{
	case DUP:
		pacsymb = (rounds%2) ? CUP : PUP;
		switch (sqtype = display[tmpy + UPINT][tmpx])
		{
		case GOLD:
		case VACANT:
		case CHOICE:
		case POTION:
		case TREASURE:

			/* erase where the pacman went */
			PLOT(tmpy, tmpx, VACANT);
			pacptr->ypos += UPINT;
			break;

		default:
			pacptr->dirn = DNULL;
			pacsymb = PACMAN;
			break;
		};
		break;
	case DDOWN:
		pacsymb = (rounds%2) ? CDOWN : PDOWN;
		switch (sqtype = display[tmpy + DOWNINT][tmpx])
		{
		case GOLD:
		case VACANT:
		case CHOICE:
		case POTION:
		case TREASURE:

			/* erase where the pacman went */
			PLOT(tmpy, tmpx, VACANT);
			pacptr->ypos += DOWNINT;
			break;

		default:
			pacptr->dirn = DNULL;
			pacsymb = PACMAN;
			break;
		};
		break;
	case DLEFT:
		if(tmpx == 0)
		{
			/* erase where the pacman went */
			PLOT(tmpy, tmpx, VACANT);
			pacptr->xpos = XWRAP;
			sqtype = VACANT;
			break;
		};
		pacsymb = (rounds%2) ? CLEFT : PLEFT;
		switch (sqtype = display[tmpy][tmpx + LEFTINT])
		{
		case GOLD:
		case VACANT:
		case CHOICE:
		case POTION:
		case TREASURE:

			/* erase where the pacman went */
			PLOT(tmpy, tmpx, VACANT);
			pacptr->xpos += LEFTINT;
			break;
		
		default:
			pacptr->dirn = DNULL;
			pacsymb = PACMAN;
			break;
		};
		break;
	case DRIGHT:
		if(tmpx == XWRAP)
		{
			/* erase where the pacman went */
			PLOT(tmpy, tmpx, VACANT);
			pacptr->xpos = 0;
			sqtype = VACANT;
			break;
		};
		pacsymb = (rounds%2) ? CRIGHT : PRIGHT;
		switch (sqtype = display[tmpy][tmpx + RIGHTINT])
		{
		case GOLD:
		case VACANT:
		case CHOICE:
		case POTION:
		case TREASURE:

			/* erase where the pacman went */
			PLOT(tmpy, tmpx, VACANT);
			pacptr->xpos += RIGHTINT;
			break;

		default:
			pacptr->dirn = DNULL;
			pacsymb = PACMAN;
			break;
		};
		break;
	case DNULL:
		pacsymb = PACMAN;
		break;
	}

	/* did the pacman get any points or eat a potion? */
	switch (sqtype)
	{
	case CHOICE:
	case GOLD:
		pscore += GOLDVAL;
		goldcnt--;
		break;

	case TREASURE:
		switch (boardcount) {
			case 0:
			case 1:          bscore =  100; break;
			case 2:          bscore =  200; break;
			case 3: case  4: bscore =  500; break;
			case 5: case  6: bscore =  700; break;
			case 7: case  8: bscore = 1000; break;
			default:
			case 9: case 10: bscore = 2000; break;
		}
		pscore += bscore;
		sprintf(msgbuf, "BONUS: %4d", bscore);
		SPLOT(7, 45, msgbuf);
		bcount = BINTVL;
		break;

	case POTION:
		SPLOT(3, 45, "COUNTDOWN: ");
		potion = TRUE;
		potioncnt = potintvl;
		monsthere = 0;
		pacptr->speed = FAST;
		pacptr->danger = TRUE;

		/* slow down monsters and make them harmless */
		mptr = &monst[0];
		for (mcnt = 0; mcnt < MAXMONSTER; mcnt++)
		{
			mptr->speed = SLOW;
			mptr->danger = FALSE;
			mptr++;
		}
		break;
	}

	/* did the pacman run into a monster? */
	killflg = FALSE;
	for (mptr = &monst[0], mcnt = 0; mcnt < MAXMONSTER; mptr++, mcnt++)
	{
		if ((mptr->xpos==pacptr->xpos) && (mptr->ypos==pacptr->ypos))
			killflg = dokill(mcnt);
	};
	if (killflg != TURKEY)
	{
		PLOT(pacptr->ypos, pacptr->xpos, pacsymb | pflash);
	};
	syncscreen();
}
