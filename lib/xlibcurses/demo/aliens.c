/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:demo/aliens.c	1.2"
/*
 * Aliens -- an animated video game
 *      the original version is from 
 *      Fall 1979                       Cambridge               Jude Miller
 *
 * Score keeping modified and general terminal handling (termcap routines
 * from UCB's ex) added by Rob Coben, BTL, June, 1980.
 *  Changes to use a nodelay pipe with unix 3.0 added July 1980,
 *	by Clem Hergenhan, BTL.
 *
 * Changed to use curses November, 1981, by Mark Horton, BTL.
 * Changed to use nodelay input in curses, October 1982.
 *
 * Modified to use colors: May 1987, by Sam Shteingart, BTL
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pwd.h>
#include <curses.h>

/*
 * defines
 */
#ifndef INTERVAL
#define INTERVAL 1
#endif
#define YES	1
#define NO	0
#define SCOREFILE  "/usr/games/lib/aliens.score"
#define DELETE '\177'
#define ABORT '\34'
#define QUIT 'q'
#define BLOODBATH '1'
#define PEACE '2'
#define NORMAL '3'
#define INVIS '4'
#define BOMB_CNT 4
#define BOMB_MAX 20
#define BUF_SIZE 32
#define MINCOL 1
/*
/* global variables
*/
int scores,bases,game;
int i,j,danger,max_danger;
int flip,flop,going_left,al_num,b;
int al_cnt,bmb_cnt;
/* int slow = 0; */
int interv = INTERVAL;
int scorefile;
int repeat;
char lastch;
char outbuf[BUF_SIZE + 1];
char combuf[2];
char inbuf[2];
long timein;
int timehi,timelo;      /* do not break up this declaration */
int fd1 = 1;
int nleft = BUF_SIZE;
int ppid,cpid;
char *nextfree = &outbuf[0];
char barrinit [4] [81]  = {
	"         ########          ########          ########          ########         ",
	"        ##########        ##########        ##########        ##########        ",
	"        ###    ###        ###    ###        ###    ###        ###    ###        ",
	"        ###    ###        ###    ###        ###    ###        ###    ###        "
};
chtype barr [4] [81];
int have_barriers = 1;

/*
 * Number of turns we get for each alien move, as a function of the number
 * of aliens still around.
 */
char nturns[55] = {
	30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
	25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
	20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15,
	14, 14, 13, 13, 12, 12, 11, 11, 10, 10,  9,
	 9,  8,  8,  7,  6,  5,  4,  3,  2,  1,  1
};
int nhit = 0;

struct
{
	int row;
	int col;
}
al[55];
struct {
	int row;
	int col;
	int vel;
} 
bas;
struct {
	int row;
	int col;
} 
bem;
struct {
	int row;
	int col;
}
bmb[BOMB_CNT];
struct {
	int val;
	int col;
	int vel;
}
shp;
int scorsave[8];


/*
 * outchar is like putchar but can be passed to tputs.
 */
outchar(ch)
char ch;
{
	putchar(ch);
};

/* ds_obj -- display an object */
ds_obj(class)
int class;
{
	if ((game==INVIS)&&(class>=0)&&(class<=5))   class = 6;
	switch (class)
	{
	case 0: 
		addch(' ');
		attrset (COLOR_PAIR(1));
		addstr("OXO");
		attrset (A_NORMAL);
		addch(' ');
		break;
	case 1: 
		addch(' ');
		attrset (COLOR_PAIR(2));
		addstr("XOX");
		attrset (A_NORMAL);
		addch(' ');
		break;
	case 2: 
		addch(' ');
		attrset (COLOR_PAIR(3));
		addstr("\\o/");
		attrset (A_NORMAL);
		addch(' ');
		break;
	case 3: 
		addch(' ');
		attrset (COLOR_PAIR(4));
		addstr("/o\\");
		attrset (A_NORMAL);
		addch(' ');
		break;
	case 4: 
		addch(' ');
		attrset (COLOR_PAIR(5));
		addstr("\"M\"");
		attrset (A_NORMAL);
		addch(' ');
		break;
	case 5: 
		addch(' ');
		attrset (COLOR_PAIR(6));
		addstr("wMw");
		attrset (A_NORMAL);
		addch(' ');
		break;
	case 6: 
		addstr("     ");
		break;
	}
};
/*
 * instructions -- print out instructions
 */
