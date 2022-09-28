/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)ttymon:stty.c	1.12.3.1"

#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <sys/types.h>
#include <termio.h>
#include <sys/stermio.h>
#include <sys/termiox.h>
#include "stty.h"

extern char *getenv();
extern void exit();
extern void perror();

static char	*STTY="stty: ";
static char	*USAGE="usage: stty [-ag] [modes]\n";
static int	pitt = 0;
static struct termios cb;
static struct termio ocb; /* for non-streams devices */
static struct stio stio;

#ifdef MERGE386
long	sc_flag;
#endif

static struct termiox termiox;
static struct winsize winsize, owinsize;
static int term;

main(argc, argv)
char	*argv[];
{

	int i;
	char	*s_arg, *sttyparse();			/* s_arg: ptr to mode to be set */

	(void)setlocale(LC_ALL, "");
	
	if((term = get_ttymode(0, &ocb, &cb, &stio, &termiox, &winsize)) < 0) {
		perror(STTY);
		exit(2);
	}
	owinsize = winsize;
	if (argc == 1) {
		prmodes();
		exit(0);
	}
	if ((argc == 2) && (argv[1][0] == '-') && (argv[1][2] == '\0'))
	switch(argv[1][1]) {
		case 'a':
			pramodes();
			exit(0);
		case 'g':
			prencode();
			exit(0);
		default:
			(void) fprintf(stderr, "%s", USAGE);
			exit(2);
	}
	if (s_arg = sttyparse(argc, argv, term, &ocb, &cb, &termiox, &winsize)) {
		(void) fprintf(stderr, "unknown mode: %s\n", s_arg);
		exit(2);
	}

	if(set_ttymode(0, term, &ocb, &cb, &stio, &termiox, &winsize, &owinsize) == -1) {
		perror(STTY);
		exit(2);
	}
	exit(0);	/*NOTREACHED*/
}

