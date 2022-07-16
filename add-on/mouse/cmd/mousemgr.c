/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mouse:cmd/mousemgr.c	1.3.2.1"

/*
 *			Mouse Manager
 *
 * This process waits for commands from the mouse driver.
 * As a result of these commands, it I_PUSH's or I_POP's
 * or I_PLINK's driver/modules.
 *
 */
#include <stdio.h>
#include "sys/types.h"
#include "sys/termio.h"
#include "sys/fcntl.h"
#include "sys/sysmacros.h"
#include "sys/signal.h"
#include "sys/stream.h"
#include "sys/open.h"
#include "sys/vt.h"
#include <sys/kd.h>
#include "sys/stropts.h"
#include <errno.h>
#include <string.h>
#include <sys/mouse.h>
#include "../io/mse.h"

#include "sys/stat.h"

#define MGR_NAME	"/dev/mousemon"
#define MOUSETAB	"/usr/lib/mousetab"

int	mgr_fd;
struct mse_mon	command;

#define MAX_DEV		100
#define MAXDEVNAME	64
struct mousemap	map[MAX_DEV];
struct mtable {
	int	mse_fd;
	int	disp_fd[16];
	int	type;
	int	linkid;
	int	link_vt;
	struct termio	saveterm;
	char	name[MAXDEVNAME];
	char	linkname[MAXDEVNAME];
	char	dname[MAXDEVNAME];
} ;
struct mtable table[MAX_DEV];
static int	n_dev = 0;
static int disp_vt= 0;
time_t	tab_time;

void do_open(), do_close();
void load_table(), download_table();


void
main()
{
	for (mgr_fd = ulimit(4); mgr_fd-- > 0;) {
		if (mgr_fd != 2)
			close(mgr_fd);
	}

	if((mgr_fd = open(MGR_NAME, O_RDWR)) < 0){
		if( errno == EBUSY ) {
			exit( 0 );
		}
		else {
			perror("mousemgr: /dev/mousemon open failed");
			exit(1);
		}
	}
/*
	sigignore(SIGHUP);
	sigignore(SIGQUIT);
*/

	load_table();
	download_table();

	for (;;) {
		if (ioctl(mgr_fd, MOUSEIOCMON, &command) < 0) {
			perror("mousemgr: MOUSEIOCMON ioctl failed");
			exit(1);
		}
		command.errno = 0;
		switch (command.cmd & 0x07) {
		case MSE_MGR_OPEN:
			do_open();
			break;
		case MSE_MGR_CLOSE:
		case MSE_MGR_LCLOSE:
			do_close();
			break;
		default:
			fprintf(stderr, "mousemgr: Unknown cmd: %d\n", command.cmd & 0x03);
		}
	}
}


void
load_table()
{
	FILE	*tabf;
	char	dname[MAXDEVNAME], mname[MAXDEVNAME];
	struct stat	statb;

	if ((tabf = fopen(MOUSETAB, "r")) == NULL)
		return;
	fstat(fileno(tabf), &statb);
	tab_time = statb.st_mtime;

	/* Format is:
	 *	disp_name	mouse_name
	 */

	n_dev = 0;
	strcpy(dname, "/dev/");
	while (fscanf(tabf, "%s %s", dname + 5, mname) > 0) {
		if (stat(dname, &statb) == -1)
			continue;
		if ((statb.st_mode & S_IFMT) != S_IFCHR)
			continue;
		map[n_dev].disp_dev = statb.st_rdev;
		if (!strncmp(mname, "m320", 4))
			table[n_dev].type = map[n_dev].type = M320;
		else if (!strncmp(mname, "bmse", 4))
			table[n_dev].type = map[n_dev].type = MBUS;
		else
			table[n_dev].type = map[n_dev].type = MSERIAL;
		strcpy(table[n_dev].name, "/dev/");
		strcat(table[n_dev].name, mname);
		strcpy(table[n_dev].dname, dname);
		if (stat(table[n_dev].name, &statb) == -1)
			continue;
		if ((statb.st_mode & S_IFMT) != S_IFCHR)
			continue;
		map[n_dev].mse_dev = statb.st_rdev;
		table[n_dev++].mse_fd = -1;
	}

	fclose(tabf);
}


void
download_table()
{
	struct mse_cfg	mse_cfg;

	mse_cfg.mapping = map;
	mse_cfg.count = n_dev;
	ioctl(mgr_fd, MOUSEIOCCONFIG, &mse_cfg);
}


int
lookup_dev()
{
	int	idx;
	struct stat	statb;

	/* Check if table needs to be reloaded */
	if (stat(MOUSETAB, &statb) != -1 && statb.st_mtime > tab_time){
#ifdef DEBUG
fprintf(stderr,"mousemgr: reloading map table\n");
#endif
		load_table();
	}

	for (idx = 0; idx < n_dev; idx++) {
		if (map[idx].mse_dev == command.mdev)
			return idx;
	}
	return -1;
}

