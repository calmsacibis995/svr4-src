/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mouse:cmd/mouseadmin.c	1.3.2.1"

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <curses.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/stream.h>
#include <sys/sysmacros.h>
#include <sys/mouse.h>
#include "../io/mse.h"

#define CFG_NAME	"/dev/mousecfg"
#define MOUSETAB	"/usr/lib/mousetab"
#define MSEERR		1
#define MSENOERR	0


#define MAX_DEV		100
#define MAXDEVNAME	64
int	row, col;
char ans[256];
int	c;
char errstr[80];
char sterminal[256], smouse[256];
char	fname[MAXDEVNAME];
char	msebusy[MAX_MSE_UNIT+1];
int	listing, deleting, adding;
int	no_download=0, bus_okay=0;
int	cfg_fd;
int	suserflg=0;

struct mousemap	map[MAX_DEV];
struct {
	char	disp[MAXDEVNAME];
	char	mouse[MAXDEVNAME];
} table[MAX_DEV];

int	n_dev;

int	cursing;

extern int printw();
int	(*print)() = printf;

void load_table(), download_table(), show_table();
void interact();
int delete_entry(), add_entry();

void
cleanup()
{
	if (cursing)
		endwin();
	exit(1);
}

void
fatal_error(fname)
char	*fname;
{
	if (cursing) {
		int	save_err;

		save_err = errno;
		endwin();
		errno = save_err;
	}

	perror(fname);
	exit(1);
}

void
_fatal_error(msg)
char	*msg;
{
	if (cursing)
		endwin();
	fprintf(stderr, "\n%s.\n\n", msg);
	exit(1);
}

void
enter_prompt()
{
	char	ch;

	row += 2;
	while (1) {
		mvaddstr(row, col, "Strike the ENTER key to continue.");
		refresh();
		ch = getchar();
		if (ch == '\n' || ch == '\r')
			break;
		else
			beep();
	}
	row++;
}

void
warn_err(msg)
char	*msg;
{

	row+=2;
	beep();
	mvaddstr(row,col,msg);
	enter_prompt();
	return;
}

get_info(strp, retp)
char *strp, *retp;
{

	row++;
	mvaddstr(row, col, strp);
	refresh();
	attron(A_BOLD);
	getstr(retp);
	attroff(A_BOLD);
}

void
main(argc, argv)
	char	*argv[];
{
	int	c, usage = 0, retval;
	extern int	optind;
	extern char	*optarg;

	while ((c = getopt(argc, argv, "ldanb")) != EOF) {
		switch (c) {
		case 'l':
			listing++;
			break;
		case 'd':
			deleting++;
			break;
		case 'a':
			adding++;
			break;
		case 'n':
			no_download++;
			break;
		case 'b':
			++bus_okay;
			break;
		default:
			usage++;
		}
	}
	switch (deleting + adding + listing) {
	case 0:
		if (argc - optind != 0)
			usage++;
		break;
	case 1:
		if (deleting && argc - optind != 1)
			usage++;
		else if (adding && argc - optind != 2)
			usage++;
		else if (listing && argc - optind != 0)
			usage++;
		break;
	default:
		usage++;
	}
	if (usage) {
		fprintf(stderr,
"Usage:  mouseadmin { -n | -b | -l | -d terminal | -a terminal mouse }\n");
		exit(1);
	}

	load_table();
	if(!no_download)
		get_mse_opened();

	if(strcmp(argv[optind], "console") == 0)
		strcpy(argv[optind], "vt00");

	if(strcmp(argv[optind + 1], "320") == 0)
		strcpy(argv[optind + 1], "m320");

	if(strcmp(argv[optind + 1], "PS2") == 0)
		strcpy(argv[optind + 1], "m320");


	if(strcmp(argv[optind + 1], "BUS") == 0 ||strcmp(argv[optind +1],"Bus")== 0 || strcmp(argv[optind +1], "bus")== 0)
		strcpy(argv[optind +1], "bmse");

	if (listing)
		show_table();
	else if (deleting) {
		if ((retval = delete_entry(argv[optind])) < 0) {
			if(retval == -1)
				fprintf(stderr,
"\nThere is no mouse assigned for %s.\n", argv[optind]);
			else if(retval == -2)
				fprintf(stderr,"\nThe mouse on %s is currently busy.\n",argv[optind]);
			else if(retval == -3)
				fprintf(stderr, "\n%s is not a valid display terminal.\n", argv[optind]);
			exit(1);
		}
		download_table();
	} else if (adding) {
		if (add_entry(argv[optind], argv[optind + 1], 0)){
			exit(1);
		}
		download_table();
	} else
		interact();

	exit(0);
}


