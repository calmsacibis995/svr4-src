/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:demo/pacman/monster.c	1.2"
#include <stdio.h>
#include	"pacdefs.h"

extern char
	brd[BRDY][BRDX],
	display[BRDY][BRDX];

extern int
	delay,
	game,
	killflg,
	potion,
	monst_often,
	mflash, rflash, pflash,
	rounds;

extern unsigned
	pscore;

extern struct pac
	*pacptr;

int	rscore[MAXMONSTER];

struct pac
	monst[MAXMONSTER];

char monst_names[] =  "BIPC";
char runner_names[] = "bipc";
char *full_names[] = {
	"Blinky", "Inky", "Pinky", "Clyde", 0
};

startmonst()
{
	register struct pac *mptr;
	register int monstnum;

	for (mptr = &monst[0], monstnum = 0; monstnum < MAXMONSTER; mptr++, monstnum++)
	{
		if (mptr->stat == START)
		{
			rscore[monstnum] = 1;

			/* clear home */
			PLOT(mptr->ypos, mptr->xpos, VACANT);

			/* initialize moving monster */
			mptr->ypos = MBEGINY;
			mptr->xpos = MBEGINX;
			mptr->ydpos = MBEGINY;
			mptr->xdpos = MBEGINX;
			mptr->stat = RUN;
			PLOT(MBEGINY, MBEGINX, mptr->danger ?
				monst_names[monstnum] | mflash | COLOR_PAIR (monstnum+1):
				runner_names[monstnum] | rflash | COLOR_PAIR (monstnum+1));

			/* DRIGHT or DLEFT? */
			mptr->dirn = getrand(2) + DLEFT;
			break;
		}
	}
}

monster(mnum)
	int mnum;
{
	register int newx,newy;
	register int tmpx, tmpy;
	struct pac *mptr;

	mptr = &monst[mnum];

	/* remember monster's current position */
	tmpx = mptr->xpos;
	tmpy = mptr->ypos;

	/* if we can, let's move a monster */
	if (mptr->stat == RUN)
	{
		/* get a new direction */
		mptr->dirn = which(mptr, tmpx, tmpy);
		switch (mptr->dirn)
		{
		case DUP:
			newy = tmpy + UPINT;
			newx = tmpx;
			break;

		case DDOWN:
			newy = tmpy + DOWNINT;
			newx = tmpx;
			break;

		case DLEFT:
			newx = tmpx + LEFTINT;
			newy = tmpy;
			if (newx <= 0)
				newx = XWRAP;	/* wrap around */
			break;

		case DRIGHT:
			newx = tmpx + RIGHTINT;
			newy = tmpy;
			if (newx >= XWRAP)
				newx = 0;	/* wrap around */
			break;
		}

		/* use brd to determine if this was a valid direction */
		switch (brd[newy][newx])
		{
		case GOLD:
		case VACANT:
		case POTION:
		case TREASURE:
		case CHOICE:
			/* set new position */
			mptr->xpos = newx;
			mptr->ypos = newy;

			/* run into a pacman? */
			if ((newy == pacptr->ypos) &&
				(newx == pacptr->xpos))
			{
				killflg = dokill(mnum);
			};
			if (rounds % monst_often == 0 || killflg == TURKEY) {
				PLOT(mptr->ydpos,mptr->xdpos,
					display[mptr->ydpos][mptr->xdpos]);
				if (mptr->danger == TRUE)
				{
					PLOT(newy, newx, monst_names[mnum] | mflash | COLOR_PAIR(mnum+1));
				}
				else if (killflg != GOTONE)
				{
					PLOT(newy, newx, runner_names[mnum] | rflash | COLOR_PAIR(mnum+1));
				};
				mptr->ydpos = newy;
				mptr->xdpos = newx;
			}
			break;

		default:
			errgen("bad direction");
			break;
		};
	}
}