instruct()
{
	clear();
	move(0,0);
	addstr("Attention: Alien invasion in progress!\n\n");
	addstr("        Type:   <j>     to move the laser base left\n");
	addstr("                <s>     as above, for lefties\n");
	addstr("                <k>     to halt the laser base\n");
	addstr("                <d>     for lefties\n");
	addstr("                <l>     to move the laser base right\n");
	addstr("                <f>     for lefties\n");
	addstr("                <space> to fire a laser beam\n\n");
	addstr("                <1>     to play \"Bloodbath\"\n");
	addstr("                <2>     to play \"We come in peace\"\n");
	addstr("                <3>     to play \"Invasion of the Aliens\"\n");
	addstr("                <4>     to play \"Invisible Alien Weasels\"\n");
	addstr("                <q>     to quit\n\n");
	refresh();
}

/*
 * over -- game over processing
 */
over()
{
	struct passwd *getpwuid(), *p;
	static char *names[4] = {
		"bloodbath",
		"peace",
		"normal",
		"invisible"
	};
	/*
	 * display the aliens if they were invisible
	 */
	if (game==INVIS) {
		game = NORMAL;       /* remove the cloak of invisibility */
		draw();
		game = INVIS;        /* be tidy */
	}
	/* high score to date processing */
	p = getpwuid(getuid());
	if ((scorefile=open(SCOREFILE,2)) != -1) {
		read(scorefile,scorsave,sizeof(scorsave));
		if (scorsave[(game-BLOODBATH)*2]<scores) {
			scorsave[(game-BLOODBATH)*2] =scores;
			scorsave[(game-BLOODBATH)*2+1] = getuid();
			lseek(scorefile,0l,0);
			write(scorefile,scorsave,sizeof(scorsave));
			close(scorefile);
		}
		else {
			setpwent();
			p = getpwuid(scorsave[(game-BLOODBATH)*2+1]);
			scores = scorsave[(game-BLOODBATH)*2];
		}
	}
	move(9,20);
	addstr(" __________________________ ");
	move(10,20);
	addstr("|                          |");
	move(11,20);
	addstr("| G A M E   O V E R        |");
	move(12,20);
	addstr("|                          |");
	move(13,20);
	printw("| Game type : %-10s   |",names[game-BLOODBATH]);
	move(14,20);
	printw("| High Score to date: %-5u|",scores);
	move(15,20);
	printw("| Player : %-8s        |",p->pw_name);
	move(16,20);
	addstr("|__________________________|");
	leave();
}

/*
 * leave -- clear bottom line, flush buffers, reset tty, and exit.
 */
leave() {
	move(23,0);
	clrtoeol();
	refresh();
	endwin();
	sleep(1);
	exit(0);
}

/*
 * init -- does global initialization and spawns a child process to read
 *      the input terminal.
 */
init()
{
	/* nice(10);       /* decrease priority */
	time(&timein);  /* get start time */
	time(&timehi);  /* get it again for seeding rand */
	srand(timelo);  /* start rand randomly */
	/*
	 * verify CRT and get proper cursor control sequence.
	 */
	vsinit();
	/*
	 * setup raw mode, no echo
	 *
	 *  if (baudrate() < 9600)
	 *	slow = 1;
	 */

	/*
	 * New game starts here
	 */
	game = 0;
	instruct();
	while (game==0) {
		poll();
		sleep(1);
	}
	scores = 0;
	bases = 3;
	danger = 11;
	max_danger = 22;
	return;
};