prmodes()				/* print modes, no options, argc is 1 */
{
	register m;

	if (!(term & ASYNC)) {
		m = stio.imode;
		if (m & IUCLC) (void) printf ("iuclc ");
		else (void) printf ("-iuclc ");
		m = stio.omode;
		if (m & OLCUC) (void) printf ("olcuc ");
		else (void) printf ("-olcuc ");
		if (m & TAB3) (void) printf ("tab3 ");
		m = stio.lmode;
		if (m & XCASE) (void) printf ("xcase ");
		else (void) printf ("-xcase ");
		if (m & STFLUSH) (void) printf ("stflush ");
		else (void) printf ("-stflush ");
		if (m & STWRAP) (void) printf ("stwrap ");
		else (void) printf ("-stwrap ");
		if (m & STAPPL) (void) printf ("stappl ");
		else (void) printf ("-stappl ");
		(void) printf ("\n");
	}
	if (term & ASYNC) {
		m = cb.c_cflag;
		if ((term & TERMIOS) && (m&CIBAUD) != 0 && ((m&CIBAUD)>>IBSHIFT) != (m&CBAUD)) {
			prspeed("ispeed ", (m&CIBAUD)>>IBSHIFT);
			prspeed("ospeed ", m&CBAUD);
		} else
			prspeed("speed ", m & CBAUD);
		if (m&PARENB) {
			if((m&PAREXT) && (term & TERMIOS)) {
				if (m&PARODD)
					(void) printf("markp ");
				else
					(void) printf("spacep ");
			} else {
				if (m&PARODD)
					(void) printf("oddp ");
				else
					(void) printf("evenp ");
			}
		} else
			(void) printf("-parity ");
		if(((m&PARENB) && !(m&CS7)) || (!(m&PARENB) && !(m&CS8)))
			(void) printf("cs%c ",'5'+(m&CSIZE)/CS6);
		if (m&CSTOPB)
			(void) printf("cstopb ");
		if (m&HUPCL)
			(void) printf("hupcl ");
		if (!(m&CREAD))
			(void) printf("-cread ");
		if (m&CLOCAL)
			(void) printf("clocal ");
		if (m&LOBLK)
			(void) printf("loblk ");
		(void) printf("\n");
		if(ocb.c_line != 0)
			(void) printf("line = %d; ", ocb.c_line);
		if(term & WINDOW) {
			(void)printf("rows = %d; columns = %d;", winsize.ws_row, winsize.ws_col);
			(void)printf(" ypixels = %d; xpixels = %d;\n", winsize.ws_ypixel, winsize.ws_xpixel);
		}
		if((cb.c_lflag&ICANON)== 0)
			(void) printf("min = %d; time = %d;\n", cb.c_cc[VMIN],
			cb.c_cc[VTIME]);
		if(cb.c_cc[VINTR] != CINTR)
			pit(cb.c_cc[VINTR], "intr", "; ");
		if(cb.c_cc[VQUIT] != CQUIT)
			pit(cb.c_cc[VQUIT], "quit", "; ");
		if(cb.c_cc[VERASE] != CERASE)
			pit(cb.c_cc[VERASE], "erase", "; ");
		if(cb.c_cc[VKILL] != CKILL)
			pit(cb.c_cc[VKILL], "kill", "; ");
		if(cb.c_cc[VEOF] != CEOF)
			pit(cb.c_cc[VEOF], "eof", "; ");
		if(cb.c_cc[VEOL] != CNUL)
			pit(cb.c_cc[VEOL], "eol", "; ");
		if(cb.c_cc[VEOL2] != CNUL)
			pit(cb.c_cc[VEOL2], "eol2", "; ");
		if(cb.c_cc[VSWTCH] != CSWTCH)
			pit(cb.c_cc[VSWTCH], "swtch", "; ");
		if(term & TERMIOS) {
			if(cb.c_cc[VSTART] != CSTART)
				pit(cb.c_cc[VSTART], "start", "; ");
			if(cb.c_cc[VSTOP] != CSTOP)
				pit(cb.c_cc[VSTOP], "stop", "; ");
			if(cb.c_cc[VSUSP] != CSUSP)
				pit(cb.c_cc[VSUSP], "susp", "; ");
			if(cb.c_cc[VDSUSP] != CDSUSP)
				pit(cb.c_cc[VDSUSP], "dsusp", "; ");
			if(cb.c_cc[VREPRINT] != CRPRNT)
				pit(cb.c_cc[VREPRINT], "rprnt", "; ");
			if(cb.c_cc[VDISCARD] != CFLUSH)
				pit(cb.c_cc[VDISCARD], "flush", "; ");
			if(cb.c_cc[VWERASE] != CWERASE)
				pit(cb.c_cc[VWERASE], "werase", "; ");
			if(cb.c_cc[VLNEXT] != CLNEXT)
				pit(cb.c_cc[VLNEXT], "lnext", "; ");
		}
		if(pitt) (void) printf("\n");
		m = cb.c_iflag;
		if (m&IGNBRK)
			(void) printf("ignbrk ");
		else if (m&BRKINT)
			(void) printf("brkint ");
		if (!(m&INPCK))
			(void) printf("-inpck ");
		else if (m&IGNPAR)
			(void) printf("ignpar ");
		if (m&PARMRK)
			(void) printf("parmrk ");
		if (!(m&ISTRIP))
			(void) printf("-istrip ");
		if (m&INLCR)
			(void) printf("inlcr ");
		if (m&IGNCR)
			(void) printf("igncr ");
		if (m&ICRNL)
			(void) printf("icrnl ");
		if (m&IUCLC)
			(void) printf("iuclc ");
		if (!(m&IXON))
			(void) printf("-ixon ");
		else if (!(m&IXANY))
			(void) printf("-ixany ");
		if (m&IXOFF)
			(void) printf("ixoff ");
		if ((term & TERMIOS) && (m&IMAXBEL))
			(void) printf("imaxbel ");
		m = cb.c_oflag;
		if (!(m&OPOST))
			(void) printf("-opost ");
		else {
			if (m&OLCUC)
				(void) printf("olcuc ");
			if (m&ONLCR)
				(void) printf("onlcr ");
			if (m&OCRNL)
				(void) printf("ocrnl ");
			if (m&ONOCR)
				(void) printf("onocr ");
			if (m&ONLRET)
				(void) printf("onlret ");
			if (m&OFILL)
				if (m&OFDEL)
					(void) printf("del-fill ");
				else
					(void) printf("nul-fill ");
			delay((m&CRDLY)/CR1, "cr");
			delay((m&NLDLY)/NL1, "nl");
			delay((m&TABDLY)/TAB1, "tab");
			delay((m&BSDLY)/BS1, "bs");
			delay((m&VTDLY)/VT1, "vt");
			delay((m&FFDLY)/FF1, "ff");
		}
		(void) printf("\n");
		m = cb.c_lflag;
		if (!(m&ISIG))
			(void) printf("-isig ");
		if (!(m&ICANON))
			(void) printf("-icanon ");
		if (m&XCASE)
			(void) printf("xcase ");
		(void) printf("-echo "+((m&ECHO)!=0));
		(void) printf("-echoe "+((m&ECHOE)!=0));
		(void) printf("-echok "+((m&ECHOK)!=0));
		if (m&ECHONL)
			(void) printf("echonl ");
		if (m&NOFLSH)
			(void) printf("noflsh ");
		if (m&TOSTOP)
			(void) printf("tostop ");
		if (m&ECHOCTL)
			(void) printf("echoctl ");
		if (m&ECHOPRT)
			(void) printf("echoprt ");
		if (m&ECHOKE)
			(void) printf("echoke ");
		if (m&DEFECHO)
			(void) printf("defecho ");
		if (m&FLUSHO)
			(void) printf("flusho ");
		if (m&PENDIN)
			(void) printf("pendin ");
		if (m&IEXTEN)
			(void) printf("iexten ");
		(void) printf("\n");
	}
	if(term & FLOW) {
		m = termiox.x_hflag;
		if(m & RTSXOFF)
			(void) printf("rtsxoff ");
		if(m & CTSXON)
			(void) printf("ctsxon ");
		if(m & DTRXOFF)
			(void) printf("dtrxoff ");
		if(m & CDXON)
			(void) printf("cdxon ");
		if(m & ISXOFF)
			(void) printf("isxoff ");
		m = termiox.x_cflag;
		switch(m & XMTCLK)
		{
			case XCIBRG: (void)printf("xcibrg ");
				     break;
			case XCTSET: (void)printf("xctset ");
				     break;
			case XCRSET: (void)printf("xcrset ");
		}
		
		switch(m & RCVCLK)
		{
			case RCIBRG: (void)printf("rcibrg ");
				     break;
			case RCTSET: (void)printf("rctset ");
				     break;
			case RCRSET: (void)printf("rcrset ");
		}
		
		switch(m & TSETCLK)
		{
			case TSETCOFF: (void)printf("tsetcoff ");
				     break;
			case TSETCRBRG: (void)printf("tsetcrbrg ");
				     break;
			case TSETCTBRG: (void)printf("tsetctbrg ");
				     break;
			case TSETCTSET: (void)printf("tsetctset ");
				     break;
			case TSETCRSET: (void)printf("tsetcrset ");
		}
		
		switch(m & RSETCLK)
		{
			case RSETCOFF: (void)printf("rsetcoff ");
				     break;
			case RSETCRBRG: (void)printf("rsetcrbrg ");
				     break;
			case RSETCTBRG: (void)printf("rsetctbrg ");
				     break;
			case RSETCTSET: (void)printf("rsetctset ");
				     break;
			case RSETCRSET: (void)printf("rsetcrset ");
		}

#ifdef MERGE386
		if (sc_flag & KB_ISSCANCODE) {
			(void) printf("isscancode ");
			if (sc_flag & KB_XSCANCODE)
				(void) printf("xscancode ");
		}
#endif	

		(void) printf("\n");
	}
}