char *
getchan(ndx)
register int ndx;
{
	static char	tmp[MAXDEVNAME], tmp1[MAXDEVNAME];
	struct stat	statb;
	char * end;
	int i;

	end = strchr(table[ndx].dname, 't');
	strncpy(tmp, table[ndx].dname,  (end - table[ndx].dname) + 1 );
	tmp[end - table[ndx].dname + 1] = NULL;
	for(i=0;i<15; i++){
		sprintf(tmp1, "%s%02d", tmp, i );
		if (stat(tmp1, &statb) == 0 ){
			if(statb.st_rdev == command.dev){
				disp_vt = i;
				return(tmp1);
			}
		}
	}
	return((char *) NULL);
}

void
do_open()
{
	struct termio	cb;
	int		idx, flags;
	int	fd;
	char 	*dispname;
	int	xxcompatmode;

#ifdef DEBUG
fprintf(stderr,"mousemgr: entered do_open\n");
#endif

	if ((idx = lookup_dev()) < 0) {
		command.errno = ENXIO;
#ifdef DEBUG
fprintf(stderr,"mousemgr:do_open: failed lookup_dev()\n");
#endif
		return;
	}

	if(table[idx].mse_fd != -1)
		return;

#ifdef DEBUG
fprintf(stderr,"mousemgr:%d: idx = %d\n", __LINE__, idx); 
#endif

	if((dispname = getchan(idx)) == (char *)NULL){
		command.errno = ENXIO;
#ifdef DEBUG
fprintf(stderr,"mousemgr: do_open() - getchan failed\n");
#endif
		return;
	}

	/* open primary display channel */
	if((fd = table[idx].disp_fd[disp_vt] = open(dispname, O_RDWR)) < 0){
#ifdef DEBUG
fprintf(stderr,"mousemgr: open dev= %s failed\n",dispname);
#endif
		command.errno = errno;
		return;
	}

#ifdef DEBUG
fprintf(stderr,"mousemgr:%d: open mouse= %s \n",__LINE__,table[idx].name);
#endif

	if((table[idx].mse_fd = open(table[idx].name, O_RDWR|O_NDELAY|O_EXCL)) == -1){
#ifdef DEBUG
fprintf(stderr,"mousemgr:%d: open mouse= %s failed\n",__LINE__,table[idx].name);
#endif
		command.errno = errno;
		close(fd);
		table[idx].disp_fd[disp_vt] = 0;
		return;
	}

#ifdef DEBUG
fprintf(stderr,"mousemgr:%d: TYPE = %d\n", __LINE__, table[idx].type);
#endif

	if(table[idx].type == MSERIAL){
		/*
		 * Save original parameters.
		 */
		if (ioctl(table[idx].mse_fd, TCGETA, &cb) == -1) {
			command.errno = errno;
			close(table[idx].mse_fd);
			table[idx].mse_fd = -1;
#ifdef DEBUG
fprintf(stderr,"mousemgr:%d: TCGETA failed\n", __LINE__);
#endif
			return;
		}
		table[idx].saveterm = cb;

		/*
		 * Put the serial device in raw mode at 1200 baud.
		 */
		cb.c_iflag = IGNBRK|IGNPAR;
		cb.c_oflag = 0;
		cb.c_cflag = B1200|CS8|CREAD|CLOCAL|PARENB|PARODD;
		cb.c_lflag = 0;
		cb.c_line = 0;
		cb.c_cc[VMIN] = 1;
		cb.c_cc[VTIME] = 0;
		if (ioctl(table[idx].mse_fd, TCSETAF, &cb) == -1) {
			command.errno = errno;
			close(table[idx].mse_fd);
			table[idx].mse_fd = -1;

#ifdef DEBUG
fprintf(stderr,"mousemgr:%d: TCSETAF failed: %d\n",__LINE__,command.errno);
#endif

			return;
		}

		flags = fcntl(table[idx].mse_fd, F_GETFL, 0);
		fcntl(table[idx].mse_fd, F_SETFL, flags & ~O_NDELAY);
		/* Hook for XENIX compatibility */
		xxcompatmode = ioctl(table[idx].disp_fd[disp_vt], WS_GETXXCOMPAT,0);
		if (xxcompatmode == 1)
			(void) ioctl(table[idx].disp_fd[disp_vt], WS_CLRXXCOMPAT,0);
		/* I_POP ldterm from asy stream */
		while(ioctl(table[idx].mse_fd, I_POP) != -1);

#ifdef DEBUG
fprintf(stderr,"mousemgr:%d: smse I_PUSH \n",__LINE__);
#endif
		if(ioctl(table[idx].mse_fd, I_PUSH, "smse") < 0){
			command.errno = errno;
#ifdef DEBUG
fprintf(stderr,"mousemgr:%d: smse I_PUSH failed: %d\n",__LINE__,command.errno);
#endif
			if (xxcompatmode == 1)
		   	   (void) ioctl(table[idx].disp_fd[disp_vt], WS_SETXXCOMPAT,0);
			return;
		}
		if (xxcompatmode == 1)
		 	   (void) ioctl(table[idx].disp_fd[disp_vt], WS_SETXXCOMPAT,0);
	}
	/* Hook for XENIX compatibility */
	xxcompatmode = ioctl(table[idx].disp_fd[disp_vt], WS_GETXXCOMPAT,0);
	if (xxcompatmode == 1)
		(void) ioctl(table[idx].disp_fd[disp_vt], WS_CLRXXCOMPAT,0);
	if((table[idx].linkid = ioctl(table[idx].disp_fd[disp_vt], I_PLINK, table[idx].mse_fd)) < 0){
		command.errno = errno;
#ifdef DEBUG
fprintf(stderr,"mousemgr:%d: I_PLINK failed: %d\n",__LINE__,command.errno);
#endif
		close(table[idx].mse_fd);
		close(table[idx].disp_fd[disp_vt]);
		table[idx].mse_fd = -1;
		if (xxcompatmode == 1)
		   (void) ioctl(table[idx].disp_fd[disp_vt], WS_SETXXCOMPAT,0);
		
		return;
	}
#ifdef DEBUG
fprintf(stderr,"mousemgr:%d: do_open :disp_vt =%x, linkid=%x\n",__LINE__,disp_vt,table[idx].linkid);
#endif
	/* Re-enable XENIX compatibility mode if it had been turned on 
	 * before we turned it off to do the I_PLINK
	 */
	if (xxcompatmode == 1)
		(void) ioctl(table[idx].disp_fd[disp_vt], WS_SETXXCOMPAT,0);

	strcpy(table[idx].linkname, dispname);
	table[idx].link_vt = table[idx].disp_fd[disp_vt];
}