/* tabl -- tableau draws the starting game tableau.  */
tabl()
{
	clear();

	/* initialize alien co-ords, display */

	al_cnt = 55;
	nhit = 0;
	for (j=0;j<=4;j++)
	{
		move(danger-(2*j),0);
		for (i=0;i<=10;i++)
		{
			al[(11*j)+i].row = danger - (2*j);
			al[(11*j)+i].col = (6*i);
		};
	};
	if (danger<max_danger)   danger++;
	al_num = 54;
	flip = 0;	/* hit left or right edge of screen: time to switch */
	flop = 0;	/* This alien is moving down a row */
	going_left = 0;	/* all aliens are moving left */
	/*
	 * initialize laser base position, velocity
	 */
	bas.row = 23;
	bas.col = 70;
	bas.vel = 0;
	bem.row = 0;
	/*
	 * initialize bomb arrays (row = 0 implies empty)
	 */
	for (i=0;i<BOMB_CNT;i++)   bmb[i].row = 0;
	b = 0;
	bmb_cnt = 0;
	/*
	 * initialize barricades
	 */
	for (i=0;i<=3;i++) {
		for (j=0;j<80;j++)
		     /* barr[i][j] = barrinit[i][j] | ((barrinit[i][j] == ' ') ?
							A_NORMAL : COLOR_PAIR(7)); */
		     barr[i][j] = (barrinit[i][j] == ' ') ?
					' ' : (ACS_BLOCK | COLOR_PAIR (7));
	}
	/*
	 * initialize mystery ships
	 */
	shp.vel = 0;
	return;
}

/* draw -- redraw screen from data structure */
draw()
{
	int a, i, j;

	werase(stdscr);

	move(0,0);
	printw("Score: %u",scores);
	move(0,18);
	printw("I N V A S I O N   O F   T H E   A L I E N S !");
	move(0,70);
	printw("Lasers: %d",bases);

	/*
	 * display barricades.  Must be first since we print blanks here too,
	 * and they should be overridden by bombs and lasers.
	 */
	for (i=0;i<=3;i++) {
		move(i+19,0);
		addchstr (&barr [i] [0]);
	}

	/*
	 * display laser.  This is done before the aliens so that when the
	 * aliens land they can stomp on the laser.
	 */
	move(bas.row, 0);
	clrtoeol();
	/* addch(' '); */
	attrset(COLOR_PAIR(2));
	mvaddstr(bas.row, bas.col, "xx|xx");
	attrset(A_NORMAL);

	/* display aliens */
	for (a=0; a<55; a++)
	{
		i = al[a].row;
		if (i > 0) {
			j = al[a].col;
			move(i,j);
			ds_obj(((i/2)&1) + (2*(a/22)));
		}
	}

	/*
	 * display laser beam
	 */
	if (bem.row > 0) {
		mvaddch(bem.row, bem.col, '|');
		if (bem.row < 22)
			mvaddch(bem.row+1, bem.col, '|');
	}

	/*
	 * display bomb arrays (row = 0 implies empty)
	 */
	for (a=0; a<BOMB_CNT; a++) {
		if (bmb[a].row > 0) {
			mvaddch(bmb[a].row, bmb[a].col, '*');
		}
	}

	/*
	 * display mystery ship
	 */
	if (shp.vel)
	{
		attrset(COLOR_PAIR(4) | A_BLINK);
		mvprintw(1, shp.col, "<=%2d=>", shp.val/3);
		attrset(A_NORMAL);
	}
	refresh();
	fflush(stdout);
}

/* poll -- read characters sent by input subprocess and set global flags */
poll()
{
	int keyhit;
	struct stat stbuf;

	if (game==BLOODBATH) {
		if (bas.col<=1)   bas.vel = 1;
		if (bas.col>=72)  bas.vel = -1;
	}

	/* nodelay input - if nothing's there we get -1 */
	keyhit = getch();
	if (keyhit < 0)
		return;

	switch (keyhit) {       /* do case char */
	/* Start laser base moving to the left */
	case 's':     
	case 'h':      
	case 'j':
	case ',':
	case KEY_LEFT:
	doleft:	/* '4' */
		if (game==BLOODBATH)   break;
		bas.vel = -1;
		break;
	/* Start laser base moving to the right */
	case 'f':    
	case 'l':     
	case '/':
	case '6':
	case KEY_RIGHT:
		if (game==BLOODBATH)   break;
		bas.vel = 1;
		break;
	/* Stop motion of laser base */
	case 'd':     
	case 'k':      
	case '.':      
	case '5':
	case KEY_HOME:
	case KEY_B2:
		if (game==BLOODBATH)   break;
		bas.vel = 0;
		break;
	/* Fire the laser */
	case ' ':      
	case '8':
	case KEY_UP:
		if (bem.row==0)   bem.row = 22;
		break;
	/* quit the game */
	case '\177':    
	case '\34':     
	case 'q':      
		over();
		break;
	/* select various games at the start */
	case BLOODBATH:     
		if (game!=0)   break;
		game = BLOODBATH;
		break;
	case PEACE:     
		if (game!=0)   break;
		game = PEACE;
		break;
	case NORMAL:     
		if (game!=0)   break;
		game = NORMAL;
		break;
	case INVIS:     
		if (game!=0)   goto doleft;
		game = INVIS;
		break;
	}
}