pramodes()				/* print all modes, -a option */
{
	register m;

	m = cb.c_cflag;
	if(term & ASYNC) {
		if ((term & TERMIOS) && (m&CIBAUD) != 0 && ((m&CIBAUD)>>IBSHIFT) != (m&CBAUD)) {
			prspeed("ispeed ", (m&CIBAUD)>>IBSHIFT);
			prspeed("ospeed ", m&CBAUD);
		} else
			prspeed("speed ", m & CBAUD);
		if(!(term & TERMIOS))
			(void) printf("line = %d; ", ocb.c_line);
		(void)printf("\n");
		if(term & WINDOW) {
			(void)printf("rows = %d; columns = %d;", winsize.ws_row, winsize.ws_col);
			(void)printf(" ypixels = %d; xpixels = %d;\n", winsize.ws_ypixel, winsize.ws_xpixel);
		}
		if((cb.c_lflag&ICANON)== 0)
			(void) printf("min = %d; time = %d;\n", cb.c_cc[VMIN],
			cb.c_cc[VTIME]);
		pit(cb.c_cc[VINTR], "intr", "; ");
		pit(cb.c_cc[VQUIT], "quit", "; ");
		pit(cb.c_cc[VERASE], "erase", "; ");
		pit(cb.c_cc[VKILL], "kill", ";\n");
		pit(cb.c_cc[VEOF], "eof", "; ");
		pit(cb.c_cc[VEOL], "eol", "; ");
		pit(cb.c_cc[VEOL2], "eol2", "; ");
		pit(cb.c_cc[VSWTCH], "swtch", ";\n");
		if(term & TERMIOS) {
			pit(cb.c_cc[VSTART], "start", "; ");
			pit(cb.c_cc[VSTOP], "stop", "; ");
			pit(cb.c_cc[VSUSP], "susp", "; ");
			pit(cb.c_cc[VDSUSP], "dsusp", ";\n");
			pit(cb.c_cc[VREPRINT], "rprnt", "; ");
			pit(cb.c_cc[VDISCARD], "flush", "; ");
			pit(cb.c_cc[VWERASE], "werase", "; ");
			pit(cb.c_cc[VLNEXT], "lnext", ";\n");
		}
	} else
		pit((unsigned)stio.tab, "ctab", "\n");
	m = cb.c_cflag;
	(void) printf("-parenb "+((m&PARENB)!=0));
	(void) printf("-parodd "+((m&PARODD)!=0));
	(void) printf("cs%c ",'5'+(m&CSIZE)/CS6);
	(void) printf("-cstopb "+((m&CSTOPB)!=0));
	(void) printf("-hupcl "+((m&HUPCL)!=0));
	(void) printf("-cread "+((m&CREAD)!=0));
	(void) printf("-clocal "+((m&CLOCAL)!=0));

	(void) printf("-loblk "+((m&LOBLK)!=0));
	if(term & TERMIOS)
		(void) printf("-parext "+((m&PAREXT)!=0));

	(void) printf("\n");
	m = cb.c_iflag;
	(void) printf("-ignbrk "+((m&IGNBRK)!=0));
	(void) printf("-brkint "+((m&BRKINT)!=0));
	(void) printf("-ignpar "+((m&IGNPAR)!=0));
	(void) printf("-parmrk "+((m&PARMRK)!=0));
	(void) printf("-inpck "+((m&INPCK)!=0));
	(void) printf("-istrip "+((m&ISTRIP)!=0));
	(void) printf("-inlcr "+((m&INLCR)!=0));
	(void) printf("-igncr "+((m&IGNCR)!=0));
	(void) printf("-icrnl "+((m&ICRNL)!=0));
	(void) printf("-iuclc "+((m&IUCLC)!=0));
	(void) printf("\n");
	(void) printf("-ixon "+((m&IXON)!=0));
	(void) printf("-ixany "+((m&IXANY)!=0));
	(void) printf("-ixoff "+((m&IXOFF)!=0));
	if(term & TERMIOS)
		(void) printf("-imaxbel "+((m&IMAXBEL)!=0));
	(void) printf("\n");
	m = cb.c_lflag;
	(void) printf("-isig "+((m&ISIG)!=0));
	(void) printf("-icanon "+((m&ICANON)!=0));
	(void) printf("-xcase "+((m&XCASE)!=0));
	(void) printf("-echo "+((m&ECHO)!=0));
	(void) printf("-echoe "+((m&ECHOE)!=0));
	(void) printf("-echok "+((m&ECHOK)!=0));
	(void) printf("-echonl "+((m&ECHONL)!=0));
	(void) printf("-noflsh "+((m&NOFLSH)!=0));
	if(term & TERMIOS) {
		(void) printf("\n");
		(void) printf("-tostop "+((m&TOSTOP)!=0));
		(void) printf("-echoctl "+((m&ECHOCTL)!=0));
		(void) printf("-echoprt "+((m&ECHOPRT)!=0));
		(void) printf("-echoke "+((m&ECHOKE)!=0));
		(void) printf("-defecho "+((m&DEFECHO)!=0));
		(void) printf("-flusho "+((m&FLUSHO)!=0));
		(void) printf("-pendin "+((m&PENDIN)!=0));
		(void) printf("-iexten "+((m&IEXTEN)!=0));
	}
	if(!(term & ASYNC)) {
		(void) printf("-stflush "+((m&STFLUSH)!=0));
		(void) printf("-stwrap "+((m&STWRAP)!=0));
		(void) printf("-stappl "+((m&STAPPL)!=0));
	}
	(void) printf("\n");
	m = cb.c_oflag;
	(void) printf("-opost "+((m&OPOST)!=0));
	(void) printf("-olcuc "+((m&OLCUC)!=0));
	(void) printf("-onlcr "+((m&ONLCR)!=0));
	(void) printf("-ocrnl "+((m&OCRNL)!=0));
	(void) printf("-onocr "+((m&ONOCR)!=0));
	(void) printf("-onlret "+((m&ONLRET)!=0));
	(void) printf("-ofill "+((m&OFILL)!=0));
	(void) printf("-ofdel "+((m&OFDEL)!=0));
	delay((m&CRDLY)/CR1, "cr");
	delay((m&NLDLY)/NL1, "nl");
	delay((m&TABDLY)/TAB1, "tab");
	delay((m&BSDLY)/BS1, "bs");
	delay((m&VTDLY)/VT1, "vt");
	delay((m&FFDLY)/FF1, "ff");
	(void) printf("\n");
	if(term & FLOW) {
		m = termiox.x_hflag;
		(void) printf("-rtsxoff "+((m&RTSXOFF)!=0));
		(void) printf("-ctsxon "+((m&CTSXON)!=0));
		(void) printf("-dtrxoff "+((m&DTRXOFF)!=0));
		(void) printf("-cdxon "+((m&CDXON)!=0));
		(void) printf("-isxoff "+((m&ISXOFF)!=0));
		m = termiox.x_cflag;
		switch(m & XMTCLK)
		{
			case XCIBRG: (void)printf("xcibrg ");
				     break;
			case XCTSET: (void)printf("xctset ");
				     break;
			case XCRSET: (void)printf("xcrset ");
		}
		
		switch(m & RCVCLK)
		{
			case RCIBRG: (void)printf("rcibrg ");
				     break;
			case RCTSET: (void)printf("rctset ");
				     break;
			case RCRSET: (void)printf("rcrset ");
		}
		
		switch(m & TSETCLK)
		{
			case TSETCOFF: (void)printf("tsetcoff ");
				     break;
			case TSETCRBRG: (void)printf("tsetcrbrg ");
				     break;
			case TSETCTBRG: (void)printf("tsetctbrg ");
				     break;
			case TSETCTSET: (void)printf("tsetctset ");
				     break;
			case TSETCRSET: (void)printf("tsetcrset ");
		}
		
		switch(m & RSETCLK)
		{
			case RSETCOFF: (void)printf("rsetcoff ");
				     break;
			case RSETCRBRG: (void)printf("rsetcrbrg ");
				     break;
			case RSETCTBRG: (void)printf("rsetctbrg ");
				     break;
			case RSETCTSET: (void)printf("rsetctset ");
				     break;
			case RSETCRSET: (void)printf("rsetcrset ");
		}
		(void) printf("\n");

#ifdef MERGE386
		(void) printf("-isscancode "+((sc_flag&KB_ISSCANCODE)!=0));
		if (sc_flag & KB_ISSCANCODE)
			(void) printf("-xscancode"+((sc_flag&KB_XSCANCODE)!=0));
		(void) printf("\n");
#endif	

	}
}