int
get_dev(name, dev_p)
char	*name;
dev_t	*dev_p;
{
	struct stat	statb;

	if (strncmp(name, "/dev/", 5) == 0) {
		strcpy(fname, name);
		strcpy(name, name + 5);
	} else {
		strcpy(fname, "/dev/");
		strcat(fname, name);
	}

	if (stat(fname, &statb) == -1)
		return -1;
	if ((statb.st_mode & S_IFMT) != S_IFCHR)
		return -2;

	*dev_p = statb.st_rdev;
	return 0;
}


void
load_table()
{
	FILE	*tabf;
	char	dname[MAXDEVNAME], mname[MAXDEVNAME];
	struct stat	statb;

	if ((tabf = fopen(MOUSETAB, "r")) == NULL)
		return;

	/* Format is:
	 *	disp_name  mouse_name
	 */

	n_dev = 0;
	while (fscanf(tabf, "%s %s", dname, mname) > 0) {
		if (get_dev(dname, &map[n_dev].disp_dev) < 0){
			continue;
		}
#ifdef DEBUG
fprintf(stderr,"load_table: mouse = %s\n",mname);
#endif
		if (strncmp(mname, "m320", 4) == 0){
			map[n_dev].type = M320;
		}else 
		if (strncmp(mname, "bmse", 4) == 0){
			map[n_dev].type = MBUS;
		}else  {
			map[n_dev].type = MSERIAL;
		}
		if (get_dev(mname, &map[n_dev].mse_dev) < 0){
			continue;
		}
		strcat(table[n_dev].disp, dname);
		strcat(table[n_dev++].mouse, mname);
	}

	fclose(tabf);
}


void
write_table()
{
	FILE	*tabf;
	int	i;

	if ((tabf = fopen(MOUSETAB, "w")) == NULL)
		fatal_error(MOUSETAB);
	chmod(MOUSETAB, 0644);

	for (i = 0; i < n_dev; i++)
		fprintf(tabf, "%s\t\t%s\n", table[i].disp, table[i].mouse);

	fclose(tabf);
}

get_mse_opened()
{
	int i;

	if(getuid() != 0)
		suserflg = 1;
	if ((cfg_fd = open(CFG_NAME, O_WRONLY)) < 0)
		fatal_error(CFG_NAME);
	if (ioctl(cfg_fd, MOUSEISOPEN, msebusy) < 0) 
		fatal_error(CFG_NAME);
	close(cfg_fd);
}

void
download_table()
{
	struct mse_cfg	mse_cfg;

	if (!no_download) {
		/* Tell the driver about the change */
		if(suserflg)
			fatal_error(CFG_NAME);
		if ((cfg_fd = open(CFG_NAME, O_WRONLY)) < 0)
			fatal_error(CFG_NAME);

		mse_cfg.mapping = map;
		mse_cfg.count = n_dev;
		if (ioctl(cfg_fd, MOUSEIOCCONFIG, &mse_cfg) < 0) {
			if (errno == EBUSY) {
				_fatal_error(
"One or more mice are in use.\nTry again later");
			}
			fatal_error(CFG_NAME);
		}

		close(cfg_fd);
	}

	/* Write the new table out to the mapping file */
	write_table();
}