/* base -- move the laser base left or right */
base()
{
	bas.col += bas.vel;
	if (bas.col<1)
		bas.col = 1;
	else if (bas.col>72)
		bas.col = 72;
};

/* beam -- activate or advance the laser beam if required */
beam()
{
	if (bem.row == 0)
		return;
	if (bem.row == 22)
		bem.col = bas.col + 2;
	/*
	 * check for contact with an alien
	 */
	for (i=0;i<55;i++) {
		if (al[i].row > 0 &&
		    al[i].row==bem.row &&
		    al[i].col+1<=bem.col &&
		    ((al[i].col+3)>=bem.col)) {
			/*
			 * contact!
			 */
			scores = scores + (i/22) + 1;   /* add points */
			beep();
			bem.row=0;
			al[i].row=0;    /* clear beam and alien state */
			al_cnt--;
			nhit++;
			return;
		}
	}
	/*
	 * check for contact with a bomb
	 */
	for (i=0;i<BOMB_CNT;i++) {
		if ((bem.row==bmb[i].row || bem.row+1==bmb[i].row)
			&& bem.col==bmb[i].col) {
			bem.row = 0;
			bmb_cnt--;
			bmb[i].row = 0;
			beep();
			return;
		}
	}
	/*
	 * check for contact with a barricade
	 */
	if ((bem.row>=19)&&(bem.row<=22)&&(barr[bem.row-19][bem.col]!=' ')) {
		barr[bem.row-19][bem.col] = ' ';
		bem.row = 0;
		beep();
		return;
	}
	/*
	 * check for contact with a mystery ship
	 */
	if ((shp.vel!=0)&&(bem.row==1)&&(bem.col>(i=shp.col-shp.vel))&&(bem.col<i+7)) {
		/*
		 * contact!
		 */
		shp.vel = 0;
		scores += shp.val/3;
		beep();
	}
	/*
	 * update beam position
	 */
	if ((--bem.row)==0) {
	}
	return;
};

/* bomb -- advance the next active bomb */
bomb()
{
	if (bmb_cnt<=0)   return;
	for (b=0; b<BOMB_CNT; b++) {
		if (bmb[b].row != 0)
			onebomb(b);
	}
}

onebomb(b)
{
	/*
	 * now advance the bomb, check for hit, and display
	 */
	bmb[b].row++;
	if (bmb[b].row==23) {
		if ((bmb[b].col>bas.col)&&
		    (bmb[b].col<=(bas.col+5))) {
			/*
			 * the base is hit!
			 */
			bases--;
			move(0,70);
			printw("Lasers: %d",bases);
			/* make heart-rending noise */
			for (i=0;i<10;i++) {
				beep();
			}
			fflush(stdout);
			if (bases==0) {
				/* game over */
				over();
			}
			sleep(2);
			bas.col = 72;
			bas.vel = 0;
		}
	}
	if((bmb[b].row>=19)&&(bmb[b].row<23)&&(barr[bmb[b].row-19][bmb[b].col]!=' ')) {
		/*
		 * the bomb has hit a barricade
		 */
		barr[bmb[b].row-19][bmb[b].col] = ' ';
		bmb[b].row = 0;
		bmb_cnt--;
		beep();
		return;
	}
	if (bmb[b].row==23) {
		bmb_cnt--;
		bmb[b].row = 0;
	}
}