pit(what, itsname, sep)		/*print function for prmodes() and pramodes() */
	unsigned char what;
	char *itsname, *sep;
{

	pitt++;
	(void) printf("%s", itsname);
	if ((term & TERMIOS) && what == _POSIX_VDISABLE || !(term & TERMIOS) && what == 0200) {
		(void) printf(" = <undef>%s", sep);
		return;
	}
	(void) printf(" = ");
	if (what & 0200 && !isprint(what)) {
		(void) printf("-");
		what &= ~ 0200;
	}
	if (what == CINTR) {
		(void) printf("DEL%s", sep);
		return;
	} else if (what < ' ') {
		(void) printf("^");
		what += '`';
	}
	(void) printf("%c%s", what, sep);
}

delay(m, s)
char *s;
{
	if(m)
		(void) printf("%s%d ", s, m);
}

long	speed[] = {
	0,50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,19200,38400
};

prspeed(c, s)
char *c;
int s;
{

	(void) printf("%s%d baud; ", c, speed[s]);
}

					/* print current settings for use with  */
prencode()				/* another stty cmd, used for -g option */
{
	int i, last;
	(void) printf("%x:%x:%x:%x:",cb.c_iflag,cb.c_oflag,cb.c_cflag,cb.c_lflag);
	if(term & TERMIOS)
	/* last control slot is unused */
		last = NCCS - 2;
	else
		last = NCC - 1;
	for(i = 0; i < last; i++)
		(void)printf("%x:", cb.c_cc[i]);

#ifdef MERGE386
	(void)printf("%x:%x\n", cb.c_cc[last],sc_flag);
#else
	(void)printf("%x\n", cb.c_cc[last]);
#endif

}