which(mptr, x, y)	/* which directions are available ? */
	struct pac *mptr;
	int x, y;
{
	register int movecnt;
	register int submovecnt;
	register int next;
	int moves[4];
	int submoves[4];
	int nydirn, nxdirn;
	int goodmoves;
	int offx, offy;
	int tmpdirn;
	char *brdptr;

	/*
	 * As a general rule: determine the set of all
	 * possible moves, but select only those moves
	 * that don't require a monster to backtrack.
	 */
	movecnt = 0;
	brdptr = &(brd[y][x]);
	if (((tmpdirn = mptr->dirn) != DDOWN) &&
		((next = *(brdptr + (BRDX * UPINT))) != WALL) &&
		(next != GATE))
	{
		moves[movecnt++] = DUP;
	};
	if ((tmpdirn != DUP) &&
		((next = *(brdptr + (BRDX * DOWNINT))) != WALL) &&
		(next != GATE))
	{
		moves[movecnt++] = DDOWN;
	};
	if ((tmpdirn != DRIGHT) &&
		((next = *(brdptr + LEFTINT)) != WALL) &&
		(next != GATE))
	{
		moves[movecnt++] = DLEFT;
	};
	if ((tmpdirn != DLEFT) &&
		((next = *(brdptr + RIGHTINT)) != WALL) &&
		(next != GATE))
	{
		moves[movecnt++] = DRIGHT;
	};

	/*
	 * If the player requested intelligent monsters and
	 * the player is scoring high ...
	 */
	if (game >= 2 && getrand(game == 2 ? 10000 : 1000) < pscore)
	{
		/* make monsters intelligent */
		if (mptr->danger == FALSE)
		{
			/*
			 * Holy Cow!! The pacman is dangerous,
			 * permit monsters to reverse direction
			 */
			switch (tmpdirn)
			{
			case DUP:
				if ((*(brdptr + (BRDX * DOWNINT)) != WALL) &&
					(*(brdptr + (BRDX * DOWNINT)) != GATE))
				{
					moves[movecnt++] = DDOWN;
				};
				break;

			case DDOWN:
				if ((*(brdptr + (BRDX * UPINT)) != WALL) &&
					(*(brdptr + (BRDX * UPINT)) != GATE))
				{
					moves[movecnt++] = DUP;
				};
				break;

			case DRIGHT:
				if ((*(brdptr + LEFTINT) != WALL) &&
					(*(brdptr + LEFTINT) != GATE))
				{
					moves[movecnt++] = DLEFT;
				};
				break;

			case DLEFT:
				if ((*(brdptr + RIGHTINT) != WALL) &&
					(*(brdptr + RIGHTINT) != GATE))
				{
					moves[movecnt++] = DRIGHT;
				};
				break;
			};
		};

		/* determine the offset from the pacman */
		offx = x - pacptr->xpos;
		offy = y - pacptr->ypos;
		if (offx > 0)
		{
			/*need to go left */
			nxdirn = DLEFT;
		}
		else
		{
			if (offx < 0)
			{
				nxdirn = DRIGHT;
			}
			else
			{
				/*need to stay here */
				nxdirn = DNULL;
			};
		};
		if (offy > 0)
		{
			/*need to go up */
			nydirn = DUP;
		}
		else
		{
			if (offy < 0)
			{
				/* need to go down */
				nydirn = DDOWN;
			}
			else
			{
				/* need to stay here */
				nydirn = DNULL;
			};
		};
		goodmoves = 0;
		for (submovecnt = 0; submovecnt < movecnt; submovecnt++)
		{
			if (mptr->danger == TRUE)
			{
				if ((moves[submovecnt] == nydirn) ||
					(moves[submovecnt] == nxdirn))
				{
					submoves[goodmoves++] = moves[submovecnt];
				};
			}
			else
			{
				if ((moves[submovecnt] != nydirn) &&
					(moves[submovecnt] != nxdirn))
				{
					submoves[goodmoves++] = moves[submovecnt];
				};
			};
		};
		if (goodmoves > 0)
		{
			return(submoves[getrand(goodmoves)]);
		};
	};
	return(moves[getrand(movecnt)]);
}