void
do_close()
{
	register int	idx;
	char	*dispname;
	char	temp[2];
	int xxcompatmode;

#ifdef DEBUG1
fprintf(stderr,"mousemgr:entered do_close \n");
#endif
	if ((idx = lookup_dev()) < 0){
#ifdef DEBUG
fprintf(stderr,"mousemgr:do_close - lookup_dev failed\n");
#endif
		return;
	}

	sprintf(temp,"");	 /* hack for weird getchan() behavior */
	if((dispname = getchan(idx)) == (char *)NULL){
		command.errno = ENXIO;
#ifdef DEBUG
fprintf(stderr,"mousemgr:do_close() - getchan failed\n");
#endif
		return;
	}
	if (table[idx].mse_fd == -1){
#ifdef DEBUG
fprintf(stderr,"mousemgr:do_close - mse_fd == -1\n");
#endif
		return;
	}
	if(command.cmd & MSE_MGR_LCLOSE ){
		if(strcmp(table[idx].linkname, dispname) != 0 ){
			close(table[idx].link_vt); 
			table[idx].disp_fd[disp_vt] = open(table[idx].linkname, O_RDWR);
			table[idx].link_vt = table[idx].disp_fd[disp_vt];
		}
		/* Hook for XENIX compatibility */
		xxcompatmode = ioctl(table[idx].disp_fd[disp_vt], WS_GETXXCOMPAT,0);
		if (xxcompatmode == 1)
			(void) ioctl(table[idx].disp_fd[disp_vt], WS_CLRXXCOMPAT,0);
		if( ioctl(table[idx].disp_fd[disp_vt], I_PUNLINK, table[idx].linkid) < 0){
			command.errno = errno;
#ifdef DEBUG
fprintf(stderr,"mousemgr: close I_PUNLINK failed -errno =%x, disp_vt=%d, linkid=%x\n",errno, disp_vt,table[idx].linkid);
#endif
			if (xxcompatmode == 1)
		   	   (void) ioctl(table[idx].disp_fd[disp_vt], WS_SETXXCOMPAT,0);
		
			return;
		}
		if (xxcompatmode == 1)
			(void) ioctl(table[idx].disp_fd[disp_vt], WS_SETXXCOMPAT,0);
	
		close(table[idx].mse_fd);
		table[idx].mse_fd = -1;
	}
	close(table[idx].disp_fd[disp_vt]);
	table[idx].disp_fd[disp_vt] = -1;
	table[idx].link_vt = -1;
}