void
show_table()
{
	int	i;

	if (n_dev == 0) {
		(*print)("\nThere are no mice assigned.\n\n");
		return;
	}

	(*print)("\nThe following terminals have mice assigned:\n\n");
	(*print)("Display terminal      Mouse device\n");
	(*print)("----------------      ------------\n");

	for (i = 0; i < n_dev; i++) {
		if(strcmp(table[i].disp, "vt00") == 0)
			(*print)("%-22s", "console");
		else
			(*print)("%-22s", table[i].disp);
		if(strncmp(table[i].mouse,"bmse", 4) == 0)
			(*print)("Bus mouse\n");
		else
		if(strncmp(table[i].mouse,"m320", 4) == 0)
			(*print)("PS2 mouse\n");
		else
			(*print)("Serial mouse on %s\n", table[i].mouse);
	}

	(*print)("\n");
}


int
lookup_disp(disp)
char	*disp;
{
	int	slot;

	for (slot = 0; slot < n_dev; slot++) {
		if (strcmp(disp, table[slot].disp) == 0)
			return slot;
	}
	return -1;
}


int
delete_entry(terminal)
char	*terminal;
{
	int	slot;
	dev_t	dummy;


	if (get_dev(terminal, &dummy) < 0 || strcmp(terminal,"vt00") != 0	&& !(strncmp(terminal,"s",1)==0 && strchr(terminal,'v')!=NULL))
		return -3;
	if ((slot = lookup_disp(terminal)) == -1)
		return -1;
	if(msebusy[slot])
		return -2;
	--n_dev;
	while (slot < n_dev) {
		table[slot] = table[slot + 1];
		map[slot] = map[slot + 1];
		slot++;
	}

	return 0;
}


int
add_entry(terminal, mouse, mesg)
char	*terminal, *mouse;
unchar mesg;
{
	int	slot,i;
	int newflag = 0;
	dev_t	disp_dev, mse_dev;


	if ((slot = lookup_disp(terminal)) == -1) {
		newflag = 1;
		if ((slot = n_dev++) >= MAX_DEV)
		{
			if(mesg)
				warn_err("Too many mice configured, one must be removed before another is added.");
			else
				fprintf(stderr, "\nToo many mice configured, one must be removed before another is added.\n\n");
			return(1);
		}
	
		if (get_dev(terminal, &disp_dev) < 0 || strcmp(terminal,"vt00") != 0	&& !(strncmp(terminal,"s",1)==0 && strchr(terminal,'v')!=NULL)){
			--n_dev;
			if (mesg) {
				sprintf(errstr, "Requested display terminal is not valid.");
				warn_err(errstr);
			} else
				fprintf(stderr, "\nRequested display terminal is not valid.\n\n");
			return(1);
		}
	}
	if ((strcmp(terminal, "vt00") ==0 && (strcmp(mouse,"bmse") != 0 && strcmp(mouse,"m320") != 0)) && (strcmp(terminal,"vt00")==0 && strncmp(mouse,"tty",3)!=0)){
		--n_dev;
		if (mesg) {
			sprintf(errstr, "Requested display/mouse pair is not valid.");
			warn_err(errstr);
		} else
			fprintf(stderr, "\nRequested display/mouse pair is not valid.\n\n");
		return(1);
	}
	if ((strncmp(terminal,"s",1)==0 && strchr(terminal,'v')!=NULL)&& (strncmp(mouse,"s",1)!=0 || strchr(mouse,'t')==NULL)){
		--n_dev;
		if (mesg) {
			sprintf(errstr, "Requested display/mouse pair is not valid.");
			warn_err(errstr);
		} else
			fprintf(stderr, "\nRequested display/mouse pair is not valid.\n\n");
		return(1);
	}
   if ((strncmp(mouse, "bmse", 4) == 0 || strncmp(mouse,"m320",4) == 0) && bus_okay) 
	goto passck;


	if (get_dev(mouse, &mse_dev) < 0)
	{
		if (mesg) {
			sprintf(errstr, "%s is not a valid mouse device.", mouse);
			warn_err(errstr);
		} else
			fprintf(stderr, "\n%s is not a valid mouse device.\n\n", mouse);
		if(newflag)
			--n_dev;
		return(1);
	}
passck:
	if((strlen(mouse) > 5 && strcmp("vt00",mouse+5)==0) || strcmp("vt00",mouse) == 0)
	{
		if (mesg) {
			sprintf(errstr, "%s is not a valid mouse device.", mouse);
			warn_err(errstr);
		} else
			fprintf(stderr, "\n%s is not a valid mouse device.\n\n", mouse);
		if(newflag)
			--n_dev;
		return(1);
	}


	if(strcmp(terminal,mouse) == 0)
	{
		if (mesg)
			warn_err("The mouse and display terminal can not be connected to the same port.");
		else
			fprintf(stderr, "\nThe mouse and display terminal can not be connected to the same port.\n\n");
		if(newflag)
			--n_dev;
		return(1);
	}
	for(i=0;i<n_dev;i++){
		if(strcmp(mouse,table[i].mouse) == 0){
			if(mesg){
				sprintf(errstr,"Device %s is already assigned to a Display terminal.",mouse);
				warn_err(errstr);
			} else
				fprintf(stderr,"\nDevice %s is already assigned to a Display terminal.\n\n",mouse);
			if(newflag)
				--n_dev;
			return(1);
		}
	}
	if(msebusy[slot]){
		if(mesg){
			sprintf(errstr,"Mouse device %s is currently in use.",mouse);
			warn_err(errstr,"The configuration was not changed.");
		}
		else
			fprintf(stderr,"\nMouse device %s is currently in use.\n\n",mouse);
		return(1);
	}

	if(!newflag)
		disp_dev = map[slot].disp_dev;
	strcpy(table[slot].disp, terminal);
	map[slot].disp_dev = disp_dev;
	strcpy(table[slot].mouse, mouse);
	map[slot].mse_dev = mse_dev;

	return 0;
}