/* ship -- create or advance a mystery ship if desired */
ship()
{
	if (shp.vel==0) {
		if ((i=rand())<32) {
			/*
			 * create a mystery ship
			 * this occurs about once every minute
			 */
			if (i<16) {
				shp.vel = -1;
				shp.col = COLS - 6;
			}
			else {
				shp.vel = 1;
				shp.col = MINCOL;
			}
			shp.val = 90;
		}
	}
	else {
		/*
		 * update an existing mystery ship
		 */
		shp.val--;
		shp.col += shp.vel;
		if (((i=shp.col)>(COLS-6))||(i<MINCOL))   {
			/*
			 * remove the mystery ship
			 */
			shp.vel = 0;
		}
	}
}

/*
 * One tick of the clock.  We use napms to get decent resolution.
 * If that fails, we assume it slept for 1 second and don't do it
 * again for a while.
 */
#define TICKTIME (1000/60)
tick()
{
	static int clock;
	int rv;

	if (clock <= 0)
		rv = napms(TICKTIME);
	clock -= TICKTIME;
	if (rv < 0)
		clock = 1000;
	else
		clock = 0;
}

/* Main loop of game */
update()
{
	int i;

	for (i=0; i<nturns[nhit]; i++) {
		tick();
		draino(100);
		poll();
		beam();
		beam();
		base();
		bomb();
		ship();
		/* if (i & 1) */
		draw();
		if (al_cnt==0)
			return;
	}
	alien();
	draw();
}

/* Advance all the aliens */
alien()
{
	if (al_cnt==0)   return; /* check if done */
	flop = 0;
	if (flip) { 
		going_left = (going_left+1) % 2;
		flop = 1;
	}
	flip = 0;
	al_num = 0;
	for (al_num=0; al_num < 55; al_num++)
	{
		if ((i = al[al_num].row)>0)
			onealien(al_num);
	}
}

/* onealien -- advance the next alien */
onealien(an)
{
	int x, y;

	if (i>=23)
	{
		/* game over, aliens have overrun base */
		over();
	}

	if (have_barriers && i >= 19) {
		/* Aliens are so low we have to take the barriers away.  */
		for (x=0; x<4; x++)
			for (y=0; y<81; y++)
				barr[x][y] = ' ';
		have_barriers = 0;
	}

	if (going_left)
		al[an].col--;
	else
		al[an].col++;

	if (((j = al[an].col)==0)||(j==75))   flip = 1;
	if (flop) {
		i = ++al[an].row;
	}
	/*
	 * check for bomb release
	 */
	if ((game==BLOODBATH)||(game==PEACE))   return;     /* disable bombs */
	for (i=an-11;i>=0;i -= 11) {
		if (al[i].row!=0)   return;
	}
	if ((al[an].col>=bas.col)&&(al[an].col<(bas.col+3))&&
	    (al[an].row<=BOMB_MAX)) {
		for (i=0;i<BOMB_CNT;i++) {
			if (bmb[i].row==0) {
				bmb[i].row = al[an].row;
				bmb[i].col = al[an].col + 2;
				bmb_cnt++;
				break;
			}
		}
	}
}

/* main -- scheduler and main entry point for aliens */
main(argc, argv)
char **argv;
{
	int cnt = 0;

	if (argc > 1)
		interv = atoi(argv[1]);
	init();
	while (1)
	{
		tabl();
		draw();
		sleep(2);
		while (1)
		{
			update();
			if (al_cnt==0)   break;
		};
	};
}

/* terminal type using curses */
vsinit()
{
	initscr();
	if (start_color() == OK)
	{
	    init_pair (1, COLOR_YELLOW, COLOR_BLUE);
	    init_pair (2, COLOR_YELLOW, COLOR_RED);
	    init_pair (3, COLOR_WHITE, COLOR_GREEN);
	    init_pair (4, COLOR_MAGENTA, COLOR_YELLOW);
	    init_pair (5, COLOR_YELLOW, COLOR_MAGENTA);
	    init_pair (6, COLOR_RED, COLOR_CYAN);
	    init_pair (7, COLOR_BLUE, COLOR_WHITE);
	}
	cbreak();
	noecho();
	nonl();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
}