int
select_term(terminal, check_table,eflag)
char	*terminal;
int	check_table;
int	eflag;
{
	dev_t	dummy;
	char c;

	if (check_table)
		return(1);
	if (get_dev(terminal, &dummy) < 0) {
		if (eflag) {
			beep();
			row++;
			sprintf(errstr, "Requested display terminal is not valid.");
			warn_err(errstr);
			refresh();
		}
		return(0);
	}
	return(1);
}


int
main_menu()
{
	char	ch, terminal[MAXDEVNAME], mouse[MAXDEVNAME];
	int oldrow, oldcol, retval, slot;
	dev_t	dummy;


	c=show_menu();
	move(0,0);
	erase();
	show_table();
	getyx(stdscr, row, col);
	row++;

	if (c == 'E') {
		if (cursing)
			endwin();
		exit(1);
	}
	getyx(stdscr, row, col);
	row++;
	if (suserflg) {
		beep();
		mvaddstr(row,col,"Permission denied, changes will not be accepted.");
		enter_prompt();
		return(1);
	}
	switch (toupper(c)) {
	case 'R':
		mvaddstr(row++,col,"Enter the display terminal from which the mouse will be removed,");
		mvaddstr(row++,col,"or strike the ENTER key to return to the main menu.");
		get_info("Display terminal:  ", terminal);
		row++;
		if(strcmp(terminal, "console") == 0)
			strcpy(terminal, "vt00");
		strcpy(sterminal,terminal);
		if (terminal[0] == '\0')
			break;
		if ((retval = delete_entry(terminal)) < 0) {
			row++;
			if(retval == -1)
				mvaddstr(row,col,"There is no mouse assigned for this terminal.");
			else if(retval == -2)
				mvaddstr(row,col,"Cannot remove mouse while busy.");
			else if(retval == -3)
				mvaddstr(row,col, "Not a valid display terminal.");
			enter_prompt();
			break;
		}
		break;
	case 'P':
	case '3':
		if (ckps2cfg()) {
			row+=2;
			mvaddstr(row++,col,"The system is not configured for an PS2 mouse.  The Mouse");
			mvaddstr(row,col,"Driver Package must be reinstalled and configured for a PS2 mouse.");
			enter_prompt();
			break;
		}
		if ((slot = lookup_disp("vt00")) >= 0)
			if(msebusy[slot]){
   				mvaddstr(row++,col,"Mouse currently assigned to console is busy, change will not be accepted. ");
				enter_prompt();
				break;
			}
		add_entry("vt00", "m320", 1);
		break;
	case 'B':
		if (ckbuscfg()) {
			row+=2;
			mvaddstr(row++,col,"The system is not configured for a Bus mouse.  The AT&T Mouse");
			mvaddstr(row,col,"Driver Package must be reinstalled and configured for a Bus mouse.");
			enter_prompt();
			break;
		}
		if ((slot = lookup_disp("vt00")) >= 0)
			if(msebusy[slot]){
   				mvaddstr(row++,col,"Mouse currently assigned to console is busy, change will not be accepted. ");
				enter_prompt();
				break;
			}
		add_entry("vt00", "bmse", 1);
		break;
	case 'S':
		while (1) {
			getyx(stdscr, row, col);
			oldrow=row;
			oldcol=col;
			clrtobot();
			mvaddstr(row++,col,"Enter the display terminal that will be using the mouse,");
			mvaddstr(row++,col,"or strike the ENTER key to return to the main menu.");
			get_info("Display terminal (i.e. console, s0vt00, etc.):  ", terminal);
			row++;
			if(strcmp(terminal, "console") == 0)
				strcpy(terminal, "vt00");
			strcpy(sterminal,terminal);
			if (terminal[0] == '\0')
				break;
			if (lookup_disp(terminal) >= 0) {
				sprintf(errstr, "Requested display terminal is already configured to use a mouse.");
				mvaddstr(row++,col,errstr);
				for (;;) {
					mvaddstr(row,col,"Do you wish to continue? [y or n] ");
					clrtobot();
					refresh();
					attron(A_BOLD);
					getstr(ans);
					attroff(A_BOLD);
					ch = toupper(ans[0]);
					if (ch == 'Y' || ch == 'N')
						break;
					beep();
				}
				if (ch == 'N')
					break;
				move(row+=2,col);
			}
			if (select_term(terminal, 0, MSEERR)) {
				if(strcmp(terminal,"vt00")==0){
					mvaddstr(row++,col,"Enter the device that the mouse will be attached to,");
					mvaddstr(row++,col,"or strike the ENTER key to return to the main menu.");
					get_info("Mouse device (i.e. tty00, tty01):  ", mouse);
					row++;
					strcpy(smouse, mouse);
					if (mouse[0] == '\0')
						break;
					if(strncmp(mouse,"ttyh",4)==0||strncmp(mouse,"ttys",4)==0||strncmp(mouse,"tty",3)!=0 || (strchr(mouse,'0')==NULL && strchr(mouse,'1')==NULL) ){
						sprintf(errstr, "Requested display/mouse pair is not valid.");
						warn_err(errstr);
						break;
					}
					add_entry(terminal, mouse, 1);
					break;
				} else if (strncmp(terminal,"s",1)==0) {
					if( strchr(terminal,'v')==NULL){
						row++;
						sprintf(errstr, "Requested display terminal is not valid.", terminal);
						warn_err(errstr);
						break;
					}
					mvaddstr(row++,col,"Enter the device that the mouse will be attached to,");
					mvaddstr(row++,col,"or strike the ENTER key to return to the main menu.");
					get_info("Mouse device (i.e. s0tty0, s3tty1):  ", mouse);
					row++;
					if (mouse[0] == '\0')
						break;
					strcpy(smouse,mouse);
					if(strncmp(mouse,"s",1) != 0 || strchr(mouse,'v') != NULL || strchr(mouse,'l') != NULL ){
						sprintf(errstr, "Requested display/mouse pair is not valid.");
						warn_err(errstr);
						break;
					}
					add_entry(terminal, mouse, 1);
					break;
				}
				sprintf(errstr, "Requested display terminal is not valid.");
				warn_err(errstr);
				break;
			} else {
				break;
			}
		}
		break;
	case 'U':
		return(0);
	}
	return 1;
}


void
interact()
{
	initscr();
	cursing = 1;
	print = printw;

	do {
		erase();
		show_table();
	} while (main_menu());

	download_table();
	endwin();
}

ckbuscfg()
{
	FILE *fopen(), *fp;
	static int irq = -1;
	char    buffer[250];

	char name[10], f2[10], f3[10], f4[10], f5[10], f6[10],
		f7[10], f8[10], f9[10], f0[10];

	/** -b flag given **/
	if(bus_okay)
	{
		return(0);
	}
	if(irq == -1)
	{
		if((fp = fopen("/etc/conf/cf.d/sdevice","r")) == NULL)
		{
			printf("can't open sdevice\n");
			exit(1);
		}

		while(fgets(buffer,250,fp) != NULL)
		{
            if(sscanf(buffer,"%10s  %10s    %10s    %10s    %10s    %10s    %10s    %10s    %10s    %10s\n",name,f2,f3,f4,f5,f6,f7,f8,f9,f0) != 10)
		continue;
            if(strcmp(name,"bmse") == 0)
            {
				irq=atoi(f6);
				break;
            }
		}
		fclose(fp);
	}


	if(irq != -1 && irq != 0 && irq != 12)
	{
		return(0);
	}

	return(1);
	
}

ckps2cfg()
{
	FILE *fopen(), *fp;
	char buffer[250];

	char name[10], f2[10], f3[10], f4[10], f5[10], f6[10],
		f7[10], f8[10], f9[10], f0[10];

	/** -b flag given **/
	if(bus_okay)
	{
		return(0);
	}
	if((fp = fopen("/etc/conf/cf.d/sdevice","r")) == NULL)
	{
		printf("can't open sdevice\n");
		exit(1);
	}

	while(fgets(buffer,128,fp) != NULL)
    {
		if(sscanf(buffer,"%10s	%10s	%10s	%10s	%10s	%10s	%10s	%10s	%10s	%10s\n",name,f2,f3,f4,f5,f6,f7,f8,f9,f0) != 10)
            continue;

        if(strcmp(name,"m320") == 0)
        {
            if(atoi(f6) == 12)
            {
#ifdef DEBUG
fprintf(stderr,"Found correct entry. About to close sdevice file.\n");
#endif
        		fclose(fp);

#ifdef DEBUG
fprintf(stderr,"Found correct entry. About to return 0.\n");
#endif
        		return(0);
            }
        }
    }
	fclose(fp);
	return(1);
}

show_menu()
{
	char	ch;

	getyx(stdscr, row, col);
	row++;

	mvaddstr(row++, col, "Select one of the following:");
	col += 5;
	mvaddstr(row++, col, "B)us mouse add");
	mvaddstr(row++, col, "P)S2 mouse add");
	mvaddstr(row++, col, "S)erial mouse add");
	if (n_dev)
		mvaddstr(row++, col, "R)emove a mouse");
	mvaddstr(row++, col, "U)pdate mouse configuration and quit");
	mvaddstr(row++, col, "E)xit (no update)");
	col -= 5;
	for (;;) {
		mvaddstr(row, col, "Enter Selection:  ");
		clrtobot();
		refresh();
		attron(A_BOLD);
		getstr(ans);
		attroff(A_BOLD);
		ch = toupper(ans[0]);
		if (strchr("BPSUE", ch) || (n_dev && ch == 'R'))
			break;
		beep();
	}
	row++;
	return(ch);
}
